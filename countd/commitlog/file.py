from countd import message, settings
#import ctypes
import os, stat, sys

class File(object):
    """
    A single commit log file which takes care of its own locking.
    """

    # TODO Put a timestamp in each line in the file so that when reading,
    # we can throw out lines older than the preceding line, making the commit
    # log safe against partially-written files.

    READ = os.O_RDONLY
    WRITE = os.O_WRONLY
    CREAT = os.O_CREAT | os.O_EXCL

    CLEAN = 0o200
    DIRTY = 0o400

    # Compute the actual filesize based on the maximum in FILESIZE and the
    # packet length.
    filesize = settings.FILESIZE - (settings.FILESIZE % message.Write.LENGTH)

    def __init__(self, index, flags, create=False):
        """
        Open the file at the given index with the given flags (usually
        File.READ or File.WRITE as defined above).  This uses hardlinks to
        lock open files.  If the file doesn't exist, it will be created and
        written full.
        """
        self.index = index
        self.len = 0
        pathname1 = "{0}/{1:010}".format(settings.DIRNAME, self.index)
        pathname2 = "{0}/lock-{1:010}".format(settings.DIRNAME, self.index)

        # Try to lock the commit log at this index using a hardlink.
        try:
            os.link(pathname1, pathname2)

        # Couldn't lock the commit log so try to create it.  If it can't be
        # created, os.open will raise and OSError and the calling code must
        # skip this commit log for now.
        except OSError, e: # FIXME for Python3.
            if not create:
                raise e
            self.fd = os.open(pathname2, flags | self.CREAT, self.CLEAN)
            self.fill()
            os.link(pathname2, pathname1)
            return

        # Try to open the commit log.  If there is a permissions mismatch,
        # unlock the file and re-raise the OSError.
        try:
            self.fd = os.open(pathname2, flags)
        except OSError, e: # FIXME for Python3.
            os.unlink("{0}/lock-{1:010}".format(settings.DIRNAME, self.index))
            raise e

    def __del__(self):
        """
        Close and unlock the file.  If anything was written into it, mark
        it dirty.
        """
        if hasattr(self, "fd"):
            os.close(self.fd)
        if 0 < self.len:
            self.dirty()
        if hasattr(self, "fd"):
            os.unlink("{0}/lock-{1:010}".format(settings.DIRNAME, self.index))

    def fill(self):
        """
        Write the current file full and sync it to disk.  The goal here is
        a contiguous area on disk for this file.
        """
        sys.stderr.write("[commitlog] filling commit log {0:010}\n".format(
            self.index
        ))

        # Preferred version would use syscall(2) to call fallocate(2) or
        # directly call posix_fallocate(3).
        #libc = ctypes.CDLL("libc.so.6")
        #libc.syscall(285, self,fd, self.WRITE, 0, self.filesize)
        #libc.posix_fallocate(self.fd, 0, self.filesize)

        # But for now, I'll settle for writing a file full manually and
        # hoping it's contiguous on disk.
        empty = message.Write("\0" * message.Write.LENGTH)
        while not self.full():
            self.write(empty, False)
        os.fsync(self.fd)
        self.len = 0
        os.lseek(self.fd, 0, os.SEEK_SET)

    def full(self):
        """
        Return true if the file is full and should be rotated out.
        """
        return self.len >= self.filesize

    def read(self):
        """
        Read LENGTH bytes to create and return a message.Write object.
        """
        buf = os.read(self.fd, message.Write.LENGTH)
        if message.Write.LENGTH != len(buf):
            return None
        return message.Write(buf)

    def write(self, m, fsync=True):
        """
        Write the contents of the given message.Write object into the file.
        """
        if message.Write.LENGTH != os.write(self.fd, str(m)):
            return False
        self.len += message.Write.LENGTH
        if fsync:
            os.fsync(self.fd)
        return True

    def clean(self):
        """
        Mark the current file as clean.
        """
        os.chmod("{0}/lock-{1:010}".format(settings.DIRNAME, self.index),
            self.CLEAN)

    def dirty(self):
        """
        Mark the current file as dirty.
        """
        os.chmod("{0}/lock-{1:010}".format(settings.DIRNAME, self.index),
            self.DIRTY)

    # Make File objects iterable to make reading easier.
    def __iter__(self):
        return self
    def next(self):
        m = self.read()
        if m is None:
            raise StopIteration()
        return m

if "__main__" == __name__:
    File(0, File.WRITE, True)

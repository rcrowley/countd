from countd import message
#import ctypes
import os
import stat
import sys

class File(object):
    """
    A single commit log file which takes care of its own locking.
    """

    DIRNAME = "/tmp/countd" # TODO Configurable
    FILESIZE = 2 << 20 # TODO Configurable

    READ = os.O_RDONLY
    WRITE = os.O_WRONLY
    CREAT = os.O_CREAT | os.O_EXCL

    CLEAN = 0600
    DIRTY = 0400

    # Compute the actual filesize based on the maximum in FILESIZE and the packet
    # length, LENGTH.
    filesize = FILESIZE - (FILESIZE % message.Write.LENGTH)

    def __init__(self, index, flags, create=False):
        """
        Open the file at the given index with the given flags (usually File.READ or
        File.WRITE as defined above).  This uses hardlinks to lock open files.  If
        the file doesn't exist, it will be created and written full.
        """
        self.index = index
        self.len = 0
        pathname1 = "%s/%010u" % (self.DIRNAME, self.index)
        pathname2 = "%s/lock-%010u" % (self.DIRNAME, self.index)

        # Try to lock the file at this index using a hardlink.
        try:
            os.link(pathname1, pathname2)
            self.fd = os.open(pathname2, flags)

        # If it can't be done and the file exists, throw the error.  Otherwise,
        # create and preallocate the file.
        except OSError, e:
            if not create:
                raise e
            try:
                s = os.stat(pathname1)
            except OSError:
                s = None
            if s is not None:
                raise e
            self.fd = os.open(pathname2, flags | self.CREAT, self.CLEAN)
            self.fill()
            os.link(pathname2, pathname1)

    def __del__(self):
        """
        Close and unlock the file.  If anything was written into it, mark it dirty.
        """
        if hasattr(self, "fd"):
            os.close(self.fd)
        if 0 < self.len:
            os.chmod("%s/lock-%010u" % (self.DIRNAME, self.index), self.DIRTY)
        try:
            os.unlink("%s/lock-%010u" % (self.DIRNAME, self.index))
        except OSError:
            pass

    def fill(self):
        """
        Write the current file full and sync it to disk.  The goal here is a contiguous
        area on disk for this file.
        """
        sys.stderr.write("[commitlog] filling commit log %010u\n" % self.index)

        # Preferred version would use syscall(2) to call fallocate(2) or directly
        # call posix_fallocate(3).
        #libc = ctypes.CDLL("libc.so.6")
        #libc.syscall(285, self,fd, self.WRITE, 0, self.filesize)
        #libc.posix_fallocate(self.fd, 0, self.filesize)

        # But for now, I'll settle for writing a file full manually and hoping it's
        # contiguous on disk.
        empty = message.Write("\0" * message.Write.LENGTH)
        while not self.full():
            self.write(empty)
        os.fsync(self.fd)
        self.len = 0

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

    def write(self, m):
        """
        Write the contents of the given message.Write object into the file.
        """
        if message.Write.LENGTH != os.write(self.fd, m.buf):
            return False
        self.len += message.Write.LENGTH
        os.fsync(self.fd)
        return True

    # Make these iterable to make reading them easy.
    def __iter__(self):
        return self
    def next(self):
        m = self.read()
        if m is None:
            raise StopIteration()
        return m

if "__main__" == __name__:
    File(0, File.WRITE, True)

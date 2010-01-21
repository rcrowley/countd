from countd import settings
from countd.commitlog.file import File
import errno, os, sys

class CommitLog(object):
    """
    The entire commit log group which takes care of rotating through logs or
    creating new ones when necessary.
    """

    # Flags for the CommitLog constructor.  READ will only allow access to
    # dirty commit logs.  WRITE will only allow access to clean commit logs.
    READ = File.READ
    WRITE = File.WRITE

    def __init__(self, flags):
        """
        Create the data directory, write all of the commit logs full and
        choose the initial commit log.
        """
        self.files = settings.FILES
        self.flags = flags

        # Create the data directory
        try:
            os.mkdir(settings.DIRNAME)
        except OSError:
            pass

        # Check that the first FILES files are in good shape.
        for i in range(self.files):
            try:
                File(i, File.WRITE, True)
            except OSError:
                pass

        # Check that any extra files are in good shape, too.
        while 1:
            i += 1
            try:
                File(i, File.WRITE, False)
                self.files += 1
            except OSError, e: # FIXME for Python3.
                if errno.ENOENT == e[0]:
                    break

        # Act like the current file is the last one so the first rotation
        # comes back to the first available file.
        class DummyFile(object):
            pass
        self.file = DummyFile()
        self.file.index = self.files - 1
        if File.WRITE == self.flags:
            self.choose()

    def choose(self):
        """
        Start from the current position and find the next available commit
        log.  If none are available, create and fill a new one.
        """

        # Loop from the current position, wrapping around the end until an
        # available commit log is found or we're back where we started.
        i = ii = self.file.index
        while 1:
            i = (i + 1) % self.files
            try:
                self.file = File(i, self.flags, False)
                break
            except OSError:
                pass

            # We're back where we started.  If we want write access, make
            # a new file to get it.  Otherwise, hunt for new files.
            if i == ii:
                while 1:
                    try:
                        self.file = File(self.files, self.flags,
                            File.WRITE == self.flags)
                        self.files += 1
                        break
                    except OSError, e: # FIXME for Python3.
                        if errno.ENOENT == e[0]:
                            return False
                        self.files += 1
                break

        sys.stderr.write("[commitlog] chose commit log {0:010}\n".format(
            self.file.index
        ))
        return True

    def write(self, message):
        """
        Write the contents of the given message.Write object into the
        current file.
        """
        if self.file.full():
            self.choose()
        self.file.write(message)

if "__main__" == __name__:
    CommitLog(File.WRITE)

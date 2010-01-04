from countd.commitlog.file import File
import os
import sys

class CommitLog(object):
    """
    The entire commit log group which takes care of rotating through logs or creating
    new ones when necessary.
    """

    FILES = 4 # TODO Configurable

    def __init__(self):
        """
        Create the data directory, write all of the commit logs full and choose
        the initial commit log.
        """
        self.files = self.FILES

        # Create the data directory
        try:
            os.mkdir(File.DIRNAME)
        except OSError, e:
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
            except OSError:
                break

        # Act like the current file is the last one so the first rotation comes
        # back to the first available file.
        class DummyFile(object):
            pass
        self.file = DummyFile()
        self.file.index = self.files - 1
        self.choose()

    def choose(self):
        """
        Start from the current position and find the next available commit log.  If
        none are available, create and fill a new one.
        """
        i = ii = self.file.index
        while 1:
            i = (i + 1) % self.files
            try:
                self.file = File(i, File.WRITE, False)
                break
            except OSError:
                pass
            if i == ii:
                while 1:
                    try:
                        self.file = File(self.files, File.WRITE, True)
                        break
                    except OSError:
                        pass
                    self.files += 1
                break
        sys.stderr.write("[commitlog] chose commit log %010u\n" % self.file.index)

    def write(self, message):
        """
        Write the contents of the given message.Write object into the current file.
        """
        if self.file.full():
            self.choose()
        self.file.write(message)

if "__main__" == __name__:
    CommitLog()

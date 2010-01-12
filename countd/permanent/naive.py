"""
A naive implementation of the permanent storage interface.
"""

from countd import settings
import os, struct, sys

class Keyspace(object):
    """
    A keyspace.
    """

    LENGTH = settings.COUNT + settings.KEY
    FORMAT = "<q%dc" % settings.KEYSPACE

    READ = os.O_RDONLY
    WRITE = os.O_WRONLY | os.O_CREAT

    def __init__(self, keyspace, flags):
        try:
            os.mkdir("%s/%s" % (settings.DIRNAME, keyspace), 0700)
        except OSError:
            pass
        self.fd = os.open("%s/%s/keyspace" % (settings.DIRNAME, keyspace), flags, 0600)
        self.index = Index(keyspace)
        self.deltas = Deltas(keyspace)

    def __del__(self):
        if hasattr(self, "fd"):
            os.close(self.fd)

    def range(self, offset, length):
        """
        Yields keys and counts from the keyspace starting at offset and continuing
        until length keys and counts have been returned.
        """
        # TODO This is a generator.
        pass

    def update(self, key, increment, fsync=True):
        """
        Increment key's count and resort the keyspace on disk.
        """
        # FIXME Use os.fcntl to lock regions of the file, otherwise this is dangerous.

        # Update a key that's already present.
        offset = self.index.find(key)
        if offset is not None:
            return True

        # Place a new key whose count falls somewhere in the middle of the file.
        offset = self.deltas.find(increment)
        if offset is not None:
            return True

        # Place a new key whose count falls at the end of the file.
        try:
            os.lseek(self.fd, 0, os.SEEK_END)
            if self.LENGTH != os.write(self.fd, self._pack(key, increment)):
                return False
            if fsync:
                os.fsync(self.fd)
            return True
        except OSError:
            return False

    def _pack(self, key, increment):
        chars = list(key)
        chars.extend("\0" * (settings.KEY - len(key)))
        return struct.pack(self.FORMAT, increment, *chars)

    def _unpack(self, buf):
        count, key = struct.unpack(self.FORMAT, buf)
        return key, count

class Index(object):
    """
    A (currently fictional) index for quickly determining the current
    position of a key in a keyspace.
    """

    def __init__(self, keyspace):
        self.pathname = "%s/%s/index" % (settings.DIRNAME, keyspace)

    def find(self, key):
        """
        Return the offset at which the key can be found.
        """
        return None

    def update(self, key, offset):
        """
        Record the offset at which the key can be found.
        """
        pass

class Deltas(object):
    """
    """

    def __init__(self, keyspace):
        self.pathname = "%s/%s/deltas" % (settings.DIRNAME, keyspace)

    def find(self, count):
        """
        Return the offset at which the last key with the given count can be found.
        """
        return None

    def update(self, count, offset):
        """
        Record the offset at which the last key with the given count can be found.
        """
        pass

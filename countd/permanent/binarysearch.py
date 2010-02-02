"""
A Deltas implementation that does a binary search over the keyspace
rather than a sequential search.
"""

from countd import settings
from countd.permanent.abstract import Keyspace, Index
from countd.permanent import abstract
import os, struct

class Deltas(abstract.Deltas):
    """
    A Deltas implementation that does a binary search over the keyspace
    rather than a sequential search.
    """

    LENGTH = settings.COUNT + settings.KEY
    FORMAT = "<q{0}c".format(settings.KEYSPACE)

    READ = os.O_RDONLY
    WRITE = os.O_RDWR | os.O_CREAT

    def __init__(self, keyspace, flags):
        try:
            os.mkdir(os.path.join(settings.DIRNAME, keyspace), 0o700)
        except OSError as e:
            pass
        self.fd = os.open(os.path.join(
            settings.DIRNAME, keyspace, "keyspace"
        ), os.O_RDONLY)

    def __del__(self):
        if hasattr(self, "fd"):
            os.close(self.fd)

    def _unpack(self, buf):
        return struct.unpack(self.FORMAT, buf)[0]

    def find(self, count, offset=0):
        """
        Return the offset at which a key with the given count should be
        placed.  Start the search from the given offset.
        """
        start = offset
        end = os.lseek(self.fd, 0, os.SEEK_END)
        while 1:
            offset = start + (end - start) / 2
            offset -= offset % self.LENGTH
            os.lseek(self.fd, offset, os.SEEK_SET)
            buf = os.read(self.fd, 2 * self.LENGTH)
            if self.LENGTH == len(buf):
                return self._unpack(buf) # FIXME
            if 2 * self.LENGTH != len(buf):
                return None # FIXME
            c1 = self._unpack(buf[0:self.LENGTH])
            c2 = self._unpack(buf[self.LENGTH:])
            if c1 < count:
                end = offset
            elif c1 > count:
                if c2 < count:
                    return offset
                start = offset
            else:
                if c1 > c2:
                    return offset
                start = offset

    def update(self, count, offset):
        """
        Nothing to do here!
        """
        pass

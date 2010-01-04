import struct

class Read(object):
    pass

class Write(object):

    LENGTH = 8 + 256 + 256 # TODO Configurable
    FORMAT = "q256c256c" # TODO Configurable

    def __init__(self, buf):
        self.buf = buf

    def _unpack(self):
        try:
            parts = struct.unpack(self.FORMAT, self.buf)
            self._increment = parts[0]
            self._keyspace = str(parts[1:256])
            self._key = str(parts[257:256])
        except struct.error:
            pass

    @property
    def increment(self):
        if self._increment is None:
            self._unpack()
        return self._increment

    @property
    def keyspace(self):
        if self._keyspace is None:
            self._unpack()
        return self._keyspace

    @property
    def key(self):
        if self._key is None:
            self._unpack()
        return self._key

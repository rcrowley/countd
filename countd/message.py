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
            self._keyspace = "".join(parts[1:256]).strip("\x00") # TODO Configurable
            self._key = "".join(parts[257:512]).strip("\x00") # TODO Configurable
        except struct.error:
            pass

    @property
    def increment(self):
        if not hasattr(self, "_increment"):
            self._unpack()
        return self._increment

    @property
    def keyspace(self):
        if not hasattr(self, "_keyspace"):
            self._unpack()
        return self._keyspace

    @property
    def key(self):
        if not hasattr(self, "_key"):
            self._unpack()
        return self._key

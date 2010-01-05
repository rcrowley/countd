from countd import settings
import struct

class Read(object):
    pass

class Write(object):

    LENGTH = settings.INCREMENT + settings.KEYSPACE + settings.KEY
    FORMAT = "q%dc%dc" % (settings.KEYSPACE, settings.KEY)

    def __init__(self, buf):
        self.buf = buf

    def _unpack(self):
        try:
            parts = struct.unpack(self.FORMAT, self.buf)
            self._increment = parts[0]
            self._keyspace = "".join(parts[1:settings.KEYSPACE]).strip("\x00")
            self._key = "".join(parts[1 + settings.KEYSPACE:settings.KEYSPACE +
                settings.KEY]).strip("\x00")
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

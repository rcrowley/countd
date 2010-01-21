from countd import settings
import struct

class Read(object):
    pass

class Write(object):

    LENGTH = settings.INCREMENT + settings.KEYSPACE + settings.KEY
    FORMAT = "<q{0}c{1}c".format(settings.KEYSPACE, settings.KEY)

    def __init__(self, buf=None, keyspace=None, key=None, increment=None):
        if buf is not None:
            self.buf = buf
        if keyspace is not None and key is not None and increment is not None:
            self._keyspace = keyspace
            self._key = key
            self._increment = increment
            self._pack()

    def __str__(self):
        return self.buf

    def _pack(self):
        try:
            chars = list(self._keyspace)
            chars.extend("\0" * (settings.KEYSPACE - len(self._keyspace)))
            chars.extend(list(self._key))
            chars.extend("\0" * (settings.KEY - len(self._key)))
            self.buf = struct.pack(self.FORMAT, long(self._increment), *chars)
        except struct.error:
            pass

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

"""
A naive implementation of the permanent storage interface.
"""

from countd import settings
import locks
import fcntl, os, struct, sys

class Keyspace(object):
    """
    A keyspace.
    """

    LENGTH = settings.COUNT + settings.KEY
    FORMAT = "<q{0}c".format(settings.KEYSPACE)

    READ = os.O_RDONLY
    WRITE = os.O_RDWR | os.O_CREAT

    def __init__(self, keyspace, flags):
        try:
            os.mkdir("{0}/{1}".format(settings.DIRNAME, keyspace), 0o700)
        except OSError:
            pass
        self.fd = os.open("{0}/{1}/keyspace".format(
            settings.DIRNAME, keyspace
        ), flags, 0o600)
        self.locks = locks.Locks()
        self.index = Index(keyspace)
        self.deltas = Deltas(keyspace)

    def __del__(self):
        if hasattr(self, "fd"):
            os.close(self.fd)

    def _pack(self, key, increment):
        chars = list(key)
        chars.extend("\0" * (settings.KEY - len(key)))
        return struct.pack(self.FORMAT, increment, *chars)

    def _unpack(self, buf):
        parts = struct.unpack(self.FORMAT, buf)
        return "".join(parts[1:settings.KEY]).strip("\x00"), parts[0]

    def range(self, offset, length, mode=fcntl.LOCK_SH):
        """
        Yields keys and counts from the keyspace starting at offset and
        continuing until length keys and counts have been returned.

        By default, the range is locked in shared mode and unlocked when all
        rows have been read.  If the range is locked in exclusive mode, the
        caller becomes responsible for unlocking the range.
        """
        os.lseek(self.fd, self.LENGTH * offset, os.SEEK_SET)
        self.locks.lock(self.fd, self.LENGTH * offset, self.LENGTH * length,
            mode)
        for i in range(length):
            buf = os.read(self.fd, self.LENGTH)
            if self.LENGTH != len(buf):
                break
            yield self._unpack(buf)
        if fcntl.LOCK_SH == mode:
            self.locks.unlock(self.fd, offset, length)

    def update(self, key, increment, fsync=True):
        """
        Increment key's count and resort the keyspace on disk.
        """
        try:

            # Update a key that's already present.
            offset = self.index.find(key)
            if offset is not None:
                r = self.range(offset, 1, fcntl.LOCK_EX)
                if r is None:
                    self.locks.unlock()
                    return False
                count = r.next()[1]
                if not self._update(key, count + increment, 0, offset):
                    self.locks.unlock()
                    return False

            # Place a new key.
            elif not self._update(key, increment):
                self.locks.unlock()
                return False

            self.locks.unlock()
            if fsync:
                os.fsync(self.fd)
            return True

        except OSError:
            return False

    def _update(self, key, count, offset=0, empty_offset=None):
        offset = self.deltas.find(count, offset)

        # Place a key whose count falls somewhere in the middle of the file.
        if offset != empty_offset and offset is not None:
            r = self.range(offset, 1, fcntl.LOCK_EX)
            if r is None:
                return False
            k, c = r.next()
            if not self._update(k, c, offset + self.LENGTH, empty_offset):
                return False
            os.lseek(self.fd, self.LENGTH * offset, os.SEEK_SET)

        # Place a key whose count falls in an empty slot.  Note that this
        # slot is already locked.
        elif offset == empty_offset and offset is not None:
            os.lseek(self.fd, self.LENGTH * offset, os.SEEK_SET)

        # Place a key whose count falls at the end of the file.
        else:
            offset = os.lseek(self.fd, 0, os.SEEK_END) / self.LENGTH
            self.locks.lock(self.fd, self.LENGTH * offset, self.LENGTH,
                fcntl.LOCK_EX)

        if self.LENGTH != os.write(self.fd, self._pack(key, count)):
            return False
        self.index.update(key, offset)
        self.deltas.update(count, offset + self.LENGTH)
        return True

    # Make Keyspace objects iterable to make reading easier.
    def __iter__(self):
        os.lseek(self.fd, 0, os.SEEK_SET)
        return self
    def next(self):
        buf = os.read(self.fd, self.LENGTH)
        if self.LENGTH != len(buf):
            raise StopIteration()
        return self._unpack(buf)

class Index(object):
    """
    A (currently fictional) index for quickly determining the current
    position of a key in a keyspace.
    """

    def __init__(self, keyspace):
        self.fd = os.open("{0}/{1}/keyspace".format(
            settings.DIRNAME, keyspace
        ), os.O_RDONLY)

    def __del__(self):
        if hasattr(self, "fd"):
            os.close(self.fd)

    def _unpack(self, buf):
        return "".join(struct.unpack(Keyspace.FORMAT,
            buf)[1:settings.KEY]).strip("\x00")

    def find(self, key):
        """
        Return the offset at which the key can be found.
        """
        for k in self:
            if k == key:
                return os.lseek(self.fd, 0, os.SEEK_CUR) / Keyspace.LENGTH - 1

    def update(self, key, offset):
        """
        Record the offset at which the key can be found.
        """
        pass

    # Make Index objects iterable to make reading easier.
    def __iter__(self):
        os.lseek(self.fd, 0, os.SEEK_SET)
        return self
    def next(self):
        buf = os.read(self.fd, Keyspace.LENGTH)
        if Keyspace.LENGTH != len(buf):
            raise StopIteration()
        return self._unpack(buf)

class Deltas(object):
    """
    A (currently fictional) index for quickly finding the offset at which
    a key with the given count should be placed.
    """

    def __init__(self, keyspace):
        self.fd = os.open("{0}/{1}/keyspace".format(
            settings.DIRNAME, keyspace
        ), os.O_RDONLY)

    def __del__(self):
        if hasattr(self, "fd"):
            os.close(self.fd)

    def _unpack(self, buf):
        return struct.unpack(Keyspace.FORMAT, buf)[0]

    def find(self, count, offset=0):
        """
        Return the offset at which a key with the given count should be
        placed.  Start the search from the given offset.
        """
        for c in self.iter(offset):
            if c < count:
                return os.lseek(self.fd, 0, os.SEEK_CUR) / Keyspace.LENGTH - 1

    def update(self, count, offset):
        """
        Record the offset at which the last key with the given count can
        be found.
        """
        pass

    # Make Deltas objects iterable to make reading easier.
    def __iter__(self):
        return self.iter(0)
    def iter(self, offset):
        os.lseek(self.fd, Keyspace.LENGTH * offset, os.SEEK_SET)
        return self
    def next(self):
        buf = os.read(self.fd, Keyspace.LENGTH)
        if Keyspace.LENGTH != len(buf):
            raise StopIteration()
        return self._unpack(buf)

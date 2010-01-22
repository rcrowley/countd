import fcntl

class Locks(object):
    """
    Manager for file-level locks.  Note that while classes in the naive
    module operate on offsets and lengths in units of messages, this class
    operates on offsets and lengths in bytes.
    """

    def __init__(self):
        self._locks = {}

    def lock(self, fd, offset, length, mode):
        """
        Lock the requested region or raise an IOError if it would deadlock.
        """
        fcntl.lockf(fd, mode, length, offset)
        if fcntl.LOCK_UN == mode:
            del self._locks[fd][offset][length]
            if 0 == len(self._locks[fd][offset]):
                del self._locks[fd][offset]
            if 0 == len(self._locks[fd]):
                del self._locks[fd]
        else:
            self._locks.setdefault(fd, {})
            self._locks[fd].setdefault(offset, {})
            self._locks[fd][offset].setdefault(length, True)

    def unlock(self, fd=None, offset=None, length=None):
        """
        Unlock some or all of the locks held by this process.  It doesn't
        make sense to supply the arguments in any order but the defined
        order above.
        """
        if fd is None:
            for fd in self._locks.keys():
                self.unlock(fd, offset, length)
        elif offset is None:
            for offset in self._locks[fd].keys():
                self.unlock(fd, offset, length)
        elif length is None:
            for length in self._locks[fd][offset].keys():
                self.unlock(fd, offset, length)
        else:
            self.lock(fd, offset, length, fcntl.LOCK_UN)

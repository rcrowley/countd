"""
Interfaces that must be implemented by Keyspace, Index, and Deltas objects.
"""

import fcntl, os

class Keyspace(object):

    READ = os.O_RDONLY
    WRITE = os.O_RDWR | os.O_CREAT

    def __init__(self, keyspace, flags, index, deltas):
        raise NotImplementedError()

    def range(self, offset, length, mode=fcntl.LOCK_SH):
        raise NotImplementedError()

    def update(self, key, increment, fsync=True):
        raise NotImplementedError()

class Index(object):

    READ = os.O_RDONLY
    WRITE = os.O_RDWR | os.O_CREAT

    def __init__(self, keyspace, flags):
        raise NotImplementedError()

    def find(self, key):
        raise NotImplementedError()

    def update(self, key, offset):
        raise NotImplementedError()

class Deltas(object):

    READ = os.O_RDONLY
    WRITE = os.O_RDWR | os.O_CREAT

    def __init__(self, keyspace, flags):
        raise NotImplementedError()

    def find(self, count, offset=0):
        raise NotImplementedError()

    def update(self, count, offset):
        raise NotImplementedError()

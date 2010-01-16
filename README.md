countd(1) -- counter server
===========================

## SYNOPSIS

countd-read
countd-write
countd-worker

## DESCRIPTION

countd stores sorted sets of counters.  Each set is called a keyspace and contains counters identified by keys.  A process writes to countd by sending a message containing a keyspace, key, and increment to the countd-write daemon.  A process reads from countd by sending a message containing a keyspace, offset, and length to the countd-read daemon.  See PROTOCOL below for further discussion.

countd is implemented in Python and makes heavy use of the `os` module, which exposes unbuffered I/O functions and fcntl(2).  Synchronization between processes is accomplished using link(2), unlink(2) and fcntl(2)'s advisory locking.

countd-write is responsible for a pool of commit logs which are rotated when they become full.  Empty commit logs which are available for writes have permissions 0400; full log files which need to be read have permissions 0200.  Commit log filenames are 10-digit, zero-padded integers and start from zero.  A commit log must be successfully hard link(2)ed to its filename prefixed by "lock-" before the process may open(2) it.  When countd-write fills a commit log, its permissions are changed to 0200 and the lock is unlink(2)ed.

countd-worker repeatedly locks a full commit log with permissions 0200 and resolves the messages in it with permanent storage.  Each keyspace is stored in its own file in permanent storage, which is kept sorted to support efficient reads.  The file format and supporting indices are very much works in progress.  countd-worker is completely decoupled from countd-write, which means read-your-own-writes consistency is not guaranteed or likely.

countd-read is imaginary.

## OPTIONS

No options are supported on the command line at this time.  The `countd.settings` module configures the data directory, the maximum length of keyspace and key names, the number of commit logs to preallocate, and the size of each commit log.

## PROTOCOL

* Write:
  Writes are fixed-length messages containing an 8-byte little-endian integer and two fixed-length, null-padded strings, the keyspace and key (256 bytes each, by default).  Writes are sent to the TCP port on which countd-write is listening (48879 by default).

* Read:
  Reads are imaginary.

## DEPENDENCIES

* Python 2.6 <http://python.org/>

## BUGS

Please report bugs to <https://github.com/rcrowley/countd/issues>.

## SECURITY CONSIDERATIONS

countd doesn't encrypt or protect anything.  Use it behind a firewall or listen on 127.0.0.1.

## AUTHOR

Richard Crowley <richard@opendns.com>

## SEE ALSO

* <http://github.com/rcrowley/countd>
* <http://varnish.projects.linpro.no/wiki/ArchitectNotes>

## LICENSE

<http://www.freebsd.org/copyright/freebsd-license.html>

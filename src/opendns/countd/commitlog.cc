#include "client.h"
#include "commitlog.h"
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

namespace opendns { namespace countd {

const char *CommitLog::DIRNAME = "/tmp/countd"; // TODO Configurable

CommitLog::CommitLog() : files(FILES), file(0), fd(-1) {

	// Check the data directory; maybe create it.
	int result = mkdir(CommitLog::DIRNAME, 0700);
	if (result && EEXIST != errno) {
		if (result) { perror("[commitlog] mkdir"); }
		throw CommitLogException();
	}

	// Check every commit log.  Create and fill those that are needed.  Check but
	// don't create any beyond those that are needed.
	for (size_t i = 0; i < this->files; ++i) { this->fill(i, true); }
	size_t i = this->files;
	do {
		char pathname[4096]; // FIXME
		CommitLog::pathname_format(pathname, i);
		struct stat s;
		if (stat(pathname, &s)) { break; } // Stop at the first nonexistent file.
		this->fill(i, true);
		++this->files;
	} while (++i);

	// Choose the initial commit log.
	this->choose();

}

CommitLog::~CommitLog() {
	fsync(this->fd);
	close(this->fd);
	char pathname_lock[4096];
	CommitLog::pathname_lock_format(pathname_lock, this->file);
	unlink(pathname_lock);
}

// Resolve the full pathname of a commit log.
void CommitLog::pathname_format(char *pathname, size_t file) {
	sprintf(pathname, "%s/%010ld", CommitLog::DIRNAME, file);
}

// Resolve the full pathname of a commit log lock.
void CommitLog::pathname_lock_format(char *pathname, size_t file) {
	sprintf(pathname, "%s/lock-%010ld", CommitLog::DIRNAME, file);
}

// Make sure the file exists and is filled.
void CommitLog::fill(size_t file, bool unlock) {
	char pathname[4096], pathname_lock[4096]; // FIXME
	struct stat s, s_lock;

	// Verify that the commit log exists and is the correct size.
	CommitLog::pathname_format(pathname, file);
	if (!stat(pathname, &s) && CommitLog::filesize == s.st_size) { return; }

	// Skip out early if the commit log is locked.
	CommitLog::pathname_lock_format(pathname_lock, file);
	if (!stat(pathname_lock, &s_lock)) { return; }

	// Open the commit log locked and write it full.
	//   http://www.mail-archive.com/freebsd-hackers@freebsd.org/msg66053.html
	int fd = open(pathname_lock, O_WRONLY | O_CREAT | O_EXCL, 0600);
	if (0 > fd) {
		perror("[commitlog] open");
		throw CommitLogException();
	}
#if defined(posix_fallocate) // FIXME
#error posix_fallocate
#elif defined(fallocate) // FIXME
#error fallocate
#elif 0 && defined(F_PREALLOCATE) // FIXME
	fstore_t fst = { F_ALLOCATECONTIG, F_PEOFPOSMODE, 0, CommitLog::filesize, 0 };
	if (fcntl(fd, F_PREALLOCATE, &fst)) {
		perror("[commitlog] fcntl");
		throw CommitLogException();
	}
#else
#warning Can't find a way to preallocate files, faking it.
	message::Write empty(true);
	for (size_t j = 0; j < CommitLog::filesize; j += sizeof(message::Write)) {
		ssize_t len = write(fd, &empty, sizeof(message::Write));
		if (0 > len) {
			perror("[commitlog] write");
			close(fd);
			throw CommitLogException();
		}
		if (sizeof(message::Write) != len) {
			close(fd);
			throw CommitLogException();
		}
	}
#endif
	fsync(fd);
	close(fd);
	if (link(pathname_lock, pathname)) {
		perror("[commitlog] link");
		throw CommitLogException();
	}

	// Unlock the commit log.
	if (unlock && unlink(pathname_lock)) {
		perror("[commitlog] unlink");
		throw CommitLogException();
	}

}

// Choose the next available commit log (where available means writable and
// specifically mode 0600).  Create a new one if it comes to that.
void CommitLog::choose() {

	// Close the current commit log.
	if (0 <= fd) { close(fd); }

	char pathname_lock[4096]; // FIXME
	size_t i = this->file;
	do {
		char pathname[4096]; // FIXME
		CommitLog::pathname_format(pathname, i);
		CommitLog::pathname_lock_format(pathname_lock, i);
printf("DEBUG CommitLog::choose, i: %lu, this->file: %lu, pathname: %s, pathname_lock: %s\n", i, this->file, pathname, pathname_lock);
		if (!link(pathname, pathname_lock)) {
			this->file = i;
			goto chosen;
		}
		if (EEXIST != errno) {
			perror("[commitlog] link");
			throw CommitLogException();
		}
		i = (i + 1) % this->files;
	} while (i != this->file);

	// Create and fill a new commit log.  This step is logged because it should
	// be skipped normally.
	this->fill((this->file = this->files++), false);
	fprintf(stderr, "[commitlog] had to create a new commit log, now up to %lu\n",
		this->files);

	// Open the newly chosen commit log.
chosen:
	this->fd = open(pathname_lock, O_WRONLY); // TODO Configurable, O_DIRECT
	if (0 > this->fd) { throw CommitLogException(); }

}

// Commit a Request.
void CommitLog::commit(client::Request *request) {
	ssize_t len = write(this->fd, &request->message, sizeof(message::Write));
	if (0 > len) {
		perror("[commitlog] write");
		throw CommitLogException();
	}
	if (fsync(this->fd)) { // TODO Configurable, use fdatasync
		perror("[commitlog] fdatasync");
		throw CommitLogException();
	}
}

}} // namespace opendns::countd

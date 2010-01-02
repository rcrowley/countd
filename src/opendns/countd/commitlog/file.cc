#include "../commitlog.h"
#include "../message.h"
#include "file.h"
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

using namespace opendns::countd;

namespace opendns { namespace countd { namespace commitlog {

File::File(size_t index) : fd(-1), len(0) {
	this->init(index);
}

File::~File() { this->destroy(); }

void File::init(size_t index) {
	this->index = index;
	//if (this->pathnames.index == index) { return; }
	sprintf(this->pathnames.normal, "%s/%010ld", CommitLog::DIRNAME, index);
	sprintf(this->pathnames.dirty, "%s/dirty-%010ld", CommitLog::DIRNAME, index);
	this->pathnames.index = index;
}

void File::destroy() {
	if (0 > this->fd) { return; }
	if (fsync(this->fd)) {
		perror("[commitlog] fsync");
	}
	if (fchmod(this->fd, 0400)) {
		perror("[commitlog] fchmod");
	}
	close(this->fd);
	this->fd = -1;
	this->clean();
}

// TODO Configurable, O_DIRECT
bool File::open() {
	if (0 > (this->fd = ::open(this->pathnames.dirty, O_WRONLY))) {
		perror("[commitlog] open");
		return false;
	}
	return true;
}

bool File::read() { return true; }

bool File::write(message::Write *message) {
	if (0 > ::write(this->fd, message, sizeof(message::Write))) {
		perror("[commitlog] write");
		return false;
	}
	this->len += sizeof(message::Write);
	if (fsync(this->fd)) { // TODO Configurable, use fdatasync
		perror("[commitlog] fsync");
		return false;
	}
	return true;
}

// Make sure the file exists and is filled.
bool File::fill(bool dirty) {
	this->init(this->index);
	struct stat s, s_dirty;

	// Skip out early if the commit log exists and is the correct size or is readonly.
	if (!stat(this->pathnames.normal, &s) && (
		CommitLog::filesize == s.st_size
		|| 0400 == (0777 & s.st_mode)
	)) {
		return true;
	}

	// Skip out early if the commit log is dirty.
	if (!stat(this->pathnames.dirty, &s_dirty)) { return true; }

	// Open the commit log dirty and write it full.
	//   http://www.mail-archive.com/freebsd-hackers@freebsd.org/msg66053.html
	int fd = ::open(this->pathnames.dirty, O_WRONLY | O_CREAT | O_EXCL, 0600);
	if (0 > fd) {
		perror("[commitlog] open");
		return false;
	}
#if defined(posix_fallocate) // FIXME
#error posix_fallocate
#elif defined(fallocate) // FIXME
#error fallocate
#elif 0 && defined(F_PREALLOCATE) // FIXME
	fstore_t fst = { F_ALLOCATECONTIG, F_PEOFPOSMODE, 0, CommitLog::filesize, 0 };
	if (fcntl(fd, F_PREALLOCATE, &fst)) {
		perror("[commitlog] fcntl");
		return false;
	}
#else
#warning Can't find a way to preallocate files, faking it.
	message::Write empty(true);
	for (size_t j = 0; j < CommitLog::filesize; j += sizeof(message::Write)) {
		ssize_t len = ::write(fd, &empty, sizeof(message::Write));
		if (0 > len) {
			perror("[commitlog] write");
			close(fd);
			return false;
		}
		if (sizeof(message::Write) != len) {
			close(fd);
			return false;
		}
	}
#endif
	fsync(fd);
	close(fd);
	if (link(this->pathnames.dirty, this->pathnames.normal)) {
		perror("[commitlog] link");
		return false;
	}

	// Mark the commit log as clean.
	if (!dirty && this->clean()) { return false; }

	return true;
}

bool File::dirty(mode_t mode) {
	struct stat s;
	if (stat(this->pathnames.normal, &s) || mode != (0777 & s.st_mode)) {
		return false;
	}
	if (link(this->pathnames.normal, this->pathnames.dirty)) {
		if (EEXIST != errno) { perror("[commitlog] link"); }
		return false;
	}
	return true;
}

bool File::clean() {
	if (unlink(this->pathnames.dirty)) {
		perror("[commitlog] unlink");
		return false;
	}
	return true;
}

bool File::readonly() {
	return true;
}

bool File::readwrite() {
	return true;
}

}}} // namespace opendns::countd::commitlog

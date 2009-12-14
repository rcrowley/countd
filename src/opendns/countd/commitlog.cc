#include "client.h"
#include "commitlog.h"
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

namespace opendns { namespace countd {

const char *CommitLog::dirname = "/tmp/countd"; // TODO Configurable

CommitLog::CommitLog() {
	this->file = CommitLog::files; // Placeholder

	// Check the data directory; maybe create it
	int result = mkdir(CommitLog::dirname, 0755);
	if (result && EEXIST != errno) {
		if (result) { perror("[commitlog] mkdir"); }
		throw CommitLogException();
	}

	// Check the log files; maybe create them and write them full
	char empty[8 + 256 + 256];
	memset(empty, 0, 8 + 256 + 256);
	for (size_t i = 0; i < CommitLog::files; ++i) {
		char pathname[256]; // FIXME
		sprintf(pathname, "%s/%010ld", CommitLog::dirname, i);

		// Make sure the file exists and is filled
		struct stat s;
		if (stat(pathname, &s) || CommitLog::filesize != s.st_size) {
			// FIXME Make sure we're not overwriting unprocessed data
			int fd = open(pathname, O_WRONLY | O_CREAT | O_TRUNC | O_EXLOCK, 0755);
			if (0 > fd) {
				perror("[commitlog] open");
				throw CommitLogException();
			}

			// FIXME Does this technique actually create contiguous files?
			for (size_t j = 0; j < CommitLog::filesize; j += 8 + 256 + 256) { // TODO Configurable
				ssize_t len = write(fd, empty, 8 + 256 + 256);
				if (0 > len) {
					perror("[commitlog] write");
					close(fd);
					throw CommitLogException();
				}
				if (8 + 256 + 256 != len) {
					close(fd);
					throw CommitLogException();
				}
			}

			fsync(fd);
			close(fd);
		}

		// Choose the initial commit log
		if (CommitLog::files != this->file) { continue; }
		int fd = open(pathname, O_RDONLY);
		if (0 > fd) { throw CommitLogException(); }
		char buf[8 + 256 + 256];
		ssize_t len = read(fd, buf, 8 + 256 + 256);
		if (0 > len) {
			perror("[commitlog] read");
			close(fd);
			throw CommitLogException();
		}
		if (8 + 256 + 256 != len) {
			close(fd);
			throw CommitLogException();
		}
		if (!memcmp(empty, buf, 8 + 256 + 256)) { this->file = i; }
		close(fd);

	}

	// Open the initial commit log
	char pathname[256]; // FIXME
	sprintf(pathname, "%s/%010ld", CommitLog::dirname, this->file);
	this->fd = open(pathname, O_WRONLY | O_EXLOCK);
	if (0 > fd) { throw CommitLogException(); }

}

CommitLog::~CommitLog() {
	fsync(this->fd); // TODO Use fdatasync
	close(this->fd);
}

void CommitLog::commit(client::Request *request) {
	ssize_t len = write(this->fd, request, 8 + 256 + 256);
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

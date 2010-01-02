#include "client.h"
#include "commitlog.h"
#include "commitlog/file.h"
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

namespace opendns { namespace countd {

const char *CommitLog::DIRNAME = "/tmp/countd"; // TODO Configurable

CommitLog::CommitLog() : files(FILES) {

	// Check the data directory; maybe create it.
	if (mkdir(CommitLog::DIRNAME, 0700) && EEXIST != errno) {
		perror("[commitlog] mkdir");
		exit(1);
	}

	// Check every commit log.  Create and fill those that are needed.  Check but
	// don't create any beyond those that are needed.
	for (size_t i = 0; i < this->files; ++i) {
		this->file.init(i);
		this->file.fill(false);
	}
	size_t i = this->files;
	do {
		this->file.init(i);
		struct stat s;
		if (stat(this->file.pathnames.normal, &s)) {
			break; // Stop at the first nonexistent file.
		}
		this->file.fill(false);
		++this->files;
	} while (++i);

	// Choose the initial commit log.
	this->file.init(0);
	this->choose();

}

CommitLog::~CommitLog() {
	this->file.destroy();
}

// Choose the next available commit log.  Create a new one if it comes to that.
//   TODO React to failures within.
bool CommitLog::choose() {

	// Close the current commit log.
	this->file.destroy();

	size_t i = this->file.index, ii = this->file.index;
	do {
		this->file.init(i);
//printf("DEBUG CommitLog::choose, i: %lu, this->file.index: %lu, this->file.pathnames.normal: %s, this->file.pathnames.dirty: %s\n", i, this->file.index, this->file.pathnames.normal, this->file.pathnames.dirty);
		if (this->file.dirty(0600)) { goto chosen; }
		i = (i + 1) % this->files;
	} while (i != ii);

	// Create and fill a new commit log.  This step is logged because it should
	// be skipped normally.
	this->file.init(this->files++);
	this->file.fill(true);
	fprintf(stderr, "[commitlog] had to create a new commit log, now up to %lu\n",
		this->files);

	// Open the newly chosen commit log.
chosen:
	this->file.open();

	return true;
}

// Commit a Request.
bool CommitLog::write(client::Request *request) {
	if (0 > ::write(this->file.fd, &request->message, sizeof(message::Write))) {
		perror("[commitlog] write");
		return false;
	}
	if (fsync(this->file.fd)) { // TODO Configurable, use fdatasync
		perror("[commitlog] fsync");
		return false;
	}
	return true;
}

}} // namespace opendns::countd

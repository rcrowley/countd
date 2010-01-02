#ifndef OPENDNS_COUNTD_COMMITLOG_H
#define OPENDNS_COUNTD_COMMITLOG_H

#include "client.h"
#include "commitlog/file.h"
#include <sys/types.h>

namespace opendns { namespace countd {

struct CommitLog {
	static const size_t FILESIZE = (64 << 20); // TODO Configurable
	static const char *DIRNAME;
	static const size_t FILES = 4; // TODO Configurable

	static const size_t filesize = (FILESIZE) - ((FILESIZE) % sizeof(message::Write));

	size_t files;

	commitlog::File file;

	CommitLog();
	virtual ~CommitLog();

	bool choose();
	bool commit(client::Request *request);

};

}} // namespace opendns::countd

#endif

#ifndef OPENDNS_COUNTD_COMMITLOG_H
#define OPENDNS_COUNTD_COMMITLOG_H

#include "client.h"
#include "message.h"
#include <exception>
#include <sys/types.h>

namespace opendns { namespace countd {

class CommitLogException : public std::exception {};

struct CommitLog {
	static const size_t FILESIZE = (64 << 20); // TODO Configurable
	static const char *DIRNAME;
	static const char *PATHNAME_FORMAT;
	static const size_t FILES = 4; // TODO Configurable

	static const size_t filesize = (FILESIZE) - ((FILESIZE) % sizeof(message::Write));

	size_t files;
	size_t file;

	int fd;

	CommitLog();
	virtual ~CommitLog();

	static void pathname_format(char *pathname, size_t file);

	void commit(client::Request *request);

};

}} // namespace opendns::countd

#endif

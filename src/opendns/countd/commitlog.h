#ifndef OPENDNS_COUNTD_COMMITLOG_H
#define OPENDNS_COUNTD_COMMITLOG_H

#include "client.h"
#include "message.h"
#include <exception>
#include <sys/types.h>

namespace opendns { namespace countd {

class CommitLogException : public std::exception {};

struct CommitLog {
	static const size_t filesize = (64 << 20) - ((64 << 20) % sizeof(message::Write)); // TODO Configurable
	static const char *dirname;
	static const size_t files = 4; // TODO Configurable
	size_t file;

	int fd;

	CommitLog();
	virtual ~CommitLog();

	void commit(client::Request *request);

};

}} // namespace opendns::countd

#endif

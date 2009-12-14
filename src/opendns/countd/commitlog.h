#ifndef OPENDNS_COUNTD_WAL_H
#define OPENDNS_COUNTD_WAL_H

#include <exception>
#include <sys/types.h>

namespace opendns { namespace countd {

class CommitLogException : public std::exception {};

class CommitLog {
public:
	static const size_t filesize = (64 << 20) - ((64 << 20) % (8 + 256 + 256)); // TODO Configurable
private:
	static const char *dirname;
	static const size_t files = 4; // TODO Configurable
	size_t file;

	int fd;

public:
	CommitLog();
	virtual ~CommitLog();

	void commit(client::Request *request);

};

}} // namespace opendns::countd

#endif

#ifndef OPENDNS_COUNTD_COMMITLOG_FILE_H
#define OPENDNS_COUNTD_COMMITLOG_FILE_H

#include "../message.h"
#include <sys/types.h>

namespace opendns { namespace countd { namespace commitlog {

struct File {

	size_t index;

	struct {
		char normal[4096], dirty[4096]; // FIXME
		size_t index;
	} pathnames;

	int fd;
	off_t len;

	File(size_t index = 0);
	~File();
	void init(size_t index = 0);
	void destroy();

	bool open();
	bool read();
	bool write(message::Write *message);

	bool fill(bool dirty);
	bool dirty(mode_t mode);
	bool clean();
	bool readonly();
	bool readwrite();

};

}}} // namespace opendns::countd::commitlog

#endif

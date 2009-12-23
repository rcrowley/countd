#include "opendns/countd/commitlog.h"
#include "opendns/countd/message.h"
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

using namespace opendns::countd;

// A tool for reading countd commit log files.
int main(int argc, char **argv) {

	// The only argument is the numeric file-id.
	if (2 != argc) {
		fprintf(stderr, "Usage: %s <file-id>\n", argv[0]);
		return 1;
	}

	// Resolve the pathname to the commit log.
	char pathname[4096]; // FIXME
	CommitLog::pathname_format(pathname, atoi(argv[1]));

	// Output a human-readable version of the commit log.
	int fd = open(pathname, O_RDONLY);
	if (0 > fd) { return 1; }
	message::Write message, empty;
	while (sizeof(message::Write) == read(fd, &message, sizeof(message::Write))) {
		if (!memcmp(&empty, &message, sizeof(message::Write))) { continue; }
		printf("%-34s %-34s %10lld\n",
			message.keyspace, message.key, message.increment);
	}

	return 0;
}

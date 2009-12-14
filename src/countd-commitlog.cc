#include "opendns/countd/commitlog.h"
#include "opendns/countd/message.h"
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

using namespace opendns::countd;

int main(int argc, char **argv) {

	if (2 != argc) {
		fprintf(stderr, "Usage: %s <file-id>\n", argv[0]);
		return 1;
	}

	char pathname[256]; // FIXME
	sprintf(pathname, "%s/%010d", CommitLog::dirname, atoi(argv[1]));

	int fd = open(pathname, O_RDONLY);
	if (0 > fd) { return 1; }
	message::Write message, empty;
	while (sizeof(message::Write) == read(fd, &message, sizeof(message::Write))) {
		if (!memcmp(&empty, &message, sizeof(message::Write))) { continue; }
		printf("%-34s %-34s %10lld\n", message.keyspace, message.key, message.increment);
	}

	return 0;
}

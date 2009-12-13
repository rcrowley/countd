/*
 * With apologies to william-os4y.
 * http://github.com/william-os4y/fapws3/blob/master/fapws/_evwsgi.c
 */

#include "opendns/countd/client.h"
#include <ev.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

using namespace opendns::countd;

static void connection_cb(struct ev_loop *loop, ev_io *io, int revents) {
	Client *client = client::resume(loop, io, revents);
	if (!(revents & EV_READ)) { return; }

	try {
		client::Request request(client);
		printf("DEBUG keyspace: %s, key: %s, increment: %lld\n",
			request.keyspace, request.key, request.increment);
		request.respond(client::Request::SUCCESS);
	}

	catch (ClientDisconnectException &e) {
		ev_io_stop(loop, io);
		delete client;
	}

	catch (ClientException &e) {
		printf("DEBUG caught ClientException\n");
	}

}

void accept_cb(struct ev_loop *loop, ev_io *io, int revents) {
	client::init(loop, io, revents, connection_cb);
}

int main(int argc, char **argv) {

	int port = 48879; // TODO Configurable

	// Listen 
	fprintf(stderr, "[countd-write] listening\n");
	int fd = socket(AF_INET, SOCK_STREAM, 0);
	if (0 > fd) {
		perror("[countd-write] socket");
		return 1;
	}
	int yes = 1;
	if (0 > setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int))) {
		perror("[countd-write] setsockopt");
		return 1;
	}
	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(struct sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY; // TODO Configurable
	addr.sin_port = htons(port);
	if (0 > bind(fd, (struct sockaddr *)&addr, sizeof(struct sockaddr_in))) {
		perror("[countd-write] bind");
		return 1;
	}
	if (0 > listen(fd, 1000)) {
		perror("[countd-write] listen");
		return 1;
	}

	// Fire up the event loop
	fprintf(stderr, "[countd-write] starting event loop\n");
	struct ev_loop *loop = ev_default_loop(0);
	ev_io io;
	ev_io_init(&io, accept_cb, fd, EV_READ);
	ev_io_start(loop, &io);
	ev_loop(loop, 0);

	return 0;
}

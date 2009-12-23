#include "opendns/countd/client.h"
#include "opendns/countd/commitlog.h"
#include <ev.h>
#include <new>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

using namespace opendns::countd;
using namespace std;

static CommitLog *commitlog = 0; // FIXME Global

// A connected Client has made a Requeset, we read() it and write() a response.
static void connection_cb(struct ev_loop *loop, ev_io *io, int revents) {
	if (!(revents & EV_READ)) { return; }
	Client *client = client::resume(loop, io, revents);

	// Create the Request (this reads from the wire).
	try {
		client::Request request(client);
		printf("DEBUG keyspace: %s, key: %s, increment: %lld\n",
			request.message.keyspace, request.message.key, request.message.increment);
		commitlog->commit(&request);
		request.respond(client::Request::SUCCESS);
	}

	catch (ClientDisconnectException &e) {
		ev_io_stop(loop, io);
		delete client;
	}

	catch (ClientException &e) {
		printf("DEBUG caught ClientException\n");
		//request.respond(client::Request::FAILURE); // FIXME
	}

}

// A new Client has connected, we accept() it.
void accept_cb(struct ev_loop *loop, ev_io *io, int revents) {
	client::init(loop, io, revents, connection_cb);
}

// The write side of the countd server.
int main(int argc, char **argv) {

	int port = 48879; // TODO Configurable

	// Commit log.
	fprintf(stderr, "[countd-write] creating commit log\n");
	commitlog = new CommitLog;

	// Listen.
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
	if (0 > listen(fd, 1000)) { // TODO Configurable?  Better hard-coded value?
		perror("[countd-write] listen");
		return 1;
	}

	// Fire up the event loop.
	//   TODO Add 1-second fdatasync timer
	fprintf(stderr, "[countd-write] starting event loop\n");
	struct ev_loop *loop = ev_default_loop(0);
	ev_io io;
	ev_io_init(&io, accept_cb, fd, EV_READ);
	ev_io_start(loop, &io);
	ev_loop(loop, 0);

	return 0;
}

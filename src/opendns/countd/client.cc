#include "client.h"
#include <ev.h>
#include <exception>
#include <fcntl.h>
#include <netinet/in.h>
#include <new>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

using namespace std;

namespace opendns { namespace countd { namespace client {

void init(struct ev_loop *loop, ev_io *io, int revents, void(callback)(
	struct ev_loop *loop,
	ev_io *io,
	int revents
)) throw (bad_alloc, ClientException) {

	Client *client = new Client;
	memset(client, 0, sizeof(Client));

	socklen_t len = sizeof(struct sockaddr_in);
	client->fd = accept(io->fd, (struct sockaddr *)&client->addr, &len);
	if (0 > client->fd) {
		perror("[client] accept");
		throw ClientException();
	}

	int flags = fcntl(client->fd, F_GETFL);
	if (0 > flags) {
		perror("[client] fcntl\n");
		throw ClientException();
	}
	flags |= O_NONBLOCK;
	if (0 > fcntl(client->fd, F_SETFL, flags)) {
		perror("[client] fcntl\n");
		throw ClientException();
	}

	// This is how we'll get this pointer back when new events come
	ev_io_init(&client->io, callback, client->fd, EV_READ);
	ev_io_start(loop, &client->io);

}

Client *resume(struct ev_loop *loop, ev_io *io, int revents) {
	return (Client *)io;
}

Request::Request(Client *client) throw (ClientException) {
	ssize_t len = read(client->fd, this, 8 + 256 + 256); // TODO Configurable
	if (!len) { throw ClientDisconnectException(); }
	if (8 + 256 + 256 != len) { throw ClientException(); }
	this->client = client;
}

void Request::respond(int code) {
	if (0 > write(client->fd, "WIN", 3)) {
		throw ClientException();
	}
}

}}} // namespace opendns::countd::client

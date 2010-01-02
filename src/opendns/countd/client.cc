#include "client.h"
#include "message.h"
#include <ev.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

using namespace std;

namespace opendns { namespace countd { namespace client {

// Create a new Client, accept() the connection and register callbacks.
bool init(struct ev_loop *loop, ev_io *io, int revents, void(callback)(
	struct ev_loop *loop,
	ev_io *io,
	int revents
)) {
	Client *client = new Client;

	socklen_t len = sizeof(struct sockaddr_in);
	client->fd = accept(io->fd, (struct sockaddr *)&client->addr, &len);
	if (0 > client->fd) {
		perror("[client] accept");
		return false;
	}

	int flags = fcntl(client->fd, F_GETFL);
	if (0 > flags) {
		perror("[client] fcntl\n");
		return false;
	}
	flags |= O_NONBLOCK;
	if (0 > fcntl(client->fd, F_SETFL, flags)) {
		perror("[client] fcntl\n");
		return false;
	}

	// This is how we'll get this pointer back when new events come.
	ev_io_init(&client->io, callback, client->fd, EV_READ);
	ev_io_start(loop, &client->io);

	return true;
}

// A Client has returned.  The ev_io is a part of the Client struct.
Client *resume(struct ev_loop *loop, ev_io *io, int revents) {
	return (Client *)io;
}

Request::Request(Client *client) : client(client) {}

// Populate a Request by read()ing from the wire.
ssize_t Request::read() {
	return ::read(this->client->fd, &this->message, sizeof(message::Write));
}

// Respond to a Request with the given code.
ssize_t Request::write(int32_t code) {
	return ::write(this->client->fd, "WIN", 3);
}

}}} // namespace opendns::countd::client

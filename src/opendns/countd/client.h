#ifndef OPENDNS_COUNTD_CLIENT_H
#define OPENDNS_COUNTD_CLIENT_H

#include "message.h"
#include <ev.h>
#include <netinet/in.h>
#include <stdint.h>

namespace opendns { namespace countd {

struct Client {

	// This must be first so that (Client *)io works.
	ev_io io;

	int fd;
	struct sockaddr_in addr;
};

namespace client {

bool init(struct ev_loop *loop, ev_io *io, int revents, void(callback)(
	struct ev_loop *loop,
	ev_io *io,
	int revents
));
Client *resume(struct ev_loop *loop, ev_io *io, int revents);

struct Request {
	enum {
		SUCCESS,
		FAILURE
	};

	message::Write message;
	Client *client;

	Request(Client *client);
	ssize_t read();
	ssize_t write(int code);

};

} // namespace opendns::countd::client

}} // namespace opendns::countd

#endif

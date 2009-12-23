#ifndef OPENDNS_COUNTD_CLIENT_H
#define OPENDNS_COUNTD_CLIENT_H

#include "message.h"
#include <ev.h>
#include <netinet/in.h>
#include <new>
#include <stdint.h>

namespace opendns { namespace countd {

class ClientException : public std::exception {};
class ClientDisconnectException : public ClientException {};

struct Client {

	// This must be first so that (Client *)io works.
	ev_io io;

	int fd;
	struct sockaddr_in addr;
};

namespace client {

void init(struct ev_loop *loop, ev_io *io, int revents, void(callback)(
	struct ev_loop *loop,
	ev_io *io,
	int revents
)) throw (std::bad_alloc, ClientException);
Client *resume(struct ev_loop *loop, ev_io *io, int revents);

struct Request {
	const static int32_t SUCCESS = 0;
	const static int32_t FAILURE = 1;

	message::Write message;
	Client *client;

	Request(Client *client) throw (ClientException);

	void respond(int code);

};

} // namespace opendns::countd::client

}} // namespace opendns::countd

#endif

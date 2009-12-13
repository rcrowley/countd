#include <ev.h>
#include <exception>
#include <netinet/in.h>
#include <stdint.h>

namespace opendns { namespace countd {

struct ClientException : public std::exception {};
struct ClientDisconnectException : public ClientException {};

struct Client {
	ev_io io;
	int fd;
	struct sockaddr_in addr;
};

namespace client {

void init(struct ev_loop *loop, ev_io *io, int revents, void(callback)(
	struct ev_loop *loop,
	ev_io *io,
	int revents
)) throw (ClientException);
Client *resume(struct ev_loop *loop, ev_io *io, int revents);

struct Request {

	enum {
		SUCCESS,
		FAILURE
	};

	// These need to be first and in this order so we can read straight
	// from the wire
	int64_t increment;
	char keyspace[256]; // TODO Configurable
	char key[256]; // TODO Configurable

	Client *client;

	Request(Client *client) throw (ClientException);
	void respond(int code);

};

} // namespace opendns::countd::client

}} // namespace opendns::countd

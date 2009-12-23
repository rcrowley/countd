#ifndef OPENDNS_COUNTD_MESSAGE_H
#define OPENDNS_COUNTD_MESSAGE_H

#include <stdint.h>
#include <sys/types.h>

namespace opendns { namespace countd { namespace message {

struct Read {};

struct Write {

	const static size_t KEYSPACE = 256; // TODO Configurable
	const static size_t KEY = 256; // TODO Configurable

	// These fields must be first and in this order so we can read straight from
	// the wire.
	int64_t increment;
	char keyspace[KEYSPACE];
	char key[KEY];

	Write();
	Write(const Write &that);
	Write(const void *that);
	Write(const int64_t increment, const char *keyspace, const char *key);

};

}}} // namespace opendns::countd::message

#endif

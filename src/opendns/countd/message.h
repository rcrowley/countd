#ifndef OPENDNS_COUNTD_MESSAGE_H
#define OPENDNS_COUNTD_MESSAGE_H

#include <stdint.h>

namespace opendns { namespace countd { namespace message {

struct Read {};

// These fields must be in this order so we can read straight from the wire
struct Write {

	int64_t increment;
	char keyspace[256]; // TODO Configurable
	char key[256]; // TODO Configurable

	Write();

};

}}} // namespace opendns::countd::message

#endif

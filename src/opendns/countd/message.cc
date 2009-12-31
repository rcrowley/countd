#include "message.h"
#include <string.h>

namespace opendns { namespace countd { namespace message {

// Create a Write message as cheaply as possible.
Write::Write(bool zero) {
	if (zero) { memset(this, 0, sizeof(Write)); }
}

// Create a Write message from another message.
Write::Write(const Write &that) { memcpy(this, &that, sizeof(Write)); }

// Create a Write message from a memory region.
Write::Write(const void *that) { memcpy(this, &that, sizeof(Write)); }

// Create a Write message from disparate components.
Write::Write(const int64_t increment, const char *keyspace, const char *key) {
	this->increment = increment;
	memcpy(this->keyspace, keyspace, Write::KEYSPACE);
	memcpy(this->key, key, Write::KEY);
}

}}} // namespace opendns::countd::message

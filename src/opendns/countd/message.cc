#include "message.h"
#include <string.h>

namespace opendns { namespace countd { namespace message {

Write::Write() { memset(this, 0, sizeof(Write)); }

}}} // namespace opendns::countd::message

#pragma once

#include <fty/messagebus/message.h>
#include "malamute.h"

namespace fty::messagebus::plugin {

zmsg_t* toMalamuteMsg(const Message& msg);
Message fromMalamuteMsg(zmsg_t* msg);

}

#pragma once

#include <string>

#include "../../toolbox/shared.hpp"

struct taggedEventData {
    toolbox::SharedPtr<Server> server;
    toolbox::SharedPtr<Client> client;
};

// Copyright 2025 Ideal Broccoli

#pragma once

#include <string>

#include "../../toolbox/shared.hpp"

struct taggedEventData {
    toolbox::SharedPtr<Server> server;
    toolbox::SharedPtr<Client> client;
};

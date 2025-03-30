#pragma once

#include <string>
#include <fcntl.h>
#include <sstream>

namespace toolbox {
    std::string to_string(int val);

    void setNonBlocking(int fd);
}

#include "string.hpp"

#include <fcntl.h>
#include <sstream>

namespace toolbox {
    std::string to_string(int val) {
        std::stringstream ss;
        ss << val;
        return ss.str();
    }
}
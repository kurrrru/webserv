#pragma once

#include <string>
#include <fcntl.h>
#include <sstream>

namespace toolbox {
    std::string to_string(int val);
    std::string trim(std::string& src, const std::string& sep);
    bool caseInsensitiveCompare(const std::string& str1, const std::string& str2);

    void setNonBlocking(int fd);
}

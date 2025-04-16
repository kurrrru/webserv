// Copyright 2025 Ideal Broccoli

#pragma once

#include <string>
#include <sstream>

namespace toolbox {
std::string to_string(int val);
std::string trim(std::string* src, const std::string& sep);
bool isEqualIgnoreCase(const std::string& str1, const std::string& str2);

void setNonBlocking(int fd);
}

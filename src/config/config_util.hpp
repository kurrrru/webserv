// Copyright 2025 Ideal Broccoli

#pragma once

#include <string>
#include <vector>

namespace config {

bool caseInsensitiveCompare(const std::string& str1, const std::string& str2);
bool stringToSizeT(const std::string& str, size_t* result);

}  // namespace config

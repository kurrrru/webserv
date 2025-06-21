// Copyright 2025 Ideal Broccoli

#pragma once

#include <string>
#include <vector>

namespace config {

bool isCaseInsensitiveIdentical(const std::string& str1, const std::string& str2);
bool stringToSizeT(const std::string& str, std::size_t* result);
int pathCmp(const std::string& s1, const std::string& s2);
void throwConfigError(const std::string& message);
bool isContextToken(const std::string& token);
bool isDirectiveToken(const std::string& token);

}  // namespace config

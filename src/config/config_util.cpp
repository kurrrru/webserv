// Copyright 2025 Ideal Broccoli

#include <string>
#include <limits>

#include <sys/types.h>

#include "config_util.hpp"
#include "config_namespace.hpp"

#include "../../toolbox/stepmark.hpp"

namespace config {

bool caseInsensitiveCompare(const std::string& str1, const std::string& str2) {
    if (str1.size() != str2.size()) {
        return false;
    }
    for (size_t i = 0; i < str1.size(); ++i) {
        char c1 = str1[i];
        char c2 = str2[i];
        c1 = (c1 >= 'A' && c1 <= 'Z') ? (c1 | 0x20) : c1;
        c2 = (c2 >= 'A' && c2 <= 'Z') ? (c2 | 0x20) : c2;
        if (c1 != c2) {
            return false;
        }
    }
    return true;
}

bool stringToSizeT(const std::string& str, size_t* result) {
    if (!result) {
        toolbox::logger::StepMark::error("Invalid result pointer");
        return false;
    }
    if (str.empty()) {
        toolbox::logger::StepMark::error("Empty numeric value");
        return false;
    }
    size_t value = 0;
    size_t cutoff = std::numeric_limits<off_t>::max() / 10;
    size_t cutlim = std::numeric_limits<off_t>::max() % 10;
    for (size_t i = 0; i < str.size(); i++) {
        char c = str[i];
        if (!std::isdigit(c)) {
            toolbox::logger::StepMark::error("Invalid character in numeric value: " + str);
            return false;
        }
        if (value > cutoff || (value == cutoff && static_cast<size_t>(c - '0') > cutlim)) {
            toolbox::logger::StepMark::error("Numeric value too large: " + str);
            return false;
        }
        value = value * 10 + (c - '0');
    }
    *result = value;
    return true;
}

}  // namespace config

// Copyright 2025 Ideal Broccoli

#pragma once

#include <string>
#include <sstream>

namespace toolbox {
std::string to_string(int val);
std::string trim(std::string* src, const std::string& sep);
bool isEqualIgnoreCase(const std::string& str1, const std::string& str2);

template <typename Iterator, typename Predicate>
bool all_true(Iterator begin, Iterator end, Predicate pred) {
    for (Iterator it = begin; it != end; ++it) {
        if (!pred(*it)) return false;
    }
    return true;
}

template <typename Iterator, typename Predicate>
bool any_true(Iterator begin, Iterator end, Predicate pred) {
    for (Iterator it = begin; it != end; ++it) {
        if (pred(*it)) return true;
    }
    return false;
}

}  // namespace toolbox

// Copyright 2025 Ideal Broccoli

#pragma once

#include <string>

struct CaseInsensitiveLess {
    bool operator()(const std::string& lhs, const std::string& rhs) const {
        std::size_t len = lhs.size() < rhs.size() ? lhs.size() : rhs.size();
        for (std::size_t i = 0; i < len; ++i) {
            unsigned char l = static_cast<unsigned char>(lhs[i]);
            unsigned char r = static_cast<unsigned char>(rhs[i]);
            int lc = std::tolower(l);
            int rc = std::tolower(r);
            if (lc < rc) return true;
            if (lc > rc) return false;
        }
        // If the lengths do not match, treat the shorter length as smaller
        return lhs.size() < rhs.size();
    }
};


#include <map>
#include <vector>
#include <string>
#include <iostream>

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

int main(void) {
    std::map<std::string, std::string, CaseInsensitiveLess> testMap;
    testMap["Host"] = "WORLD";
    testMap["Content-Length"] = "1000";
    std::map<std::string, std::string, CaseInsensitiveLess>::iterator it;
    it = testMap.find("host");
    if (it != testMap.end()) {
        std::cout << testMap.find("host")->second << std::endl;
    }
    it = testMap.find("HosT");
    if (it != testMap.end()) {
        std::cout << testMap.find("host")->second << std::endl;
    }
}

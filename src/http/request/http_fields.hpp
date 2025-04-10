#pragma once

#include <map>
#include <string>
#include <vector>

class HTTPFields {
   public:
    void add();
    std::vector<std::string> get();
    bool parse();

   private:
    std::map<std::string, std::vector<std::string>> _headersMap;
};

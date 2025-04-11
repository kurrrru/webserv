#pragma once

#include <map>
#include <string>
#include <vector>

class HTTPFields {
   public:
    HTTPFields() {};
    ~HTTPFields() {};
    bool isInitialized();
    void initFieldsMap();
    bool add(std::pair<std::string, std::vector<std::string>>& pair);
    std::vector<std::string> get(const std::string& key);
    std::map<std::string, std::vector<std::string>> _fieldsMap;

   private:
    HTTPFields(HTTPFields& other) {};
    HTTPFields& operator=(HTTPFields& other) { return *this; };
};

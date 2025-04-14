#pragma once

#include <iostream>
#include <map>
#include <string>
#include <utility>
#include <vector>

class HTTPFields {
   public:
    HTTPFields() : _isInitialized(false) {}
    ~HTTPFields() {}
    bool isInitialized();
    void initFieldsMap();
    bool addField(std::pair<std::string, std::vector<std::string>>& pair);

    std::vector<std::string>& getFieldValue(const std::string& key);
    std::map<std::string, std::vector<std::string>>& get();

   private:
    HTTPFields(HTTPFields& other) {}
    HTTPFields& operator=(HTTPFields& other) { return *this; }

    bool _isInitialized;
    std::map<std::string, std::vector<std::string>> _fieldsMap;
};

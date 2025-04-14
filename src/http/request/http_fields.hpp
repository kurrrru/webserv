#pragma once

#include <iostream>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include "../../../toolbox/string.hpp"
#include "../http_namespace.hpp"

class HTTPFields {
   public:
    HTTPFields() : _isInitialized(false) {}
    ~HTTPFields() {}
    bool isInitialized();
    void initFieldsMap();
    bool addField(
        const std::pair<std::string, std::vector<std::string> >& pair);

    std::vector<std::string>& getFieldValue(const std::string& key);
    std::map<std::string, std::vector<std::string> >& get();

   private:
    HTTPFields(const HTTPFields& other);
    HTTPFields& operator=(const HTTPFields& other);

    bool _isInitialized;
    std::map<std::string, std::vector<std::string> > _fieldsMap;
};

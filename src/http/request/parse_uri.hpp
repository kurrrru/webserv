#pragma once

#include <map>
#include <string>

class ParseURI {
   public:
    static void parseURI(std::string& uri, std::string& path,
                         std::string& query, std::string& fragment,
                         std::map<std::string, std::string> queryMap);
    static void parseQuery(std::string query,
                           std::map<std::string, std::string> queryMap);
    static std::string normalizePath(std::string& path);
    static bool isTraversalAttack(std::string& path);
};

#pragma once

#include <string>
#include <vector>
#include <set>

#include "config_namespace.hpp"

namespace config {

class ErrorPage {
 public:
    ErrorPage();
    ErrorPage(const ErrorPage&);
    ErrorPage& operator=(const ErrorPage&);
    ~ErrorPage();

    void addCode(size_t code) { _codes.push_back(code); }
    const std::vector<size_t>& getCodes() const { return _codes; }
    const std::string& getPath() const { return _path; }
    void setPath(const std::string& path) { _path = path; }

 private:
    std::vector<size_t> _codes;
    std::string _path;
};

class Listen {
 public:
    Listen();
    Listen(const Listen&);
    Listen& operator=(const Listen&);
    ~Listen();

    size_t getPort() const { return _port; }
    const std::string& getIp() const { return _ip; }
    bool isDefaultServer() const { return _default_server; }
    void setPort(size_t port) { _port = port; }
    void setIp(const std::string& ip) { _ip = ip; }
    void setDefaultServer(bool default_server) { _default_server = default_server; }

 private:
    size_t _port;
    std::string _ip;
    bool _default_server;
};

class ServerName {
 public:
    enum ServerNameType {
         EXACT,
         WILDCARD_START,
         WILDCARD_END
    };
    ServerName();
    ServerName(const ServerName&);
    ServerName& operator=(const ServerName&);
    ~ServerName();

    const std::string& getName() const { return _name; }
    ServerNameType getType() const { return _type; }
    void setName(const std::string& name) { _name = name; }
    void setType(ServerNameType type) { _type = type; }

 private:
    std::string _name;
    ServerNameType _type;
};

class Return {
 public:
    Return();
    ~Return();
    Return(const Return&);
    Return& operator=(const Return&);

    size_t getStatusCode() const { return _status_code; }
    const std::string& getTextOrUrl() const { return _text_or_url; }
    bool isTextOrUrlSetting() const { return _is_text_or_url_setting; }
    bool hasReturnValue() const { return _has_return_value; }
    void setStatusCode(size_t code) { _status_code = code; }
    void setTextOrUrl(const std::string& text_or_url) { _text_or_url = text_or_url; }
    void setIsTextOrUrlSetting(bool is_setting) { _is_text_or_url_setting = is_setting; }
    void setHasReturnValue(bool has_value) { _has_return_value = has_value; }

 private:
    size_t _status_code;
    std::string _text_or_url;
    bool _is_text_or_url_setting;
    bool _has_return_value;
};

class ConfigBase {
 public:
    ConfigBase();
    ConfigBase(const ConfigBase&);
    virtual ~ConfigBase();

    const std::vector<std::string>& getAllowedMethods() const { return _allowed_methods; }
    bool getAutoindex() const { return _autoindex; }
    const std::vector<std::string>& getCgiExtensions() const { return _cgi_extensions; }
    const std::string& getCgiPass() const { return _cgi_pass; }
    size_t getClientMaxBodySize() const { return _client_max_body_size; }
    const std::vector<ErrorPage>& getErrorPages() const { return _error_pages; }
    const std::vector<std::string>& getIndices() const { return _indices; }
    const std::string& getRoot() const { return _root; }
    const std::string& getUploadStore() const { return _upload_store; }

    void setAllowedMethods(const std::vector<std::string>& methods) { _allowed_methods = methods; }
    void addAllowedMethod(const std::string& method) { _allowed_methods.push_back(method); }
    void setAutoindex(bool value) { _autoindex = value; }
    void setCgiExtensions(const std::vector<std::string>& extensions) { _cgi_extensions = extensions; }
    void addCgiExtension(const std::string& extension) { _cgi_extensions.push_back(extension); }
    void setCgiPass(const std::string& path) { _cgi_pass = path; }
    void setClientMaxBodySize(size_t size) { _client_max_body_size = size; }
    void setErrorPages(const std::vector<ErrorPage>& pages) { _error_pages = pages; }
    void addErrorPage(const ErrorPage& page) { _error_pages.push_back(page); }
    void setIndices(const std::vector<std::string>& indices) { _indices = indices; }
    void addIndex(const std::string& index) { _indices.push_back(index); }
    void setRoot(const std::string& path) { _root = path; }
    void setUploadStore(const std::string& path) { _upload_store = path; }

 private:
    ConfigBase& operator=(const ConfigBase&);

    std::vector<std::string> _allowed_methods;
    bool _autoindex;
    std::vector<std::string> _cgi_extensions;
    std::string _cgi_pass;
    size_t _client_max_body_size;
    std::vector<ErrorPage> _error_pages;
    std::vector<std::string> _indices;
    std::string _root;
    std::string _upload_store;
};

}  // namespace config

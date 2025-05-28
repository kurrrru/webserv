#pragma once

#include <sys/socket.h>

#include <string>
#include <map>
#include <sstream>
#include <vector>
#include <utility>

#include "../../../toolbox/stepmark.hpp"

namespace http {

class Response {
 public:
    typedef bool ResponseFlag;
    typedef std::string FieldName;
    typedef std::string FieldContent;
    typedef std::pair<ResponseFlag, FieldContent> HeaderField;

    Response();
    Response(const Response& other);
    Response& operator=(const Response& other);
    ~Response();

    void setStatus(int code);
    void setHeader(const FieldName& name,
        const FieldContent& value, ResponseFlag enabled = true);
    void setHeader(const FieldName& name,
        const std::vector<FieldContent>& values, ResponseFlag enabled = true);

    void setBody(const std::string& body);

    void sendResponse(int client_fd) const;
    static std::string getStatusMessage(int code);

    int getStatus() const;

 private:
    int _status;
    std::map<FieldName, HeaderField> _headers;
    std::string _body;

    std::string buildResponse() const;
};

}  // namespace http

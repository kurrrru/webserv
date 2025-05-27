#pragma once

#include <vector>
#include <string>
#include <map>

#include "../../../toolbox/shared.hpp"
#include "../../../src/config/config.hpp"
#include "../../../src/http/request/request_parser.hpp"
#include "../../../src/http/response/response.hpp"
#include "../../../src/core/client.hpp"

namespace test {

class TestClient {
 private:
    std::string _serverIp;
    size_t _serverPort;

 public:
    TestClient(const std::string& serverIp, size_t serverPort)
        : _serverIp(serverIp), _serverPort(serverPort) {}

    std::string getServerIp() const { return _serverIp; }
    size_t getServerPort() const { return _serverPort; }
    std::string getIp() const { return "127.0.0.1"; }
    int getFd() const { return -1; }
};

class TestResponse {
 private:
    int _status;
    std::map<std::string, std::string> _headers;
    std::string _body;

 public:
    TestResponse() : _status(200) {}

    void setStatus(int status) { _status = status; }
    int getStatus() const { return _status; }

    void setHeader(const std::string& key, const std::string& value) {
        _headers[key] = value;
    }
    std::string getHeader(const std::string& key) const {
        std::map<std::string, std::string>::const_iterator it = _headers.find(key);
        return (it != _headers.end()) ? it->second : "";
    }

    void setBody(const std::string& body) { _body = body; }
    std::string getBody() const { return _body; }
    std::string getStatusMessage(int /* statusCode */) const { return "test"; }
};

class TestRequestParser {
 public:
    struct URI {
        std::string path;
    };
    struct Fields {
        std::map<std::string, std::vector<std::string> > _fields;
        void setFieldValue(const std::string& key, const std::vector<std::string>& values) {
            _fields[key] = values;
        }
        std::vector<std::string> getFieldValue(const std::string& key) const {
            std::map<std::string, std::vector<std::string> >::const_iterator it = _fields.find(key);
            return (it != _fields.end()) ? it->second : std::vector<std::string>();
        }
    };
    struct Request {
        URI uri;
        Fields fields;
    };

 private:
    Request _request;

 public:
    Request& get() { return _request; }
    const Request& get() const { return _request; }
};

class RequestTest {
 public:
    void fetchConfig();

    config::LocationConfig locationConfig;
    TestClient* client;
    TestRequestParser parsedRequest;
    TestResponse response;

    toolbox::SharedPtr<config::ServerConfig> selectServer();
    bool extractCandidateServers(
        const std::vector<toolbox::SharedPtr<config::ServerConfig> >& servers,
        std::vector<toolbox::SharedPtr<config::ServerConfig> >& candidateServers);
    std::string extractHostName();
    toolbox::SharedPtr<config::ServerConfig> matchServerByName(
        const std::vector<toolbox::SharedPtr<config::ServerConfig> >& servers,
        const std::string& hostName);
    bool processReturn(const config::Return& returnValue);
    void processReturnWithContent(size_t statusCode,
                                const std::string& content);
    void processReturnWithoutContent(size_t statusCode);
    bool isRedirectStatus(size_t statusCode) const;
    bool hasDefaultErrorPage(size_t statusCode) const;
    bool isMinimalResponse(size_t statusCode) const;
    void setRedirectResponse(size_t statusCode, const std::string& location);
    void setTextResponse(const std::string& content);
    void setHtmlErrorResponse(size_t statusCode);
    void setEmptyTextResponse();
    std::string generateDefaultBody(size_t statusCode);
    bool selectLocation(
        const toolbox::SharedPtr<config::ServerConfig>& server);
    toolbox::SharedPtr<config::LocationConfig> findDeepestMatchingLocation(
        const std::vector<toolbox::SharedPtr<config::LocationConfig> >& locations,
        const std::string& path);

    RequestTest() : client(0) {}
    ~RequestTest() { delete client; }

    void setClient(const std::string& serverIp, size_t serverPort) {
        delete client;
        client = new TestClient(serverIp, serverPort);
    }
};

}  // namespace test

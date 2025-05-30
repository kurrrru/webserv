#include <cassert>
#include <iostream>

#include "../../toolbox/stepmark.hpp"
#include "../../toolbox/shared.hpp"
#include "../../src/config/config.hpp"
#include "../../src/core/client.hpp"
#include "../../src/http/request/request_parser.hpp"
#include "../../src/http/response/response.hpp"
#include "fetch_config_test.hpp"

static int totalTests = 0;
static int passedTests = 0;

void countTest() {
    totalTests++;
    passedTests++;
}

toolbox::SharedPtr<config::HttpConfig> createTestHttpConfig() {
    toolbox::SharedPtr<config::HttpConfig> httpConfig(new config::HttpConfig());
    // Server
    {
        toolbox::SharedPtr<config::ServerConfig> server1(new config::ServerConfig());
        config::Listen listen1;
        listen1.setIp("127.0.0.1");
        listen1.setPort(8080);
        server1->addListen(listen1);
        config::ServerName serverName1;
        serverName1.setName("server1.com");
        serverName1.setType(config::ServerName::EXACT);
        server1->addServerName(serverName1);
        // location
        {
            toolbox::SharedPtr<config::LocationConfig> apiLocation(new config::LocationConfig());
            apiLocation->setPath("/api");
            apiLocation->setRoot("/var/www/api");
            server1->addLocation(apiLocation);
            toolbox::SharedPtr<config::LocationConfig> staticLocation(new config::LocationConfig());
            staticLocation->setPath("/static");
            staticLocation->setRoot("/var/www/static");
            server1->addLocation(staticLocation);
        }
        httpConfig->addServer(server1);
    }
    // Server
    {
        toolbox::SharedPtr<config::ServerConfig> server2(new config::ServerConfig());
        config::Listen listen2;
        listen2.setIp("127.0.0.1");
        listen2.setPort(8081);
        server2->addListen(listen2);
        config::ServerName serverName2;
        serverName2.setName("server2.com");
        serverName2.setType(config::ServerName::EXACT);
        server2->addServerName(serverName2);
        config::Return returnValue;
        returnValue.setHasReturnValue(true);
        returnValue.setIsTextOrUrlSetting(true);
        returnValue.setStatusCode(301);
        returnValue.setTextOrUrl("https://newsite.com/");
        server2->setReturnValue(returnValue);
        httpConfig->addServer(server2);
    }
    // Server
    {
        toolbox::SharedPtr<config::ServerConfig> server3(new config::ServerConfig());
        config::Listen listen3;
        listen3.setIp("0.0.0.0");
        listen3.setPort(8080);
        server3->addListen(listen3);
        config::ServerName serverName3;
        serverName3.setName("server3.com");
        serverName3.setType(config::ServerName::EXACT);
        server3->addServerName(serverName3);
        // location
        {
            toolbox::SharedPtr<config::LocationConfig> apiLocation(new config::LocationConfig());
            apiLocation->setPath("/api");
            apiLocation->setRoot("/var/www/api");
            server3->addLocation(apiLocation);
            toolbox::SharedPtr<config::LocationConfig> staticLocation(new config::LocationConfig());
            staticLocation->setPath("/static");
            staticLocation->setRoot("/var/www/static");
            server3->addLocation(staticLocation);
        }
        httpConfig->addServer(server3);
    }
    return httpConfig;
}

void setRequestPath(test::RequestTest& requestTest, const std::string& path) {
    requestTest.parsedRequest.get().uri.path = path;
}

void setHostHeader(test::RequestTest& requestTest, const std::string& host) {
    std::vector<std::string> hostValues;
    hostValues.push_back(host);
    requestTest.parsedRequest.get().fields.setFieldValue("Host", hostValues);
}

toolbox::SharedPtr<config::HttpConfig> createComplexTestHttpConfig() {
    toolbox::SharedPtr<config::HttpConfig> httpConfig(new config::HttpConfig());
    {
        toolbox::SharedPtr<config::ServerConfig> singleServer(new config::ServerConfig());
        config::Listen listen;
        listen.setIp("192.168.1.100");
        listen.setPort(8080);
        singleServer->addListen(listen);
        config::ServerName serverName;
        serverName.setName("single.test.com");
        serverName.setType(config::ServerName::EXACT);
        singleServer->addServerName(serverName);
        httpConfig->addServer(singleServer);
    }
    // same port, diff server_name
    {
        toolbox::SharedPtr<config::ServerConfig> vhost1(new config::ServerConfig());
        config::Listen listen1;
        listen1.setIp("10.0.0.1");
        listen1.setPort(80);
        vhost1->addListen(listen1);
        config::ServerName serverName1;
        serverName1.setName("www.example1.com");
        serverName1.setType(config::ServerName::EXACT);
        vhost1->addServerName(serverName1);
        httpConfig->addServer(vhost1);

        toolbox::SharedPtr<config::ServerConfig> vhost2(new config::ServerConfig());
        config::Listen listen2;
        listen2.setIp("10.0.0.1");
        listen2.setPort(80);
        vhost2->addListen(listen2);
        config::ServerName serverName2;
        serverName2.setName("www.example2.com");
        serverName2.setType(config::ServerName::EXACT);
        vhost2->addServerName(serverName2);
        httpConfig->addServer(vhost2);

        toolbox::SharedPtr<config::ServerConfig> vhost3(new config::ServerConfig());
        config::Listen listen3;
        listen3.setIp("10.0.0.1");
        listen3.setPort(80);
        vhost3->addListen(listen3);
        config::ServerName serverName3;
        serverName3.setName("www.example3.com");
        serverName3.setType(config::ServerName::EXACT);
        vhost3->addServerName(serverName3);
        httpConfig->addServer(vhost3);
    }
    // large amount server
    {
        toolbox::SharedPtr<config::ServerConfig> multiServer(new config::ServerConfig());
        for (int i = 0; i < 1000; ++i) {
            config::Listen listen;
            listen.setIp("172.16.0.1");
            listen.setPort(8000 + i);
            multiServer->addListen(listen);
        }
        for (int i = 0; i < 1000; ++i) {
            config::ServerName serverName;
            std::ostringstream oss;
            oss << "server" << i << ".multi.com";
            serverName.setName(oss.str());
            serverName.setType(config::ServerName::EXACT);
            multiServer->addServerName(serverName);
        }
        httpConfig->addServer(multiServer);
    }
    return httpConfig;
}

// 1. Server
void testServerSelection() {
    toolbox::logger::StepMark::info("=== Testing Server Selection ===");
    config::Config::setHttpConfig(createTestHttpConfig());
    {
        test::RequestTest requestTest;
        requestTest.setClient("127.0.0.1", 8080);
        toolbox::SharedPtr<config::ServerConfig> selectedServer = requestTest.selectServer();
        assert(selectedServer.get() != 0);
        assert(selectedServer->getServerNames()[0].getName() == "server1.com");
        toolbox::logger::StepMark::info("✓ Server selection by IP/Port: server1.com");
        countTest();
    }
    {
        test::RequestTest requestTest;
        requestTest.setClient("127.0.0.1", 8081);
        setHostHeader(requestTest, "server2.com");
        toolbox::SharedPtr<config::ServerConfig> selectedServer = requestTest.selectServer();
        assert(selectedServer.get() != 0);
        assert(selectedServer->getServerNames()[0].getName() == "server2.com");
        toolbox::logger::StepMark::info("✓ Server selection by Host: server2.com");
        countTest();
    }
    {
        test::RequestTest requestTest;
        requestTest.setClient("127.0.0.1", 8080);
        setHostHeader(requestTest, "unknown.com");
        toolbox::SharedPtr<config::ServerConfig> selectedServer = requestTest.selectServer();
        assert(selectedServer.get() != 0);
        toolbox::logger::StepMark::info("✓ Server selection fallback: unknown host");
        countTest();
    }
    {
        test::RequestTest requestTest;
        requestTest.setClient("127.0.0.1", 9999);
        setHostHeader(requestTest, "test.com");
        toolbox::SharedPtr<config::ServerConfig> selectedServer = requestTest.selectServer();
        assert(selectedServer.get() == 0);
        assert(requestTest.response.getStatus() == 400);
        toolbox::logger::StepMark::info("✓ Server selection error: no matching server");
        countTest();
    }
    config::Config::setHttpConfig(createComplexTestHttpConfig());
    {
        test::RequestTest requestTest;
        requestTest.setClient("192.168.1.100", 8080);
        setHostHeader(requestTest, "single.test.com");
        toolbox::SharedPtr<config::ServerConfig> selectedServer = requestTest.selectServer();
        assert(selectedServer.get() != 0);
        assert(selectedServer->getServerNames()[0].getName() == "single.test.com");
        toolbox::logger::StepMark::info("✓ Single server environment");
        countTest();
    }
    {
        test::RequestTest requestTest1;
        requestTest1.setClient("10.0.0.1", 80);
        setHostHeader(requestTest1, "www.example1.com");
        toolbox::SharedPtr<config::ServerConfig> selectedServer1 = requestTest1.selectServer();
        assert(selectedServer1.get() != 0);
        assert(selectedServer1->getServerNames()[0].getName() == "www.example1.com");
        toolbox::logger::StepMark::info("✓ Virtual host 1 selection");
        countTest();
        test::RequestTest requestTest2;
        requestTest2.setClient("10.0.0.1", 80);
        setHostHeader(requestTest2, "www.example2.com");
        toolbox::SharedPtr<config::ServerConfig> selectedServer2 = requestTest2.selectServer();
        assert(selectedServer2.get() != 0);
        assert(selectedServer2->getServerNames()[0].getName() == "www.example2.com");
        toolbox::logger::StepMark::info("✓ Virtual host 2 selection");
        countTest();
        test::RequestTest requestTestFallback;
        requestTestFallback.setClient("10.0.0.1", 80);
        setHostHeader(requestTestFallback, "unknown.example.com");
        toolbox::SharedPtr<config::ServerConfig> selectedServerFallback = requestTestFallback.selectServer();
        assert(selectedServerFallback.get() != 0);
        assert(selectedServer1->getServerNames()[0].getName() == "www.example1.com");
        toolbox::logger::StepMark::info("✓ Virtual host fallback");
        countTest();
    }
    {
        test::RequestTest requestTest1;
        requestTest1.setClient("172.16.0.1", 8005);
        setHostHeader(requestTest1, "server5.multi.com");
        toolbox::SharedPtr<config::ServerConfig> selectedServer1 = requestTest1.selectServer();
        assert(selectedServer1.get() != 0);
        assert(selectedServer1->getServerNames()[5].getName() == "server5.multi.com");
        toolbox::logger::StepMark::info("✓ Large Listen test: middle port");
        countTest();
        test::RequestTest requestTest2;
        requestTest2.setClient("172.16.0.1", 8099);
        setHostHeader(requestTest2, "server99.multi.com");
        toolbox::SharedPtr<config::ServerConfig> selectedServer2 = requestTest2.selectServer();
        assert(selectedServer2.get() != 0);
        assert(selectedServer2->getServerNames()[99].getName() == "server99.multi.com");
        toolbox::logger::StepMark::info("✓ Large ServerName test: last entry");
        countTest();
    }
    config::Config::setHttpConfig(createTestHttpConfig());
    {
        test::RequestTest requestTest;
        requestTest.setClient("192.168.1.1", 8080);
        setHostHeader(requestTest, "server3.com");
        toolbox::SharedPtr<config::ServerConfig> selectedServer = requestTest.selectServer();
        assert(selectedServer.get() != 0);
        assert(selectedServer->getServerNames()[0].getName() == "server3.com");
        toolbox::logger::StepMark::info("✓ 0.0.0.0 wildcard IP match: server3.com");
        countTest();
    }
    {
        test::RequestTest requestTest;
        requestTest.setClient("127.0.0.1", 8080);
        setHostHeader(requestTest, "unknown.com");
        toolbox::SharedPtr<config::ServerConfig> selectedServer = requestTest.selectServer();
        assert(selectedServer.get() != 0);
        assert(selectedServer->getServerNames()[0].getName() == "server1.com");
        toolbox::logger::StepMark::info("✓ Specific IP (127.0.0.1) prioritized over 0.0.0.0");
        countTest();
    }
}

// 2. Return
void testReturnProcessing() {
    toolbox::logger::StepMark::info("=== Testing Return Processing ===");
    {
        test::RequestTest requestTest;
        config::Return returnValue;
        bool result = requestTest.processReturn(returnValue);
        assert(result == false);
        toolbox::logger::StepMark::info("✓ Return processing: no return");
        countTest();
    }
    toolbox::logger::StepMark::info("--- Testing 2xx Success Codes (Status Only) ---");
    // 200 OK
    {
        test::RequestTest requestTest;
        config::Return returnValue;
        returnValue.setHasReturnValue(true);
        returnValue.setIsTextOrUrlSetting(false);
        returnValue.setStatusCode(200);
        bool result = requestTest.processReturn(returnValue);
        assert(result == true);
        assert(requestTest.response.getStatus() == 200);
        assert(requestTest.response.getBody() == "");
        assert(requestTest.response.getHeader("Content-Type") == "text/plain");
        assert(requestTest.response.getHeader("Content-Length") == "0");
        assert(requestTest.response.getHeader("Location") == "");
        toolbox::logger::StepMark::info("✓ Return 200: OK (status only)");
        countTest();
    }
    // 201 Created
    {
        test::RequestTest requestTest;
        config::Return returnValue;
        returnValue.setHasReturnValue(true);
        returnValue.setIsTextOrUrlSetting(false);
        returnValue.setStatusCode(201);
        bool result = requestTest.processReturn(returnValue);
        assert(result == true);
        assert(requestTest.response.getStatus() == 201);
        assert(requestTest.response.getBody() == "");
        assert(requestTest.response.getHeader("Content-Type") == "text/plain");
        assert(requestTest.response.getHeader("Content-Length") == "0");
        assert(requestTest.response.getHeader("Location") == "");
        toolbox::logger::StepMark::info("✓ Return 201: CREATED (status only)");
        countTest();
    }
    // 202 Accepted
    {
        test::RequestTest requestTest;
        config::Return returnValue;
        returnValue.setHasReturnValue(true);
        returnValue.setIsTextOrUrlSetting(false);
        returnValue.setStatusCode(202);
        bool result = requestTest.processReturn(returnValue);
        assert(result == true);
        assert(requestTest.response.getStatus() == 202);
        assert(requestTest.response.getBody() == "");
        assert(requestTest.response.getHeader("Content-Type") == "text/plain");
        assert(requestTest.response.getHeader("Content-Length") == "0");
        assert(requestTest.response.getHeader("Location") == "");
        toolbox::logger::StepMark::info("✓ Return 202: ACCEPTED (status only)");
        countTest();
    }
    // 203 Non-Authoritative Information
    {
        test::RequestTest requestTest;
        config::Return returnValue;
        returnValue.setHasReturnValue(true);
        returnValue.setIsTextOrUrlSetting(false);
        returnValue.setStatusCode(203);
        bool result = requestTest.processReturn(returnValue);
        assert(result == true);
        assert(requestTest.response.getStatus() == 203);
        assert(requestTest.response.getBody() == "");
        assert(requestTest.response.getHeader("Content-Type") == "text/plain");
        assert(requestTest.response.getHeader("Content-Length") == "0");
        assert(requestTest.response.getHeader("Location") == "");
        toolbox::logger::StepMark::info("✓ Return 203: NON_AUTHORITATIVE_INFORMATION (status only)");
        countTest();
    }
    // 204 No Content
    {
        test::RequestTest requestTest;
        config::Return returnValue;
        returnValue.setHasReturnValue(true);
        returnValue.setIsTextOrUrlSetting(false);
        returnValue.setStatusCode(204);
        bool result = requestTest.processReturn(returnValue);
        assert(result == true);
        assert(requestTest.response.getStatus() == 204);
        assert(requestTest.response.getBody() == "");
        assert(requestTest.response.getHeader("Content-Type") == "");
        assert(requestTest.response.getHeader("Content-Length") == "");
        assert(requestTest.response.getHeader("Location") == "");
        toolbox::logger::StepMark::info("✓ Return 204: NO_CONTENT (status only)");
        countTest();
    }
    // 205 Reset Content
    {
        test::RequestTest requestTest;
        config::Return returnValue;
        returnValue.setHasReturnValue(true);
        returnValue.setIsTextOrUrlSetting(false);
        returnValue.setStatusCode(205);
        bool result = requestTest.processReturn(returnValue);
        assert(result == true);
        assert(requestTest.response.getStatus() == 205);
        assert(requestTest.response.getBody() == "");
        assert(requestTest.response.getHeader("Content-Type") == "text/plain");
        assert(requestTest.response.getHeader("Content-Length") == "0");
        assert(requestTest.response.getHeader("Location") == "");
        toolbox::logger::StepMark::info("✓ Return 205: RESET_CONTENT (status only)");
        countTest();
    }
    // 206 Partial Content
    {
        test::RequestTest requestTest;
        config::Return returnValue;
        returnValue.setHasReturnValue(true);
        returnValue.setIsTextOrUrlSetting(false);
        returnValue.setStatusCode(206);
        bool result = requestTest.processReturn(returnValue);
        assert(result == true);
        assert(requestTest.response.getStatus() == 206);
        assert(requestTest.response.getBody() == "");
        assert(requestTest.response.getHeader("Content-Type") == "text/plain");
        assert(requestTest.response.getHeader("Content-Length") == "0");
        assert(requestTest.response.getHeader("Location") == "");
        toolbox::logger::StepMark::info("✓ Return 206: PARTIAL_CONTENT (status only)");
        countTest();
    }
    toolbox::logger::StepMark::info("--- Testing 3xx Redirection Codes (Status Only) ---");
    // 300 Multiple Choices
    {
        test::RequestTest requestTest;
        config::Return returnValue;
        returnValue.setHasReturnValue(true);
        returnValue.setIsTextOrUrlSetting(false);
        returnValue.setStatusCode(300);
        bool result = requestTest.processReturn(returnValue);
        assert(result == true);
        assert(requestTest.response.getStatus() == 300);
        assert(requestTest.response.getBody().empty());
        assert(requestTest.response.getHeader("Content-Type") == "text/plain");
        assert(requestTest.response.getHeader("Content-Length") == "0");
        assert(requestTest.response.getHeader("Location").empty());
        toolbox::logger::StepMark::info("✓ Return 300: MULTIPLE_CHOICES (status only)");
        countTest();
    }
    // 301 Moved Permanently
    {
        test::RequestTest requestTest;
        config::Return returnValue;
        returnValue.setHasReturnValue(true);
        returnValue.setIsTextOrUrlSetting(false);
        returnValue.setStatusCode(301);
        bool result = requestTest.processReturn(returnValue);
        assert(result == true);
        assert(requestTest.response.getStatus() == 301);
        assert(!requestTest.response.getBody().empty());
        assert(requestTest.response.getHeader("Content-Type") == "text/html");
        assert(requestTest.response.getHeader("Content-Length") != "");
        assert(requestTest.response.getHeader("Location") == "");
        toolbox::logger::StepMark::info("✓ Return 301: MOVED_PERMANENTLY (status only)");
        countTest();
    }
    // 302 Found
    {
        test::RequestTest requestTest;
        config::Return returnValue;
        returnValue.setHasReturnValue(true);
        returnValue.setIsTextOrUrlSetting(false);
        returnValue.setStatusCode(302);
        bool result = requestTest.processReturn(returnValue);
        assert(result == true);
        assert(requestTest.response.getStatus() == 302);
        assert(!requestTest.response.getBody().empty());
        assert(requestTest.response.getHeader("Content-Type") == "text/html");
        assert(requestTest.response.getHeader("Content-Length") != "");
        assert(requestTest.response.getHeader("Location") == "");
        toolbox::logger::StepMark::info("✓ Return 302: FOUND (status only)");
        countTest();
    }
    // 303 See Other
    {
        test::RequestTest requestTest;
        config::Return returnValue;
        returnValue.setHasReturnValue(true);
        returnValue.setIsTextOrUrlSetting(false);
        returnValue.setStatusCode(303);
        bool result = requestTest.processReturn(returnValue);
        assert(result == true);
        assert(requestTest.response.getStatus() == 303);
        assert(!requestTest.response.getBody().empty());
        assert(requestTest.response.getHeader("Content-Type") == "text/html");
        assert(requestTest.response.getHeader("Content-Length") != "");
        assert(requestTest.response.getHeader("Location") == "");
        toolbox::logger::StepMark::info("✓ Return 303: SEE_OTHER (status only)");
        countTest();
    }
    // 304 Not Modified
    {
        test::RequestTest requestTest;
        config::Return returnValue;
        returnValue.setHasReturnValue(true);
        returnValue.setIsTextOrUrlSetting(false);
        returnValue.setStatusCode(304);
        bool result = requestTest.processReturn(returnValue);
        assert(result == true);
        assert(requestTest.response.getStatus() == 304);
        assert(requestTest.response.getBody().empty());
        assert(requestTest.response.getHeader("Content-Type") == "text/plain");
        assert(requestTest.response.getHeader("Content-Length") == "0");
        assert(requestTest.response.getHeader("Location") == "");
        toolbox::logger::StepMark::info("✓ Return 304: NOT_MODIFIED (status only)");
        countTest();
    }
    // 307 Temporary Redirect
    {
        test::RequestTest requestTest;
        config::Return returnValue;
        returnValue.setHasReturnValue(true);
        returnValue.setIsTextOrUrlSetting(false);
        returnValue.setStatusCode(307);
        bool result = requestTest.processReturn(returnValue);
        assert(result == true);
        assert(requestTest.response.getStatus() == 307);
        assert(!requestTest.response.getBody().empty());
        assert(requestTest.response.getHeader("Content-Type") == "text/html");
        assert(requestTest.response.getHeader("Content-Length") != "");
        assert(requestTest.response.getHeader("Location") == "");
        toolbox::logger::StepMark::info("✓ Return 307: TEMPORARY_REDIRECT (status only)");
        countTest();
    }
    // 308 Permanent Redirect
    {
        test::RequestTest requestTest;
        config::Return returnValue;
        returnValue.setHasReturnValue(true);
        returnValue.setIsTextOrUrlSetting(false);
        returnValue.setStatusCode(308);
        bool result = requestTest.processReturn(returnValue);
        assert(result == true);
        assert(requestTest.response.getStatus() == 308);
        assert(!requestTest.response.getBody().empty());
        assert(requestTest.response.getHeader("Content-Type") == "text/html");
        assert(requestTest.response.getHeader("Content-Length") != "");
        assert(requestTest.response.getHeader("Location") == "");
        toolbox::logger::StepMark::info("✓ Return 308: PERMANENT_REDIRECT (status only)");
        countTest();
    }
    toolbox::logger::StepMark::info("--- Testing 4xx Client Error Codes (Status Only) ---");
    // 400 Bad Request
    {
        test::RequestTest requestTest;
        config::Return returnValue;
        returnValue.setHasReturnValue(true);
        returnValue.setIsTextOrUrlSetting(false);
        returnValue.setStatusCode(400);
        bool result = requestTest.processReturn(returnValue);
        assert(result == true);
        assert(requestTest.response.getStatus() == 400);
        assert(!requestTest.response.getBody().empty());
        assert(requestTest.response.getHeader("Content-Type") == "text/html");
        assert(requestTest.response.getHeader("Content-Length") != "");
        assert(requestTest.response.getHeader("Location") == "");
        toolbox::logger::StepMark::info("✓ Return 400: BAD_REQUEST (status only)");
        countTest();
    }
    // 401 Unauthorized
    {
        test::RequestTest requestTest;
        config::Return returnValue;
        returnValue.setHasReturnValue(true);
        returnValue.setIsTextOrUrlSetting(false);
        returnValue.setStatusCode(401);
        bool result = requestTest.processReturn(returnValue);
        assert(result == true);
        assert(requestTest.response.getStatus() == 401);
        assert(!requestTest.response.getBody().empty());
        assert(requestTest.response.getHeader("Content-Type") == "text/html");
        assert(requestTest.response.getHeader("Content-Length") != "");
        assert(requestTest.response.getHeader("Location") == "");
        toolbox::logger::StepMark::info("✓ Return 401: UNAUTHORIZED (status only)");
        countTest();
    }
    // 402 Payment Required
    {
        test::RequestTest requestTest;
        config::Return returnValue;
        returnValue.setHasReturnValue(true);
        returnValue.setIsTextOrUrlSetting(false);
        returnValue.setStatusCode(402);
        bool result = requestTest.processReturn(returnValue);
        assert(result == true);
        assert(requestTest.response.getStatus() == 402);
        assert(!requestTest.response.getBody().empty());
        assert(requestTest.response.getHeader("Content-Type") == "text/html");
        assert(requestTest.response.getHeader("Content-Length") != "");
        assert(requestTest.response.getHeader("Location") == "");
        toolbox::logger::StepMark::info("✓ Return 402: PAYMENT_REQUIRED (status only)");
        countTest();
    }
    // 403 Forbidden
    {
        test::RequestTest requestTest;
        config::Return returnValue;
        returnValue.setHasReturnValue(true);
        returnValue.setIsTextOrUrlSetting(false);
        returnValue.setStatusCode(403);
        bool result = requestTest.processReturn(returnValue);
        assert(result == true);
        assert(requestTest.response.getStatus() == 403);
        assert(!requestTest.response.getBody().empty());
        assert(requestTest.response.getHeader("Content-Type") == "text/html");
        assert(requestTest.response.getHeader("Content-Length") != "");
        assert(requestTest.response.getHeader("Location") == "");
        toolbox::logger::StepMark::info("✓ Return 403: FORBIDDEN (status only)");
        countTest();
    }
    // 404 Not Found
    {
        test::RequestTest requestTest;
        config::Return returnValue;
        returnValue.setHasReturnValue(true);
        returnValue.setIsTextOrUrlSetting(false);
        returnValue.setStatusCode(404);
        bool result = requestTest.processReturn(returnValue);
        assert(result == true);
        assert(requestTest.response.getStatus() == 404);
        assert(!requestTest.response.getBody().empty());
        assert(requestTest.response.getHeader("Content-Type") == "text/html");
        assert(requestTest.response.getHeader("Content-Length") != "");
        assert(requestTest.response.getHeader("Location") == "");
        toolbox::logger::StepMark::info("✓ Return 404: NOT_FOUND (status only)");
        countTest();
    }
    // 405 Method Not Allowed
    {
        test::RequestTest requestTest;
        config::Return returnValue;
        returnValue.setHasReturnValue(true);
        returnValue.setIsTextOrUrlSetting(false);
        returnValue.setStatusCode(405);
        bool result = requestTest.processReturn(returnValue);
        assert(result == true);
        assert(requestTest.response.getStatus() == 405);
        assert(!requestTest.response.getBody().empty());
        assert(requestTest.response.getHeader("Content-Type") == "text/html");
        assert(requestTest.response.getHeader("Content-Length") != "");
        assert(requestTest.response.getHeader("Location") == "");
        toolbox::logger::StepMark::info("✓ Return 405: METHOD_NOT_ALLOWED (status only)");
        countTest();
    }
    // 406 Not Acceptable
    {
        test::RequestTest requestTest;
        config::Return returnValue;
        returnValue.setHasReturnValue(true);
        returnValue.setIsTextOrUrlSetting(false);
        returnValue.setStatusCode(406);
        bool result = requestTest.processReturn(returnValue);
        assert(result == true);
        assert(requestTest.response.getStatus() == 406);
        assert(!requestTest.response.getBody().empty());
        assert(requestTest.response.getHeader("Content-Type") == "text/html");
        assert(requestTest.response.getHeader("Content-Length") != "");
        assert(requestTest.response.getHeader("Location") == "");
        toolbox::logger::StepMark::info("✓ Return 406: NOT_ACCEPTABLE (status only)");
        countTest();
    }
    // 407 Proxy Authentication Required
    {
        test::RequestTest requestTest;
        config::Return returnValue;
        returnValue.setHasReturnValue(true);
        returnValue.setIsTextOrUrlSetting(false);
        returnValue.setStatusCode(407);
        bool result = requestTest.processReturn(returnValue);
        assert(result == true);
        assert(requestTest.response.getStatus() == 407);
        assert(requestTest.response.getBody() == "");
        assert(requestTest.response.getHeader("Content-Type") == "");
        assert(requestTest.response.getHeader("Content-Length") == "0");
        assert(requestTest.response.getHeader("Location") == "");
        toolbox::logger::StepMark::info("✓ Return 407: PROXY_AUTHENTICATION_REQUIRED (status only)");
        countTest();
    }
    // 408 Request Timeout
    {
        test::RequestTest requestTest;
        config::Return returnValue;
        returnValue.setHasReturnValue(true);
        returnValue.setIsTextOrUrlSetting(false);
        returnValue.setStatusCode(408);
        bool result = requestTest.processReturn(returnValue);
        assert(result == true);
        assert(requestTest.response.getStatus() == 408);
        assert(!requestTest.response.getBody().empty());
        assert(requestTest.response.getHeader("Content-Type") == "text/html");
        assert(requestTest.response.getHeader("Content-Length") != "");
        assert(requestTest.response.getHeader("Location") == "");
        toolbox::logger::StepMark::info("✓ Return 408: REQUEST_TIMEOUT (status only)");
        countTest();
    }
    // 409 Conflict
    {
        test::RequestTest requestTest;
        config::Return returnValue;
        returnValue.setHasReturnValue(true);
        returnValue.setIsTextOrUrlSetting(false);
        returnValue.setStatusCode(409);
        bool result = requestTest.processReturn(returnValue);
        assert(result == true);
        assert(requestTest.response.getStatus() == 409);
        assert(!requestTest.response.getBody().empty());
        assert(requestTest.response.getHeader("Content-Type") == "text/html");
        assert(requestTest.response.getHeader("Content-Length") != "");
        assert(requestTest.response.getHeader("Location") == "");
        toolbox::logger::StepMark::info("✓ Return 409: CONFLICT (status only)");
        countTest();
    }
    // 410 Gone
    {
        test::RequestTest requestTest;
        config::Return returnValue;
        returnValue.setHasReturnValue(true);
        returnValue.setIsTextOrUrlSetting(false);
        returnValue.setStatusCode(410);
        bool result = requestTest.processReturn(returnValue);
        assert(result == true);
        assert(requestTest.response.getStatus() == 410);
        assert(!requestTest.response.getBody().empty());
        assert(requestTest.response.getHeader("Content-Type") == "text/html");
        assert(requestTest.response.getHeader("Content-Length") != "");
        assert(requestTest.response.getHeader("Location") == "");
        toolbox::logger::StepMark::info("✓ Return 410: GONE (status only)");
        countTest();
    }
    // 411 Length Required
    {
        test::RequestTest requestTest;
        config::Return returnValue;
        returnValue.setHasReturnValue(true);
        returnValue.setIsTextOrUrlSetting(false);
        returnValue.setStatusCode(411);
        bool result = requestTest.processReturn(returnValue);
        assert(result == true);
        assert(requestTest.response.getStatus() == 411);
        assert(!requestTest.response.getBody().empty());
        assert(requestTest.response.getHeader("Content-Type") == "text/html");
        assert(requestTest.response.getHeader("Content-Length") != "");
        assert(requestTest.response.getHeader("Location") == "");
        toolbox::logger::StepMark::info("✓ Return 411: LENGTH_REQUIRED (status only)");
        countTest();
    }
    // 412 Precondition Failed
    {
        test::RequestTest requestTest;
        config::Return returnValue;
        returnValue.setHasReturnValue(true);
        returnValue.setIsTextOrUrlSetting(false);
        returnValue.setStatusCode(412);
        bool result = requestTest.processReturn(returnValue);
        assert(result == true);
        assert(requestTest.response.getStatus() == 412);
        assert(!requestTest.response.getBody().empty());
        assert(requestTest.response.getHeader("Content-Type") == "text/html");
        assert(requestTest.response.getHeader("Content-Length") != "");
        assert(requestTest.response.getHeader("Location") == "");
        toolbox::logger::StepMark::info("✓ Return 412: PRECONDITION_FAILED (status only)");
        countTest();
    }
    // 413 Payload Too Large
    {
        test::RequestTest requestTest;
        config::Return returnValue;
        returnValue.setHasReturnValue(true);
        returnValue.setIsTextOrUrlSetting(false);
        returnValue.setStatusCode(413);
        bool result = requestTest.processReturn(returnValue);
        assert(result == true);
        assert(requestTest.response.getStatus() == 413);
        assert(!requestTest.response.getBody().empty());
        assert(requestTest.response.getHeader("Content-Type") == "text/html");
        assert(requestTest.response.getHeader("Content-Length") != "");
        assert(requestTest.response.getHeader("Location") == "");
        toolbox::logger::StepMark::info("✓ Return 413: PAYLOAD_TOO_LARGE (status only)");
        countTest();
    }
    // 414 URI Too Long
    {
        test::RequestTest requestTest;
        config::Return returnValue;
        returnValue.setHasReturnValue(true);
        returnValue.setIsTextOrUrlSetting(false);
        returnValue.setStatusCode(414);
        bool result = requestTest.processReturn(returnValue);
        assert(result == true);
        assert(requestTest.response.getStatus() == 414);
        assert(!requestTest.response.getBody().empty());
        assert(requestTest.response.getHeader("Content-Type") == "text/html");
        assert(requestTest.response.getHeader("Content-Length") != "");
        assert(requestTest.response.getHeader("Location") == "");
        toolbox::logger::StepMark::info("✓ Return 414: URI_TOO_LONG (status only)");
        countTest();
    }
    // 415 Unsupported Media Type
    {
        test::RequestTest requestTest;
        config::Return returnValue;
        returnValue.setHasReturnValue(true);
        returnValue.setIsTextOrUrlSetting(false);
        returnValue.setStatusCode(415);
        bool result = requestTest.processReturn(returnValue);
        assert(result == true);
        assert(requestTest.response.getStatus() == 415);
        assert(!requestTest.response.getBody().empty());
        assert(requestTest.response.getHeader("Content-Type") == "text/html");
        assert(requestTest.response.getHeader("Content-Length") != "");
        assert(requestTest.response.getHeader("Location") == "");
        toolbox::logger::StepMark::info("✓ Return 415: UNSUPPORTED_MEDIA_TYPE (status only)");
        countTest();
    }
    // 416 Range Not Satisfiable
    {
        test::RequestTest requestTest;
        config::Return returnValue;
        returnValue.setHasReturnValue(true);
        returnValue.setIsTextOrUrlSetting(false);
        returnValue.setStatusCode(416);
        bool result = requestTest.processReturn(returnValue);
        assert(result == true);
        assert(requestTest.response.getStatus() == 416);
        assert(!requestTest.response.getBody().empty());
        assert(requestTest.response.getHeader("Content-Type") == "text/html");
        assert(requestTest.response.getHeader("Content-Length") != "");
        assert(requestTest.response.getHeader("Location") == "");
        toolbox::logger::StepMark::info("✓ Return 416: RANGE_NOT_SATISFIABLE (status only)");
        countTest();
    }
    // 417 Expectation Failed
    {
        test::RequestTest requestTest;
        config::Return returnValue;
        returnValue.setHasReturnValue(true);
        returnValue.setIsTextOrUrlSetting(false);
        returnValue.setStatusCode(417);
        bool result = requestTest.processReturn(returnValue);
        assert(result == true);
        assert(requestTest.response.getStatus() == 417);
        assert(requestTest.response.getBody() == "");
        assert(requestTest.response.getHeader("Content-Type") == "");
        assert(requestTest.response.getHeader("Content-Length") == "0");
        assert(requestTest.response.getHeader("Location") == "");
        toolbox::logger::StepMark::info("✓ Return 417: EXPECTATION_FAILED (status only)");
        countTest();
    }
    // 418 I'm a teapot
    {
        test::RequestTest requestTest;
        config::Return returnValue;
        returnValue.setHasReturnValue(true);
        returnValue.setIsTextOrUrlSetting(false);
        returnValue.setStatusCode(418);
        bool result = requestTest.processReturn(returnValue);
        assert(result == true);
        assert(requestTest.response.getStatus() == 418);
        assert(requestTest.response.getBody() == "");
        assert(requestTest.response.getHeader("Content-Type") == "");
        assert(requestTest.response.getHeader("Content-Length") == "0");
        assert(requestTest.response.getHeader("Location") == "");
        toolbox::logger::StepMark::info("✓ Return 418: IM_A_TEAPOT (status only)");
        countTest();
    }
    // 421 Misdirected Request
    {
        test::RequestTest requestTest;
        config::Return returnValue;
        returnValue.setHasReturnValue(true);
        returnValue.setIsTextOrUrlSetting(false);
        returnValue.setStatusCode(421);
        bool result = requestTest.processReturn(returnValue);
        assert(result == true);
        assert(requestTest.response.getStatus() == 421);
        assert(!requestTest.response.getBody().empty());
        assert(requestTest.response.getHeader("Content-Type") == "text/html");
        assert(requestTest.response.getHeader("Content-Length") != "");
        assert(requestTest.response.getHeader("Location") == "");
        toolbox::logger::StepMark::info("✓ Return 421: MISDIRECTED_REQUEST (status only)");
        countTest();
    }
    // 422 Unprocessable Entity
    {
        test::RequestTest requestTest;
        config::Return returnValue;
        returnValue.setHasReturnValue(true);
        returnValue.setIsTextOrUrlSetting(false);
        returnValue.setStatusCode(422);
        bool result = requestTest.processReturn(returnValue);
        assert(result == true);
        assert(requestTest.response.getStatus() == 422);
        assert(requestTest.response.getBody() == "");
        assert(requestTest.response.getHeader("Content-Type") == "");
        assert(requestTest.response.getHeader("Content-Length") == "0");
        assert(requestTest.response.getHeader("Location") == "");
        toolbox::logger::StepMark::info("✓ Return 422: UNPROCESSABLE_ENTITY (status only)");
        countTest();
    }
    // 426 Upgrade Required
    {
        test::RequestTest requestTest;
        config::Return returnValue;
        returnValue.setHasReturnValue(true);
        returnValue.setIsTextOrUrlSetting(false);
        returnValue.setStatusCode(426);
        bool result = requestTest.processReturn(returnValue);
        assert(result == true);
        assert(requestTest.response.getStatus() == 426);
        assert(requestTest.response.getBody() == "");
        assert(requestTest.response.getHeader("Content-Type") == "");
        assert(requestTest.response.getHeader("Content-Length") == "0");
        assert(requestTest.response.getHeader("Location") == "");
        toolbox::logger::StepMark::info("✓ Return 426: UPGRADE_REQUIRED (status only)");
        countTest();
    }
    toolbox::logger::StepMark::info("--- Testing 5xx Server Error Codes (Status Only) ---");
    // 500 Internal Server Error
    {
        test::RequestTest requestTest;
        config::Return returnValue;
        returnValue.setHasReturnValue(true);
        returnValue.setIsTextOrUrlSetting(false);
        returnValue.setStatusCode(500);
        bool result = requestTest.processReturn(returnValue);
        assert(result == true);
        assert(requestTest.response.getStatus() == 500);
        assert(!requestTest.response.getBody().empty());
        assert(requestTest.response.getHeader("Content-Type") == "text/html");
        assert(requestTest.response.getHeader("Content-Length") != "");
        assert(requestTest.response.getHeader("Location") == "");
        toolbox::logger::StepMark::info("✓ Return 500: INTERNAL_SERVER_ERROR (status only)");
        countTest();
    }
    // 501 Not Implemented
    {
        test::RequestTest requestTest;
        config::Return returnValue;
        returnValue.setHasReturnValue(true);
        returnValue.setIsTextOrUrlSetting(false);
        returnValue.setStatusCode(501);
        bool result = requestTest.processReturn(returnValue);
        assert(result == true);
        assert(requestTest.response.getStatus() == 501);
        assert(!requestTest.response.getBody().empty());
        assert(requestTest.response.getHeader("Content-Type") == "text/html");
        assert(requestTest.response.getHeader("Content-Length") != "");
        assert(requestTest.response.getHeader("Location") == "");
        toolbox::logger::StepMark::info("✓ Return 501: NOT_IMPLEMENTED (status only)");
        countTest();
    }
    // 502 Bad Gateway
    {
        test::RequestTest requestTest;
        config::Return returnValue;
        returnValue.setHasReturnValue(true);
        returnValue.setIsTextOrUrlSetting(false);
        returnValue.setStatusCode(502);
        bool result = requestTest.processReturn(returnValue);
        assert(result == true);
        assert(requestTest.response.getStatus() == 502);
        assert(!requestTest.response.getBody().empty());
        assert(requestTest.response.getHeader("Content-Type") == "text/html");
        assert(requestTest.response.getHeader("Content-Length") != "");
        assert(requestTest.response.getHeader("Location") == "");
        toolbox::logger::StepMark::info("✓ Return 502: BAD_GATEWAY (status only)");
        countTest();
    }
    // 503 Service Unavailable
    {
        test::RequestTest requestTest;
        config::Return returnValue;
        returnValue.setHasReturnValue(true);
        returnValue.setIsTextOrUrlSetting(false);
        returnValue.setStatusCode(503);
        bool result = requestTest.processReturn(returnValue);
        assert(result == true);
        assert(requestTest.response.getStatus() == 503);
        assert(!requestTest.response.getBody().empty());
        assert(requestTest.response.getHeader("Content-Type") == "text/html");
        assert(requestTest.response.getHeader("Content-Length") != "");
        assert(requestTest.response.getHeader("Location") == "");
        toolbox::logger::StepMark::info("✓ Return 503: SERVICE_UNAVAILABLE (status only)");
        countTest();
    }
    // 504 Gateway Timeout
    {
        test::RequestTest requestTest;
        config::Return returnValue;
        returnValue.setHasReturnValue(true);
        returnValue.setIsTextOrUrlSetting(false);
        returnValue.setStatusCode(504);
        bool result = requestTest.processReturn(returnValue);
        assert(result == true);
        assert(requestTest.response.getStatus() == 504);
        assert(!requestTest.response.getBody().empty());
        assert(requestTest.response.getHeader("Content-Type") == "text/html");
        assert(requestTest.response.getHeader("Content-Length") != "");
        assert(requestTest.response.getHeader("Location") == "");
        toolbox::logger::StepMark::info("✓ Return 504: GATEWAY_TIMEOUT (status only)");
        countTest();
    }
    // 505 HTTP Version Not Supported
    {
        test::RequestTest requestTest;
        config::Return returnValue;
        returnValue.setHasReturnValue(true);
        returnValue.setIsTextOrUrlSetting(false);
        returnValue.setStatusCode(505);
        bool result = requestTest.processReturn(returnValue);
        assert(result == true);
        assert(requestTest.response.getStatus() == 505);
        assert(!requestTest.response.getBody().empty());
        assert(requestTest.response.getHeader("Content-Type") == "text/html");
        assert(requestTest.response.getHeader("Content-Length") != "");
        assert(requestTest.response.getHeader("Location") == "");
        toolbox::logger::StepMark::info("✓ Return 505: HTTP_VERSION_NOT_SUPPORTED (status only)");
        countTest();
    }
    // === 2xx Success Codes (With Text/URL) ===
    toolbox::logger::StepMark::info("--- Testing 2xx Success Codes (With Text/URL) ---");
    // 200 OK with text
    {
        test::RequestTest requestTest;
        config::Return returnValue;
        returnValue.setHasReturnValue(true);
        returnValue.setIsTextOrUrlSetting(true);
        returnValue.setStatusCode(200);
        returnValue.setTextOrUrl("Success response");
        bool result = requestTest.processReturn(returnValue);
        assert(result == true);
        assert(requestTest.response.getStatus() == 200);
        assert(requestTest.response.getBody() == "Success response");
        assert(requestTest.response.getHeader("Content-Type") == "text/plain");
        assert(requestTest.response.getHeader("Content-Length") == "16");
        assert(requestTest.response.getHeader("Location") == "");
        toolbox::logger::StepMark::info("✓ Return 200: OK (with text)");
        countTest();
    }
    // 201 Created with text
    {
        test::RequestTest requestTest;
        config::Return returnValue;
        returnValue.setHasReturnValue(true);
        returnValue.setIsTextOrUrlSetting(true);
        returnValue.setStatusCode(201);
        returnValue.setTextOrUrl("Resource created successfully");
        bool result = requestTest.processReturn(returnValue);
        assert(result == true);
        assert(requestTest.response.getStatus() == 201);
        assert(requestTest.response.getBody() == "Resource created successfully");
        assert(requestTest.response.getHeader("Content-Type") == "text/plain");
        assert(requestTest.response.getHeader("Content-Length") == "29");
        assert(requestTest.response.getHeader("Location") == "");
        toolbox::logger::StepMark::info("✓ Return 201: CREATED (with text)");
        countTest();
    }
    // 202 Accepted with text
    {
        test::RequestTest requestTest;
        config::Return returnValue;
        returnValue.setHasReturnValue(true);
        returnValue.setIsTextOrUrlSetting(true);
        returnValue.setStatusCode(202);
        returnValue.setTextOrUrl("Request accepted for processing");
        bool result = requestTest.processReturn(returnValue);
        assert(result == true);
        assert(requestTest.response.getStatus() == 202);
        assert(requestTest.response.getBody() == "Request accepted for processing");
        assert(requestTest.response.getHeader("Content-Type") == "text/plain");
        assert(requestTest.response.getHeader("Content-Length") == "31"); 
        assert(requestTest.response.getHeader("Location") == "");
        toolbox::logger::StepMark::info("✓ Return 202: ACCEPTED (with text)");
        countTest();
    }
    // 203 Non-Authoritative Information with text
    {
        test::RequestTest requestTest;
        config::Return returnValue;
        returnValue.setHasReturnValue(true);
        returnValue.setIsTextOrUrlSetting(true);
        returnValue.setStatusCode(203);
        returnValue.setTextOrUrl("Non-authoritative information");
        bool result = requestTest.processReturn(returnValue);
        assert(result == true);
        assert(requestTest.response.getStatus() == 203);
        assert(requestTest.response.getBody() == "Non-authoritative information");
        assert(requestTest.response.getHeader("Content-Type") == "text/plain");
        assert(requestTest.response.getHeader("Content-Length") == "29");
        assert(requestTest.response.getHeader("Location") == "");
        toolbox::logger::StepMark::info("✓ Return 203: NON_AUTHORITATIVE_INFORMATION (with text)");
        countTest();
    }
    // 204 No Content with text
    {
        test::RequestTest requestTest;
        config::Return returnValue;
        returnValue.setHasReturnValue(true);
        returnValue.setIsTextOrUrlSetting(true);
        returnValue.setStatusCode(204);
        returnValue.setTextOrUrl("No content available");
        bool result = requestTest.processReturn(returnValue);
        assert(result == true);
        assert(requestTest.response.getStatus() == 204);
        assert(requestTest.response.getBody() == "");
        assert(requestTest.response.getHeader("Content-Type") == "");
        assert(requestTest.response.getHeader("Content-Length") == "");
        assert(requestTest.response.getHeader("Location") == "");
        toolbox::logger::StepMark::info("✓ Return 204: NO_CONTENT (with text)");
        countTest();
    }
    // 205 Reset Content with text
    {
        test::RequestTest requestTest;
        config::Return returnValue;
        returnValue.setHasReturnValue(true);
        returnValue.setIsTextOrUrlSetting(true);
        returnValue.setStatusCode(205);
        returnValue.setTextOrUrl("Reset content requested");
        bool result = requestTest.processReturn(returnValue);
        assert(result == true);
        assert(requestTest.response.getStatus() == 205);
        assert(requestTest.response.getBody() == "Reset content requested");
        assert(requestTest.response.getHeader("Content-Type") == "text/plain");
        assert(requestTest.response.getHeader("Content-Length") == "23");
        assert(requestTest.response.getHeader("Location") == "");
        toolbox::logger::StepMark::info("✓ Return 205: RESET_CONTENT (with text)");
        countTest();
    }
    // 206 Partial Content with text
    {
        test::RequestTest requestTest;
        config::Return returnValue;
        returnValue.setHasReturnValue(true);
        returnValue.setIsTextOrUrlSetting(true);
        returnValue.setStatusCode(206);
        returnValue.setTextOrUrl("Partial content delivered");
        bool result = requestTest.processReturn(returnValue);
        assert(result == true);
        assert(requestTest.response.getStatus() == 206);
        assert(requestTest.response.getBody() == "Partial content delivered");
        assert(requestTest.response.getHeader("Content-Type") == "text/plain");
        assert(requestTest.response.getHeader("Content-Length") == "25");
        assert(requestTest.response.getHeader("Location") == "");
        toolbox::logger::StepMark::info("✓ Return 206: PARTIAL_CONTENT (with text)");
        countTest();
    }
    // === 3xx Redirection Codes (With URL) ===
    toolbox::logger::StepMark::info("--- Testing 3xx Redirection Codes (With URL) ---");
    // 300 Multiple Choices with URL
    {
        test::RequestTest requestTest;
        config::Return returnValue;
        returnValue.setHasReturnValue(true);
        returnValue.setIsTextOrUrlSetting(true);
        returnValue.setStatusCode(300);
        returnValue.setTextOrUrl("https://example.com/choices");
        bool result = requestTest.processReturn(returnValue);
        assert(result == true);
        assert(requestTest.response.getStatus() == 300);
        assert(requestTest.response.getBody() == "https://example.com/choices");
        assert(requestTest.response.getHeader("Content-Type") == "text/plain");
        assert(requestTest.response.getHeader("Content-Length") == "27");
        assert(requestTest.response.getHeader("Location") == "");
        toolbox::logger::StepMark::info("✓ Return 300: MULTIPLE_CHOICES (with URL)");
        countTest();
    }
    // 301 Moved Permanently with URL
    {
        test::RequestTest requestTest;
        config::Return returnValue;
        returnValue.setHasReturnValue(true);
        returnValue.setIsTextOrUrlSetting(true);
        returnValue.setStatusCode(301);
        returnValue.setTextOrUrl("https://newsite.com/");
        bool result = requestTest.processReturn(returnValue);
        assert(result == true);
        assert(requestTest.response.getStatus() == 301);
        assert(requestTest.response.getHeader("Location") == "https://newsite.com/");
        assert(!requestTest.response.getBody().empty());
        assert(requestTest.response.getHeader("Content-Type") == "text/html");
        assert(requestTest.response.getHeader("Content-Length") != "");
        toolbox::logger::StepMark::info("✓ Return 301: MOVED_PERMANENTLY (with URL)");
        countTest();
    }
    // 302 Found with URL
    {
        test::RequestTest requestTest;
        config::Return returnValue;
        returnValue.setHasReturnValue(true);
        returnValue.setIsTextOrUrlSetting(true);
        returnValue.setStatusCode(302);
        returnValue.setTextOrUrl("https://temp.redirect.com/");
        bool result = requestTest.processReturn(returnValue);
        assert(result == true);
        assert(requestTest.response.getStatus() == 302);
        assert(requestTest.response.getHeader("Location") == "https://temp.redirect.com/");
        assert(!requestTest.response.getBody().empty());
        assert(requestTest.response.getHeader("Content-Type") == "text/html");
        assert(requestTest.response.getHeader("Content-Length") != "");
        toolbox::logger::StepMark::info("✓ Return 302: FOUND (with URL)");
        countTest();
    }
    // 303 See Other with URL
    {
        test::RequestTest requestTest;
        config::Return returnValue;
        returnValue.setHasReturnValue(true);
        returnValue.setIsTextOrUrlSetting(true);
        returnValue.setStatusCode(303);
        returnValue.setTextOrUrl("https://see.other.com/");
        bool result = requestTest.processReturn(returnValue);
        assert(result == true);
        assert(requestTest.response.getStatus() == 303);
        assert(requestTest.response.getHeader("Location") == "https://see.other.com/");
        assert(!requestTest.response.getBody().empty());
        assert(requestTest.response.getHeader("Content-Type") == "text/html");
        assert(requestTest.response.getHeader("Content-Length") != "");
        toolbox::logger::StepMark::info("✓ Return 303: SEE_OTHER (with URL)");
        countTest();
    }
    // 304 Not Modified with URL
    {
        test::RequestTest requestTest;
        config::Return returnValue;
        returnValue.setHasReturnValue(true);
        returnValue.setIsTextOrUrlSetting(true);
        returnValue.setStatusCode(304);
        returnValue.setTextOrUrl("https://not.modified.com/");
        bool result = requestTest.processReturn(returnValue);
        assert(result == true);
        assert(requestTest.response.getStatus() == 304);
        assert(requestTest.response.getBody() == "https://not.modified.com/");
        assert(requestTest.response.getHeader("Content-Type") == "text/plain");
        assert(requestTest.response.getHeader("Content-Length") == "25");
        assert(requestTest.response.getHeader("Location") == "");
        toolbox::logger::StepMark::info("✓ Return 304: NOT_MODIFIED (with URL)");
        countTest();
    }
    // 307 Temporary Redirect with URL
    {
        test::RequestTest requestTest;
        config::Return returnValue;
        returnValue.setHasReturnValue(true);
        returnValue.setIsTextOrUrlSetting(true);
        returnValue.setStatusCode(307);
        returnValue.setTextOrUrl("https://temp.preserve.com/");
        bool result = requestTest.processReturn(returnValue);
        assert(result == true);
        assert(requestTest.response.getStatus() == 307);
        assert(requestTest.response.getHeader("Location") == "https://temp.preserve.com/");
        assert(!requestTest.response.getBody().empty());
        assert(requestTest.response.getHeader("Content-Type") == "text/html");
        assert(requestTest.response.getHeader("Content-Length") != "");
        toolbox::logger::StepMark::info("✓ Return 307: TEMPORARY_REDIRECT (with URL)");
        countTest();
    }
    // 308 Permanent Redirect with URL
    {
        test::RequestTest requestTest;
        config::Return returnValue;
        returnValue.setHasReturnValue(true);
        returnValue.setIsTextOrUrlSetting(true);
        returnValue.setStatusCode(308);
        returnValue.setTextOrUrl("https://perm.preserve.com/");
        bool result = requestTest.processReturn(returnValue);
        assert(result == true);
        assert(requestTest.response.getStatus() == 308);
        assert(requestTest.response.getHeader("Location") == "https://perm.preserve.com/");
        assert(!requestTest.response.getBody().empty());
        assert(requestTest.response.getHeader("Content-Type") == "text/html");
        assert(requestTest.response.getHeader("Content-Length") != "");
        toolbox::logger::StepMark::info("✓ Return 308: PERMANENT_REDIRECT (with URL)");
        countTest();
    }
    // === 4xx Client Error Codes (With Text) ===
    toolbox::logger::StepMark::info("--- Testing 4xx Client Error Codes (With Text) ---");
    // 400 Bad Request with text
    {
        test::RequestTest requestTest;
        config::Return returnValue;
        returnValue.setHasReturnValue(true);
        returnValue.setIsTextOrUrlSetting(true);
        returnValue.setStatusCode(400);
        returnValue.setTextOrUrl("Bad Request: Invalid syntax");
        bool result = requestTest.processReturn(returnValue);
        assert(result == true);
        assert(requestTest.response.getStatus() == 400);
        assert(requestTest.response.getBody() == "Bad Request: Invalid syntax");
        assert(requestTest.response.getHeader("Content-Type") == "text/plain");
        assert(requestTest.response.getHeader("Content-Length") == "27");
        assert(requestTest.response.getHeader("Location") == "");
        toolbox::logger::StepMark::info("✓ Return 400: BAD_REQUEST (with text)");
        countTest();
    }
    // 401 Unauthorized with text
    {
        test::RequestTest requestTest;
        config::Return returnValue;
        returnValue.setHasReturnValue(true);
        returnValue.setIsTextOrUrlSetting(true);
        returnValue.setStatusCode(401);
        returnValue.setTextOrUrl("Authentication required");
        bool result = requestTest.processReturn(returnValue);
        assert(result == true);
        assert(requestTest.response.getStatus() == 401);
        assert(requestTest.response.getBody() == "Authentication required");
        assert(requestTest.response.getHeader("Content-Type") == "text/plain");
        assert(requestTest.response.getHeader("Content-Length") == "23");
        assert(requestTest.response.getHeader("Location") == "");
        toolbox::logger::StepMark::info("✓ Return 401: UNAUTHORIZED (with text)");
        countTest();
    }
    // 402 Payment Required with text
    {
        test::RequestTest requestTest;
        config::Return returnValue;
        returnValue.setHasReturnValue(true);
        returnValue.setIsTextOrUrlSetting(true);
        returnValue.setStatusCode(402);
        returnValue.setTextOrUrl("Payment is required for this resource");
        bool result = requestTest.processReturn(returnValue);
        assert(result == true);
        assert(requestTest.response.getStatus() == 402);
        assert(requestTest.response.getBody() == "Payment is required for this resource");
        assert(requestTest.response.getHeader("Content-Type") == "text/plain");
        assert(requestTest.response.getHeader("Content-Length") == "37");
        assert(requestTest.response.getHeader("Location") == "");
        toolbox::logger::StepMark::info("✓ Return 402: PAYMENT_REQUIRED (with text)");
        countTest();
    }
    // 403 Forbidden with text
    {
        test::RequestTest requestTest;
        config::Return returnValue;
        returnValue.setHasReturnValue(true);
        returnValue.setIsTextOrUrlSetting(true);
        returnValue.setStatusCode(403);
        returnValue.setTextOrUrl("Access forbidden to this resource");
        bool result = requestTest.processReturn(returnValue);
        assert(result == true);
        assert(requestTest.response.getStatus() == 403);
        assert(requestTest.response.getBody() == "Access forbidden to this resource");
        assert(requestTest.response.getHeader("Content-Type") == "text/plain");
        assert(requestTest.response.getHeader("Content-Length") == "33");
        assert(requestTest.response.getHeader("Location") == "");
        toolbox::logger::StepMark::info("✓ Return 403: FORBIDDEN (with text)");
        countTest();
    }
    // 404 Not Found with text
    {
        test::RequestTest requestTest;
        config::Return returnValue;
        returnValue.setHasReturnValue(true);
        returnValue.setIsTextOrUrlSetting(true);
        returnValue.setStatusCode(404);
        returnValue.setTextOrUrl("The requested resource was not found");
        bool result = requestTest.processReturn(returnValue);
        assert(result == true);
        assert(requestTest.response.getStatus() == 404);
        assert(requestTest.response.getBody() == "The requested resource was not found");
        assert(requestTest.response.getHeader("Content-Type") == "text/plain");
        assert(requestTest.response.getHeader("Content-Length") == "36");
        assert(requestTest.response.getHeader("Location") == "");
        toolbox::logger::StepMark::info("✓ Return 404: NOT_FOUND (with text)");
        countTest();
    }
    // 405 Method Not Allowed with text
    {
        test::RequestTest requestTest;
        config::Return returnValue;
        returnValue.setHasReturnValue(true);
        returnValue.setIsTextOrUrlSetting(true);
        returnValue.setStatusCode(405);
        returnValue.setTextOrUrl("Method not allowed for this resource");
        bool result = requestTest.processReturn(returnValue);
        assert(result == true);
        assert(requestTest.response.getStatus() == 405);
        assert(requestTest.response.getBody() == "Method not allowed for this resource");
        assert(requestTest.response.getHeader("Content-Type") == "text/plain");
        assert(requestTest.response.getHeader("Content-Length") == "36");
        assert(requestTest.response.getHeader("Location") == "");
        toolbox::logger::StepMark::info("✓ Return 405: METHOD_NOT_ALLOWED (with text)");
        countTest();
    }
    // 406 Not Acceptable with text
    {
        test::RequestTest requestTest;
        config::Return returnValue;
        returnValue.setHasReturnValue(true);
        returnValue.setIsTextOrUrlSetting(true);
        returnValue.setStatusCode(406);
        returnValue.setTextOrUrl("Not acceptable content type");
        bool result = requestTest.processReturn(returnValue);
        assert(result == true);
        assert(requestTest.response.getStatus() == 406);
        assert(requestTest.response.getBody() == "Not acceptable content type");
        assert(requestTest.response.getHeader("Content-Type") == "text/plain");
        assert(requestTest.response.getHeader("Content-Length") == "27");
        assert(requestTest.response.getHeader("Location") == "");
        toolbox::logger::StepMark::info("✓ Return 406: NOT_ACCEPTABLE (with text)");
        countTest();
    }
    // 407 Proxy Authentication Required with text
    {
        test::RequestTest requestTest;
        config::Return returnValue;
        returnValue.setHasReturnValue(true);
        returnValue.setIsTextOrUrlSetting(true);
        returnValue.setStatusCode(407);
        returnValue.setTextOrUrl("Proxy authentication required");
        bool result = requestTest.processReturn(returnValue);
        assert(result == true);
        assert(requestTest.response.getStatus() == 407);
        assert(requestTest.response.getBody() == "Proxy authentication required");
        assert(requestTest.response.getHeader("Content-Type") == "text/plain");
        assert(requestTest.response.getHeader("Content-Length") == "29");
        assert(requestTest.response.getHeader("Location") == "");
        toolbox::logger::StepMark::info("✓ Return 407: PROXY_AUTHENTICATION_REQUIRED (with text)");
        countTest();
    }
    // 408 Request Timeout with text
    {
        test::RequestTest requestTest;
        config::Return returnValue;
        returnValue.setHasReturnValue(true);
        returnValue.setIsTextOrUrlSetting(true);
        returnValue.setStatusCode(408);
        returnValue.setTextOrUrl("Request timeout");
        bool result = requestTest.processReturn(returnValue);
        assert(result == true);
        assert(requestTest.response.getStatus() == 408);
        assert(requestTest.response.getBody() == "Request timeout");
        assert(requestTest.response.getHeader("Content-Type") == "text/plain");
        assert(requestTest.response.getHeader("Content-Length") == "15");
        assert(requestTest.response.getHeader("Location") == "");
        toolbox::logger::StepMark::info("✓ Return 408: REQUEST_TIMEOUT (with text)");
        countTest();
    }
    // 409 Conflict with text
    {
        test::RequestTest requestTest;
        config::Return returnValue;
        returnValue.setHasReturnValue(true);
        returnValue.setIsTextOrUrlSetting(true);
        returnValue.setStatusCode(409);
        returnValue.setTextOrUrl("Conflict with current state");
        bool result = requestTest.processReturn(returnValue);
        assert(result == true);
        assert(requestTest.response.getStatus() == 409);
        assert(requestTest.response.getBody() == "Conflict with current state");
        assert(requestTest.response.getHeader("Content-Type") == "text/plain");
        assert(requestTest.response.getHeader("Content-Length") == "27");
        assert(requestTest.response.getHeader("Location") == "");
        toolbox::logger::StepMark::info("✓ Return 409: CONFLICT (with text)");
        countTest();
    }
    // 410 Gone with text
    {
        test::RequestTest requestTest;
        config::Return returnValue;
        returnValue.setHasReturnValue(true);
        returnValue.setIsTextOrUrlSetting(true);
        returnValue.setStatusCode(410);
        returnValue.setTextOrUrl("This resource is permanently gone");
        bool result = requestTest.processReturn(returnValue);
        assert(result == true);
        assert(requestTest.response.getStatus() == 410);
        assert(requestTest.response.getBody() == "This resource is permanently gone");
        assert(requestTest.response.getHeader("Content-Type") == "text/plain");
        assert(requestTest.response.getHeader("Content-Length") == "33");
        assert(requestTest.response.getHeader("Location") == "");
        toolbox::logger::StepMark::info("✓ Return 410: GONE (with text)");
        countTest();
    }
    // 411 Length Required with text
    {
        test::RequestTest requestTest;
        config::Return returnValue;
        returnValue.setHasReturnValue(true);
        returnValue.setIsTextOrUrlSetting(true);
        returnValue.setStatusCode(411);
        returnValue.setTextOrUrl("Content-Length header required");
        bool result = requestTest.processReturn(returnValue);
        assert(result == true);
        assert(requestTest.response.getStatus() == 411);
        assert(requestTest.response.getBody() == "Content-Length header required");
        assert(requestTest.response.getHeader("Content-Type") == "text/plain");
        assert(requestTest.response.getHeader("Content-Length") == "30");
        assert(requestTest.response.getHeader("Location") == "");
        toolbox::logger::StepMark::info("✓ Return 411: LENGTH_REQUIRED (with text)");
        countTest();
    }
    // 412 Precondition Failed with text
    {
        test::RequestTest requestTest;
        config::Return returnValue;
        returnValue.setHasReturnValue(true);
        returnValue.setIsTextOrUrlSetting(true);
        returnValue.setStatusCode(412);
        returnValue.setTextOrUrl("Precondition failed");
        bool result = requestTest.processReturn(returnValue);
        assert(result == true);
        assert(requestTest.response.getStatus() == 412);
        assert(requestTest.response.getBody() == "Precondition failed");
        assert(requestTest.response.getHeader("Content-Type") == "text/plain");
        assert(requestTest.response.getHeader("Content-Length") == "19");
        assert(requestTest.response.getHeader("Location") == "");
        toolbox::logger::StepMark::info("✓ Return 412: PRECONDITION_FAILED (with text)");
        countTest();
    }
    // 413 Payload Too Large with text
    {
        test::RequestTest requestTest;
        config::Return returnValue;
        returnValue.setHasReturnValue(true);
        returnValue.setIsTextOrUrlSetting(true);
        returnValue.setStatusCode(413);
        returnValue.setTextOrUrl("Request payload too large");
        bool result = requestTest.processReturn(returnValue);
        assert(result == true);
        assert(requestTest.response.getStatus() == 413);
        assert(requestTest.response.getBody() == "Request payload too large");
        assert(requestTest.response.getHeader("Content-Type") == "text/plain");
        assert(requestTest.response.getHeader("Content-Length") == "25");
        assert(requestTest.response.getHeader("Location") == "");
        toolbox::logger::StepMark::info("✓ Return 413: PAYLOAD_TOO_LARGE (with text)");
        countTest();
    }
    // 414 URI Too Long with text
    {
        test::RequestTest requestTest;
        config::Return returnValue;
        returnValue.setHasReturnValue(true);
        returnValue.setIsTextOrUrlSetting(true);
        returnValue.setStatusCode(414);
        returnValue.setTextOrUrl("Request URI too long");
        bool result = requestTest.processReturn(returnValue);
        assert(result == true);
        assert(requestTest.response.getStatus() == 414);
        assert(requestTest.response.getBody() == "Request URI too long");
        assert(requestTest.response.getHeader("Content-Type") == "text/plain");
        assert(requestTest.response.getHeader("Content-Length") == "20");
        assert(requestTest.response.getHeader("Location") == "");
        toolbox::logger::StepMark::info("✓ Return 414: URI_TOO_LONG (with text)");
        countTest();
    }
    // 415 Unsupported Media Type with text
    {
        test::RequestTest requestTest;
        config::Return returnValue;
        returnValue.setHasReturnValue(true);
        returnValue.setIsTextOrUrlSetting(true);
        returnValue.setStatusCode(415);
        returnValue.setTextOrUrl("Unsupported media type");
        bool result = requestTest.processReturn(returnValue);
        assert(result == true);
        assert(requestTest.response.getStatus() == 415);
        assert(requestTest.response.getBody() == "Unsupported media type");
        assert(requestTest.response.getHeader("Content-Type") == "text/plain");
        assert(requestTest.response.getHeader("Content-Length") == "22");
        assert(requestTest.response.getHeader("Location") == "");
        toolbox::logger::StepMark::info("✓ Return 415: UNSUPPORTED_MEDIA_TYPE (with text)");
        countTest();
    }
    // 416 Range Not Satisfiable with text
    {
        test::RequestTest requestTest;
        config::Return returnValue;
        returnValue.setHasReturnValue(true);
        returnValue.setIsTextOrUrlSetting(true);
        returnValue.setStatusCode(416);
        returnValue.setTextOrUrl("Range not satisfiable");
        bool result = requestTest.processReturn(returnValue);
        assert(result == true);
        assert(requestTest.response.getStatus() == 416);
        assert(requestTest.response.getBody() == "Range not satisfiable");
        assert(requestTest.response.getHeader("Content-Type") == "text/plain");
        assert(requestTest.response.getHeader("Content-Length") == "21");
        assert(requestTest.response.getHeader("Location") == "");
        toolbox::logger::StepMark::info("✓ Return 416: RANGE_NOT_SATISFIABLE (with text)");
        countTest();
    }
    // 417 Expectation Failed with text
    {
        test::RequestTest requestTest;
        config::Return returnValue;
        returnValue.setHasReturnValue(true);
        returnValue.setIsTextOrUrlSetting(true);
        returnValue.setStatusCode(417);
        returnValue.setTextOrUrl("Expectation failed");
        bool result = requestTest.processReturn(returnValue);
        assert(result == true);
        assert(requestTest.response.getStatus() == 417);
        assert(requestTest.response.getBody() == "Expectation failed");
        assert(requestTest.response.getHeader("Content-Type") == "text/plain");
        assert(requestTest.response.getHeader("Content-Length") == "18");
        assert(requestTest.response.getHeader("Location") == "");
        toolbox::logger::StepMark::info("✓ Return 417: EXPECTATION_FAILED (with text)");
        countTest();
    }
    // 418 I'm a teapot with text
    {
        test::RequestTest requestTest;
        config::Return returnValue;
        returnValue.setHasReturnValue(true);
        returnValue.setIsTextOrUrlSetting(true);
        returnValue.setStatusCode(418);
        returnValue.setTextOrUrl("I'm a teapot, not a coffee maker");
        bool result = requestTest.processReturn(returnValue);
        assert(result == true);
        assert(requestTest.response.getStatus() == 418);
        assert(requestTest.response.getBody() == "I'm a teapot, not a coffee maker");
        assert(requestTest.response.getHeader("Content-Type") == "text/plain");
        assert(requestTest.response.getHeader("Content-Length") == "32");
        assert(requestTest.response.getHeader("Location") == "");
        toolbox::logger::StepMark::info("✓ Return 418: IM_A_TEAPOT (with text)");
        countTest();
    }
    // 421 Misdirected Request with text
    {
        test::RequestTest requestTest;
        config::Return returnValue;
        returnValue.setHasReturnValue(true);
        returnValue.setIsTextOrUrlSetting(true);
        returnValue.setStatusCode(421);
        returnValue.setTextOrUrl("Misdirected request");
        bool result = requestTest.processReturn(returnValue);
        assert(result == true);
        assert(requestTest.response.getStatus() == 421);
        assert(requestTest.response.getBody() == "Misdirected request");
        assert(requestTest.response.getHeader("Content-Type") == "text/plain");
        assert(requestTest.response.getHeader("Content-Length") == "19");
        assert(requestTest.response.getHeader("Location") == "");
        toolbox::logger::StepMark::info("✓ Return 421: MISDIRECTED_REQUEST (with text)");
        countTest();
    }
    // 422 Unprocessable Entity with text
    {
        test::RequestTest requestTest;
        config::Return returnValue;
        returnValue.setHasReturnValue(true);
        returnValue.setIsTextOrUrlSetting(true);
        returnValue.setStatusCode(422);
        returnValue.setTextOrUrl("Unprocessable entity");
        bool result = requestTest.processReturn(returnValue);
        assert(result == true);
        assert(requestTest.response.getStatus() == 422);
        assert(requestTest.response.getBody() == "Unprocessable entity");
        assert(requestTest.response.getHeader("Content-Type") == "text/plain");
        assert(requestTest.response.getHeader("Content-Length") == "20");
        assert(requestTest.response.getHeader("Location") == "");
        toolbox::logger::StepMark::info("✓ Return 422: UNPROCESSABLE_ENTITY (with text)");
        countTest();
    }
    // 426 Upgrade Required with text
    {
        test::RequestTest requestTest;
        config::Return returnValue;
        returnValue.setHasReturnValue(true);
        returnValue.setIsTextOrUrlSetting(true);
        returnValue.setStatusCode(426);
        returnValue.setTextOrUrl("Upgrade required to access this resource");
        bool result = requestTest.processReturn(returnValue);
        assert(result == true);
        assert(requestTest.response.getStatus() == 426);
        assert(requestTest.response.getBody() == "Upgrade required to access this resource");
        assert(requestTest.response.getHeader("Content-Type") == "text/plain");
        assert(requestTest.response.getHeader("Content-Length") == "40");
        assert(requestTest.response.getHeader("Location") == "");
        toolbox::logger::StepMark::info("✓ Return 426: UPGRADE_REQUIRED (with text)");
        countTest();
    }
    toolbox::logger::StepMark::info("--- Testing 5xx Server Error Codes (With Text) ---");
    // 500 Internal Server Error with text
    {
        test::RequestTest requestTest;
        config::Return returnValue;
        returnValue.setHasReturnValue(true);
        returnValue.setIsTextOrUrlSetting(true);
        returnValue.setStatusCode(500);
        returnValue.setTextOrUrl("Internal server error occurred");
        bool result = requestTest.processReturn(returnValue);
        assert(result == true);
        assert(requestTest.response.getStatus() == 500);
        assert(requestTest.response.getBody() == "Internal server error occurred");
        assert(requestTest.response.getHeader("Content-Type") == "text/plain");
        assert(requestTest.response.getHeader("Content-Length") == "30");
        assert(requestTest.response.getHeader("Location") == "");
        toolbox::logger::StepMark::info("✓ Return 500: INTERNAL_SERVER_ERROR (with text)");
        countTest();
    }
    // 501 Not Implemented with text
    {
        test::RequestTest requestTest;
        config::Return returnValue;
        returnValue.setHasReturnValue(true);
        returnValue.setIsTextOrUrlSetting(true);
        returnValue.setStatusCode(501);
        returnValue.setTextOrUrl("Feature not implemented");
        bool result = requestTest.processReturn(returnValue);
        assert(result == true);
        assert(requestTest.response.getStatus() == 501);
        assert(requestTest.response.getBody() == "Feature not implemented");
        assert(requestTest.response.getHeader("Content-Type") == "text/plain");
        assert(requestTest.response.getHeader("Content-Length") == "23");
        assert(requestTest.response.getHeader("Location") == "");
        toolbox::logger::StepMark::info("✓ Return 501: NOT_IMPLEMENTED (with text)");
        countTest();
    }
    // 502 Bad Gateway with text
    {
        test::RequestTest requestTest;
        config::Return returnValue;
        returnValue.setHasReturnValue(true);
        returnValue.setIsTextOrUrlSetting(true);
        returnValue.setStatusCode(502);
        returnValue.setTextOrUrl("Bad gateway response");
        bool result = requestTest.processReturn(returnValue);
        assert(result == true);
        assert(requestTest.response.getStatus() == 502);
        assert(requestTest.response.getBody() == "Bad gateway response");
        assert(requestTest.response.getHeader("Content-Type") == "text/plain");
        assert(requestTest.response.getHeader("Content-Length") == "20");
        assert(requestTest.response.getHeader("Location") == "");
        toolbox::logger::StepMark::info("✓ Return 502: BAD_GATEWAY (with text)");
        countTest();
    }
    // 503 Service Unavailable with text
    {
        test::RequestTest requestTest;
        config::Return returnValue;
        returnValue.setHasReturnValue(true);
        returnValue.setIsTextOrUrlSetting(true);
        returnValue.setStatusCode(503);
        returnValue.setTextOrUrl("Service temporarily unavailable");
        bool result = requestTest.processReturn(returnValue);
        assert(result == true);
        assert(requestTest.response.getStatus() == 503);
        assert(requestTest.response.getBody() == "Service temporarily unavailable");
        assert(requestTest.response.getHeader("Content-Type") == "text/plain");
        assert(requestTest.response.getHeader("Content-Length") == "31");
        assert(requestTest.response.getHeader("Location") == "");
        toolbox::logger::StepMark::info("✓ Return 503: SERVICE_UNAVAILABLE (with text)");
        countTest();
    }
    // 504 Gateway Timeout with text
    {
        test::RequestTest requestTest;
        config::Return returnValue;
        returnValue.setHasReturnValue(true);
        returnValue.setIsTextOrUrlSetting(true);
        returnValue.setStatusCode(504);
        returnValue.setTextOrUrl("Gateway timeout occurred");
        bool result = requestTest.processReturn(returnValue);
        assert(result == true);
        assert(requestTest.response.getStatus() == 504);
        assert(requestTest.response.getBody() == "Gateway timeout occurred");
        assert(requestTest.response.getHeader("Content-Type") == "text/plain");
        assert(requestTest.response.getHeader("Content-Length") == "24");
        assert(requestTest.response.getHeader("Location") == "");
        toolbox::logger::StepMark::info("✓ Return 504: GATEWAY_TIMEOUT (with text)");
        countTest();
    }
    // 505 HTTP Version Not Supported with text
    {
        test::RequestTest requestTest;
        config::Return returnValue;
        returnValue.setHasReturnValue(true);
        returnValue.setIsTextOrUrlSetting(true);
        returnValue.setStatusCode(505);
        returnValue.setTextOrUrl("HTTP version not supported");
        bool result = requestTest.processReturn(returnValue);
        assert(result == true);
        assert(requestTest.response.getStatus() == 505);
        assert(requestTest.response.getBody() == "HTTP version not supported");
        assert(requestTest.response.getHeader("Content-Type") == "text/plain");
        assert(requestTest.response.getHeader("Content-Length") == "26");
        assert(requestTest.response.getHeader("Location") == "");
        toolbox::logger::StepMark::info("✓ Return 505: HTTP_VERSION_NOT_SUPPORTED (with text)");
        countTest();
    }
}

toolbox::SharedPtr<config::ServerConfig> createBasicTestServer() {
    toolbox::SharedPtr<config::ServerConfig> testServer(new config::ServerConfig());
    // Location /api
    toolbox::SharedPtr<config::LocationConfig> loc1(new config::LocationConfig());
    loc1->setPath("/api");
    loc1->setRoot("/var/www/api");
    testServer->addLocation(loc1);
    // Location /static
    toolbox::SharedPtr<config::LocationConfig> loc2(new config::LocationConfig());
    loc2->setPath("/static");
    loc2->setRoot("/var/www/static");
    testServer->addLocation(loc2);
    // Location /api/v1
    toolbox::SharedPtr<config::LocationConfig> loc3(new config::LocationConfig());
    loc3->setPath("/api/v1");
    loc3->setRoot("/var/www/api/v1");
    testServer->addLocation(loc3);
    return testServer;
}

toolbox::SharedPtr<config::ServerConfig> createExistingRootTestServer() {
    toolbox::SharedPtr<config::ServerConfig> testServer(new config::ServerConfig());
    // Location /api
    toolbox::SharedPtr<config::LocationConfig> loc1(new config::LocationConfig());
    loc1->setPath("/api");
    loc1->setRoot("/var/www/api");
    testServer->addLocation(loc1);
    // Location /static
    toolbox::SharedPtr<config::LocationConfig> loc2(new config::LocationConfig());
    loc2->setPath("/static");
    loc2->setRoot("/var/www/static");
    testServer->addLocation(loc2);
    // Location /api/v1
    toolbox::SharedPtr<config::LocationConfig> loc3(new config::LocationConfig());
    loc3->setPath("/api/v1");
    loc3->setRoot("/var/www/api/v1");
    testServer->addLocation(loc3);
    // Location /
    toolbox::SharedPtr<config::LocationConfig> rootLoc(new config::LocationConfig());
    rootLoc->setPath("/");
    rootLoc->setRoot("/var/www/html");
    testServer->addLocation(rootLoc);
    return testServer;
}

// location /abc {
//     root /var/www/abc;
//     location /abc/def {
//         root /var/www/nest;
//     }
// }
// location /abc/def {
//     root /var/www/not_nest;
// }
toolbox::SharedPtr<config::ServerConfig> createNestedLocationTestServer() {
    toolbox::SharedPtr<config::ServerConfig> testServer(new config::ServerConfig());
    toolbox::SharedPtr<config::LocationConfig> parentLoc(new config::LocationConfig());
    parentLoc->setPath("/abc");
    parentLoc->setRoot("/var/www/abc");
    parentLoc->setServerParent(testServer.get());
    toolbox::SharedPtr<config::LocationConfig> nestedLoc(new config::LocationConfig());
    nestedLoc->setPath("/abc/def");
    nestedLoc->setRoot("/var/www/nest");
    nestedLoc->setLocationParent(parentLoc.get());
    parentLoc->addLocation(nestedLoc);
    testServer->addLocation(parentLoc);
    toolbox::SharedPtr<config::LocationConfig> sameLevelLoc(new config::LocationConfig());
    sameLevelLoc->setPath("/abc/def");
    sameLevelLoc->setRoot("/var/www/not_nest");
    sameLevelLoc->setServerParent(testServer.get());
    testServer->addLocation(sameLevelLoc);
    return testServer;
}

// location /abc {
//     root /var/www/abc;
//     location /abc/def {
//         root /var/www/abc_def_nested;
//         location /abc/def/ghi {
//             root /var/www/nested_deep;
//         }
//     }
//     location /abc/xyz {
//         root /var/www/abc_xyz;
//     }
// }
// location /abc/def {
//     root /var/www/not_nested;
// }
// location /abc/def/ghi {
//     root /var/www/same_level_deep;
// }
toolbox::SharedPtr<config::ServerConfig> createComplexNestedLocationTestServer() {
    toolbox::SharedPtr<config::ServerConfig> testServer(new config::ServerConfig());
    toolbox::SharedPtr<config::LocationConfig> rootLoc(new config::LocationConfig());
    rootLoc->setPath("/abc");
    rootLoc->setRoot("/var/www/abc");
    rootLoc->setServerParent(testServer.get());
    toolbox::SharedPtr<config::LocationConfig> level1Loc(new config::LocationConfig());
    level1Loc->setPath("/abc/def");
    level1Loc->setRoot("/var/www/abc_def_nested");
    level1Loc->setLocationParent(rootLoc.get());
    toolbox::SharedPtr<config::LocationConfig> level2Loc(new config::LocationConfig());
    level2Loc->setPath("/abc/def/ghi");
    level2Loc->setRoot("/var/www/nested_deep");
    level2Loc->setLocationParent(level1Loc.get());
    level1Loc->addLocation(level2Loc);
    rootLoc->addLocation(level1Loc);
    testServer->addLocation(rootLoc);
    toolbox::SharedPtr<config::LocationConfig> sameLevelLoc(new config::LocationConfig());
    sameLevelLoc->setPath("/abc/def");
    sameLevelLoc->setRoot("/var/www/not_nested");
    sameLevelLoc->setServerParent(testServer.get());
    testServer->addLocation(sameLevelLoc);
    toolbox::SharedPtr<config::LocationConfig> sameLevelDeepLoc(new config::LocationConfig());
    sameLevelDeepLoc->setPath("/abc/def/ghi");
    sameLevelDeepLoc->setRoot("/var/www/same_level_deep");
    sameLevelDeepLoc->setServerParent(testServer.get());
    testServer->addLocation(sameLevelDeepLoc);
    toolbox::SharedPtr<config::LocationConfig> siblingLoc(new config::LocationConfig());
    siblingLoc->setPath("/abc/xyz");
    siblingLoc->setRoot("/var/www/abc_xyz");
    siblingLoc->setLocationParent(rootLoc.get());
    rootLoc->addLocation(siblingLoc);
    return testServer;
}

// 3. Location
void testLocationSelection() {
    toolbox::logger::StepMark::info("=== Testing Location Selection ===");
    // location path / is not existing
    {
        toolbox::logger::StepMark::info("--- Testing without root location ---");
        {
            toolbox::SharedPtr<config::ServerConfig> testServer = createBasicTestServer();
            test::RequestTest requestTest;
            setRequestPath(requestTest, "/api");
            bool result = requestTest.selectLocation(testServer);
            assert(result == true);
            assert(requestTest.locationConfig.getPath() == "/api");
            assert(requestTest.locationConfig.getRoot() == "/var/www/api");
            toolbox::logger::StepMark::info("✓ Location selection: exact match (/api)");
            countTest();
        }
        {
            toolbox::SharedPtr<config::ServerConfig> testServer = createBasicTestServer();
            test::RequestTest requestTest;
            setRequestPath(requestTest, "/api/users");
            bool result = requestTest.selectLocation(testServer);
            assert(result == true);
            assert(requestTest.locationConfig.getPath() == "/api");
            toolbox::logger::StepMark::info("✓ Location selection: prefix match (/api/users → /api)");
            countTest();
        }
        {
            toolbox::SharedPtr<config::ServerConfig> testServer = createBasicTestServer();
            test::RequestTest requestTest;
            setRequestPath(requestTest, "/api/v1/users");
            bool result = requestTest.selectLocation(testServer);
            assert(result == true);
            assert(requestTest.locationConfig.getPath() == "/api/v1");
            toolbox::logger::StepMark::info("✓ Location selection: deeper match (/api/v1/users → /api/v1)");
            countTest();
        }
        {
            toolbox::SharedPtr<config::ServerConfig> testServer = createBasicTestServer();
            test::RequestTest requestTest;
            setRequestPath(requestTest, "/static/css/style.css");
            bool result = requestTest.selectLocation(testServer);
            assert(result == true);
            assert(requestTest.locationConfig.getPath() == "/static");
            toolbox::logger::StepMark::info("✓ Location selection: static file (/static/css/style.css → /static)");
            countTest();
        }
        {
            toolbox::SharedPtr<config::ServerConfig> testServer = createBasicTestServer();
            test::RequestTest requestTest;
            setRequestPath(requestTest, "/");
            bool result = requestTest.selectLocation(testServer);
            assert(result == true);
            assert(requestTest.locationConfig.getPath() == "/");
            toolbox::logger::StepMark::info("✓ Location selection: root path without root location");
            countTest();
        }
        {
            toolbox::SharedPtr<config::ServerConfig> testServer = createBasicTestServer();
            test::RequestTest requestTest;
            setRequestPath(requestTest, "/nonexistent");
            bool result = requestTest.selectLocation(testServer);
            assert(result == false);
            toolbox::logger::StepMark::info("✓ Location selection: no match (/nonexistent)");
            countTest();
        }
    }
    // location path / is existing
    {
        toolbox::logger::StepMark::info("--- Testing with root location ---");
        {
            toolbox::SharedPtr<config::ServerConfig> testServerWithRoot = createExistingRootTestServer();
            test::RequestTest requestTest;
            setRequestPath(requestTest, "/");
            bool result = requestTest.selectLocation(testServerWithRoot);
            assert(result == true);
            assert(requestTest.locationConfig.getPath() == "/");
            assert(requestTest.locationConfig.getRoot() == "/var/www/html");
            toolbox::logger::StepMark::info("✓ Location selection: root path with root location");
            countTest();
        }
        {
            toolbox::SharedPtr<config::ServerConfig> testServerWithRoot = createExistingRootTestServer();
            test::RequestTest requestTest;
            setRequestPath(requestTest, "/nonexistent");
            bool result = requestTest.selectLocation(testServerWithRoot);
            assert(result == true);
            assert(requestTest.locationConfig.getPath() == "/");
            toolbox::logger::StepMark::info("✓ Location selection: fallback to root location");
            countTest();
        }
    }
    {
        toolbox::logger::StepMark::info("--- Testing Basic Nested vs Same Level ---");
        {
            toolbox::SharedPtr<config::ServerConfig> testServer = createNestedLocationTestServer();
            test::RequestTest requestTest;
            setRequestPath(requestTest, "/abc/def");
            bool result = requestTest.selectLocation(testServer);
            assert(result == true);
            assert(requestTest.locationConfig.getPath() == "/abc/def");
            std::string selectedRoot = requestTest.locationConfig.getRoot();
            toolbox::logger::StepMark::info("Selected root for /abc/def: " + selectedRoot);
            if (selectedRoot == "/var/www/not_nest") {
                toolbox::logger::StepMark::info("✓ Same level location has priority over nested location");
            } else if (selectedRoot == "/var/www/nest") {
                toolbox::logger::StepMark::info("✓ Nested location has priority over same level location");
                assert(false);
            }
            countTest();
        }
        {
            toolbox::SharedPtr<config::ServerConfig> testServer = createNestedLocationTestServer();
            test::RequestTest requestTest;
            setRequestPath(requestTest, "/abc");
            bool result = requestTest.selectLocation(testServer);
            assert(result == true);
            assert(requestTest.locationConfig.getPath() == "/abc");
            assert(requestTest.locationConfig.getRoot() == "/var/www/abc");
            toolbox::logger::StepMark::info("✓ Parent location direct match (/abc → /var/www/abc)");
            countTest();
        }
        {
            toolbox::SharedPtr<config::ServerConfig> testServer = createNestedLocationTestServer();
            test::RequestTest requestTest;
            setRequestPath(requestTest, "/abc/def/file.txt");
            bool result = requestTest.selectLocation(testServer);
            assert(result == true);
            assert(requestTest.locationConfig.getPath() == "/abc/def");
            std::string selectedRoot = requestTest.locationConfig.getRoot();
            toolbox::logger::StepMark::info("Selected root for /abc/def/file.txt: " + selectedRoot);
            toolbox::logger::StepMark::info("✓ Prefix match with /abc/def location");
            countTest();
        }
        {
            toolbox::SharedPtr<config::ServerConfig> testServer = createNestedLocationTestServer();
            test::RequestTest requestTest;
            setRequestPath(requestTest, "/abc/other");
            bool result = requestTest.selectLocation(testServer);
            assert(result == true);
            assert(requestTest.locationConfig.getPath() == "/abc");
            assert(requestTest.locationConfig.getRoot() == "/var/www/abc");
            toolbox::logger::StepMark::info("✓ Fallback to parent location (/abc/other → /abc)");
            countTest();
        }
    }
    {
        toolbox::logger::StepMark::info("--- Testing Complex Nested Structure ---");
        {
            toolbox::SharedPtr<config::ServerConfig> testServer = createComplexNestedLocationTestServer();
            test::RequestTest requestTest;
            setRequestPath(requestTest, "/abc/def/ghi");
            bool result = requestTest.selectLocation(testServer);
            assert(result == true);
            assert(requestTest.locationConfig.getPath() == "/abc/def/ghi");
            std::string selectedRoot = requestTest.locationConfig.getRoot();
            toolbox::logger::StepMark::info("Selected root for /abc/def/ghi: " + selectedRoot);
            if (selectedRoot == "/var/www/nested_deep") {
                toolbox::logger::StepMark::info("✓ Deeply nested location selected");
                assert(false);
            } else if (selectedRoot == "/var/www/same_level_deep") {
                toolbox::logger::StepMark::info("✓ Same level deep location selected");
            }
            countTest();
        }
        {
            toolbox::SharedPtr<config::ServerConfig> testServer = createComplexNestedLocationTestServer();
            test::RequestTest requestTest;
            setRequestPath(requestTest, "/abc/def");
            bool result = requestTest.selectLocation(testServer);
            assert(result == true);
            assert(requestTest.locationConfig.getPath() == "/abc/def");
            std::string selectedRoot = requestTest.locationConfig.getRoot();
            toolbox::logger::StepMark::info("Selected root for /abc/def: " + selectedRoot);
            if (selectedRoot == "/var/www/abc_def_nested") {
                toolbox::logger::StepMark::info("✓ Nested location selected");
            } else if (selectedRoot == "/var/www/not_nested") {
                toolbox::logger::StepMark::info("✓ Same level location selected");
            }
            countTest();
        }
        {
            toolbox::SharedPtr<config::ServerConfig> testServer = createComplexNestedLocationTestServer();
            test::RequestTest requestTest;
            setRequestPath(requestTest, "/abc/xyz");
            bool result = requestTest.selectLocation(testServer);
            assert(result == true);
            assert(requestTest.locationConfig.getPath() == "/abc/xyz");
            assert(requestTest.locationConfig.getRoot() == "/var/www/abc_xyz");
            toolbox::logger::StepMark::info("✓ Sibling nested location (/abc/xyz → abc_xyz)");
            countTest();
        }
    }
}

// 4. 統合テスト
void testFetchConfigIntegration() {
    toolbox::logger::StepMark::info("=== Testing fetchConfig Integration ===");
    config::Config::setHttpConfig(createTestHttpConfig());
    // Normal
    {
        test::RequestTest requestTest;
        requestTest.setClient("127.0.0.1", 8080);
        setHostHeader(requestTest, "server1.com");
        setRequestPath(requestTest, "/api");
        requestTest.fetchConfig();
        assert(requestTest.response.getStatus() != 500);
        assert(requestTest.locationConfig.getPath() == "/api");
        toolbox::logger::StepMark::info("✓ fetchConfig integration: normal request");
        countTest();
    }
    // Return
    {
        test::RequestTest requestTest;
        requestTest.setClient("127.0.0.1", 8081);
        setHostHeader(requestTest, "server2.com");
        setRequestPath(requestTest, "/anything");
        requestTest.fetchConfig();
        assert(requestTest.response.getStatus() == 301);
        assert(requestTest.response.getHeader("Location") == "https://newsite.com/");
        toolbox::logger::StepMark::info("✓ fetchConfig integration: return server");
        countTest();
    }
    // Server not found
    {
        test::RequestTest requestTest;
        requestTest.setClient("127.0.0.1", 9999);
        setHostHeader(requestTest, "unknown.com");
        setRequestPath(requestTest, "/");
        requestTest.fetchConfig();
        assert(requestTest.response.getStatus() == 500);
        toolbox::logger::StepMark::info("✓ fetchConfig integration: server not found");
        countTest();
    }
}


void fetchConfigTest() {
    toolbox::logger::StepMark::info("Starting fetchConfig Test...");
    testServerSelection();
    testReturnProcessing();
    testLocationSelection();
    testFetchConfigIntegration();
    std::cout << "fetchConfig testcase " << passedTests
                << " / " << totalTests << std::endl;
}

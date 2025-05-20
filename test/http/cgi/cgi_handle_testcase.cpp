#include <iostream>
#include <string>
#include <fstream>
#include <sys/stat.h>
#include <cstdio>
#include <ctime> 
#include <vector>

#include "../../../toolbox/stepmark.hpp"
#include "../../../src/config/config_parser.hpp"
#include "../../../src/http/cgi/cgi_handler.hpp"
#include "../../../src/http/request/http_request.hpp"
#include "../../../src/http/response/response.hpp"
#include "cgi_handle_test.hpp"

bool createTestScript(const std::string& filename, const std::string& content, bool executable) {
    std::ofstream file(filename.c_str());
    if (!file.is_open()) {
        return false;
    }
    file << content;
    file.close();
    if (executable) {
        if (chmod(filename.c_str(), S_IRWXU) != 0) {
            return false;
        }
    } else {
        if (chmod(filename.c_str(), S_IRUSR | S_IWUSR) != 0) {
            return false;
        }
    }
    return true;
}

bool runTest(CgiHandleTest& test) {
    toolbox::logger::StepMark::info(test.name + "の実行 -------------------------");
    if (!createTestScript(test.scriptPath, test.scriptContent, test.isExecutable)) {
        toolbox::logger::StepMark::error(test.name + ": スクリプト作成失敗");
        return false;
    }
    http::Response response;
    http::CgiHandler handler;
    toolbox::SharedPtr<config::Config> config =
                        config::ConfigParser::parseFile("conf/cgi.conf");
    bool result = handler.handleRequest(
        *(test.request),
        response,
        test.rootPath,
        test.cgiExtension,
        test.cgiPass,
        *config);
    std::remove(test.scriptPath.c_str());
    if (result != test.shouldSucceed) {
        if (test.shouldSucceed) {
            toolbox::logger::StepMark::error(test.name + ": 成功するはずが失敗");
        } else {
            toolbox::logger::StepMark::error(test.name + ": 失敗するはずが成功");
        }
        return false;
    }
    if (test.validateStatus && response.getStatus() != test.expectedStatus) {
        toolbox::logger::StepMark::error(test.name + ": ステータスコードが期待値と異なる - 期待値: " 
                                        + toolbox::to_string(test.expectedStatus) 
                                        + " 実際: " + toolbox::to_string(response.getStatus()));
        return false;
    }
    if (test.validateHeader) {
        const std::map<std::string, std::pair<bool, std::string> >& headers = response.getHeaders();
        for (std::map<std::string, std::string>::const_iterator it = test.expectedHeaders.begin();
            it != test.expectedHeaders.end(); ++it) {
            std::map<std::string, std::pair<bool, std::string> >::const_iterator found = 
                                                                headers.find(it->first);
            if (found == headers.end() || found->second.second != it->second) {
                toolbox::logger::StepMark::error(test.name + ": ヘッダー " + it->first 
                                               + " が期待値と異なるか存在しません");
                return false;
            }
        }
    }
    if (test.validateLocation) {
        const std::map<std::string, std::pair<bool, std::string> >& headers = response.getHeaders();
        std::map<std::string, std::pair<bool, std::string> >::const_iterator found = 
                                                                headers.find("Location");
        if (found == headers.end() || found->second.second != test.expectedLocation) {
            toolbox::logger::StepMark::error(test.name + ": Location ヘッダーが期待値と異なるか存在しません");
            return false;
        }
    }
    if (test.validateBody && response.getBody() != test.expectedBody) {
        toolbox::logger::StepMark::error(test.name + ": レスポンスボディが期待値と異なります");
        return false;
    }
    toolbox::logger::StepMark::info(test.name + ": テスト成功");
    return true;
}

bool testBasicGetRequest() {
    CgiHandleTest test;
    test.name = "基本的なGETリクエスト";
    test.scriptPath = "test_cgi.py";
    test.scriptContent =
        "#!/usr/bin/python3\n"
        "import sys\n"
        "print(\"Content-Type: text/html\")\n"
        "print(\"\")\n"
        "print(\"<html><body>\")\n"
        "print(\"<h1>CGI Test Successful</h1>\")\n"
        "print(\"<p>This is a test CGI script.</p>\")\n"
        "print(\"</body></html>\")\n";
    test.isExecutable = true;
    test.request->method = http::method::GET;
    test.request->uri.path = test.scriptPath;
    test.cgiExtension.push_back(".py");
    test.cgiPass = "/usr/bin/python3";
    test.rootPath = "";
    test.shouldSucceed = true;
    test.validateStatus = true;
    test.expectedStatus = 200;
    return runTest(test);
}

bool testPostRequest() {
    CgiHandleTest test;
    test.name = "POSTリクエスト";
    test.scriptPath = "post_test.py";
    test.scriptContent =
        "#!/usr/bin/python3\n"
        "import sys\n"
        "import os\n"
        "content_length = int(os.environ.get('CONTENT_LENGTH', 0))\n"
        "post_data = sys.stdin.read(content_length)\n"
        "print(\"Content-Type: text/html\\r\")\n"
        "print(\"\\r\")\n"
        "print(\"<html><body>\")\n"
        "print(\"<h1>POST Test</h1>\")\n"
        "print(\"<p>Received data: \" + post_data + \"</p>\")\n"
        "print(\"</body></html>\")\n";
    test.isExecutable = true;
    test.request->method = http::method::POST;
    test.request->uri.path = test.scriptPath;
    std::string postBody = "name=test&value=123";
    test.request->body.content = postBody;
    test.request->body.contentLength = postBody.length();
    http::HTTPFields::FieldValue contentTypeValue;
    contentTypeValue.push_back("application/x-www-form-urlencoded");
    test.request->fields.getFieldValue("Content-Type") = contentTypeValue;
    test.cgiExtension.push_back(".py");
    test.cgiPass = "/usr/bin/python3";
    test.rootPath = "";
    test.shouldSucceed = true;
    test.validateStatus = true;
    test.expectedStatus = 200;
    return runTest(test);
}

bool testEnvironmentVariables() {
    CgiHandleTest test;
    test.name = "環境変数";
    test.scriptPath = "env_test.py";
    test.scriptContent =
        "#!/usr/bin/python3\n"
        "import os\n"
        "print(\"Content-Type: text/plain\\r\")\n"
        "print(\"\\r\")\n"
        "variables = [\n"
        "    'AUTH_TYPE',\n"
        "    'CONTENT_LENGTH',\n"
        "    'CONTENT_TYPE',\n"
        "    'GATEWAY_INTERFACE',\n"
        "    'PATH_INFO',\n"
        "    'PATH_TRANSLATED',\n"
        "    'QUERY_STRING',\n"
        "    'REMOTE_ADDR',\n"
        "    'REMOTE_HOST',\n"
        "    'REMOTE_IDENT',\n"
        "    'REMOTE_USER',\n"
        "    'REQUEST_METHOD',\n"
        "    'SCRIPT_NAME',\n"
        "    'SERVER_NAME',\n"
        "    'SERVER_PORT',\n"
        "    'SERVER_PROTOCOL',\n"
        "    'SERVER_SOFTWARE'\n"
        "]\n"
        "print(\"=== CGI環境変数 ===\")\n"
        "for key in variables:\n"
        "    print(f\"{key} = {os.environ.get(key, 'NOT SET')}\")\n"
        "print(\"\\n=== HTTP_* 環境変数 ===\")\n"
        "for key in os.environ:\n"
        "    if key.startswith('HTTP_'):\n"
        "        print(f\"{key} = {os.environ[key]}\")\n";
    test.isExecutable = true;
    test.request->method = http::method::POST;
    test.request->uri.path = test.scriptPath + "/extra/path";
    test.request->uri.fullQuery = "test=value";
    test.request->version = "HTTP/1.1";
    std::string postBody = "name=test&value=123&data=environment_test";
    test.request->body.content = postBody;
    test.request->body.contentLength = postBody.length();
    http::HTTPFields::FieldValue contentTypeValue;
    contentTypeValue.push_back("application/x-www-form-urlencoded");
    test.request->fields.getFieldValue("Content-Type") = contentTypeValue;
    http::HTTPFields::FieldValue userAgentValue;
    userAgentValue.push_back("CGI-TestClient/1.0");
    test.request->fields.getFieldValue("User-Agent") = userAgentValue;
    http::HTTPFields::FieldValue acceptValue;
    acceptValue.push_back("text/html,application/xhtml+xml");
    test.request->fields.getFieldValue("Accept") = acceptValue;
    http::HTTPFields::FieldValue hostValue;
    hostValue.push_back("test.example.com");
    test.request->fields.getFieldValue("Host") = hostValue;
    http::HTTPFields::FieldValue cookieValue;
    cookieValue.push_back("sessionid=abc123");
    test.request->fields.getFieldValue("Cookie") = cookieValue;
    http::HTTPFields::FieldValue customValue;
    customValue.push_back("TestValue");
    test.request->fields.getFieldValue("X-Custom-Header") = customValue;
    test.cgiExtension.push_back(".py");
    test.cgiPass = "/usr/bin/python3";
    test.rootPath = "";
    test.shouldSucceed = true;
    test.validateStatus = true;
    test.expectedStatus = 200;
    return runTest(test);
}

bool testNoExecutablePermission() {
    CgiHandleTest test;
    test.name = "実行権限なし";
    test.scriptPath = "no_exec_test.py";
    test.scriptContent = "#!/usr/bin/python3\nprint('This should not be executed')";
    test.isExecutable = false;
    test.request->method = http::method::GET;
    test.request->uri.path = test.scriptPath;
    test.cgiExtension.push_back(".py");
    test.cgiPass = "/usr/bin/python3";
    test.rootPath = "";
    test.shouldSucceed = false;
    test.validateStatus = true;
    test.expectedStatus = 403;
    return runTest(test);
}

bool testTimeout() {
    CgiHandleTest test;
    test.name = "タイムアウト";
    test.scriptPath = "timeout_test.py";
    test.scriptContent =
        "#!/usr/bin/python3\n"
        "import time\n"
        "import sys\n"
        "time.sleep(60)\n"
        "print(\"Content-Type: text/html\")\n"
        "print(\"\")\n"
        "print(\"<html><body>\")\n"
        "print(\"<h1>This should not be displayed due to timeout</h1>\")\n"
        "print(\"</body></html>\")\n";
    test.isExecutable = true;
    test.request->method = http::method::GET;
    test.request->uri.path = test.scriptPath;
    test.cgiExtension.push_back(".py");
    test.cgiPass = "/usr/bin/python3";
    test.rootPath = "";
    test.shouldSucceed = false;
    test.validateStatus = true;
    test.expectedStatus = 504;
    return runTest(test);
}

bool testStatusCode200() {
    CgiHandleTest test;
    test.name = "ステータスコード 200";
    test.scriptPath = "status_test_200.py";
    test.scriptContent =
        "#!/usr/bin/python3\n"
        "print(\"Status: 200 OK\\r\")\n"
        "print(\"Content-Type: text/plain\\r\")\n"
        "print(\"\\r\")\n"
        "print(\"Status code test: 200\")\n";
    test.isExecutable = true;
    test.request->method = http::method::GET;
    test.request->uri.path = test.scriptPath;
    test.cgiExtension.push_back(".py");
    test.cgiPass = "/usr/bin/python3";
    test.rootPath = "";
    test.shouldSucceed = true;
    test.validateStatus = true;
    test.expectedStatus = 200;
    return runTest(test);
}

bool testStatusCode201() {
    CgiHandleTest test;
    test.name = "ステータスコード 201";
    test.scriptPath = "status_test_201.py";
    test.scriptContent =
        "#!/usr/bin/python3\n"
        "print(\"Status: 201 Created\\r\")\n"
        "print(\"Content-Type: text/plain\\r\")\n"
        "print(\"\\r\")\n"
        "print(\"Status code test: 201\")\n";
    test.isExecutable = true;
    test.request->method = http::method::GET;
    test.request->uri.path = test.scriptPath;
    test.cgiExtension.push_back(".py");
    test.cgiPass = "/usr/bin/python3";
    test.rootPath = "";
    test.shouldSucceed = true;
    test.validateStatus = true;
    test.expectedStatus = 201;
    return runTest(test);
}

bool testStatusCode404() {
    CgiHandleTest test;
    test.name = "ステータスコード 404";
    test.scriptPath = "status_test_404.py";
    test.scriptContent =
        "#!/usr/bin/python3\n"
        "print(\"Status: 404 Not Found\\r\")\n"
        "print(\"Content-Type: text/plain\\r\")\n"
        "print(\"\\r\")\n"
        "print(\"Status code test: 404\")\n";
    test.isExecutable = true;
    test.request->method = http::method::GET;
    test.request->uri.path = test.scriptPath;
    test.cgiExtension.push_back(".py");
    test.cgiPass = "/usr/bin/python3";
    test.rootPath = "";
    test.shouldSucceed = true;
    test.validateStatus = true;
    test.expectedStatus = 404;
    return runTest(test);
}

bool testStatusCode500() {
    CgiHandleTest test;
    test.name = "ステータスコード 500";
    test.scriptPath = "status_test_500.py";
    test.scriptContent =
        "#!/usr/bin/python3\n"
        "print(\"Status: 500 Internal Server Error\\r\")\n"
        "print(\"Content-Type: text/plain\\r\")\n"
        "print(\"\\r\")\n"
        "print(\"Status code test: 500\")\n";
    test.isExecutable = true;
    test.request->method = http::method::GET;
    test.request->uri.path = test.scriptPath;
    test.cgiExtension.push_back(".py");
    test.cgiPass = "/usr/bin/python3";
    test.rootPath = "";
    test.shouldSucceed = false;
    test.validateStatus = true;
    test.expectedStatus = 500;
    return runTest(test);
}

bool testSetCookieHeader() {
    CgiHandleTest test;
    test.name = "Set-Cookieヘッダー";
    test.scriptPath = "set_cookie_test.py";
    test.scriptContent =
        "#!/usr/bin/python3\n"
        "print(\"Content-Type: text/html\\r\")\n"
        "print(\"Set-Cookie: session=abc123; Path=/; HttpOnly\\r\")\n"
        "print(\"Set-Cookie: user=john; Max-Age=3600\\r\")\n"
        "print(\"\\r\")\n"
        "print(\"<html><body>\")\n"
        "print(\"<h1>Set-Cookie Test</h1>\")\n"
        "print(\"<p>Cookies have been set.</p>\")\n"
        "print(\"</body></html>\")\n";
    test.isExecutable = true;
    test.request->method = http::method::GET;
    test.request->uri.path = test.scriptPath;
    test.cgiExtension.push_back(".py");
    test.cgiPass = "/usr/bin/python3";
    test.rootPath = "";
    test.shouldSucceed = true;
    test.validateStatus = true;
    test.expectedStatus = 200;
    return runTest(test);
}

bool testRedirectWithDocument() {
    CgiHandleTest test;
    test.name = "リダイレクト+ドキュメント";
    test.scriptPath = "redirect_doc_test.py";
    test.scriptContent =
        "#!/usr/bin/python3\n"
        "print(\"Status: 302 Found\\r\")\n"
        "print(\"Location: http://example.com/page.html\\r\")\n"
        "print(\"Content-Type: text/html\\r\")\n"
        "print(\"\\r\")\n"
        "print(\"<html><body>\")\n"
        "print(\"<h1>リダイレクト中...</h1>\")\n"
        "print(\"<p>このページは自動的にリダイレクトされます。</p>\")\n"
        "print(\"</body></html>\")\n";
    test.isExecutable = true;
    test.request->method = http::method::GET;
    test.request->uri.path = test.scriptPath;
    test.cgiExtension.push_back(".py");
    test.cgiPass = "/usr/bin/python3";
    test.rootPath = "";
    test.shouldSucceed = true;
    test.validateStatus = true;
    test.expectedStatus = 302;
    test.validateLocation = true;
    test.expectedLocation = "http://example.com/page.html";
    return runTest(test);
}

bool testValidCgiRequest() {
    CgiHandleTest test;
    test.name = "正規のCGIリクエスト判別";
    test.scriptPath = "valid_script.py";
    test.scriptContent =
        "#!/usr/bin/python3\n"
        "print(\"Content-Type: text/plain\\r\")\n"
        "print(\"\\r\")\n"
        "print(\"Valid CGI Script\")\n";
    test.isExecutable = true;
    if (!createTestScript(test.scriptPath, test.scriptContent, test.isExecutable)) {
        toolbox::logger::StepMark::error(test.name + ": スクリプト作成失敗");
        return false;
    }
    test.request->method = http::method::GET;
    test.request->uri.path = test.scriptPath;
    std::vector<std::string> cgiExtension;
    cgiExtension.push_back(".py");
    std::string cgiPass = "/usr/bin/python3";
    http::CgiHandler handler;
    bool result = handler.isCgiRequest(test.scriptPath, cgiExtension, cgiPass);
    std::remove(test.scriptPath.c_str());
    if (!result) {
        toolbox::logger::StepMark::error(test.name + ": 有効なCGIリクエストと判定されるべき");
        return false;
    }
    toolbox::logger::StepMark::info(test.name + ": テスト成功");
    return true;
}

bool testEmptyCgiExtension() {
    CgiHandleTest test;
    test.name = "CGI拡張子未定義";
    test.scriptPath = "test_script.py";
    test.scriptContent = "#!/usr/bin/python3\nprint('Hello')";
    test.isExecutable = true;
    if (!createTestScript(test.scriptPath, test.scriptContent, test.isExecutable)) {
        toolbox::logger::StepMark::error(test.name + ": スクリプト作成失敗");
        return false;
    }
    test.request->method = http::method::GET;
    test.request->uri.path = test.scriptPath;
    std::vector<std::string> cgiExtension;
    std::string cgiPass = "/usr/bin/python3";
    http::CgiHandler handler;
    bool result = handler.isCgiRequest(test.scriptPath, cgiExtension, cgiPass);
    std::remove(test.scriptPath.c_str());
    if (result) {
        toolbox::logger::StepMark::error(test.name + ": CGI拡張子がない場合はfalseを返すべき");
        return false;
    }
    toolbox::logger::StepMark::info(test.name + ": テスト成功");
    return true;
}

bool testEmptyCgiPass() {
    CgiHandleTest test;
    test.name = "インタープリタ未指定";
    test.scriptPath = "test_script.py";
    test.scriptContent = "#!/usr/bin/python3\nprint('Hello')";
    test.isExecutable = true;
    if (!createTestScript(test.scriptPath, test.scriptContent, test.isExecutable)) {
        toolbox::logger::StepMark::error(test.name + ": スクリプト作成失敗");
        return false;
    }
    test.request->method = http::method::GET;
    test.request->uri.path = test.scriptPath;
    std::vector<std::string> cgiExtension;
    cgiExtension.push_back(".py");
    std::string cgiPass = "";
    http::CgiHandler handler;
    bool result = handler.isCgiRequest(test.scriptPath, cgiExtension, cgiPass);
    std::remove(test.scriptPath.c_str());
    if (result) {
        toolbox::logger::StepMark::error(test.name + ": CGIパスが空の場合はfalseを返すべき");
        return false;
    }
    toolbox::logger::StepMark::info(test.name + ": テスト成功");
    return true;
}

bool testNonExistentFile() {
    CgiHandleTest test;
    test.name = "存在しないファイル";
    test.scriptPath = "non_existent_file.py";
    test.request->method = http::method::GET;
    test.request->uri.path = test.scriptPath;
    std::vector<std::string> cgiExtension;
    cgiExtension.push_back(".py");
    std::string cgiPass = "/usr/bin/python3";
    http::CgiHandler handler;
    bool result = handler.isCgiRequest(test.scriptPath, cgiExtension, cgiPass);
    if (result) {
        toolbox::logger::StepMark::error(test.name + ": 存在しないファイルの場合はfalseを返すべき");
        return false;
    }
    toolbox::logger::StepMark::info(test.name + ": テスト成功");
    return true;
}

bool testInvalidExtension() {
    CgiHandleTest test;
    test.name = "許可されていない拡張子";
    test.scriptPath = "test_script.invalid";
    test.scriptContent = "#!/usr/bin/python3\nprint('Hello')";
    test.isExecutable = true;
    if (!createTestScript(test.scriptPath, test.scriptContent, test.isExecutable)) {
        toolbox::logger::StepMark::error(test.name + ": スクリプト作成失敗");
        return false;
    }
    test.request->method = http::method::GET;
    test.request->uri.path = test.scriptPath;
    test.cgiExtension.push_back(".py");
    test.cgiPass = "/usr/bin/python3";
    test.rootPath = "";
    test.shouldSucceed = false;
    test.validateStatus = true;
    test.expectedStatus = 403;
    bool result = runTest(test);
    std::remove(test.scriptPath.c_str());
    return result;
}

bool testDirectoryAsScript() {
    CgiHandleTest test;
    test.name = "ディレクトリをスクリプト実行";
    test.scriptPath = "test_directory.py";
    if (mkdir(test.scriptPath.c_str(), 0755) != 0) {
        toolbox::logger::StepMark::error(test.name + ": ディレクトリ作成失敗");
        return false;
    }
    test.request->method = http::method::GET;
    test.request->uri.path = test.scriptPath;
    test.cgiExtension.push_back(".py");
    test.cgiPass = "/usr/bin/python3";
    test.rootPath = "";
    test.shouldSucceed = false;
    test.validateStatus = true;
    test.expectedStatus = 403;
    // runTest関数を使用
    bool result = runTest(test);
    // 後処理
    rmdir(test.scriptPath.c_str());
    if (result) {
        toolbox::logger::StepMark::error(test.name + ": ディレクトリの場合は実行失敗すべき");
        return false;
    }
    toolbox::logger::StepMark::info(test.name + ": テスト成功");
    return true;
}

// 存在しない/実行権限のないインタープリタ
bool testInvalidInterpreter() {
    CgiHandleTest test;
    test.name = "無効なインタープリタ";
    test.scriptPath = "test_script.py";
    test.scriptContent = "#!/usr/bin/python3\nprint('Hello')";
    test.isExecutable = true;
    if (!createTestScript(test.scriptPath, test.scriptContent, test.isExecutable)) {
        toolbox::logger::StepMark::error(test.name + ": スクリプト作成失敗");
        return false;
    }
    test.request->method = http::method::GET;
    test.request->uri.path = test.scriptPath;
    test.cgiExtension.push_back(".py");
    test.cgiPass = "/non_existent_interpreter";
    test.rootPath = "";
    test.shouldSucceed = false;
    test.validateStatus = true;
    test.expectedStatus = 403;
    bool result = runTest(test);
    std::remove(test.scriptPath.c_str());
    return result;
}

bool testInvalidCgiResponse() {
    CgiHandleTest test;
    test.name = "無効なCGIレスポンス";
    test.scriptPath = "invalid_response.py";
    test.scriptContent =
        "#!/usr/bin/python3\n"
        "# 無効なCGIレスポンス（ヘッダーとボディの区切りがない）\n"
        "print(\"This is invalid CGI response\")\n";
    test.isExecutable = true;
    if (!createTestScript(test.scriptPath, test.scriptContent, test.isExecutable)) {
        toolbox::logger::StepMark::error(test.name + ": スクリプト作成失敗");
        return false;
    }
    test.request->method = http::method::GET;
    test.request->uri.path = test.scriptPath;
    test.cgiExtension.push_back(".py");
    test.cgiPass = "/usr/bin/python3";
    test.rootPath = "";
    test.shouldSucceed = false;
    bool result = runTest(test);
    std::remove(test.scriptPath.c_str());
    return result;
}

bool testLocalRedirect() {
    CgiHandleTest test;
    test.name = "ローカルリダイレクト";
    std::string folderPath = "redirect";
    if (mkdir(folderPath.c_str(), 0755) != 0) {
        toolbox::logger::StepMark::error(test.name + ": フォルダ作成失敗");
        return false;
    }
    std::string redirectPage = "local_page.html";
    std::ofstream redirectFile(redirectPage.c_str());
    redirectFile << "<html><body><h1>リダイレクト成功</h1></body></html>";
    redirectFile.close();
    test.scriptPath = folderPath + "local_redirect.py";
    test.scriptContent =
        "#!/usr/bin/python3\n"
        "print(\"Location: /local_page.html\\r\")\n"
        "print(\"\\r\")\n";
    test.isExecutable = true;
    if (!createTestScript(test.scriptPath, test.scriptContent, test.isExecutable)) {
        toolbox::logger::StepMark::error(test.name + ": スクリプト作成失敗");
        return false;
    }
    test.request->method = http::method::POST;
    test.request->uri.path = test.scriptPath;
    test.cgiExtension.push_back(".py");
    test.cgiPass = "/usr/bin/python3";
    test.rootPath = "";
    test.shouldSucceed = true;
    bool result = runTest(test);
    std::remove(test.scriptPath.c_str());
    std::remove(redirectPage.c_str());
    rmdir(folderPath.c_str());
    return result;
}

bool testMultiStepLocalRedirect() {
    CgiHandleTest test;
    test.name = "多段階ローカルリダイレクト";
    std::string folderPath = "redirect/";
    if (mkdir(folderPath.c_str(), 0755) != 0) {
        toolbox::logger::StepMark::error(test.name + ": フォルダ作成失敗");
        return false;
    }
    std::string finalPage =  folderPath + "final_page.py";
    std::string finalScript = 
        "#!/usr/bin/python3\n"
        "print(\"Content-Type: text/html\\r\")\n"
        "print(\"\\r\")\n"
        "print(\"<html><body><h1>最終リダイレクト先</h1>\")\n"
        "print(\"<p>リダイレクトチェーンが正常に終了しました</p>\")\n"
        "print(\"</body></html>\")\n";
    if (!createTestScript(finalPage, finalScript, true)) {
        toolbox::logger::StepMark::error(test.name + ": 最終スクリプト作成失敗");
        return false;
    }
    std::string middleScriptPath = folderPath + "middle_redirect.py";
    std::string middleScriptContent =
        "#!/usr/bin/python3\n"
        "print(\"Location: /final_page.py\\r\")\n"
        "print(\"\\r\")\n";
    if (!createTestScript(middleScriptPath, middleScriptContent, true)) {
        toolbox::logger::StepMark::error(test.name + ": 中間スクリプト作成失敗");
        return false;
    }
    test.scriptPath = folderPath + "first_redirect.py";
    test.scriptContent =
        "#!/usr/bin/python3\n"
        "print(\"Location: /middle_redirect.py\\r\")\n"
        "print(\"\\r\")\n";
    test.isExecutable = true;
    if (!createTestScript(test.scriptPath, test.scriptContent, test.isExecutable)) {
        toolbox::logger::StepMark::error(test.name + ": 最初のスクリプト作成失敗");
        return false;
    }
    test.request->method = http::method::GET;
    test.request->uri.path = test.scriptPath;
    test.cgiExtension.push_back(".py");
    test.cgiPass = "/usr/bin/python3";
    test.rootPath = "";
    test.shouldSucceed = true;
    test.validateStatus = true;
    test.expectedStatus = 200;
    toolbox::logger::StepMark::debug("多段階リダイレクトテスト開始");
    bool result = runTest(test);
    toolbox::logger::StepMark::debug("多段階リダイレクトテスト終了");
    std::remove(test.scriptPath.c_str());
    std::remove(middleScriptPath.c_str());
    std::remove(finalPage.c_str());
    rmdir(folderPath.c_str());
    return result;
}

bool testInvalidLocationHeader() {
    CgiHandleTest test;
    test.name = "無効なLocationヘッダー";
    test.scriptPath = "invalid_location.py";
    test.scriptContent =
        "#!/usr/bin/python3\n"
        "print(\"Location/some/path\\r\")  # コロンがない\n"
        "print(\"Content-Type: text/html\\r\")\n"
        "print(\"\\r\")\n"
        "print(\"<html><body><h1>Invalid Location Header Syntax</h1></body></html>\")\n";
    test.isExecutable = true;
    if (!createTestScript(test.scriptPath, test.scriptContent, test.isExecutable)) {
        toolbox::logger::StepMark::error(test.name + ": スクリプト作成失敗");
        return false;
    }
    test.request->method = http::method::GET;
    test.request->uri.path = test.scriptPath;
    test.cgiExtension.push_back(".py");
    test.cgiPass = "/usr/bin/python3";
    test.rootPath = "";
    test.shouldSucceed = false;
    test.validateLocation = false;
    bool result = runTest(test);
    std::remove(test.scriptPath.c_str());
    return result;
}

bool testRedirectLoop() {
    CgiHandleTest test;
    test.name = "リダイレクトループ検出";
    std::string folderPath = "redirect";
    if (mkdir(folderPath.c_str(), 0755) != 0) {
        toolbox::logger::StepMark::error(test.name + ": フォルダ作成失敗");
        return false;
    }
    test.scriptPath = folderPath + "/redirect_loop.py";
    test.scriptContent =
        "#!/usr/bin/python3\n"
        "print(\"Location: /redirect_loop.py\\r\")\n"
        "print(\"\\r\")\n";
    test.isExecutable = true;
    if (!createTestScript(test.scriptPath, test.scriptContent, test.isExecutable)) {
        toolbox::logger::StepMark::error(test.name + ": スクリプト作成失敗");
        return false;
    }
    test.request->method = http::method::GET;
    test.request->uri.path = test.scriptPath;
    test.cgiExtension.push_back(".py");
    test.cgiPass = "/usr/bin/python3";
    test.rootPath = "";
    test.shouldSucceed = false;
    test.validateStatus = true;
    test.expectedStatus = 500;
    bool result = runTest(test);
    std::remove(test.scriptPath.c_str());
    rmdir(folderPath.c_str());
    return result;
}

bool testClientRedirect() {
    CgiHandleTest test;
    test.name = "クライアントリダイレクト（本文なし）";
    test.scriptPath = "client_redirect.py";
    test.scriptContent =
        "#!/usr/bin/python3\n"
        "print(\"Location: https://example.com/\\r\")\n"
        "print(\"\\r\")\n";
    test.isExecutable = true;
    test.request->method = http::method::GET;
    test.request->uri.path = test.scriptPath;
    test.cgiExtension.push_back(".py");
    test.cgiPass = "/usr/bin/python3";
    test.rootPath = "";
    test.shouldSucceed = true;
    test.validateStatus = true;
    test.expectedStatus = 302;
    test.validateLocation = true;
    test.expectedLocation = "https://example.com/";
    test.validateBody = true;
    test.expectedBody = "";
    bool result = runTest(test);
    std::remove(test.scriptPath.c_str());
    return result;
}

bool testClientRedirectWithBody() {
    CgiHandleTest test;
    test.name = "クライアントリダイレクト（ボディ付き）";
    test.scriptPath = "client_redirect_body.py";
    test.scriptContent =
        "#!/usr/bin/python3\n"
        "print(\"Status: 301 Moved Permanently\\r\")\n"
        "print(\"Location: https://example.com/\\r\")\n"
        "print(\"Content-Type: text/html\\r\")\n"
        "print(\"\\r\")\n"
        "print(\"<html><body>\")\n"
        "print(\"<h1>ページが移動しました</h1>\")\n"
        "print(\"<p>新しいURLに自動的にリダイレクトされます。</p>\")\n"
        "print(\"<p><a href=\\\"https://example.com/\\\">こちらをクリック</a>してください。</p>\")\n"
        "print(\"</body></html>\")\n";
    test.isExecutable = true;
    if (!createTestScript(test.scriptPath, test.scriptContent, test.isExecutable)) {
        toolbox::logger::StepMark::error(test.name + ": スクリプト作成失敗");
        return false;
    }
    test.request->method = http::method::GET;
    test.request->uri.path = test.scriptPath;
    test.cgiExtension.push_back(".py");
    test.cgiPass = "/usr/bin/python3";
    test.rootPath = "";
    test.shouldSucceed = true;
    test.validateStatus = true;
    test.expectedStatus = 301;
    test.validateLocation = true;
    test.expectedLocation = "https://example.com/";
    test.validateBody = true;
    test.expectedBody = 
        "<html><body>\n"
        "<h1>ページが移動しました</h1>\n"
        "<p>新しいURLに自動的にリダイレクトされます。</p>\n"
        "<p><a href=\"https://example.com/\">こちらをクリック</a>してください。</p>\n"
        "</body></html>\n";
    bool result = runTest(test);
    std::remove(test.scriptPath.c_str());
    return result;
}

bool testErrorStatusCodeConversion() {
    CgiHandleTest test;
    test.name = "エラーステータスコード変換";
    test.scriptPath = "error_script.py";
    test.scriptContent = 
        "#!/usr/bin/python3\n"
        "import sys\n"
        "sys.exit(1)\n";
    test.isExecutable = true;
    if (!createTestScript(test.scriptPath, test.scriptContent, test.isExecutable)) {
        toolbox::logger::StepMark::error(test.name + ": スクリプト作成失敗");
        return false;
    }
    test.request->method = http::method::GET;
    test.request->uri.path = test.scriptPath;
    test.cgiExtension.push_back(".py");
    test.cgiPass = "/usr/bin/python3";
    test.rootPath = "";
    test.shouldSucceed = false;
    test.validateStatus = true;
    test.expectedStatus = 500;
    bool result = runTest(test);
    std::remove(test.scriptPath.c_str());
    return result;
}

bool testMultipleHeaders() {
    CgiHandleTest test;
    test.name = "複数ヘッダー処理";
    test.scriptPath = "multiple_headers.py";
    test.scriptContent =
        "#!/usr/bin/python3\n"
        "print(\"Content-Type: text/html\\r\")\n"
        "print(\"X-Custom-Header1: Value1\\r\")\n"
        "print(\"X-Custom-Header1: Value2\\r\")\n"
        "print(\"X-Custom-Header2: Value3\\r\")\n"
        "print(\"\\r\")\n"
        "print(\"<html><body><h1>Multiple Headers Test</h1></body></html>\")\n";
    test.isExecutable = true;
    if (!createTestScript(test.scriptPath, test.scriptContent, test.isExecutable)) {
        toolbox::logger::StepMark::error(test.name + ": スクリプト作成失敗");
        return false;
    }
    test.request->method = http::method::GET;
    test.request->uri.path = test.scriptPath;
    test.cgiExtension.push_back(".py");
    test.cgiPass = "/usr/bin/python3";
    test.rootPath = "";
    test.shouldSucceed = true;
    bool result = runTest(test);
    std::remove(test.scriptPath.c_str());
    return result;
}

void handleTest() {
    toolbox::logger::StepMark::info("CGI Handler テスト開始");
    int totalTests = 0;
    int passedCount = 0;
    bool result;
    totalTests++;
    result = testBasicGetRequest();
    if (result) passedCount++;

    totalTests++;
    result = testPostRequest();
    if (result) passedCount++;

    totalTests++;
    result = testEnvironmentVariables();
    if (result) passedCount++;

    totalTests++;
    result = testNoExecutablePermission();
    if (result) passedCount++;

    totalTests++;
    result = testTimeout();
    if (result) passedCount++;

    totalTests++;
    result = testStatusCode200();
    if (result) passedCount++;

    totalTests++;
    result = testStatusCode201();
    if (result) passedCount++;

    totalTests++;
    result = testStatusCode404();
    if (result) passedCount++;

    totalTests++;
    result = testStatusCode500();
    if (result) passedCount++;

    totalTests++;
    result = testSetCookieHeader();
    if (result) passedCount++;

    totalTests++;
    result = testRedirectWithDocument();
    if (result) passedCount++;

    totalTests++;
    result = testValidCgiRequest();
    if (result) passedCount++;

    totalTests++;
    result = testEmptyCgiExtension();
    if (result) passedCount++;

    totalTests++;
    result = testEmptyCgiPass();
    if (result) passedCount++;

    totalTests++;
    result = testNonExistentFile();
    if (result) passedCount++;

    totalTests++;
    result = testInvalidExtension();
    if (result) passedCount++;

    totalTests++;
    result = testDirectoryAsScript();
    if (result) passedCount++;

    totalTests++;
    result = testInvalidInterpreter();
    if (result) passedCount++;

    totalTests++;
    result = testInvalidCgiResponse();
    if (result) passedCount++;

    totalTests++;
    result = testLocalRedirect();
    if (result) passedCount++;

    totalTests++;
    result = testMultiStepLocalRedirect();
    if (result) passedCount++;

    totalTests++;
    result = testInvalidLocationHeader();
    if (result) passedCount++;

    totalTests++;
    result = testRedirectLoop();
    if (result) passedCount++;

    totalTests++;
    result = testClientRedirect();
    if (result) passedCount++;

    totalTests++;
    result = testClientRedirectWithBody();
    if (result) passedCount++;

    totalTests++;
    result = testErrorStatusCodeConversion();
    if (result) passedCount++;

    totalTests++;
    result = testMultipleHeaders();
    if (result) passedCount++;

    std::cout << "CGI handle testcase " <<
            passedCount << " / " << totalTests << std::endl;
}

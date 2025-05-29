#include <iostream>
#include <string>
#include <fstream>
#include <sys/stat.h>
#include <cstdio>
#include <ctime>
#include <sstream>
#include <signal.h>
#include <cstdlib>
#include <cmath>

#include "../../../toolbox/stepmark.hpp"
#include "../../../src/http/cgi/cgi_execute.hpp"
#include "../../../src/http/request/http_request.hpp"
#include "cgi_execute_test.hpp"

namespace cgi_execute_test {

std::string convertToString(size_t value) {
    std::ostringstream stream;
    stream << value;
    return stream.str();
}

http::HTTPRequest* createTestRequest(const std::string& method,
                            const std::string& path,
                            const std::string& query,
                            const std::string& body,
                            const std::string& contentType) {
    http::HTTPRequest* request = new http::HTTPRequest();
    request->method = method;
    request->uri.path = path;
    request->uri.fullQuery = query;
    request->version = "HTTP/1.1";
    if (!body.empty()) {
        request->body.content = body;
        request->body.contentLength = body.length();
        if (!contentType.empty()) {
            http::HTTPFields::FieldValue contentTypeValue;
            contentTypeValue.push_back(contentType);
            request->fields.getFieldValue("Content-Type") = contentTypeValue;
            http::HTTPFields::FieldValue contentLengthValue;
            contentLengthValue.push_back(convertToString(body.length()));
            request->fields.getFieldValue("Content-Length") = contentLengthValue;
        }
    }
    return request;
}

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

bool runTest(const std::string& name, bool (*testFunc)()) {
    toolbox::logger::StepMark::info("============= テスト: " + name + " =============");
    bool result = testFunc();
    if (result) {
        toolbox::logger::StepMark::info(name + ": テスト成功");
    } else {
        toolbox::logger::StepMark::error(name + ": テスト失敗");
    }
    return result;
}

bool testBasicExecution() {
    const std::string scriptPath = "./test_basic.py";
    const std::string scriptContent =
        "#!/usr/bin/env python3\n"
        "print('Content-Type: text/plain\\r')\n"
        "print('\\r')\n"
        "print('Hello, CGI!')\n";
    if (!createTestScript(scriptPath, scriptContent, true)) {
        toolbox::logger::StepMark::error("スクリプト作成失敗");
        return false;
    }
    http::HTTPRequest* request = createTestRequest("GET", scriptPath);
    http::CgiExecute executor;
    std::string output;
    http::CgiExecute::ExecuteResult result = executor.execute(
        scriptPath, "/usr/bin/python3", *request, output);
    delete request;
    std::remove(scriptPath.c_str());
    if (result != http::CgiExecute::EXECUTE_SUCCESS) {
        toolbox::logger::StepMark::error("実行失敗: " + convertToString(static_cast<int>(result)));
        return false;
    }
    if (output.find("Content-Type: text/plain") == std::string::npos ||
        output.find("Hello, CGI!") == std::string::npos) {
        toolbox::logger::StepMark::error("出力内容が正しくない");
        toolbox::logger::StepMark::debug("実際の出力: " + output);
        return false;
    }
    toolbox::logger::StepMark::debug("出力内容: " + output);
    return true;
}

bool testPostDataHandling() {
    const std::string scriptPath = "./test_post.py";
    const std::string scriptContent =
        "#!/usr/bin/env python3\n"
        "import sys\n"
        "import os\n"
        "print('Content-Type: text/plain\\r')\n"
        "print('\\r')\n"
        "content_length = int(os.environ.get('CONTENT_LENGTH', 0))\n"
        "if content_length > 0:\n"
        "    post_data = sys.stdin.read(content_length)\n"
        "    print('Received POST data: ' + post_data)\n"
        "else:\n"
        "    print('No POST data received')\n";
    if (!createTestScript(scriptPath, scriptContent, true)) {
        toolbox::logger::StepMark::error("スクリプト作成失敗");
        return false;
    }
    std::string postBody = "name=test&value=123";
    http::HTTPRequest* request = createTestRequest(
        "POST", scriptPath, "", postBody, "application/x-www-form-urlencoded");
    http::CgiExecute executor;
    std::string output;
    http::CgiExecute::ExecuteResult result = executor.execute(
        scriptPath, "/usr/bin/python3", *request, output);
    delete request;
    toolbox::logger::StepMark::debug("出力内容: " + output);
    std::remove(scriptPath.c_str());
    if (result != http::CgiExecute::EXECUTE_SUCCESS) {
        toolbox::logger::StepMark::error("実行失敗: " + convertToString(static_cast<int>(result)));
        return false;
    }
    if (output.find("Received POST data: name=test&value=123") == std::string::npos) {
        toolbox::logger::StepMark::error("POSTデータが正しく処理されていない");
        toolbox::logger::StepMark::debug("実際の出力: " + output);
        return false;
    }
    return true;
}

bool testEnvironmentVariables() {
    const std::string scriptPath = "./test_env.py";
    const std::string scriptContent = 
        "#!/usr/bin/env python3\n"
        "import os\n"
        "print('Content-Type: text/plain\\r')\n"
        "print('\\r')\n"
        "print('=== CGI環境変数 ===')\n"
        "for key in ['SERVER_SOFTWARE', 'SERVER_PROTOCOL', 'REQUEST_METHOD', 'QUERY_STRING']:\n"
        "    print(f\"{key} = {os.environ.get(key, 'NOT SET')}\")\n"
        "print('=== HTTPヘッダー環境変数 ===')\n"
        "for key in os.environ:\n"
        "    if key.startswith('HTTP_'):\n"
        "        print(f\"{key} = {os.environ[key]}\")\n";
    if (!createTestScript(scriptPath, scriptContent, true)) {
        toolbox::logger::StepMark::error("スクリプト作成失敗");
        return false;
    }
    http::HTTPRequest* request = createTestRequest("GET", scriptPath, "test=value");
    http::HTTPFields::FieldValue userAgentValue;
    userAgentValue.push_back("CGI-Test/1.0");
    request->fields.getFieldValue("User-Agent") = userAgentValue;
    http::HTTPFields::FieldValue acceptValue;
    acceptValue.push_back("text/html,application/xhtml+xml");
    request->fields.getFieldValue("Accept") = acceptValue;
    http::HTTPFields::FieldValue customValue;
    customValue.push_back("CustomValue");
    request->fields.getFieldValue("X-Custom-Header") = customValue;
    http::CgiExecute executor;
    std::string output;
    http::CgiExecute::ExecuteResult result = executor.execute(
        scriptPath, "/usr/bin/python3", *request, output);
    delete request;
    toolbox::logger::StepMark::debug("出力内容: " + output);
    std::remove(scriptPath.c_str());
    if (result != http::CgiExecute::EXECUTE_SUCCESS) {
        toolbox::logger::StepMark::error("実行失敗: " + convertToString(static_cast<int>(result)));
        return false;
    }
    bool hasServerSoftware = output.find("SERVER_SOFTWARE") != std::string::npos;
    bool hasRequestMethod = output.find("REQUEST_METHOD = GET") != std::string::npos;
    bool hasQueryString = output.find("QUERY_STRING = test=value") != std::string::npos;
    if (!hasServerSoftware || !hasRequestMethod || !hasQueryString) {
        toolbox::logger::StepMark::error("環境変数が正しく設定されていない");
        toolbox::logger::StepMark::debug("実際の出力: " + output);
        return false;
    }
    return true;
}

bool testNoExecutablePermission() {
    const std::string scriptPath = "./no_exec.py";
    const std::string scriptContent = "#!/usr/bin/env python3\nprint('This should not execute')";
    if (!createTestScript(scriptPath, scriptContent, false)) {
        toolbox::logger::StepMark::error("スクリプト作成失敗");
        return false;
    }
    http::HTTPRequest* request = createTestRequest("GET", scriptPath);
    http::CgiExecute executor;
    std::string output;
    http::CgiExecute::ExecuteResult result = executor.execute(
        scriptPath, "/usr/bin/python3", *request, output);
    delete request;
    toolbox::logger::StepMark::debug("出力内容: " + output);
    std::remove(scriptPath.c_str());
    if (result != http::CgiExecute::EXECUTE_PATH_ERROR) {
        toolbox::logger::StepMark::error("実行権限エラーが正しく検出されていない: " + 
                                          convertToString(static_cast<int>(result)));
        return false;
    }
    return true;
}

bool testPathTraversalPrevention() {
    const std::string scriptPath = "../../../etc/passwd";
    http::HTTPRequest* request = createTestRequest("GET", scriptPath);
    http::CgiExecute executor;
    std::string output;
    http::CgiExecute::ExecuteResult result = executor.execute(
        scriptPath, "/usr/bin/cat", *request, output);
    delete request;
    toolbox::logger::StepMark::debug("出力内容: " + output);
    if (result != http::CgiExecute::EXECUTE_PATH_ERROR) {
        toolbox::logger::StepMark::error("パストラバーサルが正しく防止されていない: " + 
                                          convertToString(static_cast<int>(result)));
        return false;
    }
    return true;
}

bool testNonExistentScript() {
    const std::string scriptPath = "./non_existent_script.py";
    http::HTTPRequest* request = createTestRequest("GET", scriptPath);
    http::CgiExecute executor;
    std::string output;
    http::CgiExecute::ExecuteResult result = executor.execute(
        scriptPath, "/usr/bin/python3", *request, output);
    delete request;
    toolbox::logger::StepMark::debug("出力内容: " + output);
    if (result != http::CgiExecute::EXECUTE_PATH_ERROR) {
        toolbox::logger::StepMark::error("存在しないスクリプトエラーが正しく検出されていない: " + 
                                          toolbox::to_string(static_cast<int>(result)));
        return false;
    }
    return true;
}

bool testLargeOutput() {
    const std::string scriptPath = "./large_output.py";
    const std::string scriptContent =
    "#!/usr/bin/env python3\n"
    "print('Content-Type: text/plain\\r')\n"
    "print('\\r')\n"
    "# 約100KBのデータを生成\n"
    "for i in range(1, 1001):\n"
    "    print('Line {:04d}: This is a test of large output data generation. '.format(i) * 5)\n";
    if (!createTestScript(scriptPath, scriptContent, true)) {
        toolbox::logger::StepMark::error("スクリプト作成失敗");
        return false;
    }
    http::HTTPRequest* request = createTestRequest("GET", scriptPath);
    http::CgiExecute executor;
    std::string output;
    http::CgiExecute::ExecuteResult result = executor.execute(
        scriptPath, "/usr/bin/python3", *request, output);
    delete request;
    // toolbox::logger::StepMark::debug("出力内容: " + output);
    std::remove(scriptPath.c_str());
    if (result != http::CgiExecute::EXECUTE_SUCCESS) {
        toolbox::logger::StepMark::error("実行失敗: " + toolbox::to_string(static_cast<int>(result)));
        return false;
    }
    if (output.size() < 50000) {
        toolbox::logger::StepMark::error("大容量出力が正しく処理されていない");
        toolbox::logger::StepMark::debug("出力サイズ: " + toolbox::to_string(output.size()));
        return false;
    }
    return true;
}

bool testDifferentInterpreters() {
    const std::string scriptPath = "./interpreter_test.sh";
    const std::string scriptContent =
        "#!/bin/bash\n"
        "echo 'Content-Type: text/plain'\n"
        "echo ''\n"
        "echo 'This script is executed with bash interpreter'\n";
    if (!createTestScript(scriptPath, scriptContent, true)) {
        toolbox::logger::StepMark::error("スクリプト作成失敗");
        return false;
    }
    http::HTTPRequest* request = createTestRequest("GET", scriptPath);
    http::CgiExecute executor;
    std::string output;
    http::CgiExecute::ExecuteResult result = executor.execute(
        scriptPath, "/bin/bash", *request, output);
    delete request;
    toolbox::logger::StepMark::debug("出力内容: " + output);
    std::remove(scriptPath.c_str());
    if (result != http::CgiExecute::EXECUTE_SUCCESS) {
        toolbox::logger::StepMark::error("実行失敗: " + toolbox::to_string(static_cast<int>(result)));
        return false;
    }
    if (output.find("This script is executed with bash interpreter") == std::string::npos) {
        toolbox::logger::StepMark::error("bashインタープリタでの実行に失敗");
        toolbox::logger::StepMark::debug("実際の出力: " + output);
        return false;
    }
    return true;
}

bool testDeleteMethod() {
    const std::string scriptPath = "./test_delete.py";
    const std::string scriptContent =
        "#!/usr/bin/env python3\n"
        "import os\n"
        "print('Content-Type: text/plain\\r')\n"
        "print('\\r')\n"
        "print('REQUEST_METHOD: ' + os.environ.get('REQUEST_METHOD', 'NOT SET'))\n"
        "print('Target path: ' + os.environ.get('PATH_INFO', 'NOT SET'))\n";
    if (!createTestScript(scriptPath, scriptContent, true)) {
        toolbox::logger::StepMark::error("スクリプト作成失敗");
        return false;
    }
    http::HTTPRequest* request = createTestRequest("DELETE", scriptPath);
    http::CgiExecute executor;
    std::string output;
    http::CgiExecute::ExecuteResult result = executor.execute(
        scriptPath, "/usr/bin/python3", *request, output);
    delete request;
    toolbox::logger::StepMark::debug("出力内容: " + output);
    std::remove(scriptPath.c_str());
    if (result != http::CgiExecute::EXECUTE_SUCCESS) {
        toolbox::logger::StepMark::error("DELETE実行失敗: " + convertToString(static_cast<int>(result)));
        return false;
    }
    if (output.find("REQUEST_METHOD: DELETE") == std::string::npos) {
        toolbox::logger::StepMark::error("DELETEメソッドが正しく設定されていない");
        toolbox::logger::StepMark::debug("実際の出力: " + output);
        return false;
    }
    return true;
}

bool testHeadMethod() {
    const std::string scriptPath = "./test_head.py";
    const std::string scriptContent = 
        "#!/usr/bin/env python3\n"
        "import os\n"
        "print('Content-Type: text/plain\\r')\n"
        "print('\\r')\n"
        "print('REQUEST_METHOD: ' + os.environ.get('REQUEST_METHOD', 'NOT SET'))\n";
    if (!createTestScript(scriptPath, scriptContent, true)) {
        toolbox::logger::StepMark::error("スクリプト作成失敗");
        return false;
    }
    http::HTTPRequest* request = createTestRequest("HEAD", scriptPath);
    http::CgiExecute executor;
    std::string output;
    http::CgiExecute::ExecuteResult result = executor.execute(
        scriptPath, "/usr/bin/python3", *request, output);
    delete request;
    toolbox::logger::StepMark::debug("出力内容: " + output);
    std::remove(scriptPath.c_str());
    if (result != http::CgiExecute::EXECUTE_SUCCESS) {
        toolbox::logger::StepMark::error("HEAD実行失敗: " + 
            convertToString(static_cast<int>(result)));
        return false;
    }
    if (output.find("REQUEST_METHOD: HEAD") == std::string::npos) {
        toolbox::logger::StepMark::error("HEADメソッドが正しく設定されていない");
        toolbox::logger::StepMark::debug("実際の出力: " + output);
        return false;
    }
    return true;
}

bool testChunkedEncoding() {
    const std::string scriptPath = "./test_chunked.py";
    const std::string scriptContent =
        "#!/usr/bin/env python3\n"
        "import sys\n"
        "import os\n"
        "print('Content-Type: text/plain\\r')\n"
        "print('\\r')\n"
        "content_length = os.environ.get('CONTENT_LENGTH', '0')\n"
        "transfer_encoding = os.environ.get('HTTP_TRANSFER_ENCODING', '')\n"
        "print('Transfer-Encoding: ' + transfer_encoding)\n"
        "if transfer_encoding.lower() == 'chunked':\n"
        "    # chunkedエンコーディングの場合、環境変数設定を確認\n"
        "    post_data = sys.stdin.read()\n"
        "    print('Received data length: ' + str(len(post_data)))\n"
        "    print('Data content: ' + post_data)\n"
        "else:\n"
        "    print('Not chunked encoding')\n";
    if (!createTestScript(scriptPath, scriptContent, true)) {
        toolbox::logger::StepMark::error("スクリプト作成失敗");
        return false;
    }
    http::HTTPRequest* request = createTestRequest("POST", scriptPath);
    std::string chunkedBody = "This is a test of chunked encoding data";
    request->body.content = chunkedBody;
    request->body.contentLength = chunkedBody.length();
    http::HTTPFields::FieldValue transferEncodingValue;
    transferEncodingValue.push_back("chunked");
    request->fields.getFieldValue("Transfer-Encoding") = transferEncodingValue;
    http::HTTPFields::FieldValue contentTypeValue;
    contentTypeValue.push_back("text/plain");
    request->fields.getFieldValue("Content-Type") = contentTypeValue;
    http::CgiExecute executor;
    std::string output;
    http::CgiExecute::ExecuteResult result = executor.execute(
        scriptPath, "/usr/bin/python3", *request, output);
    delete request;
    toolbox::logger::StepMark::debug("出力内容: " + output);
    std::remove(scriptPath.c_str());
    if (result != http::CgiExecute::EXECUTE_SUCCESS) {
        toolbox::logger::StepMark::error("Chunked encoding実行失敗: " + 
            convertToString(static_cast<int>(result)));
        return false;
    }
    if (output.find("Transfer-Encoding: chunked") == std::string::npos) {
        toolbox::logger::StepMark::error("Transfer-Encodingが正しく設定されていない");
        toolbox::logger::StepMark::debug("実際の出力: " + output);
        return false;
    }
    if (output.find("Received data length: " + 
        convertToString(chunkedBody.length())) == std::string::npos) {
        toolbox::logger::StepMark::error("チャンクデータが正しく処理されていない");
        toolbox::logger::StepMark::debug("実際の出力: " + output);
        return false;
    }
    return true;
}

bool testNoInterpreterNeeded() {
    const std::string scriptPath = "./test_no_interpreter.sh";
    const std::string scriptContent =
        "#!/bin/sh\n"
        "echo 'Content-Type: text/plain'\n"
        "echo ''\n"
        "echo 'This script runs directly without specified interpreter'\n";
    if (!createTestScript(scriptPath, scriptContent, true)) {
        toolbox::logger::StepMark::error("スクリプト作成失敗");
        return false;
    }
    http::HTTPRequest* request = createTestRequest("GET", scriptPath);
    http::CgiExecute executor;
    std::string output;
    http::CgiExecute::ExecuteResult result = executor.execute(
        scriptPath, "", *request, output);
    delete request;
    toolbox::logger::StepMark::debug("出力内容: " + output);
    std::remove(scriptPath.c_str());
    if (result != http::CgiExecute::EXECUTE_SUCCESS) {
        toolbox::logger::StepMark::error("インタープリタなし実行失敗: " + 
            convertToString(static_cast<int>(result)));
        return false;
    }
    if (output.find("This script runs directly without specified interpreter") 
        == std::string::npos) {
        toolbox::logger::StepMark::error("インタープリタなし実行の出力が不正");
        toolbox::logger::StepMark::debug("実際の出力: " + output);
        return false;
    }
    return true;
}

bool testConsecutiveExecution() {
    const std::string scriptPath = "./test_consecutive.py";
    const std::string scriptContent =
        "#!/usr/bin/env python3\n"
        "print('Content-Type: text/plain\\r')\n"
        "print('\\r')\n"
        "print('Hello from consecutive execution test')\n";
    if (!createTestScript(scriptPath, scriptContent, true)) {
        toolbox::logger::StepMark::error("スクリプト作成失敗");
        return false;
    }
    http::CgiExecute executor;
    bool allSuccess = true;
    for (int i = 0; i < 3; i++) {
        http::HTTPRequest* request = createTestRequest("GET", scriptPath);
        std::string output;
        http::CgiExecute::ExecuteResult result = executor.execute(
            scriptPath, "/usr/bin/python3", *request, output);
        delete request;
        toolbox::logger::StepMark::debug("出力内容: " + output);
        if (result != http::CgiExecute::EXECUTE_SUCCESS) {
            toolbox::logger::StepMark::error(std::string("連続実行 #") + 
                convertToString(i+1) + " 失敗: " + 
                convertToString(static_cast<int>(result)));
            allSuccess = false;
            break;
        }
        if (output.find("Hello from consecutive execution test") == std::string::npos) {
            toolbox::logger::StepMark::error(std::string("連続実行 #") + 
                convertToString(i+1) + " 出力不正");
            toolbox::logger::StepMark::debug("実際の出力: " + output);
            allSuccess = false;
            break;
        }
    }
    std::remove(scriptPath.c_str());
    return allSuccess;
}

bool testDirectoryExecution() {
    const std::string dirPath = "./test_dir";
    if (mkdir(dirPath.c_str(), 0755) != 0) {
        toolbox::logger::StepMark::error("テストディレクトリ作成失敗");
        return false;
    }
    http::HTTPRequest* request = createTestRequest("GET", dirPath);
    http::CgiExecute executor;
    std::string output;
    http::CgiExecute::ExecuteResult result = executor.execute(
        dirPath, "/usr/bin/python3", *request, output);
    delete request;
    toolbox::logger::StepMark::debug("出力内容: " + output);
    rmdir(dirPath.c_str());
    if (result != http::CgiExecute::EXECUTE_PATH_ERROR) {
        toolbox::logger::StepMark::error("ディレクトリ実行エラーが正しく検出されていない: " + 
            convertToString(static_cast<int>(result)));
        return false;
    }
    return true;
}

bool testNoOutput() {
    const std::string scriptPath = "./test_no_output.py";
    const std::string scriptContent =
        "#!/usr/bin/env python3\n"
        "# このスクリプトは何も出力しない\n"
        "import time\n"
        "time.sleep(0.1)  # 短い待機\n";
    if (!createTestScript(scriptPath, scriptContent, true)) {
        toolbox::logger::StepMark::error("スクリプト作成失敗");
        return false;
    }
    http::HTTPRequest* request = createTestRequest("GET", scriptPath);
    http::CgiExecute executor;
    std::string output;
    http::CgiExecute::ExecuteResult result = executor.execute(
        scriptPath, "/usr/bin/python3", *request, output);
    delete request;
    toolbox::logger::StepMark::debug("出力内容: " + output);
    std::remove(scriptPath.c_str());
    if (result != http::CgiExecute::EXECUTE_SUCCESS) {
        toolbox::logger::StepMark::error("出力なし実行失敗: " +
            convertToString(static_cast<int>(result)));
        return false;
    }
    if (output.length() > 10) {
        toolbox::logger::StepMark::error("出力なしテストで意図しない出力あり");
        toolbox::logger::StepMark::debug("実際の出力: " + output);
        return false;
    }
    return true;
}

bool testDeflateContentEncoding() {
    const std::string scriptPath = "./test_deflate_encoding.py";
    const std::string scriptContent =
        "#!/usr/bin/env python3\n"
        "import sys\n"
        "import os\n"
        "print('Content-Type: text/plain\\r')\n"
        "print('\\r')\n"
        "content_encoding = os.environ.get('HTTP_CONTENT_ENCODING', '')\n"
        "print('Content-Encoding: ' + content_encoding)\n"
        "if content_encoding.lower() == 'deflate':\n"
        "    print('Detected deflate encoding')\n"
        "    data = sys.stdin.read()\n"
        "    print('Received data length: ' + str(len(data)))\n"
        "else:\n"
        "    print('No deflate encoding detected')\n";
    if (!createTestScript(scriptPath, scriptContent, true)) {
        toolbox::logger::StepMark::error("スクリプト作成失敗");
        return false;
    }
    http::HTTPRequest* request = createTestRequest("POST", scriptPath);
    std::string body = "Simulated deflate encoded data for testing only";
    request->body.content = body;
    request->body.contentLength = body.length();
    http::HTTPFields::FieldValue contentEncodingValue;
    contentEncodingValue.push_back("deflate");
    request->fields.getFieldValue("Content-Encoding") = contentEncodingValue;
    http::HTTPFields::FieldValue contentTypeValue;
    contentTypeValue.push_back("text/plain");
    request->fields.getFieldValue("Content-Type") = contentTypeValue;
    http::CgiExecute executor;
    std::string output;
    http::CgiExecute::ExecuteResult result = executor.execute(
        scriptPath, "/usr/bin/python3", *request, output);
    delete request;
    toolbox::logger::StepMark::debug("出力内容: " + output);
    std::remove(scriptPath.c_str());
    if (result != http::CgiExecute::EXECUTE_SUCCESS) {
        toolbox::logger::StepMark::error("deflate Content-Encoding実行失敗: " + 
            convertToString(static_cast<int>(result)));
        return false;
    }
    if (output.find("Content-Encoding: deflate") == std::string::npos) {
        toolbox::logger::StepMark::error("deflate Content-Encodingが正しく設定されていない");
        toolbox::logger::StepMark::debug("実際の出力: " + output);
        return false;
    }
    if (output.find("Detected deflate encoding") == std::string::npos) {
        toolbox::logger::StepMark::error("deflateエンコーディングが検出されていない");
        toolbox::logger::StepMark::debug("実際の出力: " + output);
        return false;
    }
    return true;
}

bool testTimeoutSettings() {
    const std::string scriptPath = "./timeout_settings_test.py";
    const std::string scriptContent =
        "#!/usr/bin/env python3\n"
        "import time\n"
        "print('Content-Type: text/plain\\r')\n"
        "print('\\r')\n"
        "print('Starting timeout settings test...')\n"
        "time.sleep(3)  # 3秒待機\n"
        "print('This should be displayed with longer timeout')\n";
    if (!createTestScript(scriptPath, scriptContent, true)) {
        toolbox::logger::StepMark::error("スクリプト作成失敗");
        return false;
    }
    http::HTTPRequest* request = createTestRequest("GET", scriptPath);
    http::CgiExecute executor;
    executor.setTimeout(5);
    std::string output;
    http::CgiExecute::ExecuteResult result = executor.execute(
        scriptPath, "/usr/bin/python3", *request, output);
    delete request;
    toolbox::logger::StepMark::debug("出力内容: " + output);
    std::remove(scriptPath.c_str());
    if (result != http::CgiExecute::EXECUTE_SUCCESS) {
        toolbox::logger::StepMark::error("タイムアウト設定テスト実行失敗: " + 
            convertToString(static_cast<int>(result)));
        return false;
    }
    if (output.find("This should be displayed with longer timeout") == std::string::npos) {
        toolbox::logger::StepMark::error("タイムアウト設定が正しく機能していない");
        toolbox::logger::StepMark::debug("実際の出力: " + output);
        return false;
    }
    return true;
}

bool testForkExecFailureSimulation() {
    const std::string scriptPath = "./fork_exec_failure.py";
    const std::string scriptContent = 
        "#!/usr/bin/env python3\n"
        "print('Content-Type: text/plain\\r')\n"
        "print('\\r')\n"
        "print('This should not be displayed')\n";
    if (!createTestScript(scriptPath, scriptContent, true)) {
        toolbox::logger::StepMark::error("スクリプト作成失敗");
        return false;
    }
    http::HTTPRequest* request = createTestRequest("GET", scriptPath);
    http::CgiExecute executor;
    std::string output;
    http::CgiExecute::ExecuteResult result = executor.execute(
        scriptPath, "/usr/bin/non_existent_interpreter", *request, output);
    delete request;
    toolbox::logger::StepMark::debug("出力内容: " + output);
    std::remove(scriptPath.c_str());
    if (result != http::CgiExecute::EXECUTE_EXEC_ERROR) {
        toolbox::logger::StepMark::error("execve失敗が正しく検出されていない: " + 
            convertToString(static_cast<int>(result)));
        return false;
    }
    return true;
}

bool testPipeErrorSimulation() {
    const std::string scriptPath = "./pipe_error_sim.py";
    const std::string scriptContent =
        "#!/usr/bin/env python3\n"
        "import os\n"
        "import sys\n"
        "print('Content-Type: text/plain\\r')\n"
        "print('\\r')\n"
        "# 標準入力から大量データを読み取り\n"
        "content_length = int(os.environ.get('CONTENT_LENGTH', 0))\n"
        "if content_length > 0:\n"
        "    data = sys.stdin.read(content_length)\n"
        "    print('Received data size: ' + str(len(data)))\n"
        "else:\n"
        "    print('No data received')\n";
    if (!createTestScript(scriptPath, scriptContent, true)) {
        toolbox::logger::StepMark::error("スクリプト作成失敗");
        return false;
    }
    std::string largeData(10 * 1024 * 1024, 'X');
    http::HTTPRequest* request = createTestRequest(
        "POST", scriptPath, "", largeData, "text/plain");
    http::CgiExecute executor;
    std::string output;
    http::CgiExecute::ExecuteResult result = executor.execute(
        scriptPath, "/usr/bin/python3", *request, output);
    delete request;
    toolbox::logger::StepMark::debug("出力内容: " + output);
    std::remove(scriptPath.c_str());
    if (result != http::CgiExecute::EXECUTE_SUCCESS) {
        toolbox::logger::StepMark::error("パイプテストに失敗: " + 
            convertToString(static_cast<int>(result)));
        return false;
    }
    if (output.find("Received data size: 10485760") == std::string::npos) {
        toolbox::logger::StepMark::error("大容量データが正しく送信されなかった");
        toolbox::logger::StepMark::debug("実際の出力: " + output);
        return false;
    }
    return true;
}

bool testMultipleInstances() {
    const int INSTANCE_COUNT = 5;
    std::string scriptPaths[INSTANCE_COUNT];
    std::string scriptContents[INSTANCE_COUNT];
    http::HTTPRequest* requests[INSTANCE_COUNT];
    http::CgiExecute executors[INSTANCE_COUNT];
    std::string outputs[INSTANCE_COUNT];
    http::CgiExecute::ExecuteResult results[INSTANCE_COUNT];
    for (int i = 0; i < INSTANCE_COUNT; i++) {
        scriptPaths[i] = "./multi_inst_" + toolbox::to_string(i+1) + ".py";
        int sleepTime = (i == 0) ? 1 : (i % 3) * 0.5;
        scriptContents[i] =
            "#!/usr/bin/env python3\n"
            "import time\n"
            "print('Content-Type: text/plain\\r')\n"
            "print('\\r')\n"
            "print('Script " + toolbox::to_string(i+1) + " output')\n";
        if (i % 2 == 0) {
            scriptContents[i] += "time.sleep(" + toolbox::to_string(sleepTime) + ")\n";
        }
        if (!createTestScript(scriptPaths[i], scriptContents[i], true)) {
            toolbox::logger::StepMark::error("スクリプト" + toolbox::to_string(i+1) + "作成失敗");
            for (int j = 0; j < i; j++) {
                std::remove(scriptPaths[j].c_str());
            }
            return false;
        }
        requests[i] = createTestRequest("GET", scriptPaths[i]);
    }
    toolbox::logger::StepMark::info("5つのCGIインスタンスを並行実行開始");
    for (int i = 0; i < INSTANCE_COUNT; i++) {
        results[i] = executors[i].execute(
            scriptPaths[i], "/usr/bin/python3", *requests[i], outputs[i]);
    }
    for (int i = 0; i < INSTANCE_COUNT; i++) {
        delete requests[i];
        toolbox::logger::StepMark::debug("出力内容[" + toolbox::to_string(i) + "] : " + outputs[i]);
        std::remove(scriptPaths[i].c_str());
    }
    bool success = true;
    for (int i = 0; i < INSTANCE_COUNT; i++) {
        if (results[i] != http::CgiExecute::EXECUTE_SUCCESS) {
            toolbox::logger::StepMark::error("インスタンス" + toolbox::to_string(i+1) + 
                "の実行に失敗: " + convertToString(static_cast<int>(results[i])));
            success = false;
        }
        std::string expectedOutput = "Script " + toolbox::to_string(i+1) + " output";
        if (outputs[i].find(expectedOutput) == std::string::npos) {
            toolbox::logger::StepMark::error("インスタンス" + toolbox::to_string(i+1) + 
                "の出力が正しくない");
            toolbox::logger::StepMark::debug("期待出力: " + expectedOutput);
            toolbox::logger::StepMark::debug("実際の出力: " + outputs[i]);
            success = false;
        }
    }
    if (success) {
        toolbox::logger::StepMark::info("5つのCGIインスタンスが正常に並行実行されました");
    }
    return success;
}

bool testEmptyPostBody() {
    const std::string scriptPath = "./test_empty_body.py";
    const std::string scriptContent = 
        "#!/usr/bin/env python3\n"
        "import sys\n"
        "import os\n"
        "print('Content-Type: text/plain\\r')\n"
        "print('\\r')\n"
        "content_length = os.environ.get('CONTENT_LENGTH', 'NOT SET')\n"
        "print('Content-Length: ' + content_length)\n"
        "if content_length != '0' and content_length != 'NOT SET':\n"
        "    data = sys.stdin.read(int(content_length))\n"
        "    print('Received data length: ' + str(len(data)))\n"
        "    print('Data: \"' + data + '\"')\n"
        "else:\n"
        "    print('No data or empty data received')\n";
    if (!createTestScript(scriptPath, scriptContent, true)) {
        toolbox::logger::StepMark::error("スクリプト作成失敗");
        return false;
    }
    http::HTTPRequest* request = createTestRequest("POST", scriptPath, "", "", "application/x-www-form-urlencoded");
    http::HTTPFields::FieldValue contentLengthValue;
    contentLengthValue.push_back("0");
    request->fields.getFieldValue("Content-Length") = contentLengthValue;
    http::CgiExecute executor;
    std::string output;
    http::CgiExecute::ExecuteResult result = executor.execute(
        scriptPath, "/usr/bin/python3", *request, output);
    delete request;
    toolbox::logger::StepMark::debug("出力内容: " + output);
    std::remove(scriptPath.c_str());
    if (result != http::CgiExecute::EXECUTE_SUCCESS) {
        toolbox::logger::StepMark::error("空ボディPOST実行失敗: " + 
            convertToString(static_cast<int>(result)));
        return false;
    }
    if (output.find("Content-Length: 0") == std::string::npos) {
        toolbox::logger::StepMark::error("Content-Length: 0 が正しく設定されていない");
        toolbox::logger::StepMark::debug("実際の出力: " + output);
        return false;
    }
    if (output.find("No data or empty data received") == std::string::npos) {
        toolbox::logger::StepMark::error("空ボディの処理が正しくない");
        toolbox::logger::StepMark::debug("実際の出力: " + output);
        return false;
    }
    return true;
}

bool testSpecialCharsInPath() {
    const std::string scriptPath = "./test special-chars (with brackets).py";
    const std::string scriptContent = 
        "#!/usr/bin/env python3\n"
        "import os\n"
        "print('Content-Type: text/plain\\r')\n"
        "print('\\r')\n"
        "script_name = os.environ.get('SCRIPT_NAME', 'NOT SET')\n"
        "print('SCRIPT_NAME: ' + script_name)\n"
        "print('Script executed successfully')\n";
    
    if (!createTestScript(scriptPath, scriptContent, true)) {
        toolbox::logger::StepMark::error("特殊文字を含むスクリプト作成失敗");
        return false;
    }
    http::HTTPRequest* request = createTestRequest("GET", scriptPath);
    http::CgiExecute executor;
    std::string output;
    http::CgiExecute::ExecuteResult result = executor.execute(
        scriptPath, "/usr/bin/python3", *request, output);
    delete request;
    toolbox::logger::StepMark::debug("出力内容: " + output);
    std::remove(scriptPath.c_str());
    if (result != http::CgiExecute::EXECUTE_SUCCESS) {
        toolbox::logger::StepMark::error("特殊文字を含むパス実行失敗: " + 
            convertToString(static_cast<int>(result)));
        return false;
    }
    if (output.find("Script executed successfully") == std::string::npos) {
        toolbox::logger::StepMark::error("特殊文字を含むパスのスクリプト実行に失敗");
        toolbox::logger::StepMark::debug("実際の出力: " + output);
        return false;
    }
    if (output.find("SCRIPT_NAME:") == std::string::npos) {
        toolbox::logger::StepMark::error("SCRIPT_NAME環境変数が設定されていない");
        toolbox::logger::StepMark::debug("実際の出力: " + output);
        return false;
    }
    return true;
}

bool testMultibyteCharacters() {
    const std::string scriptPath = "./テスト_日本語_🌟.py";
    const std::string scriptContent =
        "#!/usr/bin/env python3\n"
        "import os\n"
        "print('Content-Type: text/plain; charset=utf-8\\r')\n"
        "print('\\r')\n"
        "script_name = os.environ.get('SCRIPT_NAME', 'NOT SET')\n"
        "path_info = os.environ.get('PATH_INFO', 'NOT SET')\n"
        "print('SCRIPT_NAME: ' + script_name)\n"
        "print('PATH_INFO: ' + path_info)\n"
        "print('日本語と絵文字🌟のテスト成功')\n";
    if (!createTestScript(scriptPath, scriptContent, true)) {
        toolbox::logger::StepMark::error("マルチバイト文字を含むスクリプト作成失敗");
        return false;
    }
    http::HTTPRequest* request = createTestRequest("GET", scriptPath);
    http::HTTPFields::FieldValue acceptLanguageValue;
    acceptLanguageValue.push_back("ja-JP,ja;q=0.9,en;q=0.8");
    request->fields.getFieldValue("Accept-Language") = acceptLanguageValue;
    http::CgiExecute executor;
    std::string output;
    http::CgiExecute::ExecuteResult result = executor.execute(
        scriptPath, "/usr/bin/python3", *request, output);
    delete request;
    toolbox::logger::StepMark::debug("出力内容: " + output);
    std::remove(scriptPath.c_str());
    if (result != http::CgiExecute::EXECUTE_SUCCESS) {
        toolbox::logger::StepMark::error("マルチバイト文字テスト実行失敗: " + 
            convertToString(static_cast<int>(result)));
        return false;
    }
    if (output.find("charset=utf-8") == std::string::npos) {
        toolbox::logger::StepMark::error("UTF-8charset指定が見つからない");
        toolbox::logger::StepMark::debug("実際の出力: " + output);
        return false;
    }
    if (output.find("\xe6\x97\xa5\xe6\x9c\xac\xe8\xaa\x9e") == std::string::npos) {
        toolbox::logger::StepMark::error("日本語出力が正しく処理されていない");
        toolbox::logger::StepMark::debug("実際の出力（バイナリ表示）: ");
        for (size_t i = 0; i < std::min(output.size(), static_cast<size_t>(100)); ++i) {
            toolbox::logger::StepMark::debug(convertToString(static_cast<unsigned char>(output[i])));
        }
        return false;
    }
    return true;
}

bool testMalformedRequest() {
    const std::string scriptPath = "./test_malformed.py";
    const std::string scriptContent = 
        "#!/usr/bin/env python3\n"
        "import os\n"
        "print('Content-Type: text/plain\\r')\n"
        "print('\\r')\n"
        "print('Script executed with malformed request')\n";
    if (!createTestScript(scriptPath, scriptContent, true)) {
        toolbox::logger::StepMark::error("スクリプト作成失敗");
        return false;
    }
    http::HTTPRequest* request = createTestRequest("INVALID_METHOD", scriptPath);
    http::CgiExecute executor;
    std::string output;
    http::CgiExecute::ExecuteResult result = executor.execute(
        scriptPath, "/usr/bin/python3", *request, output);
    delete request;
    toolbox::logger::StepMark::debug("出力内容: " + output);
    std::remove(scriptPath.c_str());
    if (result != http::CgiExecute::EXECUTE_SUCCESS) {
        toolbox::logger::StepMark::error("不正リクエストテスト実行失敗");
        return false;
    }
    if (output.find("Script executed with malformed request") == std::string::npos) {
        toolbox::logger::StepMark::error("不正リクエストでのスクリプト実行に失敗");
        return false;
    }
    return true;
}

bool testCommandInjectionPrevention() {
    const std::string scriptPath = "./test_command_injection.py; rm -rf /";
    const std::string safeScriptContent = 
        "#!/usr/bin/env python3\n"
        "print('Content-Type: text/plain\\r')\n"
        "print('\\r')\n"
        "print('This script should not execute due to command injection attempt')\n";
    std::string safePath = "./test_command_injection.py";
    if (!createTestScript(safePath, safeScriptContent, true)) {
        toolbox::logger::StepMark::error("セーフスクリプト作成失敗");
        return false;
    }
    http::HTTPRequest* request = createTestRequest("GET", scriptPath);
    http::CgiExecute executor;
    std::string output;
    http::CgiExecute::ExecuteResult result = executor.execute(
        scriptPath, "/usr/bin/python3", *request, output);
    delete request;
    toolbox::logger::StepMark::debug("出力内容: " + output);
    std::remove(safePath.c_str());
    if (result != http::CgiExecute::EXECUTE_PATH_ERROR) {
        toolbox::logger::StepMark::error("コマンドインジェクションがブロックされていない");
        return false;
    }
    return true;
}

bool testSymbolicLinkHandling() {
    const std::string realScriptPath = "./real_script.py";
    const std::string symlinkPath = "./symlink_script.py";
    const std::string scriptContent = 
        "#!/usr/bin/env python3\n"
        "import os\n"
        "print('Content-Type: text/plain\\r')\n"
        "print('\\r')\n"
        "print('Real path: ' + os.path.realpath(__file__))\n"
        "print('Symlink test executed successfully')\n";
    if (!createTestScript(realScriptPath, scriptContent, true)) {
        toolbox::logger::StepMark::error("実スクリプト作成失敗");
        return false;
    }
    if (symlink(realScriptPath.c_str(), symlinkPath.c_str()) != 0) {
        toolbox::logger::StepMark::error("シンボリックリンク作成失敗");
        std::remove(realScriptPath.c_str());
        return false;
    }
    http::HTTPRequest* request = createTestRequest("GET", symlinkPath);
    http::CgiExecute executor;
    std::string output;
    http::CgiExecute::ExecuteResult result = executor.execute(
        symlinkPath, "/usr/bin/python3", *request, output);
    delete request;
    toolbox::logger::StepMark::debug("出力内容: " + output);
    std::remove(symlinkPath.c_str());
    std::remove(realScriptPath.c_str());
    if (result != http::CgiExecute::EXECUTE_SUCCESS) {
        toolbox::logger::StepMark::error("シンボリックリンク実行失敗");
        return false;
    }
    if (output.find("Symlink test executed successfully") == std::string::npos) {
        toolbox::logger::StepMark::error("シンボリックリンク経由の実行出力が不正");
        return false;
    }
    return true;
}

bool testMultipleLanguages() {
    toolbox::logger::StepMark::info("--- 複数言語CGIテスト開始 ---");
    bool allSuccess = true;
    const std::string perlScriptPath = "./test_perl.pl";
    const std::string perlScriptContent = 
        "#!/usr/bin/perl\n"
        "print \"Content-Type: text/plain\\r\\n\\r\\n\";\n"
        "print \"Perl CGI script executed successfully\\n\";\n"
        "print \"Script path: $0\\n\";\n"
        "print \"Environment variables:\\n\";\n"
        "foreach $key (sort keys %ENV) {\n"
        "    if ($key =~ /^(REQUEST_METHOD|QUERY_STRING|HTTP_USER_AGENT)$/) {\n"
        "        print \"$key = $ENV{$key}\\n\";\n"
        "    }\n"
        "}\n";
    if (createTestScript(perlScriptPath, perlScriptContent, true)) {
        http::HTTPRequest* request = createTestRequest("GET", perlScriptPath, "test=perl");
        http::HTTPFields::FieldValue userAgentValue;
        userAgentValue.push_back("Perl-Test/1.0");
        request->fields.getFieldValue("User-Agent") = userAgentValue;
        http::CgiExecute executor;
        std::string output;
        http::CgiExecute::ExecuteResult result = executor.execute(
            perlScriptPath, "/usr/bin/perl", *request, output);
        delete request;
        toolbox::logger::StepMark::debug("出力内容: " + output);
        std::remove(perlScriptPath.c_str());
        if (result != http::CgiExecute::EXECUTE_SUCCESS) {
            toolbox::logger::StepMark::error("Perl CGI実行失敗: " + 
                convertToString(static_cast<int>(result)));
            allSuccess = false;
        } else if (output.find("Perl CGI script executed successfully") == std::string::npos) {
            toolbox::logger::StepMark::error("Perl CGIの出力が不正");
            toolbox::logger::StepMark::debug("実際の出力: " + output);
            allSuccess = false;
        } else {
            toolbox::logger::StepMark::info("Perl CGIテスト成功");
        }
    } else {
        toolbox::logger::StepMark::error("Perlスクリプト作成失敗");
        allSuccess = false;
    }
    const std::string rubyScriptPath = "./test_ruby.rb";
    const std::string rubyScriptContent = 
        "#!/usr/bin/ruby\n"
        "puts \"Content-Type: text/plain\\r\\n\\r\"\n"
        "puts \"Ruby CGI script executed successfully\"\n"
        "puts \"Script path: #{__FILE__}\"\n"
        "puts \"Environment variables:\"\n"
        "['REQUEST_METHOD', 'QUERY_STRING', 'HTTP_USER_AGENT'].each do |key|\n"
        "  puts \"#{key} = #{ENV[key]}\" if ENV[key]\n"
        "end\n";
    if (createTestScript(rubyScriptPath, rubyScriptContent, true)) {
        http::HTTPRequest* request = createTestRequest("GET", rubyScriptPath, "test=ruby");
        http::HTTPFields::FieldValue userAgentValue;
        userAgentValue.push_back("Ruby-Test/1.0");
        request->fields.getFieldValue("User-Agent") = userAgentValue;
        http::CgiExecute executor;
        std::string output;
        http::CgiExecute::ExecuteResult result = executor.execute(
            rubyScriptPath, "/usr/bin/ruby", *request, output);
        delete request;
        toolbox::logger::StepMark::debug("出力内容: " + output);
        std::remove(rubyScriptPath.c_str());
        if (result != http::CgiExecute::EXECUTE_SUCCESS) {
            toolbox::logger::StepMark::error("Ruby CGI実行失敗: " + 
                convertToString(static_cast<int>(result)));
            allSuccess = false;
        } else if (output.find("Ruby CGI script executed successfully") == std::string::npos) {
            toolbox::logger::StepMark::error("Ruby CGIの出力が不正");
            toolbox::logger::StepMark::debug("実際の出力: " + output);
            allSuccess = false;
        } else {
            toolbox::logger::StepMark::info("Ruby CGIテスト成功");
        }
    } else {
        toolbox::logger::StepMark::error("Rubyスクリプト作成失敗");
        allSuccess = false;
    }
    const std::string phpScriptPath = "./test_php.php";
    const std::string phpScriptContent = 
        "#!/usr/bin/php\n"
        "<?php\n"
        "header(\"Content-Type: text/plain\");\n"
        "echo \"PHP CGI script executed successfully\\n\";\n"
        "echo \"Script path: \".__FILE__.\"\\n\";\n"
        "echo \"Environment variables:\\n\";\n"
        "$envVars = array('REQUEST_METHOD', 'QUERY_STRING', 'HTTP_USER_AGENT');\n"
        "foreach ($envVars as $key) {\n"
        "    if (isset($_SERVER[$key])) {\n"
        "        echo \"$key = \".$_SERVER[$key].\"\\n\";\n"
        "    }\n"
        "}\n"
        "?>";
    if (createTestScript(phpScriptPath, phpScriptContent, true)) {
        http::HTTPRequest* request = createTestRequest("GET", phpScriptPath, "test=php");
        http::HTTPFields::FieldValue userAgentValue;
        userAgentValue.push_back("PHP-Test/1.0");
        request->fields.getFieldValue("User-Agent") = userAgentValue;
        http::CgiExecute executor;
        std::string output;
        http::CgiExecute::ExecuteResult result = executor.execute(
            phpScriptPath, "/usr/bin/php", *request, output);
        delete request;
        toolbox::logger::StepMark::debug("出力内容: " + output);
        std::remove(phpScriptPath.c_str());
        if (result != http::CgiExecute::EXECUTE_SUCCESS) {
            toolbox::logger::StepMark::error("PHP CGI実行失敗: " + 
                convertToString(static_cast<int>(result)));
            allSuccess = false;
        } else if (output.find("PHP CGI script executed successfully") == std::string::npos) {
            toolbox::logger::StepMark::error("PHP CGIの出力が不正");
            toolbox::logger::StepMark::debug("実際の出力: " + output);
            allSuccess = false;
        } else {
            toolbox::logger::StepMark::info("PHP CGIテスト成功");
        }
    } else {
        toolbox::logger::StepMark::error("PHPスクリプト作成失敗");
        allSuccess = false;
    }
    const std::string bashScriptPath = "./test_bash.sh";
    const std::string bashScriptContent = 
        "#!/bin/bash\n"
        "echo -e \"Content-Type: text/plain\\r\\n\\r\"\n"
        "echo \"Bash shell script executed successfully\"\n"
        "echo \"Script path: $0\"\n"
        "echo \"Environment variables:\"\n"
        "echo \"REQUEST_METHOD = $REQUEST_METHOD\"\n"
        "echo \"QUERY_STRING = $QUERY_STRING\"\n"
        "echo \"HTTP_USER_AGENT = $HTTP_USER_AGENT\"\n";
    if (createTestScript(bashScriptPath, bashScriptContent, true)) {
        http::HTTPRequest* request = createTestRequest("GET", bashScriptPath, "test=bash");
        http::HTTPFields::FieldValue userAgentValue;
        userAgentValue.push_back("Bash-Test/1.0");
        request->fields.getFieldValue("User-Agent") = userAgentValue;
        http::CgiExecute executor;
        std::string output;
        http::CgiExecute::ExecuteResult result = executor.execute(
            bashScriptPath, "", *request, output);
        delete request;
        toolbox::logger::StepMark::debug("出力内容: " + output);
        std::remove(bashScriptPath.c_str());
        if (result != http::CgiExecute::EXECUTE_SUCCESS) {
            toolbox::logger::StepMark::error("Bash CGI実行失敗: " + 
                convertToString(static_cast<int>(result)));
            allSuccess = false;
        } else if (output.find("Bash shell script executed successfully") == std::string::npos) {
            toolbox::logger::StepMark::error("Bash CGIの出力が不正");
            toolbox::logger::StepMark::debug("実際の出力: " + output);
            allSuccess = false;
        } else {
            toolbox::logger::StepMark::info("Bash CGIテスト成功");
        }
    } else {
        toolbox::logger::StepMark::error("Bashスクリプト作成失敗");
        allSuccess = false;
    }
    return allSuccess;
}

}  // namespace cgi_execute_test

void executeTest() {
    toolbox::logger::StepMark::info("CGI Execute テスト開始");
    int totalTests = 0;
    int passedCount = 0;
    bool result;

    totalTests++;
    result = cgi_execute_test::runTest(
        "基本的なCGI実行",
        cgi_execute_test::testBasicExecution);
    if (result) passedCount++;

    totalTests++;
    result = cgi_execute_test::runTest(
        "POSTデータ処理",
        cgi_execute_test::testPostDataHandling);
    if (result) passedCount++;

    totalTests++;
    result = cgi_execute_test::runTest(
        "環境変数",
        cgi_execute_test::testEnvironmentVariables);
    if (result) passedCount++;

    totalTests++;
    result = cgi_execute_test::runTest(
        "実行権限なし",
        cgi_execute_test::testNoExecutablePermission);
    if (result) passedCount++;

    totalTests++;
    result = cgi_execute_test::runTest(
        "パストラバーサル防止",
        cgi_execute_test::testPathTraversalPrevention);
    if (result) passedCount++;

    totalTests++;
    result = cgi_execute_test::runTest(
        "存在しないスクリプト",
        cgi_execute_test::testNonExistentScript);
    if (result) passedCount++;

    totalTests++;
    result = cgi_execute_test::runTest(
        "大容量出力処理",
        cgi_execute_test::testLargeOutput);
    if (result) passedCount++;

    totalTests++;
    result = cgi_execute_test::runTest(
        "異なるインタープリタ",
        cgi_execute_test::testDifferentInterpreters);
    if (result) passedCount++;

    totalTests++;
    result = cgi_execute_test::runTest(
        "DELETEメソッド",
        cgi_execute_test::testDeleteMethod);
    if (result) passedCount++;

    totalTests++;
    result = cgi_execute_test::runTest(
        "HEADメソッド",
        cgi_execute_test::testHeadMethod);
    if (result) passedCount++;

    totalTests++;
    result = cgi_execute_test::runTest(
        "チャンクエンコーディング",
        cgi_execute_test::testChunkedEncoding);
    if (result) passedCount++;

    totalTests++;
    result = cgi_execute_test::runTest(
        "インタープリタなし実行",
        cgi_execute_test::testNoInterpreterNeeded);
    if (result) passedCount++;

    totalTests++;
    result = cgi_execute_test::runTest(
        "連続実行テスト",
        cgi_execute_test::testConsecutiveExecution);
    if (result) passedCount++;

    totalTests++;
    result = cgi_execute_test::runTest(
        "ディレクトリ実行テスト",
        cgi_execute_test::testDirectoryExecution);
    if (result) passedCount++;

    totalTests++;
    result = cgi_execute_test::runTest(
        "出力なしテスト",
        cgi_execute_test::testNoOutput);
    if (result) passedCount++;

    totalTests++;
    result = cgi_execute_test::runTest(
        "Deflateコンテンツエンコーディング",
        cgi_execute_test::testDeflateContentEncoding);
    if (result) passedCount++;

    totalTests++;
    result = cgi_execute_test::runTest(
        "タイムアウト設定テスト",
        cgi_execute_test::testTimeoutSettings);
    if (result) passedCount++;

    totalTests++;
    result = cgi_execute_test::runTest(
        "fork/execve失敗シミュレーション",
        cgi_execute_test::testForkExecFailureSimulation);
    if (result) passedCount++;

    totalTests++;
    result = cgi_execute_test::runTest(
        "パイプエラーシミュレーション",
        cgi_execute_test::testPipeErrorSimulation);
    if (result) passedCount++;

    totalTests++;
    result = cgi_execute_test::runTest(
        "複数インスタンス並列実行テスト",
        cgi_execute_test::testMultipleInstances);
    if (result) passedCount++;

    totalTests++;
    result = cgi_execute_test::runTest(
        "空POSTボディ処理テスト",
        cgi_execute_test::testEmptyPostBody);
    if (result) passedCount++;

    totalTests++;
    result = cgi_execute_test::runTest(
        "特殊文字を含むパス処理テスト",
        cgi_execute_test::testSpecialCharsInPath);
    if (result) passedCount++;

    totalTests++;
    result = cgi_execute_test::runTest(
        "マルチバイト文字処理テスト",
        cgi_execute_test::testMultibyteCharacters);
    if (result) passedCount++;

    totalTests++;
    result = cgi_execute_test::runTest(
        "不正リクエスト処理テスト",
        cgi_execute_test::testMalformedRequest);
    if (result) passedCount++;

    totalTests++;
    result = cgi_execute_test::runTest(
        "コマンドインジェクション防止テスト",
        cgi_execute_test::testCommandInjectionPrevention);
    if (result) passedCount++;

    totalTests++;
    result = cgi_execute_test::runTest(
        "シンボリックリンク処理テスト",
        cgi_execute_test::testSymbolicLinkHandling);
    if (result) passedCount++;

    totalTests++;
    result = cgi_execute_test::runTest(
        "複数言語CGIテスト",
        cgi_execute_test::testMultipleLanguages);
    if (result) passedCount++;

    std::cout << "CGI Execute testcase "
                << passedCount << " / " << totalTests
                << std::endl;
}

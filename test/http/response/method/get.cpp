#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <cassert>
#include <iostream>
#include <string>
#include <cstdio>
#include <fstream>
#include <map>

#include "../../../../src/http/http_status.hpp"
#include "../../../../src/http/case_insensitive_less.hpp"

typedef std::map<std::string, std::string, http::CaseInsensitiveLess>
ExtentionMap;

http::HttpStatus executeGet(const std::string& path, std::string& responseBody,
    const std::string& indexPath, bool isAutoindex, std::string& contentType,
    ExtentionMap& extensionMap);
void initExtensionMap(ExtentionMap& extensionMap);

void setupTestEnvironment() {
    mkdir("./test_dir", 0755);
    mkdir("./test_dir/empty_dir", 0755);
    mkdir("./test_dir/no_permission_dir", 0000);

    std::ofstream test_file("./test_dir/test.html");
    test_file << "<html><body><h1>Test HTML</h1></body></html>";
    test_file.close();

    std::ofstream index_file("./test_dir/index.html");
    index_file << "<html><body><h1>Index HTML</h1></body></html>";
    index_file.close();

    std::ofstream no_perm_file("./test_dir/no_permission.html");
    no_perm_file << "<html><body><h1>No Permission</h1></body></html>";
    no_perm_file.close();
    chmod("./test_dir/no_permission.html", 0000);
}

void cleanupTestEnvironment() {
    chmod("./test_dir/no_permission.html", 0644);
    chmod("./test_dir/no_permission_dir", 0755);

    remove("./test_dir/test.html");
    remove("./test_dir/index.html");
    remove("./test_dir/no_permission.html");

    rmdir("./test_dir/empty_dir");
    rmdir("./test_dir/no_permission_dir");
    rmdir("./test_dir");
}

void runTests() {
    std::string responseBody;
    std::string contentType;
    ExtentionMap extensionMap;
    initExtensionMap(extensionMap);
    http::HttpStatus status;

    std::cout << "===== get nomal file =====" << std::endl;
    responseBody.clear();
    contentType.clear();
    status = executeGet("./test_dir/test.html", responseBody, "", false,
        contentType, extensionMap);
    assert(status == http::OK);
    assert(contentType == "text/html");
    assert(responseBody.find("Test HTML") != std::string::npos);
    std::cout << "OK: " << status << std::endl;

    std::cout << "===== get nonexistent file =====" << std::endl;
    responseBody.clear();
    contentType.clear();
    status = executeGet("./test_dir/nonexistent.html", responseBody, "", false,
        contentType, extensionMap);
    assert(status == http::NOT_FOUND);
    std::cout << "OK: " << status << std::endl;

    std::cout << "===== get dir indexfile =====" << std::endl;
    responseBody.clear();
    contentType.clear();
    status = executeGet("./test_dir/", responseBody, "index.html", false,
        contentType, extensionMap);
    assert(status == http::OK);
    assert(contentType == "text/html");
    assert(responseBody.find("Index HTML") != std::string::npos);
    std::cout << "OK: " << status << std::endl;

    std::cout << "===== get dir autoindex =====" << std::endl;
    responseBody.clear();
    contentType.clear();
    status = executeGet("./test_dir/", responseBody, "", true,
        contentType, extensionMap);
    assert(status == http::OK);
    assert(contentType == "text/html");
    assert(responseBody.find("Index of") != std::string::npos);
    std::cout << "OK: " << status << std::endl;

    std::cout << "===== get no permission =====" << std::endl;
    responseBody.clear();
    contentType.clear();
    status = executeGet("./test_dir/no_permission.html", responseBody, "",
        false, contentType, extensionMap);
    assert(status == http::FORBIDDEN);
    std::cout << "OK: " << status << std::endl;

    std::cout << "===== get empty dir ====="
        << std::endl;
    responseBody.clear();
    contentType.clear();
    status = executeGet("./test_dir/empty_dir/", responseBody, "", true,
        contentType, extensionMap);
    assert(status == http::OK);
    assert(contentType == "text/html");
    std::cout << "OK: " << status << std::endl;

    std::cout << "===== get empty dir indexfile =====" << std::endl;
    responseBody.clear();
    contentType.clear();
    status = executeGet("./test_dir/empty_dir/", responseBody, "index.html",
        false, contentType, extensionMap);
    assert(status == http::NOT_FOUND);
    std::cout << "OK: " << status << std::endl;

    std::cout << "===== get no permission dir =====" << std::endl;
    responseBody.clear();
    contentType.clear();
    status = executeGet("./test_dir/no_permission_dir/", responseBody, "",
        true, contentType, extensionMap);
    assert(status == http::FORBIDDEN);
    std::cout << "OK: " << status << std::endl;
}

void get_test() {
    try {
        setupTestEnvironment();
        runTests();
        cleanupTestEnvironment();
        std::cout << "OK" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "FAILED" << e.what() << std::endl;
        cleanupTestEnvironment();
    }
}
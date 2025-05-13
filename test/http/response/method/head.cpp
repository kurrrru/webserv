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
#include "../../../../src/http/request/http_fields.hpp"
#include "../../../../src/http/response/method_utils.hpp"
#include "../../../../src/http/response/head_method.hpp"
#include "../../../../src/http/response/response.hpp"
#include "../../../../src/http/http_namespace.hpp"

static void setupTestEnvironment() {
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

static void cleanupTestEnvironment() {
    chmod("./test_dir/no_permission.html", 0644);
    chmod("./test_dir/no_permission_dir", 0755);

    remove("./test_dir/test.html");
    remove("./test_dir/index.html");
    remove("./test_dir/no_permission.html");

    rmdir("./test_dir/empty_dir");
    rmdir("./test_dir/no_permission_dir");
    rmdir("./test_dir");
}

static void runTests() {
    http::ExtensionMap extensionMap;
    http::HttpStatus::EHttpStatus status;
    http::Response response;

    std::cout << "===== head normal file (modified time) =====" << std::endl;
    status = http::runHead("./test_dir/test.html", "", false,
        extensionMap, response);
    assert(status == http::HttpStatus::OK);
    std::cout << "OK: " << status << std::endl;

    std::cout << "===== head nonexistent file =====" << std::endl;
    status = http::runHead("./test_dir/nonexistent.html", "", false,
        extensionMap, response);
    assert(status == http::HttpStatus::NOT_FOUND);
    std::cout << "OK: " << status << std::endl;

    std::cout << "===== head dir indexfile (modified time) =====" << std::endl;
    status = http::runHead("./test_dir/", "index.html", false,
        extensionMap, response);
    assert(status == http::HttpStatus::OK);
    std::cout << "OK: " << status << std::endl;

    std::cout << "===== head dir autoindex =====" << std::endl;
    status = http::runHead("./test_dir/", "", true,
        extensionMap, response);
    assert(status == http::HttpStatus::OK);
    std::cout << "OK: " << status << std::endl;

    std::cout << "===== head no permission =====" << std::endl;
    status = http::runHead("./test_dir/no_permission.html", "",
        false, extensionMap, response);
    assert(status == http::HttpStatus::FORBIDDEN);
    std::cout << "OK: " << status << std::endl;

    std::cout << "===== head empty dir ====="
        << std::endl;
    status = http::runHead("./test_dir/empty_dir/", "",
        true, extensionMap, response);
    assert(status == http::HttpStatus::OK);
    std::cout << "OK: " << status << std::endl;

    std::cout << "===== head empty dir indexfile =====" << std::endl;
    status = http::runHead("./test_dir/empty_dir/", "index.html",
        false, extensionMap, response);
    assert(status == http::HttpStatus::NOT_FOUND);
    std::cout << "OK: " << status << std::endl;

    std::cout << "===== head no permission dir =====" << std::endl;
    status = http::runHead("./test_dir/no_permission_dir/", "",
        true, extensionMap, response);
    assert(status == http::HttpStatus::FORBIDDEN);
    std::cout << "OK: " << status << std::endl;
}

void head_test() {
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
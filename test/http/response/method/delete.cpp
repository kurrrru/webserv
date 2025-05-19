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
#include "../../../../src/http/response/delete_method.hpp"
#include "../../../../src/http/response/response.hpp"
#include "../../../../src/http/http_namespace.hpp"


static void setupTestEnvironment() {
    mkdir("./test_dir", 0755);

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

    remove("./test_dir/test.html");
    remove("./test_dir/index.html");
    remove("./test_dir/no_permission.html");

    rmdir("./test_dir");
}

static void runTests() {
    http::Response response;

    std::string path = "./test_dir/test.html";
    toolbox::logger::StepMark::info("===== delete normal file =====");
    toolbox::logger::StepMark::info("except status: " +
        toolbox::to_string(http::HttpStatus::NO_CONTENT));
    http::runDelete(path, response);

    path = "./test_dir/nonexistent.html";
    toolbox::logger::StepMark::info("===== delete nonexistent file =====");
    toolbox::logger::StepMark::info("except status: " +
        toolbox::to_string(http::HttpStatus::NOT_FOUND));
    http::runDelete(path, response);

    path = "./test_dir/no_permission.html";
    toolbox::logger::StepMark::info("===== delete no_permission file =====");
    toolbox::logger::StepMark::info("except status: " +
        toolbox::to_string(http::HttpStatus::FORBIDDEN));
    http::runDelete(path, response);

    path = "./test_dir";
    toolbox::logger::StepMark::info("===== delete dir =====");
    toolbox::logger::StepMark::info("except status: " +
        toolbox::to_string(http::HttpStatus::FORBIDDEN));
    http::runDelete(path, response);
}

void delete_test() {
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
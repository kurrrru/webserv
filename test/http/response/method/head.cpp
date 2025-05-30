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
#include "../../../../src/http/response/response.hpp"
#include "../../../../src/http/http_namespace.hpp"
#include "../../../../toolbox/stepmark.hpp"
#include "../../../../src/http/response/server_method_handler.hpp"


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
    http::Response response;
    std::vector<std::string> indices;
    indices.push_back("index.html");
    std::vector<std::string> emptyIndices;

    toolbox::logger::StepMark::info("==== [HEAD] SUCCESS:200 normal file ====");
    http::serverMethod::runHead("./test_dir/test.html", emptyIndices, false, response);

    toolbox::logger::StepMark::info("==== [HEAD] FAIL:404 nonexistent file ====");
    http::serverMethod::runHead("./test_dir/nonexistent.html", indices, false, response);

    toolbox::logger::StepMark::info("==== [HEAD] SUCCESS:200 dir indexfile (modified time) ====");
    http::serverMethod::runHead("./test_dir/", indices, false, response);

    toolbox::logger::StepMark::info("==== [HEAD] SUCCESS:200 dir autoindex ====");
    http::serverMethod::runHead("./test_dir/", emptyIndices, true, response);

    toolbox::logger::StepMark::info("==== [HEAD] FAIL:403 no permission ====");
    http::serverMethod::runHead("./test_dir/no_permission.html", emptyIndices, false, response);

    toolbox::logger::StepMark::info("==== [HEAD] SUCCESS:200 empty dir ====");
    http::serverMethod::runHead("./test_dir/empty_dir/", emptyIndices, true, response);

    toolbox::logger::StepMark::info("==== [HEAD] FAIL:404 empty dir indexfile ====");
    http::serverMethod::runHead("./test_dir/empty_dir/", indices, false, response);

    toolbox::logger::StepMark::info("==== [HEAD] FAIL:403 no permission dir ====");
    http::serverMethod::runHead("./test_dir/no_permission_dir/", emptyIndices, true, response);
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
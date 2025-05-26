#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>

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
#include "../../../../src/http/response/server_method_handler.hpp"


void setupTestEnvironment() {
    mkdir("./uploads", 0755);
    mkdir("./uploads/no_permission_dir", 0000);
}

void testSimplePost() {
    toolbox::logger::StepMark::info("==== [POST] SUCCESS:201 simple post: success case ====");

    http::HTTPFields fields;
    http::Response response;

    std::string content = "normal post test";
    fields.get()["Content-Length"].push_back(toolbox::to_string(content.length()));
    http::serverMethod::runPost("./uploads", content, fields, response);
}

void test_normal_post() {
    http::HTTPFields fields;
    http::Response response;

    toolbox::logger::StepMark::info("==== [POST] SUCCESS:201 normal post: success case ====");
    std::string successBody = "This is a test content for normal POST request";
    std::ostringstream lengthStr;
    lengthStr << successBody.length();

    fields.get()["Content-Length"].push_back(lengthStr.str());
    http::serverMethod::runPost("./uploads", successBody, fields, response);
}

void testUrlEncodedForm() {
    toolbox::logger::StepMark::info("==== [POST] SUCCESS:201 urlencoded: success case ====");

    http::HTTPFields fields;
    http::Response response;
    std::string formData = "name=test_user&message=Hello%20World&age=25";

    fields.get()["Content-Type"].push_back("application/x-www-form-urlencoded");
    fields.get()["Content-Length"].push_back(toolbox::to_string(formData.length()));

    http::serverMethod::runPost("./uploads", formData, fields, response);
}

void test_urlencoded() {
    http::HTTPFields fields;
    http::Response response;

    toolbox::logger::StepMark::info("==== [POST] SUCCESS:201 urlencoded: success case ====");
    std::string successBody = "name=test_user&age=25&comment=Hello%20World";
    std::ostringstream lengthStr1;
    lengthStr1 << successBody.length();

    fields.get()["Content-Length"].push_back(lengthStr1.str());
    fields.get()["Content-Type"].push_back("application/x-www-form-urlencoded");
    http::serverMethod::runPost("./uploads", successBody, fields, response);
}

void test_urlencoded_failure() {
    http::HTTPFields fields;
    http::Response response;

    toolbox::logger::StepMark::info("==== [POST] FAIL:400 urlencoded failure case ====");
    std::string failureBody = "name=fail_user%1operation=write_to_forbidden";
    std::ostringstream lengthStr2;
    lengthStr2 << failureBody.length();

    fields.get()["Content-Length"].push_back(lengthStr2.str());
    fields.get()["Content-Type"].push_back("application/x-www-form-urlencoded");
    http::serverMethod::runPost("./uploads", failureBody, fields, response);
}

void testMultipartFormData() {
    toolbox::logger::StepMark::info("==== [POST] SUCCESS:201 multipart/form-data: success case ====");

    http::HTTPFields fields;
    http::Response response;

    std::string boundary = "testboundary";
    std::string formData = 
        "--" + boundary + "\r\n"
        "Content-Disposition: form-data; name=\"field1\"; filename=\"test1.txt\"\r\n"
        "\r\n"
        "これはテキストファイルの内容です\r\n"
        "--" + boundary + "\r\n"
        "Content-Disposition: form-data; name=\"field2\"; filename=\"test2.txt\"\r\n"
        "\r\n"
        "これは2つ目のファイルの内容です\r\n"
        "--" + boundary + "--";

    fields.get()["Content-Type"].push_back("multipart/form-data; boundary=" + boundary);
    fields.get()["Content-Length"].push_back(toolbox::to_string(formData.length()));
    http::serverMethod::runPost("./uploads", formData, fields, response);
}

void test_multipart_form_data() {
    http::HTTPFields fields;
    http::Response response;

    toolbox::logger::StepMark::info("==== [POST] SUCCESS:201 multipart/form-data: success case ====");
    std::string successBody = "--boundary\r\n"
                              "Content-Disposition: form-data; name=\"field1\"; filename=\"example1.txt\"\r\n"
                              "\r\n"
                              "value1\r\n"
                              "--boundary\r\n"
                              "Content-Disposition: form-data; name=\"field2\"; filename=\"example2.txt\"\r\n"
                              "\r\n"
                              "value2\r\n"
                              "--boundary\r\n"
                              "Content-Disposition: form-data; name=\"field1\"; filename=\"example1.txt\"\r\n"
                              "\r\n"
                              "value1\r\n"
                              "--boundary--";
    std::ostringstream lengthStr3;
    lengthStr3 << successBody.length();

    fields.get()["Content-Length"].push_back(lengthStr3.str());
    fields.get()["Content-Type"].push_back("multipart/form-data; boundary=boundary");
    http::serverMethod::runPost("./uploads", successBody, fields, response);
}

void test_multipart_form_data_failure() {
    http::HTTPFields fields;
    http::Response response;

    toolbox::logger::StepMark::info("==== [POST] SUCCESS:201 multipart/form-data: success case ====");
    std::string failureBody = "--boundary\r\n"
                              "Content-Disposition: form-data; name=\"field1\"\r\n"
                              "\r\n"
                              "value1\r\n"
                              "--boundary--";
    std::ostringstream lengthStr4;
    lengthStr4 << failureBody.length();
    fields.get()["Content-Length"].push_back(lengthStr4.str());
    fields.get()["Content-Type"].push_back("multipart/form-data; boundary=boundary");
    http::serverMethod::runPost("./uploads", failureBody, fields, response);
}

void test_image_upload() {
    http::HTTPFields fields;
    http::Response response;

    toolbox::logger::StepMark::info("==== [POST] SUCCESS:201 image upload ====");

    const unsigned char png_data[] = {
        0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A, 0x00, 0x00, 0x00, 0x0D,
        0x49, 0x48, 0x44, 0x52, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01,
        0x08, 0x06, 0x00, 0x00, 0x00, 0x1F, 0x15, 0xC4, 0x89, 0x00, 0x00, 0x00,
        0x0A, 0x49, 0x44, 0x41, 0x54, 0x78, 0x9C, 0x63, 0x00, 0x01, 0x00, 0x00,
        0x05, 0x00, 0x01, 0x0D, 0x0A, 0x2D, 0xB4, 0x00, 0x00, 0x00, 0x00, 0x49,
        0x45, 0x4E, 0x44, 0xAE, 0x42, 0x60, 0x82
    };

    std::string imageBody(reinterpret_cast<const char*>(png_data), sizeof(png_data));
    std::ostringstream lengthStr;
    lengthStr << imageBody.length();

    fields.get()["Content-Length"].push_back(lengthStr.str());
    fields.get()["Content-Type"].push_back("image/png");
    http::serverMethod::runPost("./uploads", imageBody, fields, response);
}

void test_multipart_image_upload() {
    http::HTTPFields fields;
    http::Response response;

    toolbox::logger::StepMark::info("==== [POST] SUCCESS:201 multipart/form-data image upload ====");

    const unsigned char png_data[] = {
        0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A, 0x00, 0x00, 0x00, 0x0D,
        0x49, 0x48, 0x44, 0x52, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01,
        0x08, 0x06, 0x00, 0x00, 0x00, 0x1F, 0x15, 0xC4, 0x89, 0x00, 0x00, 0x00,
        0x0A, 0x49, 0x44, 0x41, 0x54, 0x78, 0x9C, 0x63, 0x00, 0x01, 0x00, 0x00,
        0x05, 0x00, 0x01, 0x0D, 0x0A, 0x2D, 0xB4, 0x00, 0x00, 0x00, 0x00, 0x49,
        0x45, 0x4E, 0x44, 0xAE, 0x42, 0x60, 0x82
    };

    std::string imageData(reinterpret_cast<const char*>(png_data), sizeof(png_data));

    std::string boundary = "boundary123";
    std::string header = "--" + boundary + "\r\n"
                       "Content-Disposition: form-data; name=\"image\"; filename=\"test.png\"\r\n"
                       "Content-Type: image/png\r\n"
                       "\r\n";

    std::string descHeader = "\r\n--" + boundary + "\r\n"
                           "Content-Disposition: form-data; name=\"description\"; filename=\"test.txt\"\r\n"
                           "\r\n";

    std::string descBody = "This is a test image upload";
    std::string endBoundary = "\r\n--" + boundary + "--";
    std::string successBody = header + imageData + descHeader + descBody + endBoundary;

    std::ostringstream lengthStr;
    lengthStr << successBody.length();

    fields.get()["Content-Length"].push_back(lengthStr.str());
    fields.get()["Content-Type"].push_back("multipart/form-data; boundary=" + boundary);
    http::serverMethod::runPost("./uploads", successBody, fields, response);
}

void testErrorCase() {
    toolbox::logger::StepMark::info("==== [POST] FAIL:500 no permission directory ====");
    http::HTTPFields fields;
    http::Response response;

    std::string content = "no permission directory";
    fields.get()["Content-Length"].push_back(toolbox::to_string(content.length()));
    http::serverMethod::runPost("./uploads/no_permission", content, fields, response);
}

static void runTests() {
    testSimplePost();
    test_normal_post();

    testUrlEncodedForm();
    test_urlencoded();
    test_urlencoded_failure();

    testMultipartFormData();
    test_multipart_form_data();
    test_multipart_form_data_failure();

    test_image_upload();
    test_multipart_image_upload();

    testErrorCase();
}

void post_test() {
    try {
        setupTestEnvironment();
        runTests();
        std::cout << "OK" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "FAILED" << e.what() << std::endl;
    }
}

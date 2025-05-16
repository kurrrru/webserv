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
#include "../../../../src/http/response/post_method.hpp"
#include "../../../../src/http/response/response.hpp"
#include "../../../../src/http/http_namespace.hpp"

static void setupTestEnvironment() {
    mkdir("./uploads", 0755);
    mkdir("./uploads/no_permission_dir", 0000);
}

static void cleanupTestEnvironment() {
    // chmod("./test_dir/no_permission_dir", 0755);
    // rmdir("./test_dir/no_permission_dir");
    // rmdir("./test_dir");
}

void test_urlencoded() {
    http::HTTPFields fields;
    http::Response response;

    // 成功ケース: application/x-www-form-urlencoded形式の正常なリクエスト
    toolbox::logger::StepMark::info("==== POST test_urlencoded: success case ====");
    std::string successBody = "name=test_user&age=25&comment=Hello%20World";
    std::ostringstream lengthStr1;
    lengthStr1 << successBody.length();
    
    fields.get()["Content-Length"].push_back(lengthStr1.str());
    fields.get()["Content-Type"].push_back("application/x-www-form-urlencoded");
    http::runPost("./uploads", successBody, fields, response);
}

void test_urlencoded_failure() {
    http::HTTPFields fields;
    http::Response response;

    // 失敗ケース: invalid percent-encoding
    toolbox::logger::StepMark::info("==== POST test_urlencoded_failure: failure case ====");
    std::string failureBody = "name=fail_user%1operation=write_to_forbidden";
    std::ostringstream lengthStr2;
    lengthStr2 << failureBody.length();
    
    fields.get()["Content-Length"].push_back(lengthStr2.str());
    fields.get()["Content-Type"].push_back("application/x-www-form-urlencoded");
    http::runPost("./uploads", failureBody, fields, response);
}

void test_multipart_form_data() {
    http::HTTPFields fields;
    http::Response response;

    toolbox::logger::StepMark::info("==== POST multipart/form-data: success case ====");
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
    http::runPost("./uploads", successBody, fields, response);
}

void test_multipart_form_data_failure() {
    http::HTTPFields fields;
    http::Response response;

    toolbox::logger::StepMark::info("==== POST multipart/form-data: failure case ====");
    std::string failureBody = "--boundary\r\n"
                              "Content-Disposition: form-data; name=\"field1\"\r\n"
                              "\r\n"
                              "value1\r\n"
                              "--boundary--";
    std::ostringstream lengthStr4;
    lengthStr4 << failureBody.length();
    fields.get()["Content-Length"].push_back(lengthStr4.str());
    fields.get()["Content-Type"].push_back("multipart/form-data; boundary=boundary");
    http::runPost("./uploads", failureBody, fields, response);
}

void test_normal_post() {
    http::HTTPFields fields;
    http::Response response;

    toolbox::logger::StepMark::info("==== POST test_normal_post: success case ====");
    std::string successBody = "This is a test content for normal POST request";
    std::ostringstream lengthStr;
    lengthStr << successBody.length();
    
    fields.get()["Content-Length"].push_back(lengthStr.str());
    http::runPost("./uploads", successBody, fields, response);
}

void test_image_upload() {
    http::HTTPFields fields;
    http::Response response;

    toolbox::logger::StepMark::info("==== POST test_image_upload: success case ====");
    
    // 1x1 ピクセルのPNG画像のバイナリデータ
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
    http::runPost("./uploads", imageBody, fields, response);
}

void test_multipart_image_upload() {
    http::HTTPFields fields;
    http::Response response;

    toolbox::logger::StepMark::info("==== POST test_multipart_image_upload: success case ====");
    
    // const unsigned char png_data[] = {
    //     0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A, 0x00, 0x00, 0x00, 0x0D,
    //     0x49, 0x48, 0x44, 0x52, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01,
    //     0x08, 0x06, 0x00, 0x00, 0x00, 0x1F, 0x15, 0xC4, 0x89, 0x00, 0x00, 0x00,
    //     0x0A, 0x49, 0x44, 0x41, 0x54, 0x78, 0x9C, 0x63, 0x00, 0x01, 0x00, 0x00,
    //     0x05, 0x00, 0x01, 0x0D, 0x0A, 0x2D, 0xB4, 0x00, 0x00, 0x00, 0x00, 0x49,
    //     0x45, 0x4E, 0x44, 0xAE, 0x42, 0x60, 0x82
    // };

    const unsigned char png_data[] = {
    // PNG署名 (8バイト)
    0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A,
    
    // IHDRチャンク
    0x00, 0x00, 0x00, 0x0D,  // チャンク長: 13バイト
    0x49, 0x48, 0x44, 0x52,  // "IHDR"
    0x00, 0x00, 0x00, 0x0A,  // 幅: 10ピクセル (変更点)
    0x00, 0x00, 0x00, 0x0A,  // 高さ: 10ピクセル (変更点)
    0x08, 0x06, 0x00, 0x00, 0x00,  // ビット深度, カラータイプ, その他フラグ
    0x3A, 0x98, 0x2F, 0x0A,  // IHDRのCRC (幅・高さ変更に伴い更新)
    
    // IDATチャンク
    0x00, 0x00, 0x00, 0x2C,  // チャンク長: 44バイト (10×10用に増加)
    0x49, 0x44, 0x41, 0x54,  // "IDAT"
    // 10×10ピクセル用の圧縮データ (44バイト)
    0x78, 0x9C, 0x63, 0x60, 0x60, 0x60, 0xF0, 0xFF, 0xFF, 0xFF, 0x3F, 0x03,
    0x03, 0x03, 0xC3, 0x7F, 0x06, 0x30, 0x60, 0x80, 0xD2, 0x0C, 0x0C, 0x70,
    0x1A, 0x01, 0x30, 0x20, 0x68, 0x06, 0x28, 0x0D, 0xC6, 0x20, 0x1A, 0x43,
    0x0C, 0x1D, 0x60, 0x68, 0x60, 0x60, 0x00, 0x00, 0x76, 0x21, 0x6D, 0x24,
    
    // CRC for IDAT
    0x0E, 0x7C, 0x75, 0x31,
    
    // IENDチャンク (変更なし)
    0x00, 0x00, 0x00, 0x00,  // チャンク長: 0バイト
    0x49, 0x45, 0x4E, 0x44,  // "IEND"
    0xAE, 0x42, 0x60, 0x82   // IENDのCRC
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
    http::runPost("./uploads", successBody, fields, response);
}

static void runTests() {
    test_normal_post();
    sleep(1);

    // test_image_upload();  // 拡張子を確認する必要がある
    // sleep(1);

    test_multipart_image_upload();
    sleep(1);

    test_urlencoded();
    sleep(1);

    test_urlencoded_failure();
    sleep(1);

    test_multipart_form_data();
    sleep(1);

    test_multipart_form_data_failure();
    sleep(1);
}

void post_test() {
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
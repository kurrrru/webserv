// Copyright 2025 Ideal Broccoli

#include <iostream>
#include <vector>
#include <string>

#include <cstdio>

#include "../../../toolbox/stepmark.hpp"
#include "../../../src/http/cgi/cgi_response_parser.hpp"
#include "cgi_response_test.hpp"

typedef std::vector<CgiResponseTest> TestVector;

void showField(http::CgiResponseParser& r, CgiResponseTest& t) {
    std::cout << "----- field -----" << std::endl;
    std::cout << "nginx status  : " << t.httpStatus.get() << std::endl;
    std::cout << "webserv status: " << r.get().httpStatus.get() << std::endl;
    for (http::HTTPFields::FieldMap::iterator it = r.get().fields.get().begin();
                it != r.get().fields.get().end(); ++it) {
        if (!it->second.empty()) {
            std::cout << "webserv: " << it->first << " ";
            for (std::size_t i = 0; i < it->second.size(); ++i) {
                std::cout << it->second[i] << ", ";
            }
            std::cout << std::endl;
            http::HTTPFields::FieldMap::iterator it_t =
                t.exceptMap.find(it->first);
            if (it_t == t.exceptMap.end()) {
                continue;
            }
            std::cout << "nginx  : " << it_t->first << " ";
            for (std::size_t i = 0; i < it_t->second.size(); ++i) {
                std::cout << it_t->second[i] << ", ";
            }
            std::cout << std::endl;
        }
    }
}

bool compareFields(http::CgiResponseParser& r, CgiResponseTest& t) {
    if (r.get().httpStatus.get() != t.httpStatus.get()) {
        return false;
    }
    for (http::HTTPFields::FieldMap::iterator it_r =
            r.get().fields.get().begin();
            it_r != r.get().fields.get().end(); ++it_r) {
                http::HTTPFields::FieldMap::iterator it_t
                    = t.exceptMap.find(it_r->first);
        if (it_r->second.empty()) {
            if (it_t == t.exceptMap.end()) {
                continue;
            }
            return false;
        }
        if (it_t->second.empty() || it_r->second != it_t->second) {
            return false;
        }
    }
    return true;
}

bool runTest(CgiResponseTest& t) {
    toolbox::logger::StepMark::debug(t.name);
    http::CgiResponseParser r;
    try {
        r.run(t.request);
    } catch (std::exception& e) {
    }
    if (t.isSuccessTest && compareFields(r, t)) {
        return true;
    } else if (t.isSuccessTest) {
        std::cout << "==== Failed: true->false "
            << t.name << " ====" << std::endl;
        showField(r, t);
        return false;
    } else if (!t.isSuccessTest && compareFields(r, t)) {
        return true;
    } else {
        std::cout << "==== Failed: false->true "
            << t.name << " ====" << std::endl;
        showField(r, t);
        return false;
    }
}

void addFieldToMap(http::HTTPFields::FieldMap& map,
    const std::string& key,
    const std::string& value) {
map[key].push_back(value);
}

void makeCgiDefaultTests(TestVector& t) {
    CgiResponseTest r;

    r.name = "基本的なCGIレスポンス";

    r.body = "<!DOCTYPE html>\r\n<html>\r\n"
                "<head>\r\n\t<title>CGIResponseSample</title>\r\n</head>\r\n"
                "<body>\r\n\t<h1>Hello, World!</h1>\r\n"
                "\t<p>This is CGI response sample text</p>\r\n</body>\r\n"
                "</html>\r\n";
    r.request = "Content-Type: text/html\r\n\r\n" + r.body;
    r.httpStatus.set(http::HttpStatus::OK);
    r.isSuccessTest = true;
    addFieldToMap(r.exceptMap, "Content-Type", "text/html");
    t.push_back(r);
    r.exceptMap.clear();

    r.name = "リダイレクトCGIレスポンス";
    r.body = "";
    r.request = "Location: https://example.com/new-location\r\n\r\n" + r.body;
    r.httpStatus.set(http::HttpStatus::FOUND);
    r.isSuccessTest = true;
    addFieldToMap(r.exceptMap, "Location", "https://example.com/new-location");
    t.push_back(r);
    r.exceptMap.clear();

    r.name = "カスタムステータスCGIレスポンス";
    r.body = "<!DOCTYPE html>\r\n<html>\r\n"
            "<body>\r\n\t<h1>Not Found</h1>\r\n"
            "\t<p>The requested resource was not found on this server.</p>\r\n"
            "</body>\r\n</html>\r\n";
    r.request = "Status: 404 Not Found\r\nContent-Type: text/html\r\n\r\n"
        + r.body;
    r.httpStatus.set(http::HttpStatus::NOT_FOUND);
    r.isSuccessTest = true;
    addFieldToMap(r.exceptMap, "Content-Type", "text/html");
    t.push_back(r);
    r.exceptMap.clear();

    r.name = "JSON CGIレスポンス";
    r.body = "{\r\n\t\"status\": \"success\",\r\n"
            "\t\"data\": {\r\n"
            "\t\t\"id\": 123,\r\n"
            "\t\t\"name\": \"サンプルデータ\"\r\n"
            "\t}\r\n}\r\n";
    r.request = "Content-Type: application/json\r\n\r\n" + r.body;
    r.httpStatus.set(http::HttpStatus::OK);
    r.isSuccessTest = true;
    addFieldToMap(r.exceptMap, "Content-Type", "application/json");
    t.push_back(r);
    r.exceptMap.clear();

    r.name = "リダイレクト+ステータスCGIレスポンス";
    r.body = "<!DOCTYPE html>\r\n<html>\r\n"
            "<body>\r\n\t<h1>Moved Permanently</h1>\r\n"
            "\t<p>The document has moved "
            "<a href=\"https://example.com/permanent\">here</a>"
            ".</p>\r\n</body>\r\n</html>\r\n";
    r.request = "Status: 301 Moved Permanently\r\n"
        "Location: https://example.com/permanent\r\n"
        "Content-Type: text/html\r\n\r\n" + r.body;
    r.httpStatus.set(http::HttpStatus::MOVED_PERMANENTLY);
    r.isSuccessTest = true;
    addFieldToMap(r.exceptMap, "Content-Type", "text/html");
    addFieldToMap(r.exceptMap, "Location", "https://example.com/permanent");
    t.push_back(r);
    r.exceptMap.clear();

    // テストケース5: Content-Length指定あり
    r.name = "Content-Length付きCGIレスポンス";
    r.body = "Hello, World!";
    r.request = "Content-Type: text/plain\r\n"
        "Content-Length: 13\r\n\r\n" + r.body;
    r.httpStatus.set(http::HttpStatus::OK);
    r.isSuccessTest = true;
    addFieldToMap(r.exceptMap, "Content-Type", "text/plain");
    addFieldToMap(r.exceptMap, "Content-Length", "13");
    t.push_back(r);
    r.exceptMap.clear();

    // テストケース6: Set-Cookieヘッダー
    r.name = "クッキー設定CGIレスポンス";
    r.body = "<!DOCTYPE html>\r\n<html>\r\n"
            "<body>\r\n\t<h1>Cookie Set</h1>\r\n"
            "\t<p>A cookie has been set for your session.</p>\r\n</body>\r\n"
            "</html>\r\n";
    r.request = "Content-Type: text/html\r\n"
        "Set-Cookie: session=abc123; Path=/; HttpOnly\r\n\r\n" + r.body;
    r.httpStatus.set(http::HttpStatus::OK);
    r.isSuccessTest = true;
    addFieldToMap(r.exceptMap, "Content-Type", "text/html");
    addFieldToMap
        (r.exceptMap, "Set-Cookie", "session=abc123; Path=/; HttpOnly");
    t.push_back(r);
    r.exceptMap.clear();

    // テストケース7: NPHスクリプト風（エラーテスト）
    r.name = "疑似NPHスクリプトテスト（エラー）";
    r.body = "<!DOCTYPE html>\r\n<html>\r\n"
            "<body>\r\n\t<h1>Error</h1>\r\n"
            "\t<p>This test is expected to fail as it's missing proper"
            " headers.</p>\r\n</body>\r\n"
            "</html>\r\n";
    r.request = "HTTP/1.1 200 OK\r\n\r\n" + r.body;
    r.httpStatus.set(http::HttpStatus::INTERNAL_SERVER_ERROR);
    r.isSuccessTest = false;  // 失敗を期待するテスト
    t.push_back(r);
    r.exceptMap.clear();

    // テストケース8: 不正なヘッダーフォーマット
    r.name = "不正なヘッダーフォーマット";
    r.body = "エラーメッセージ";
    r.request = "Content Type: text/plain\r\n\r\n" + r.body;  // スペースが不正
    r.httpStatus.set(http::HttpStatus::INTERNAL_SERVER_ERROR);
    r.isSuccessTest = false;
    t.push_back(r);
    r.exceptMap.clear();
}

void requestLineTest() {
    TestVector tests;
    makeCgiDefaultTests(tests);

    std::size_t pass = 0;
    for (std::size_t i = 0; i < tests.size(); ++i) {
        if (runTest(tests[i])) {
            ++pass;
        }
    }
    std::cout << "RequestLine testcase " << pass << " / "
        << tests.size() << std::endl;
}

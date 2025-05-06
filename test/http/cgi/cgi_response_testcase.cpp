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
    std::cout << "nginx cgiType  : " << t.cgiType << std::endl;
    std::cout << "webserv cgiType: " << r.get().cgiType << std::endl;
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
    if (r.get().cgiType != t.cgiType) {
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
        r.get().identifyCgiType();
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

void makeDocumentCgiTests(TestVector& t) {
    CgiResponseTest r;

    r.name = "基本的なHTMLドキュメント";
    r.body = "<!DOCTYPE html>\r\n<html>\r\n"
                "<head>\r\n\t<title>CGIドキュメントサンプル</title>\r\n"
                "</head>\r\n<body>\r\n\t<h1>こんにちは、世界！</h1>\r\n"
                "\t<p>これはCGIレスポンスのサンプルテキストです</p>\r\n"
                "</body>\r\n</html>\r\n";
    r.request = "Content-Type: text/html\r\n\r\n" + r.body;
    r.httpStatus.set(http::HttpStatus::OK);
    r.cgiType = http::CgiResponse::DOCUMENT;
    r.isSuccessTest = true;
    addFieldToMap(r.exceptMap, "Content-Type", "text/html");
    t.push_back(r);
    r.exceptMap.clear();

    r.name = "JSONドキュメント";
    r.body = "{\r\n\t\"status\": \"success\",\r\n"
            "\t\"data\": {\r\n"
            "\t\t\"id\": 123,\r\n"
            "\t\t\"name\": \"サンプルデータ\"\r\n"
            "\t}\r\n}\r\n";
    r.request = "Content-Type: application/json\r\n\r\n" + r.body;
    r.httpStatus.set(http::HttpStatus::OK);
    r.cgiType = http::CgiResponse::DOCUMENT;
    r.isSuccessTest = true;
    addFieldToMap(r.exceptMap, "Content-Type", "application/json");
    t.push_back(r);
    r.exceptMap.clear();

    r.name = "Content-Length付きテキストドキュメント";
    r.body = "Hello, World!";
    r.request = "Content-Type: text/plain\r\n"
        "Content-Length: 13\r\n\r\n" + r.body;
    r.httpStatus.set(http::HttpStatus::OK);
    r.cgiType = http::CgiResponse::DOCUMENT;
    r.isSuccessTest = true;
    addFieldToMap(r.exceptMap, "Content-Type", "text/plain");
    addFieldToMap(r.exceptMap, "Content-Length", "13");
    t.push_back(r);
    r.exceptMap.clear();

    r.name = "クッキー設定付きドキュメント";
    r.body = "<!DOCTYPE html>\r\n<html>\r\n"
            "<body>\r\n\t<h1>クッキーが設定されました</h1>\r\n"
            "\t<p>セッション用のクッキーが設定されました。</p>\r\n</body>\r\n"
            "</html>\r\n";
    r.request = "Content-Type: text/html\r\n"
        "Set-Cookie: session=abc123; Path=/; HttpOnly\r\n\r\n" + r.body;
    r.httpStatus.set(http::HttpStatus::OK);
    r.cgiType = http::CgiResponse::DOCUMENT;
    r.isSuccessTest = true;
    addFieldToMap(r.exceptMap, "Content-Type", "text/html");
    addFieldToMap(r.exceptMap, "Set-Cookie",
        "session=abc123; Path=/; HttpOnly");
    t.push_back(r);
    r.exceptMap.clear();

    r.name = "カスタムステータスとヘッダーのドキュメント";
    r.body = "<!DOCTYPE html>\r\n<html>\r\n"
            "<body>\r\n\t<h1>Not Found</h1>\r\n"
            "\t<p>リクエストされたリソースは見つかりませんでした。</p>\r\n"
            "</body>\r\n</html>\r\n";
    r.request = "Status: 404 Not Found\r\n"
        "Content-Type: text/html\r\n"
        "X-Custom-Header: CustomValue\r\n\r\n" + r.body;
    r.httpStatus.set(http::HttpStatus::NOT_FOUND);
    r.cgiType = http::CgiResponse::DOCUMENT;
    r.isSuccessTest = true;
    addFieldToMap(r.exceptMap, "Content-Type", "text/html");
    addFieldToMap(r.exceptMap, "X-Custom-Header", "CustomValue");
    t.push_back(r);
    r.exceptMap.clear();
}

void makeLocalRedirectCgiTests(TestVector& t) {
    CgiResponseTest r;

    r.name = "基本的なローカルリダイレクト";
    r.body = "";
    r.request = "Location: /new-location\r\n\r\n" + r.body;
    r.httpStatus.set(http::HttpStatus::UNSET);
    r.cgiType = http::CgiResponse::LOCAL_REDIRECT;
    r.isSuccessTest = true;
    addFieldToMap(r.exceptMap, "Location", "/new-location");
    t.push_back(r);
    r.exceptMap.clear();

    r.name = "絶対パスへのローカルリダイレクト";
    r.body = "";
    r.request = "Location: /absolute/path/resource\r\n\r\n" + r.body;
    r.httpStatus.set(http::HttpStatus::UNSET);
    r.cgiType = http::CgiResponse::LOCAL_REDIRECT;
    r.isSuccessTest = true;
    addFieldToMap(r.exceptMap, "Location", "/absolute/path/resource");
    t.push_back(r);
    r.exceptMap.clear();

    r.name = "相対パスへのローカルリダイレクト";
    r.body = "";
    r.request = "Location: ../parent/resource\r\n\r\n" + r.body;
    r.httpStatus.set(http::HttpStatus::UNSET);
    r.cgiType = http::CgiResponse::INVALID;
    r.isSuccessTest = false;
    addFieldToMap(r.exceptMap, "Location", "../parent/resource");
    t.push_back(r);
    r.exceptMap.clear();

    r.name = "クエリパラメータ付きローカルリダイレクト";
    r.body = "";
    r.request = "Location: /search?q=test&page=1\r\n\r\n" + r.body;
    r.httpStatus.set(http::HttpStatus::UNSET);
    r.cgiType = http::CgiResponse::LOCAL_REDIRECT;
    r.isSuccessTest = true;
    addFieldToMap(r.exceptMap, "Location", "/search?q=test&page=1");
    t.push_back(r);
    r.exceptMap.clear();
}

void makeClientRedirectTests(TestVector& t) {
    CgiResponseTest r;

    r.name = "基本的なクライアントリダイレクト";
    r.body = "";
    r.request = "Location: https://example.com/new-location\r\n\r\n" + r.body;
    r.httpStatus.set(http::HttpStatus::UNSET);
    r.cgiType = http::CgiResponse::CLIENT_REDIRECT;
    r.isSuccessTest = true;
    addFieldToMap(r.exceptMap, "Location", "https://example.com/new-location");
    t.push_back(r);
    r.exceptMap.clear();

    r.name = "外部サイトへのリダイレクト";
    r.body = "";
    r.request = "Location: https://external-site.org/resource\r\n\r\n" + r.body;
    r.httpStatus.set(http::HttpStatus::UNSET);
    r.cgiType = http::CgiResponse::CLIENT_REDIRECT;
    r.isSuccessTest = true;
    addFieldToMap(r.exceptMap, "Location",
        "https://external-site.org/resource");
    t.push_back(r);
    r.exceptMap.clear();

    r.name = "クエリパラメータ付きリダイレクト";
    r.body = "";
    r.request ="Location: https://example.com/search"
        "?q=webserv&category=project\r\n\r\n" + r.body;
    r.httpStatus.set(http::HttpStatus::UNSET);
    r.cgiType = http::CgiResponse::CLIENT_REDIRECT;
    r.isSuccessTest = true;
    addFieldToMap(r.exceptMap, "Location",
        "https://example.com/search?q=webserv&category=project");
    t.push_back(r);
    r.exceptMap.clear();

    r.name = "フラグメント識別子付きリダイレクト";
    r.body = "";
    r.request = "Location: https://example.com/page#section2\r\n\r\n" + r.body;
    r.httpStatus.set(http::HttpStatus::UNSET);
    r.cgiType = http::CgiResponse::CLIENT_REDIRECT;
    r.isSuccessTest = true;
    addFieldToMap(r.exceptMap, "Location", "https://example.com/page#section2");
    t.push_back(r);
    r.exceptMap.clear();

    r.name = "相対URLへのリダイレクト";
    r.body = "";
    r.request = "Location: ../parent/resource\r\n\r\n" + r.body;
    r.httpStatus.set(http::HttpStatus::UNSET);
    r.cgiType = http::CgiResponse::INVALID;
    r.isSuccessTest = true;
    addFieldToMap(r.exceptMap, "Location", "../parent/resource");
    t.push_back(r);
    r.exceptMap.clear();

    r.name = "追加ヘッダー付きリダイレクト";
    r.body = "";
    r.request = "Location: https://example.com/redirect\r\n"
                "Cache-Control: no-cache\r\n\r\n" + r.body;
    r.httpStatus.set(http::HttpStatus::UNSET);
    r.cgiType = http::CgiResponse::INVALID;
    r.isSuccessTest = false;
    addFieldToMap(r.exceptMap, "Location", "https://example.com/redirect");
    addFieldToMap(r.exceptMap, "Cache-Control", "no-cache");
    t.push_back(r);
    r.exceptMap.clear();

    r.name = "ポート指定付きリダイレクト";
    r.body = "";
    r.request = "Location: https://example.com:8080/custom-port\r\n\r\n"
        + r.body;
    r.httpStatus.set(http::HttpStatus::UNSET);
    r.cgiType = http::CgiResponse::CLIENT_REDIRECT;
    r.isSuccessTest = true;
    addFieldToMap(r.exceptMap, "Location",
        "https://example.com:8080/custom-port");
    t.push_back(r);
    r.exceptMap.clear();

    r.name = "プロトコルなしURLへのリダイレクト";
    r.body = "";
    r.request = "Location: //example.com/protocol-relative\r\n\r\n" + r.body;
    r.httpStatus.set(http::HttpStatus::UNSET);
    r.cgiType = http::CgiResponse::INVALID;
    r.isSuccessTest = false;
    addFieldToMap(r.exceptMap, "Location", "//example.com/protocol-relative");
    t.push_back(r);
    r.exceptMap.clear();

    r.name = "ローカルホストへのリダイレクト";
    r.body = "";
    r.request = "Location: http://localhost:8080/test\r\n\r\n" + r.body;
    r.httpStatus.set(http::HttpStatus::UNSET);
    r.cgiType = http::CgiResponse::CLIENT_REDIRECT;
    r.isSuccessTest = true;
    addFieldToMap(r.exceptMap, "Location", "http://localhost:8080/test");
    t.push_back(r);
    r.exceptMap.clear();
}

void makeClientRedirectWithDocumentTests(TestVector& t) {
    CgiResponseTest r;

    r.name = "基本的なクライアントリダイレクト";
    r.body = "<!DOCTYPE html>\r\n<html>\r\n"
            "<body>\r\n\t<h1>Found</h1>\r\n"
            "\t<p>The document has moved "
            "<a href=\"https://example.com/temp\">here</a>"
            ".</p>\r\n</body>\r\n</html>\r\n";
    r.request = "Status: 302 Found\r\n"
        "Location: https://example.com/temp\r\n"
        "Content-Type: text/html\r\n\r\n" + r.body;
    r.httpStatus.set(http::HttpStatus::FOUND);
    r.cgiType = http::CgiResponse::CLIENT_REDIRECT_DOCUMENT;
    r.isSuccessTest = true;
    addFieldToMap(r.exceptMap, "Content-Type", "text/html");
    addFieldToMap(r.exceptMap, "Location", "https://example.com/temp");
    t.push_back(r);
    r.exceptMap.clear();

    r.name = "永続的クライアントリダイレクト";
    r.body = "<!DOCTYPE html>\r\n<html>\r\n"
            "<body>\r\n\t<h1>Moved Permanently</h1>\r\n"
            "\t<p>The document has moved "
            "<a href=\"https://example.com/permanent\">here</a>"
            ".</p>\r\n</body>\r\n</html>\r\n";
    r.request = "Status: 301 Moved Permanently\r\n"
        "Location: https://example.com/permanent\r\n"
        "Content-Type: text/html\r\n\r\n" + r.body;
    r.httpStatus.set(http::HttpStatus::MOVED_PERMANENTLY);
    r.cgiType = http::CgiResponse::CLIENT_REDIRECT_DOCUMENT;
    r.isSuccessTest = true;
    addFieldToMap(r.exceptMap, "Content-Type", "text/html");
    addFieldToMap(r.exceptMap, "Location", "https://example.com/permanent");
    t.push_back(r);
    r.exceptMap.clear();

    r.name = "別ドメインへのクライアントリダイレクト";
    r.body = "<!DOCTYPE html>\r\n<html>\r\n"
            "<body>\r\n\t<h1>Redirect</h1>\r\n"
            "\t<p>リダイレクト中...</p>\r\n</body>\r\n"
            "</html>\r\n";
    r.request = "Status: 302 Found\r\n"
        "Location: https://otherdomain.com/resource\r\n"
        "Content-Type: text/html\r\n\r\n" + r.body;
    r.httpStatus.set(http::HttpStatus::FOUND);
    r.cgiType = http::CgiResponse::CLIENT_REDIRECT_DOCUMENT;
    r.isSuccessTest = true;
    addFieldToMap(r.exceptMap, "Content-Type", "text/html");
    addFieldToMap(r.exceptMap, "Location", "https://otherdomain.com/resource");
    t.push_back(r);
    r.exceptMap.clear();

    r.name = "一時的リダイレクト";
    r.body = "<!DOCTYPE html>\r\n<html>\r\n"
            "<body>\r\n\t<h1>Temporary Redirect</h1>\r\n"
            "\t<p>一時的にリダイレクトされています</p>\r\n</body>\r\n"
            "</html>\r\n";
    r.request = "Status: 307 Temporary Redirect\r\n"
        "Location: https://example.com/temporary\r\n"
        "Content-Type: text/html\r\n\r\n" + r.body;
    r.httpStatus.set(http::HttpStatus::TEMPORARY_REDIRECT);
    r.cgiType = http::CgiResponse::CLIENT_REDIRECT_DOCUMENT;
    r.isSuccessTest = true;
    addFieldToMap(r.exceptMap, "Content-Type", "text/html");
    addFieldToMap(r.exceptMap, "Location", "https://example.com/temporary");
    t.push_back(r);
    r.exceptMap.clear();

    r.name = "カスタムヘッダー付きクライアントリダイレクト";
    r.body = "<!DOCTYPE html>\r\n<html>\r\n"
            "<body>\r\n\t<h1>リダイレクト</h1>\r\n"
            "\t<p>カスタムヘッダー付きのリダイレクトです</p>\r\n</body>\r\n"
            "</html>\r\n";
    r.request = "Status: 302 Found\r\n"
        "Location: https://example.com/custom\r\n"
        "Content-Type: text/html\r\n"
        "X-Redirect-Reason: Maintenance\r\n\r\n" + r.body;
    r.httpStatus.set(http::HttpStatus::FOUND);
    r.cgiType = http::CgiResponse::CLIENT_REDIRECT_DOCUMENT;
    r.isSuccessTest = true;
    addFieldToMap(r.exceptMap, "Content-Type", "text/html");
    addFieldToMap(r.exceptMap, "Location", "https://example.com/custom");
    addFieldToMap(r.exceptMap, "X-Redirect-Reason", "Maintenance");
    t.push_back(r);
    r.exceptMap.clear();
}

void makeInvalidCgiTests(TestVector& t) {
    CgiResponseTest r;

    r.name = "不正なヘッダーフォーマット";
    r.body = "エラーメッセージ";
    r.request = "Content Type: text/plain\r\n\r\n" + r.body;  // スペースが不正
    r.httpStatus.set(http::HttpStatus::INTERNAL_SERVER_ERROR);
    r.cgiType = http::CgiResponse::INVALID;
    r.isSuccessTest = false;
    t.push_back(r);
    r.exceptMap.clear();

    r.name = "存在しないステータスコード";
    r.body = "<!DOCTYPE html>\r\n<html>\r\n"
            "<body>\r\n\t<h1>エラー</h1>\r\n"
            "\t<p>不正なステータスコード</p>\r\n</body>\r\n"
            "</html>\r\n";
    r.request = "Status: 999 Invalid Status\r\n"
        "Content-Type: text/html\r\n\r\n" + r.body;
    r.httpStatus.set(http::HttpStatus::INTERNAL_SERVER_ERROR);
    r.cgiType = http::CgiResponse::INVALID;
    r.isSuccessTest = false;
    addFieldToMap(r.exceptMap, "Content-Type", "text/html");
    t.push_back(r);
    r.exceptMap.clear();

    r.name = "不正なContent-Length";
    r.body = "テスト本文";
    r.request = "Content-Type: text/plain\r\n"
        "Content-Length: invalid\r\n\r\n" + r.body;
    r.httpStatus.set(http::HttpStatus::INTERNAL_SERVER_ERROR);
    r.cgiType = http::CgiResponse::INVALID;
    r.isSuccessTest = false;
    addFieldToMap(r.exceptMap, "Content-Type", "text/plain");
    addFieldToMap(r.exceptMap, "Content-Length", "invalid");
    t.push_back(r);
    r.exceptMap.clear();

    r.name = "空のリクエスト";
    r.body = "";
    r.request = "";
    r.httpStatus.set(http::HttpStatus::UNSET);
    r.cgiType = http::CgiResponse::INVALID;
    r.isSuccessTest = false;
    t.push_back(r);
    r.exceptMap.clear();

    r.name = "ヘッダーとボディの区切りがない";
    r.body = "ボディ部分";
    r.request = "Content-Type: text/plain\r\n" + r.body;
    r.httpStatus.set(http::HttpStatus::OK);
    r.cgiType = http::CgiResponse::DOCUMENT;
    r.isSuccessTest = false;
    addFieldToMap(r.exceptMap, "Content-Type", "text/plain");
    t.push_back(r);
    r.exceptMap.clear();
}

void requestLineTest() {
    TestVector tests;
    makeDocumentCgiTests(tests);
    makeLocalRedirectCgiTests(tests);
    makeClientRedirectWithDocumentTests(tests);
    makeClientRedirectTests(tests);
    makeInvalidCgiTests(tests);

    std::size_t pass = 0;
    for (std::size_t i = 0; i < tests.size(); ++i) {
        if (runTest(tests[i])) {
            ++pass;
        }
    }
    std::cout << "RequestLine testcase " << pass << " / "
        << tests.size() << std::endl;
}

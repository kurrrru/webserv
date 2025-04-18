// Copyright 2025 Ideal Broccoli

#include <vector>
#include <string>
#include <iostream>

#include "../../../src/http/request/request_parser.hpp"
#include "../../../src/http/request/http_fields.hpp"
#include "request/field_test.hpp"

typedef std::vector<FieldTest> TestVector;

void showField(http::RequestParser& r) {
    for (HTTPFields::FieldMap::iterator it = r.get().fields.get().begin();
                it != r.get().fields.get().end(); ++it) {
        if (!it->second.empty()) {
            std::cout << it->first << ": ";
            for (std::size_t i = 0; i < it->second.size(); ++i) {
                std::cout << it->second[i] << " ";
            }
            std::cout << std::endl;
        }
    }
}

bool runTest(FieldTest& t) {
    http::RequestParser r;
    try {
        r.run(t._request);
        if (t._isSuccessTest) {
            return true;
        } else {
            std::cout << "==== Failed: false->true " \
                << t._name << " ====" << std::endl;
            showField(r);
            return false;
        }
    } catch (std::exception& e) {
        if (!t._isSuccessTest) {
            return true;
        } else {
            std::cout << "==== Failed: true->false " \
                << t._name << " ====" << std::endl;
            std::cout << e.what() << std::endl;
            showField(r);
            return false;
        }
    }
}

void makeFieldTest(TestVector& tests) {
    FieldTest f;
    // -------------------------------------------------------------------------
    // 基本的なHTTPリクエストのテスト
    // -------------------------------------------------------------------------
    f._name = "基本的なGETリクエスト";
    f._request = "GET / HTTP/1.1\r\nHost: sample\r\n\r\n";
    f._isSuccessTest = true;  // 200 OK
    tests.push_back(f);

    f._name = "複数のヘッダーフィールド";
    f._request = "GET / HTTP/1.1\r\nHost: sample\r\nAccept: text/html\r\n\r\n";
    f._isSuccessTest = true;  // 200 OK
    tests.push_back(f);

    f._name = "Hostフィールドが先頭にない";
    f._request = "GET / HTTP/1.1\r\nAccept: text/html\r\nHost: sample\r\n\r\n";
    f._isSuccessTest = true;  // 200 OK
    tests.push_back(f);

    f._name = "フィールド値が空";
    f._request = "GET / HTTP/1.1\r\nHost: sample\r\nAccept:\r\n\r\n";
    f._isSuccessTest = true;  // 200 OK
    tests.push_back(f);

    f._name = "存在しないフィールド";
    f._request = "GET / HTTP/1.1\r\nHost: sample\r\nINVALID: text/html\r\n\r\n";
    f._isSuccessTest = true;  // 200 OK
    tests.push_back(f);

    f._name = "存在しないフィールドの重複";
    f._request = "GET / HTTP/1.1\r\nHost: sample\r\nINVALID: text/html\r\n"
                    "INVALID: text/html\r\n\r\n";
    f._isSuccessTest = true;  // 200 OK
    tests.push_back(f);

    // -------------------------------------------------------------------------
    // Hostヘッダーのフォーマット検証
    // -------------------------------------------------------------------------
    f._name = "Hostヘッダーが区切り文字前にスペースなし";
    f._request = "GET / HTTP/1.1\r\nHost:sample\r\n\r\n";
    f._isSuccessTest = true;  // 200 OK
    tests.push_back(f);

    f._name = "Hostヘッダー値の前に複数スペース";
    f._request = "GET / HTTP/1.1\r\nHost:  sample\r\n\r\n";
    f._isSuccessTest = true;  // 200 OK
    tests.push_back(f);

    f._name = "ヘッダーフィールド値が引用符で囲まれている";
    f._request = "GET / HTTP/1.1\r\nHost: \"sample\"\r\n\r\n";
    f._isSuccessTest = true;  // 200 OK
    tests.push_back(f);

    f._name = "Hostヘッダーの後にスペース";
    f._request = "GET / HTTP/1.1\r\nHost : sample\r\n\r\n";
    f._isSuccessTest = false;  // 400 Bad Request
    tests.push_back(f);

    f._name = "ホストヘッダーの前に複数のスペース";
    f._request = "GET / HTTP/1.1\r\n  Host: sample\r\n\r\n";
    f._isSuccessTest = false;  // 400 Bad Request
    tests.push_back(f);

    f._name = "Hostヘッダーに改行を含む";
    f._request = "GET / HTTP/1.1\r\nHost: sam\r\nple\r\n\r\n";
    f._isSuccessTest = false;  // 400 Bad Request
    tests.push_back(f);

    f._name = "Hostヘッダー値にスペース";
    f._request = "GET / HTTP/1.1\r\nHost: sa mple\r\n\r\n";
    f._isSuccessTest = false;  // 400 Bad Request
    tests.push_back(f);

    f._name = "不正なホスト名（Hos）";
    f._request = "GET / HTTP/1.1\r\nHos: sample\r\n\r\n";
    f._isSuccessTest = false;  // 400 Bad Request
    tests.push_back(f);

    f._name = "フィールド名なし、コロンから始まる";
    f._request = "GET / HTTP/1.1\r\n: sample\r\n\r\n";
    f._isSuccessTest = false;  // 400 Bad Request
    tests.push_back(f);

    f._name = "Host重複";
    f._request = "GET / HTTP/1.1\r\nHost: sample\r\nHost: sample\r\n\r\n";
    f._isSuccessTest = false;  // 400 Bad Request
    tests.push_back(f);

    f._name = "Host重複（大文字小文字混在）";
    f._request = "GET / HTTP/1.1\r\nHost: sample\r\nhost: sample\r\n\r\n";
    f._isSuccessTest = false;  // 400 Bad Request
    tests.push_back(f);

    f._name = "Host重複（値が異なる）";
    f._request = "GET / HTTP/1.1\r\nHost: sample\r\nHost: sam\r\n\r\n";
    f._isSuccessTest = false;  // 400 Bad Request
    tests.push_back(f);

    f._name = "フィールド名なし（コロンのみ）";
    f._request = "GET / HTTP/1.1\r\nHost: sample\r\n: text/html\r\n\r\n";
    f._isSuccessTest = false;  // 400 Bad Request
    tests.push_back(f);

    f._name = "コロンなしのヘッダー";
    f._request = "GET / HTTP/1.1\r\nHost: sample\r\nAccept text/html\r\n\r\n";
    f._isSuccessTest = false;  // 400 Bad Request
    tests.push_back(f);

    f._name = "ヘッダー値内に制御文字";
    f._request = "GET / HTTP/1.1\r\nHost: sam\tple\r\n\r\n";
    f._isSuccessTest = false;  // 400 Bad Request
    tests.push_back(f);

    f._name = "ヘッダーフィールド名に空白";
    f._request = "GET / HTTP/1.1\r\nHo st: sample\r\n\r\n";
    f._isSuccessTest = false;  // 400 Bad Request
    tests.push_back(f);

    f._name = "ヘッダーフィールド名に特殊文字";
    f._request = "GET / HTTP/1.1\r\nHo*st: sample\r\n\r\n";
    f._isSuccessTest = false;  // 400 Bad Request
    tests.push_back(f);

    f._name = "ヘッダーフィールドの値が空白のみ";
    f._request = "GET / HTTP/1.1\r\nHost:    \r\n\r\n";
    f._isSuccessTest = false;  // 400 Bad Request
    tests.push_back(f);

    // -------------------------------------------------------------------------
    // Content-Length ヘッダーの検証
    // -------------------------------------------------------------------------
    f._name = "Content-Lengthの検証（正常値）";
    f._request = "POST / HTTP/1.1\r\nHost: sample\r\n"
                    "Content-Length: 10\r\n\r\n";
    f._isSuccessTest = true;  // 200 OK
    tests.push_back(f);

    f._name = "Content-Lengthが複数成功";
    f._request = "POST / HTTP/1.1\r\nHost: sample\r\nContent-Length: 20\r\n"
                    "Content-Length: 20\r\n\r\n";
    f._isSuccessTest = true;  // 200 OK
    tests.push_back(f);

    f._name = "Content-Lengthが複数成功";
    f._request = "POST / HTTP/1.1\r\nHost: sample\r\n"
                    "Content-Length: 10, 10\r\n\r\n";
    f._isSuccessTest = true;  // 200 OK
    tests.push_back(f);

    f._name = "Content-Lengthが数値でない";
    f._request = "POST / HTTP/1.1\r\nHost: sample\r\n"
                    "Content-Length: abc\r\n\r\n";
    f._isSuccessTest = false;  // 400 Bad Request
    tests.push_back(f);

    f._name = "Content-Lengthが負の値";
    f._request = "POST / HTTP/1.1\r\nHost: sample\r\n"
                    "Content-Length: -10\r\n\r\n";
    f._isSuccessTest = false;  // 400 Bad Request
    tests.push_back(f);

    f._name = "Content-Lengthが複数失敗";
    f._request = "POST / HTTP/1.1\r\nHost: sample\r\nContent-Length: 10\r\n"
                    "Content-Length: 20\r\n\r\n";
    f._isSuccessTest = false;  // 400 Bad Request
    tests.push_back(f);

    f._name = "Content-Lengthが複数失敗";
    f._request = "POST / HTTP/1.1\r\nHost: sample\r\n"
                    "Content-Length: 10, 20\r\n\r\n";
    f._isSuccessTest = false;  // 400 Bad Request
    tests.push_back(f);

    f._name = "制限を超えるContent-Length";
    f._request = "POST / HTTP/1.1\r\nHost: sample\r\n"
                    "Content-Length: 10000000\r\n\r\n";
    f._isSuccessTest = false;  // 413 Request Entity Too Large
    tests.push_back(f);

    // -------------------------------------------------------------------------
    // 大きなリクエストとエンコーディングの検証
    // -------------------------------------------------------------------------
    f._name = "非常に長いヘッダー値";
    {
        std::string longValue = "";
        for (int i = 0; i < 8192; ++i) {
            longValue += "a";
        }
        f._request = "GET / HTTP/1.1\r\nHost: " + longValue + "\r\n\r\n";
    }
    f._isSuccessTest = false;  // 長さ制限がなければ成功
    tests.push_back(f);

    // f._name = "最大ヘッダー数超過";
    // {
    //     std::string manyHeaders = "GET / HTTP/1.1\r\nHost: sample\r\n";
    //     for (int i = 0; i < 100; ++i) {
    //         manyHeaders += "X-Custom-" + toolbox::to_string(i)
                // + ": value\r\n";
    //     }
    //     manyHeaders += "\r\n";
    //     f._request = manyHeaders;
    // }
    // f._isSuccessTest = false;  // 431 Request Header Fields Too Large
    // tests.push_back(f);

    f._name = "Transfer-Encoding: chunkedの検証";
    f._request = "POST / HTTP/1.1\r\nHost: sample\r\n"
                    "Transfer-Encoding: chunked\r\n\r\n0\r\n\r\n\r\n";
    f._isSuccessTest = true;  // 200 OK
    tests.push_back(f);

    f._name = "Accept-Encodingヘッダー処理";
    f._request = "GET / HTTP/1.1\r\nHost: sample\r\n"
                    "Accept-Encoding: gzip, deflate\r\n\r\n";
    f._isSuccessTest = true;  // 200 OK
    tests.push_back(f);

    // -------------------------------------------------------------------------
    // サーバー設定とルーティングの検証
    // -------------------------------------------------------------------------
    f._name = "サーバー名指定（server_name一致）";
    f._request = "GET / HTTP/1.1\r\nHost: localhost\r\n\r\n";
    f._isSuccessTest = true;  // 200 OK
    tests.push_back(f);

    f._name = "サーバー名指定（server_name不一致）";
    f._request = "GET / HTTP/1.1\r\nHost: unknown-server\r\n\r\n";
    f._isSuccessTest = true;  // 200 OK（デフォルトサーバーで処理）
    tests.push_back(f);

    f._name = "サーバー名とポート指定";
    f._request = "GET / HTTP/1.1\r\nHost: localhost:8080\r\n\r\n";
    f._isSuccessTest = true;  // 200 OK
    tests.push_back(f);

    // -------------------------------------------------------------------------
    // 認証とセキュリティ関連の検証
    // -------------------------------------------------------------------------

    f._name = "Cookie設定検証";
    f._request = "GET / HTTP/1.1\r\nHost: sample\r\n"
                    "Cookie: session=abc123\r\n\r\n";
    f._isSuccessTest = true;  // 200 OK
    tests.push_back(f);

    // -------------------------------------------------------------------------
    // CGI関連の検証
    // -------------------------------------------------------------------------
    f._name = "CGI実行用のContent-Typeテスト（PHP）";
    f._request = "POST /cgi-bin/script.php HTTP/1.1\r\nHost: sample\r\n"
                    "Content-Type: application/x-www-form-urlencoded\r\n"
                    "Content-Length: 9\r\n\r\nkey=value";
    f._isSuccessTest = true;  // 200 OK
    tests.push_back(f);
}

void fieldTest() {
    TestVector tests;

    makeFieldTest(tests);
    int pass = 0;
    for (std::size_t i = 0; i < tests.size(); ++i) {
        if (runTest(tests[i])) {
            ++pass;
        }
    }
    std::cout << pass << " / " << tests.size() << std::endl;
}

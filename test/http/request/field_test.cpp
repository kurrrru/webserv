// Copyright 2025 Ideal Broccoli

#include <vector>
#include <string>
#include <iostream>
#include <map>

#include "request/field_test.hpp"

typedef std::vector<FieldTest> TestVector;

void showField(http::RequestParser& r, FieldTest& t) {
    std::cout << "----- field -----" << std::endl;
    for (HTTPFields::FieldMap::iterator it = r.get().fields.get().begin();
                it != r.get().fields.get().end(); ++it) {
        if (!it->second.empty()) {
            std::cout << it->first << ": ";
            for (std::size_t i = 0; i < it->second.size(); ++i) {
                std::cout << it->second[i] << " ";
            }
            std::cout << std::endl;
            HTTPFields::FieldMap::iterator it_t = t._exceptMap.find(it->first);
            std::cout << it_t->first << ": ";
            for (std::size_t i = 0; i < it_t->second.size(); ++i) {
                std::cout << it_t->second[i] << " ";
            }
            std::cout << std::endl;
        }
    }
}

bool compareFields(http::RequestParser& r, FieldTest& t) {
    for (HTTPFields::FieldMap::iterator it_r = r.get().fields.get().begin();
            it_r != r.get().fields.get().end(); ++it_r) {
        HTTPFields::FieldMap::iterator it_t  = t._exceptMap.find(it_r->first);
        if (it_r->second.empty()) {
            if (it_t == t._exceptMap.end()) {
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

bool runTest(FieldTest& t) {
    http::RequestParser r;
    try {
        r.run(t._request);
        if (t._isSuccessTest && compareFields(r, t)) {
            return true;
        } else {
            std::cout << "==== Failed: false->true " \
                << t._name << " ====" << std::endl;
            std::cout << t._request;
            showField(r, t);
            return false;
        }
    } catch (std::exception& e) {
        if (!t._isSuccessTest) {
            return true;
        } else {
            std::cout << "==== Failed: true->false " \
                << t._name << " ====" << std::endl;
            std::cout << e.what() << std::endl;
            std::cout << t._request;
            showField(r, t);
            return false;
        }
    }
}

void addFieldToMap(std::map<std::string, std::vector<std::string> >& map,
                    const std::string& key,
                    const std::string& value) {
        map[key].push_back(value);
}

// void makeFieldTest(TestVector& tests) {
//     FieldTest f;

//     // -------------------------------------------------------------------------
//     // HTTPヘッダーフィールドの一般的なフォーマットテスト
//     // General format tests for HTTP header fields
//     // -------------------------------------------------------------------------
//     f._name = "基本的なGETリクエスト";
//     f._request = "GET / HTTP/1.1\r\nHost: sample\r\n\r\n";
//     f._isSuccessTest = true;  // 200 OK
//     addFieldToMap(f._exceptMap, http::fields::HOST, "sample");
//     tests.push_back(f);
//     f._exceptMap.clear();

//     f._name = "複数のヘッダーフィールド";
//     f._request = "GET / HTTP/1.1\r\nHost: sample\r\nAccept: text/html\r\n\r\n";
//     f._isSuccessTest = true;  // 200 OK
//     addFieldToMap(f._exceptMap, http::fields::HOST, "sample");
//     addFieldToMap(f._exceptMap, http::fields::ACCEPT, "text/html");
//     tests.push_back(f);
//     f._exceptMap.clear();

//     f._name = "Hostフィールドが先頭にない";
//     f._request = "GET / HTTP/1.1\r\nAccept: text/html\r\nHost: sample\r\n\r\n";
//     f._isSuccessTest = true;  // 200 OK
//     addFieldToMap(f._exceptMap, http::fields::HOST, "sample");
//     addFieldToMap(f._exceptMap, http::fields::ACCEPT, "text/html");
//     tests.push_back(f);
//     f._exceptMap.clear();

//     f._name = "フィールド値が空";
//     f._request = "GET / HTTP/1.1\r\nHost: sample\r\nAccept:\r\n\r\n";
//     f._isSuccessTest = true;  // 200 OK
//     addFieldToMap(f._exceptMap, http::fields::HOST, "sample");
//     tests.push_back(f);
//     f._exceptMap.clear();

//     f._name = "存在しないフィールド";
//     f._request = "GET / HTTP/1.1\r\nHost: sample\r\nINVALID: text/html\r\n\r\n";
//     f._isSuccessTest = true;  // 200 OK
//     addFieldToMap(f._exceptMap, http::fields::HOST, "sample");
//     tests.push_back(f);
//     f._exceptMap.clear();

//     f._name = "存在しないフィールドの重複";
//     f._request = "GET / HTTP/1.1\r\nHost: sample\r\nINVALID: text/html\r\n"
//                     "INVALID: text/html\r\n\r\n";
//     f._isSuccessTest = true;  // 200 OK
//     addFieldToMap(f._exceptMap, http::fields::HOST, "sample");
//     tests.push_back(f);
//     f._exceptMap.clear();

//     f._name = "ヘッダーフィールド名の大文字小文字混在";
//     f._request = "GET / HTTP/1.1\r\nHost: sample\r\n"
//                     "CoNtEnT-TyPe: text/html\r\n\r\n";
//     f._isSuccessTest = true;  // 200 OK
//     addFieldToMap(f._exceptMap, http::fields::HOST, "sample");
//     addFieldToMap(f._exceptMap, http::fields::CONTENT_TYPE, "text/html");
//     tests.push_back(f);
//     f._exceptMap.clear();

//     f._name = "ヘッダー値の末尾の空白";
//     f._request = "GET / HTTP/1.1\r\nHost: sample\r\n"
//                         "Accept: text/html   \r\n\r\n";
//     f._isSuccessTest = true;  // 200 OK
//     addFieldToMap(f._exceptMap, http::fields::HOST, "sample");
//     addFieldToMap(f._exceptMap, http::fields::ACCEPT, "text/html");
//     tests.push_back(f);
//     f._exceptMap.clear();

//     f._name = "非常に長いフィールド名";
//     f._request = "GET / HTTP/1.1\r\nHost: sample\r\n"
//                     "X-Really-Really-Really-Really-"
//                     "Really-Really-Long-Header-Name: value\r\n\r\n";
//     f._isSuccessTest = true;  // 200 OK
//     addFieldToMap(f._exceptMap, http::fields::HOST, "sample");
//     addFieldToMap(f._exceptMap, "X-Really-Really-Really-Really-Really"
//                                     "-Really-Long-Header-Name", "value");
//     tests.push_back(f);
//     f._exceptMap.clear();

//     // -------------------------------------------------------------------------
//     // Hostヘッダーのフォーマット検証
//     // -------------------------------------------------------------------------
//     f._name = "Hostヘッダーが区切り文字前にスペースなし";
//     f._request = "GET / HTTP/1.1\r\nHost:sample\r\n\r\n";
//     f._isSuccessTest = true;  // 200 OK
//     addFieldToMap(f._exceptMap, http::fields::HOST, "sample");
//     tests.push_back(f);
//     f._exceptMap.clear();


//     f._name = "Hostヘッダー値の前に複数スペース";
//     f._request = "GET / HTTP/1.1\r\nHost:  sample\r\n\r\n";
//     f._isSuccessTest = true;  // 200 OK
//     addFieldToMap(f._exceptMap, http::fields::HOST, "sample");
//     tests.push_back(f);
//     f._exceptMap.clear();


//     f._name = "ヘッダーフィールド値が引用符で囲まれている";
//     f._request = "GET / HTTP/1.1\r\nHost: \"sample\"\r\n\r\n";
//     f._isSuccessTest = true;  // 200 OK
//     addFieldToMap(f._exceptMap, http::fields::HOST, "\"sample\"");
//     tests.push_back(f);
//     f._exceptMap.clear();


//     f._name = "Hostヘッダーの後にスペース";
//     f._request = "GET / HTTP/1.1\r\nHost : sample\r\n\r\n";
//     f._isSuccessTest = false;  // 400 Bad Request
//     tests.push_back(f);
//     f._exceptMap.clear();


//     f._name = "ホストヘッダーの前に複数のスペース";
//     f._request = "GET / HTTP/1.1\r\n  Host: sample\r\n\r\n";
//     f._isSuccessTest = false;  // 400 Bad Request
//     tests.push_back(f);
//     f._exceptMap.clear();


//     f._name = "Hostヘッダーに改行を含む";
//     f._request = "GET / HTTP/1.1\r\nHost: sam\r\nple\r\n\r\n";
//     f._isSuccessTest = false;  // 400 Bad Request
//     tests.push_back(f);
//     f._exceptMap.clear();


//     f._name = "Hostヘッダー値にスペース";
//     f._request = "GET / HTTP/1.1\r\nHost: sa mple\r\n\r\n";
//     f._isSuccessTest = false;  // 400 Bad Request
//     tests.push_back(f);
//     f._exceptMap.clear();


//     f._name = "不正なホスト名（Hos）";
//     f._request = "GET / HTTP/1.1\r\nHos: sample\r\n\r\n";
//     f._isSuccessTest = false;  // 400 Bad Request
//     tests.push_back(f);
//     f._exceptMap.clear();


//     f._name = "フィールド名なし、コロンから始まる";
//     f._request = "GET / HTTP/1.1\r\n: sample\r\n\r\n";
//     f._isSuccessTest = false;  // 400 Bad Request
//     tests.push_back(f);
//     f._exceptMap.clear();


//     f._name = "Host重複";
//     f._request = "GET / HTTP/1.1\r\nHost: sample\r\nHost: sample\r\n\r\n";
//     f._isSuccessTest = false;  // 400 Bad Request
//     tests.push_back(f);
//     f._exceptMap.clear();


//     f._name = "Host重複";
//     f._request = "GET / HTTP/1.1\r\nHost: sample, samplee\r\n\r\n";
//     f._isSuccessTest = false;  // 400 Bad Request
//     tests.push_back(f);
//     f._exceptMap.clear();


//     f._name = "Host重複（大文字小文字混在）";
//     f._request = "GET / HTTP/1.1\r\nHost: sample\r\nhost: sample\r\n\r\n";
//     f._isSuccessTest = false;  // 400 Bad Request
//     tests.push_back(f);
//     f._exceptMap.clear();


//     f._name = "Host重複（値が異なる）";
//     f._request = "GET / HTTP/1.1\r\nHost: sample\r\nHost: sam\r\n\r\n";
//     f._isSuccessTest = false;  // 400 Bad Request
//     tests.push_back(f);
//     f._exceptMap.clear();

//     // -------------------------------------------------------------------------
//     // 日本語を含むヘッダーフィールドのテスト
//     // Tests for header fields containing Japanese characters
//     // -------------------------------------------------------------------------
//     f._name = "ヘッダーフィールド値に日本語"; // Japanese characters in header field value
//     f._request = "GET / HTTP/1.1\r\nHost: sample\r\nX-Comment: こんにちは世界\r\n\r\n";
//     f._isSuccessTest = true;  // 200 OK
//     addFieldToMap(f._exceptMap, http::fields::HOST, "sample");
//     addFieldToMap(f._exceptMap, "X-Comment", "こんにちは世界");
//     tests.push_back(f);
//     f._exceptMap.clear();

//     // -------------------------------------------------------------------------
//     // 特殊文字を含むヘッダーフィールドのテスト
//     // Tests for header fields containing special characters
//     // -------------------------------------------------------------------------
//     f._name = "フィールド値に特殊文字"; // Special characters in field value
//     f._request = "GET / HTTP/1.1\r\nHost: sample\r\nX-Special: !@#$%^&*()_+\r\n\r\n";
//     f._isSuccessTest = true;  // 200 OK
//     addFieldToMap(f._exceptMap, http::fields::HOST, "sample");
//     addFieldToMap(f._exceptMap, "X-Special", "!@#$%^&*()_+");
//     tests.push_back(f);
//     f._exceptMap.clear();

//     // -------------------------------------------------------------------------
//     // 誤った区切り文字を使用したヘッダーのテスト
//     // Tests for headers with incorrect delimiters
//     // -------------------------------------------------------------------------
//     f._name = "ヘッダーフィールドにセミコロン使用"; // Semicolon in header field instead of colon
//     f._request = "GET / HTTP/1.1\r\nHost: sample\r\nAccept; text/html\r\n\r\n";
//     f._isSuccessTest = false;  // 400 Bad Request
//     tests.push_back(f);
//     f._exceptMap.clear();

//     f._name = "ヘッダー区切りが不完全"; // Incomplete header delimiter
//     f._request = "GET / HTTP/1.1\r\nHost: sample\r\nAccept:\r\n\r\n";
//     f._isSuccessTest = true;  // 200 OK (empty field value is allowed)
//     addFieldToMap(f._exceptMap, http::fields::HOST, "sample");
//     tests.push_back(f);
//     f._exceptMap.clear();

//     f._name = "フィールド名なし（コロンのみ）";
//     f._request = "GET / HTTP/1.1\r\nHost: sample\r\n: text/html\r\n\r\n";
//     f._isSuccessTest = false;  // 400 Bad Request
//     tests.push_back(f);
//     f._exceptMap.clear();

//     f._name = "コロンなしのヘッダー";
//     f._request = "GET / HTTP/1.1\r\nHost: sample\r\nAccept text/html\r\n\r\n";
//     f._isSuccessTest = false;  // 400 Bad Request
//     tests.push_back(f);
//     f._exceptMap.clear();

//     f._name = "ヘッダー値内に制御文字";
//     f._request = "GET / HTTP/1.1\r\nHost: sam\tple\r\n\r\n";
//     f._isSuccessTest = false;  // 400 Bad Request
//     tests.push_back(f);
//     f._exceptMap.clear();

//     f._name = "ヘッダーフィールド名に空白";
//     f._request = "GET / HTTP/1.1\r\nHo st: sample\r\n\r\n";
//     f._isSuccessTest = false;  // 400 Bad Request
//     tests.push_back(f);
//     f._exceptMap.clear();

//     f._name = "ヘッダーフィールド名に特殊文字";
//     f._request = "GET / HTTP/1.1\r\nHo*st: sample\r\n\r\n";
//     f._isSuccessTest = false;  // 400 Bad Request
//     tests.push_back(f);
//     f._exceptMap.clear();

//     f._name = "ヘッダーフィールドの値が空白のみ";
//     f._request = "GET / HTTP/1.1\r\nHost:    \r\n\r\n";
//     f._isSuccessTest = false;  // 400 Bad Request
//     tests.push_back(f);
//     f._exceptMap.clear();

//     // -------------------------------------------------------------------------
//     // Content-Length ヘッダーの検証
//     // -------------------------------------------------------------------------
//     f._name = "Content-Lengthの検証（正常値）";
//     f._request = "POST / HTTP/1.1\r\nHost: sample\r\n"
//                     "Content-Length: 10\r\n\r\n";
//     addFieldToMap(f._exceptMap, http::fields::HOST, "sample");
//     addFieldToMap(f._exceptMap, http::fields::CONTENT_LENGTH, "10");
//     f._isSuccessTest = true;  // 200 OK
//     tests.push_back(f);
//     f._exceptMap.clear();


//     f._name = "Content-Lengthが複数失敗";
//     f._request = "POST / HTTP/1.1\r\nHost: sample\r\nContent-Length: 20\r\n"
//                     "Content-Length: 20\r\n\r\n";
//     f._isSuccessTest = false;  // 400 Bad Request
//     tests.push_back(f);
//     f._exceptMap.clear();


//     f._name = "Content-Lengthが複数成功";
//     f._request = "POST / HTTP/1.1\r\nHost: sample\r\n"
//                     "Content-Length: 10, 10\r\n\r\n";
//     f._isSuccessTest = true;  // 200 OK
//     addFieldToMap(f._exceptMap, http::fields::HOST, "sample");
//     addFieldToMap(f._exceptMap, http::fields::CONTENT_LENGTH, "10");
//     addFieldToMap(f._exceptMap, http::fields::CONTENT_LENGTH, "10");
//     tests.push_back(f);
//     f._exceptMap.clear();


//     f._name = "Content-Lengthが数値でない";
//     f._request = "POST / HTTP/1.1\r\nHost: sample\r\n"
//                     "Content-Length: abc\r\n\r\n";
//     f._isSuccessTest = false;  // 400 Bad Request
//     tests.push_back(f);
//     f._exceptMap.clear();


//     f._name = "Content-Lengthが負の値";
//     f._request = "POST / HTTP/1.1\r\nHost: sample\r\n"
//                     "Content-Length: -10\r\n\r\n";
//     f._isSuccessTest = false;  // 400 Bad Request
//     tests.push_back(f);
//     f._exceptMap.clear();


//     f._name = "Content-Lengthが複数失敗";
//     f._request = "POST / HTTP/1.1\r\nHost: sample\r\nContent-Length: 10\r\n"
//                     "Content-Length: 20\r\n\r\n";
//     f._isSuccessTest = false;  // 400 Bad Request
//     tests.push_back(f);
//     f._exceptMap.clear();


//     f._name = "Content-Lengthが複数失敗";
//     f._request = "POST / HTTP/1.1\r\nHost: sample\r\n"
//                     "Content-Length: 10, 20\r\n\r\n";
//     f._isSuccessTest = false;  // 400 Bad Request
//     tests.push_back(f);
//     f._exceptMap.clear();


//     f._name = "制限を超えるContent-Length";
//     f._request = "POST / HTTP/1.1\r\nHost: sample\r\n"
//                     "Content-Length: 10000000\r\n\r\n";
//     f._isSuccessTest = false;  // 413 Request Entity Too Large
//     tests.push_back(f);
//     f._exceptMap.clear();


//     // -------------------------------------------------------------------------
//     // 大きなリクエストとエンコーディングの検証
//     // -------------------------------------------------------------------------
//     f._name = "非常に長いヘッダー値";
//     {
//         std::string longValue = "";
//         for (int i = 0; i < 8192; ++i) {
//             longValue += "a";
//         }
//         f._request = "GET / HTTP/1.1\r\nHost: " + longValue + "\r\n\r\n";
//     }
//     f._isSuccessTest = false;  // 長さ制限がなければ成功
//     tests.push_back(f);
//     f._exceptMap.clear();


//     // f._name = "最大ヘッダー数超過";
//     // {
//     //     std::string manyHeaders = "GET / HTTP/1.1\r\nHost: sample\r\n";
//     //     for (int i = 0; i < 100; ++i) {
//     //         manyHeaders += "X-Custom-" + toolbox::to_string(i)
//                 // + ": value\r\n";
//     //     }
//     //     manyHeaders += "\r\n";
//     //     f._request = manyHeaders;
//     // }
//     // f._isSuccessTest = false;  // 431 Request Header Fields Too Large
//     // tests.push_back(f);
//     // f._exceptMap.clear();


//     f._name = "Transfer-Encoding: chunkedの検証";
//     f._request = "POST / HTTP/1.1\r\nHost: sample\r\n"
//                     "Transfer-Encoding: chunked\r\n\r\n0\r\n\r\n\r\n";
//     f._isSuccessTest = true;  // 200 OK
//     addFieldToMap(f._exceptMap, http::fields::HOST, "sample");
//     addFieldToMap(f._exceptMap, http::fields::TRANSFER_ENCODING, "chunked");

//     tests.push_back(f);
//     f._exceptMap.clear();


//     f._name = "Accept-Encodingヘッダー処理";
//     f._request = "GET / HTTP/1.1\r\nHost: sample\r\n"
//                     "Accept-Encoding: gzip, deflate\r\n\r\n";
//     f._isSuccessTest = true;  // 200 OK
//     addFieldToMap(f._exceptMap, http::fields::HOST, "sample");
//     addFieldToMap(f._exceptMap, http::fields::ACCEPT_ENCODING, "gzip");
//     addFieldToMap(f._exceptMap, http::fields::ACCEPT_ENCODING, "deflate");
//     tests.push_back(f);
//     f._exceptMap.clear();


//     // -------------------------------------------------------------------------
//     // サーバー設定とルーティングの検証
//     // -------------------------------------------------------------------------
//     f._name = "サーバー名指定（server_name一致）";
//     f._request = "GET / HTTP/1.1\r\nHost: localhost\r\n\r\n";
//     f._isSuccessTest = true;  // 200 OK
//     addFieldToMap(f._exceptMap, http::fields::HOST, "localhost");
//     tests.push_back(f);
//     f._exceptMap.clear();


//     f._name = "サーバー名指定（server_name不一致）";
//     f._request = "GET / HTTP/1.1\r\nHost: unknown-server\r\n\r\n";
//     f._isSuccessTest = true;  // 200 OK（デフォルトサーバーで処理）
//     addFieldToMap(f._exceptMap, http::fields::HOST, "unknown-server");
//     tests.push_back(f);
//     f._exceptMap.clear();


//     f._name = "サーバー名とポート指定";
//     f._request = "GET / HTTP/1.1\r\nHost: localhost:8080\r\n\r\n";
//     f._isSuccessTest = true;  // 200 OK
//     addFieldToMap(f._exceptMap, http::fields::HOST, "localhost:8080");
//     tests.push_back(f);
//     f._exceptMap.clear();


//     // -------------------------------------------------------------------------
//     // 認証とセキュリティ関連の検証
//     // -------------------------------------------------------------------------

//     f._name = "Cookie設定検証";
//     f._request = "GET / HTTP/1.1\r\nHost: sample\r\n"
//                     "Cookie: session=abc123\r\n\r\n";
//     f._isSuccessTest = true;  // 200 OK
//     addFieldToMap(f._exceptMap, http::fields::HOST, "sample");
//     addFieldToMap(f._exceptMap, http::fields::COOKIE, "session=abc123");
//     tests.push_back(f);
//     f._exceptMap.clear();


//     // -------------------------------------------------------------------------
//     // CGI関連の検証
//     // -------------------------------------------------------------------------
//     f._name = "CGI実行用のContent-Typeテスト（PHP）";
//     f._request = "POST /cgi-bin/script.php HTTP/1.1\r\nHost: sample\r\n"
//                     "Content-Type: application/x-www-form-urlencoded\r\n"
//                     "Content-Length: 9\r\n\r\nkey=value";
//     f._isSuccessTest = true;  // 200 OK
//     addFieldToMap(f._exceptMap, http::fields::HOST, "sample");
//     addFieldToMap(f._exceptMap, http::fields::CONTENT_TYPE, "application/x-www-form-urlencoded");
//     addFieldToMap(f._exceptMap, http::fields::CONTENT_LENGTH, "9");
//     tests.push_back(f);
//     f._exceptMap.clear();

// }

void makeBasicFieldTests(TestVector& tests) {
    FieldTest f;

    // 基本的なGETリクエスト
    f._name = "基本的なGETリクエスト";
    f._request = "GET / HTTP/1.1\r\nHost: sample\r\n\r\n";
    f._isSuccessTest = true;  // 200 OK
    addFieldToMap(f._exceptMap, http::fields::HOST, "sample");
    tests.push_back(f);
    f._exceptMap.clear();

    // 複数のヘッダーフィールド
    f._name = "複数のヘッダーフィールド";
    f._request = "GET / HTTP/1.1\r\nHost: sample\r\nAccept: text/html\r\n\r\n";
    f._isSuccessTest = true;  // 200 OK
    addFieldToMap(f._exceptMap, http::fields::HOST, "sample");
    addFieldToMap(f._exceptMap, http::fields::ACCEPT, "text/html");
    tests.push_back(f);
    f._exceptMap.clear();

    // Hostフィールドが先頭にない
    f._name = "Hostフィールドが先頭にない";
    f._request = "GET / HTTP/1.1\r\nAccept: text/html\r\nHost: sample\r\n\r\n";
    f._isSuccessTest = true;  // 200 OK
    addFieldToMap(f._exceptMap, http::fields::HOST, "sample");
    addFieldToMap(f._exceptMap, http::fields::ACCEPT, "text/html");
    tests.push_back(f);
    f._exceptMap.clear();

    // フィールド値が空
    f._name = "フィールド値が空";
    f._request = "GET / HTTP/1.1\r\nHost: sample\r\nAccept:\r\n\r\n";
    f._isSuccessTest = true;  // 200 OK
    addFieldToMap(f._exceptMap, http::fields::HOST, "sample");
    tests.push_back(f);
    f._exceptMap.clear();

    // ヘッダーフィールド名の大文字小文字混在
    f._name = "ヘッダーフィールド名の大文字小文字混在";
    f._request = "GET / HTTP/1.1\r\nHost: sample\r\n"
                "CoNtEnT-TyPe: text/html\r\n\r\n";
    f._isSuccessTest = true;  // 200 OK
    addFieldToMap(f._exceptMap, http::fields::HOST, "sample");
    addFieldToMap(f._exceptMap, http::fields::CONTENT_TYPE, "text/html");
    tests.push_back(f);
    f._exceptMap.clear();

    // ヘッダー値の末尾の空白
    f._name = "ヘッダー値の末尾の空白";
    f._request = "GET / HTTP/1.1\r\nHost: sample\r\n"
                "Accept: text/html   \r\n\r\n";
    f._isSuccessTest = true;  // 200 OK
    addFieldToMap(f._exceptMap, http::fields::HOST, "sample");
    addFieldToMap(f._exceptMap, http::fields::ACCEPT, "text/html");
    tests.push_back(f);
    f._exceptMap.clear();
}

/**
 * 存在しないフィールドに関するテスト
 * @param tests テストベクター
 */
void makeInvalidFieldTests(TestVector& tests) {
    FieldTest f;

    // 存在しないフィールド
    f._name = "存在しないフィールド";
    f._request = "GET / HTTP/1.1\r\nHost: sample\r\nINVALID: text/html\r\n\r\n";
    f._isSuccessTest = true;  // 200 OK
    addFieldToMap(f._exceptMap, http::fields::HOST, "sample");
    tests.push_back(f);
    f._exceptMap.clear();

    // 存在しないフィールドの重複
    f._name = "存在しないフィールドの重複";
    f._request = "GET / HTTP/1.1\r\nHost: sample\r\nINVALID: text/html\r\n"
                "INVALID: text/html\r\n\r\n";
    f._isSuccessTest = true;  // 200 OK
    addFieldToMap(f._exceptMap, http::fields::HOST, "sample");
    tests.push_back(f);
    f._exceptMap.clear();

    // 非常に長いフィールド名
    f._name = "非常に長いフィールド名";
    f._request = "GET / HTTP/1.1\r\nHost: sample\r\n"
                "X-Really-Really-Really-Really-"
                "Really-Really-Long-Header-Name: value\r\n\r\n";
    f._isSuccessTest = true;  // 200 OK
    addFieldToMap(f._exceptMap, http::fields::HOST, "sample");
    addFieldToMap(f._exceptMap, "X-Really-Really-Really-Really-Really"
                                "-Really-Long-Header-Name", "value");
    tests.push_back(f);
    f._exceptMap.clear();
}

/**
 * Hostヘッダーのフォーマット検証テスト
 * @param tests テストベクター
 */
void makeHostHeaderTests(TestVector& tests) {
    FieldTest f;

    // Hostヘッダーが区切り文字前にスペースなし
    f._name = "Hostヘッダーが区切り文字前にスペースなし";
    f._request = "GET / HTTP/1.1\r\nHost:sample\r\n\r\n";
    f._isSuccessTest = true;  // 200 OK
    addFieldToMap(f._exceptMap, http::fields::HOST, "sample");
    tests.push_back(f);
    f._exceptMap.clear();

    // Hostヘッダー値の前に複数スペース
    f._name = "Hostヘッダー値の前に複数スペース";
    f._request = "GET / HTTP/1.1\r\nHost:  sample\r\n\r\n";
    f._isSuccessTest = true;  // 200 OK
    addFieldToMap(f._exceptMap, http::fields::HOST, "sample");
    tests.push_back(f);
    f._exceptMap.clear();

    // ヘッダーフィールド値が引用符で囲まれている
    f._name = "ヘッダーフィールド値が引用符で囲まれている";
    f._request = "GET / HTTP/1.1\r\nHost: \"sample\"\r\n\r\n";
    f._isSuccessTest = true;  // 200 OK
    addFieldToMap(f._exceptMap, http::fields::HOST, "\"sample\"");
    tests.push_back(f);
    f._exceptMap.clear();

    // Hostヘッダーの後にスペース
    f._name = "Hostヘッダーの後にスペース";
    f._request = "GET / HTTP/1.1\r\nHost : sample\r\n\r\n";
    f._isSuccessTest = false;  // 400 Bad Request
    tests.push_back(f);
    f._exceptMap.clear();

    // ホストヘッダーの前に複数のスペース
    f._name = "ホストヘッダーの前に複数のスペース";
    f._request = "GET / HTTP/1.1\r\n  Host: sample\r\n\r\n";
    f._isSuccessTest = false;  // 400 Bad Request
    tests.push_back(f);
    f._exceptMap.clear();

    // Hostヘッダーに改行を含む
    f._name = "Hostヘッダーに改行を含む";
    f._request = "GET / HTTP/1.1\r\nHost: sam\r\nple\r\n\r\n";
    f._isSuccessTest = false;  // 400 Bad Request
    tests.push_back(f);
    f._exceptMap.clear();

    // Hostヘッダー値にスペース
    f._name = "Hostヘッダー値にスペース";
    f._request = "GET / HTTP/1.1\r\nHost: sa mple\r\n\r\n";
    f._isSuccessTest = false;  // 400 Bad Request
    tests.push_back(f);
    f._exceptMap.clear();

    // 不正なホスト名（Hos）
    f._name = "不正なホスト名（Hos）";
    f._request = "GET / HTTP/1.1\r\nHos: sample\r\n\r\n";
    f._isSuccessTest = false;  // 400 Bad Request
    tests.push_back(f);
    f._exceptMap.clear();

    // フィールド名なし、コロンから始まる
    f._name = "フィールド名なし、コロンから始まる";
    f._request = "GET / HTTP/1.1\r\n: sample\r\n\r\n";
    f._isSuccessTest = false;  // 400 Bad Request
    tests.push_back(f);
    f._exceptMap.clear();

    // Host重複
    f._name = "Host重複";
    f._request = "GET / HTTP/1.1\r\nHost: sample\r\nHost: sample\r\n\r\n";
    f._isSuccessTest = false;  // 400 Bad Request
    tests.push_back(f);
    f._exceptMap.clear();

    // Hostにカンマを含む値
    f._name = "Hostにカンマを含む値";
    f._request = "GET / HTTP/1.1\r\nHost: sample, samplee\r\n\r\n";
    f._isSuccessTest = false;  // 400 Bad Request
    tests.push_back(f);
    f._exceptMap.clear();

    // Host重複（大文字小文字混在）
    f._name = "Host重複（大文字小文字混在）";
    f._request = "GET / HTTP/1.1\r\nHost: sample\r\nhost: sample\r\n\r\n";
    f._isSuccessTest = false;  // 400 Bad Request
    tests.push_back(f);
    f._exceptMap.clear();

    // Host重複（値が異なる）
    f._name = "Host重複（値が異なる）";
    f._request = "GET / HTTP/1.1\r\nHost: sample\r\nHost: sam\r\n\r\n";
    f._isSuccessTest = false;  // 400 Bad Request
    tests.push_back(f);
    f._exceptMap.clear();

    // Hostなし
    f._name = "Hostなし";
    f._request = "GET / HTTP/1.1\r\nAccept: text/html\r\n\r\n";
    f._isSuccessTest = false;  // 400 Bad Request
    tests.push_back(f);
    f._exceptMap.clear();

    // ポート番号を含むHost
    f._name = "ポート番号を含むHost";
    f._request = "GET / HTTP/1.1\r\nHost: localhost:8080\r\n\r\n";
    f._isSuccessTest = true;  // 200 OK
    addFieldToMap(f._exceptMap, http::fields::HOST, "localhost:8080");
    tests.push_back(f);
    f._exceptMap.clear();

    // // 不正なポート番号を含むHost
    // f._name = "不正なポート番号を含むHost";
    // f._request = "GET / HTTP/1.1\r\nHost: localhost:abcd\r\n\r\n";
    // f._isSuccessTest = false;  // 400 Bad Request
    // tests.push_back(f);
    // f._exceptMap.clear();
}

/**
 * 日本語と特殊文字を含むヘッダーフィールドのテスト
 * @param tests テストベクター
 */
void makeSpecialCharacterTests(TestVector& tests) {
    FieldTest f;

    // ヘッダーフィールド値に日本語
    f._name = "ヘッダーフィールド値に日本語";
    f._request = "GET / HTTP/1.1\r\nHost: sample\r\n"
                "X-Comment: こんにちは世界\r\n\r\n";
    f._isSuccessTest = true;  // 200 OK
    addFieldToMap(f._exceptMap, http::fields::HOST, "sample");
    addFieldToMap(f._exceptMap, "X-Comment", "こんにちは世界");
    tests.push_back(f);
    f._exceptMap.clear();

    // フィールド値に特殊文字
    f._name = "フィールド値に特殊文字";
    f._request = "GET / HTTP/1.1\r\nHost: sample\r\n"
                "X-Special: !@#$%^&*()_+\r\n\r\n";
    f._isSuccessTest = true;  // 200 OK
    addFieldToMap(f._exceptMap, http::fields::HOST, "sample");
    addFieldToMap(f._exceptMap, "X-Special", "!@#$%^&*()_+");
    tests.push_back(f);
    f._exceptMap.clear();
}

/**
 * 誤った区切り文字を使用したヘッダーのテスト
 * @param tests テストベクター
 */
void makeDelimiterTests(TestVector& tests) {
    FieldTest f;

    // ヘッダーフィールドにセミコロン使用
    f._name = "ヘッダーフィールドにセミコロン使用";
    f._request = "GET / HTTP/1.1\r\nHost: sample\r\nAccept; text/html\r\n\r\n";
    f._isSuccessTest = false;  // 400 Bad Request
    tests.push_back(f);
    f._exceptMap.clear();

    // ヘッダー区切りが不完全
    f._name = "ヘッダー区切りが不完全";
    f._request = "GET / HTTP/1.1\r\nHost: sample\r\nAccept:\r\n\r\n";
    f._isSuccessTest = true;  // 200 OK (empty field value is allowed)
    addFieldToMap(f._exceptMap, http::fields::HOST, "sample");
    tests.push_back(f);
    f._exceptMap.clear();

    // フィールド名なし（コロンのみ）
    f._name = "フィールド名なし（コロンのみ）";
    f._request = "GET / HTTP/1.1\r\nHost: sample\r\n: text/html\r\n\r\n";
    f._isSuccessTest = false;  // 400 Bad Request
    tests.push_back(f);
    f._exceptMap.clear();

    // コロンなしのヘッダー
    f._name = "コロンなしのヘッダー";
    f._request = "GET / HTTP/1.1\r\nHost: sample\r\nAccept text/html\r\n\r\n";
    f._isSuccessTest = false;  // 400 Bad Request
    tests.push_back(f);
    f._exceptMap.clear();

    // ヘッダー値内に制御文字
    f._name = "ヘッダー値内に制御文字";
    f._request = "GET / HTTP/1.1\r\nHost: sam\tple\r\n\r\n";
    f._isSuccessTest = false;  // 400 Bad Request
    tests.push_back(f);
    f._exceptMap.clear();

    // ヘッダーフィールド名に空白
    f._name = "ヘッダーフィールド名に空白";
    f._request = "GET / HTTP/1.1\r\nHo st: sample\r\n\r\n";
    f._isSuccessTest = false;  // 400 Bad Request
    tests.push_back(f);
    f._exceptMap.clear();

    // ヘッダーフィールド名に特殊文字
    f._name = "ヘッダーフィールド名に特殊文字";
    f._request = "GET / HTTP/1.1\r\nHo*st: sample\r\n\r\n";
    f._isSuccessTest = false;  // 400 Bad Request
    tests.push_back(f);
    f._exceptMap.clear();

    // ヘッダーフィールドの値が空白のみ
    f._name = "ヘッダーフィールドの値が空白のみ";
    f._request = "GET / HTTP/1.1\r\nHost:    \r\n\r\n";
    f._isSuccessTest = false;  // 400 Bad Request
    tests.push_back(f);
    f._exceptMap.clear();
}

/**
 * Content-Length ヘッダーの検証
 * @param tests テストベクター
 */
void makeContentLengthTests(TestVector& tests) {
    FieldTest f;

    // Content-Lengthの検証（正常値）
    f._name = "Content-Lengthの検証（正常値）";
    f._request = "POST / HTTP/1.1\r\nHost: sample\r\n"
                "Content-Length: 10\r\n\r\n";
    addFieldToMap(f._exceptMap, http::fields::HOST, "sample");
    addFieldToMap(f._exceptMap, http::fields::CONTENT_LENGTH, "10");
    f._isSuccessTest = true;  // 200 OK
    tests.push_back(f);
    f._exceptMap.clear();

    // Content-Lengthが複数失敗
    f._name = "Content-Lengthが複数失敗";
    f._request = "POST / HTTP/1.1\r\nHost: sample\r\nContent-Length: 20\r\n"
                "Content-Length: 20\r\n\r\n";
    f._isSuccessTest = false;  // 400 Bad Request
    tests.push_back(f);
    f._exceptMap.clear();

    // Content-Lengthが複数成功
    f._name = "Content-Lengthが複数成功";
    f._request = "POST / HTTP/1.1\r\nHost: sample\r\n"
                "Content-Length: 10, 10\r\n\r\n";
    f._isSuccessTest = true;  // 200 OK
    addFieldToMap(f._exceptMap, http::fields::HOST, "sample");
    addFieldToMap(f._exceptMap, http::fields::CONTENT_LENGTH, "10");
    addFieldToMap(f._exceptMap, http::fields::CONTENT_LENGTH, "10");
    tests.push_back(f);
    f._exceptMap.clear();

    // Content-Lengthが数値でない
    f._name = "Content-Lengthが数値でない";
    f._request = "POST / HTTP/1.1\r\nHost: sample\r\n"
                "Content-Length: abc\r\n\r\n";
    f._isSuccessTest = false;  // 400 Bad Request
    tests.push_back(f);
    f._exceptMap.clear();

    // Content-Lengthが負の値
    f._name = "Content-Lengthが負の値";
    f._request = "POST / HTTP/1.1\r\nHost: sample\r\n"
                "Content-Length: -10\r\n\r\n";
    f._isSuccessTest = false;  // 400 Bad Request
    tests.push_back(f);
    f._exceptMap.clear();

    // Content-Lengthが複数異なる値
    f._name = "Content-Lengthが複数異なる値";
    f._request = "POST / HTTP/1.1\r\nHost: sample\r\nContent-Length: 10\r\n"
                "Content-Length: 20\r\n\r\n";
    f._isSuccessTest = false;  // 400 Bad Request
    tests.push_back(f);
    f._exceptMap.clear();

    // Content-Lengthにカンマ区切りで異なる値
    f._name = "Content-Lengthにカンマ区切りで異なる値";
    f._request = "POST / HTTP/1.1\r\nHost: sample\r\n"
                "Content-Length: 10, 20\r\n\r\n";
    f._isSuccessTest = false;  // 400 Bad Request
    tests.push_back(f);
    f._exceptMap.clear();

    // 制限を超えるContent-Length
    f._name = "制限を超えるContent-Length";
    f._request = "POST / HTTP/1.1\r\nHost: sample\r\n"
                "Content-Length: 10000000\r\n\r\n";
    f._isSuccessTest = false;  // 413 Request Entity Too Large
    tests.push_back(f);
    f._exceptMap.clear();

    // Content-LengthとTransfer-Encodingの両方が存在する場合
    f._name = "Content-LengthとTransfer-Encodingの両方が存在";
    f._request = "POST / HTTP/1.1\r\nHost: sample\r\n"
                "Content-Length: 10\r\n"
                "Transfer-Encoding: chunked\r\n\r\n";
    f._isSuccessTest = false;  // 400 Bad Request
    tests.push_back(f);
    f._exceptMap.clear();

    // Content-Lengthが0
    f._name = "Content-Lengthが0";
    f._request = "POST / HTTP/1.1\r\nHost: sample\r\n"
                "Content-Length: 0\r\n\r\n";
    f._isSuccessTest = true;  // 200 OK
    addFieldToMap(f._exceptMap, http::fields::HOST, "sample");
    addFieldToMap(f._exceptMap, http::fields::CONTENT_LENGTH, "0");
    tests.push_back(f);
    f._exceptMap.clear();
}

/**
 * Transfer-Encoding ヘッダーの検証
 * @param tests テストベクター
 */
void makeTransferEncodingTests(TestVector& tests) {
    FieldTest f;

    // Transfer-Encoding: chunkedの検証
    f._name = "Transfer-Encoding: chunkedの検証";
    f._request = "POST / HTTP/1.1\r\nHost: sample\r\n"
                "Transfer-Encoding: chunked\r\n\r\n0\r\n\r\n\r\n";
    f._isSuccessTest = true;  // 200 OK
    addFieldToMap(f._exceptMap, http::fields::HOST, "sample");
    addFieldToMap(f._exceptMap, http::fields::TRANSFER_ENCODING, "chunked");
    tests.push_back(f);
    f._exceptMap.clear();

    // Transfer-Encoding: chunkedとその他のエンコーディング
    f._name = "Transfer-Encoding: chunkedとその他のエンコーディング";
    f._request = "POST / HTTP/1.1\r\nHost: sample\r\n"
                "Transfer-Encoding: gzip, chunked\r\n\r\n0\r\n\r\n\r\n";
    f._isSuccessTest = true;  // 200 OK
    addFieldToMap(f._exceptMap, http::fields::HOST, "sample");
    addFieldToMap(f._exceptMap, http::fields::TRANSFER_ENCODING, "gzip");
    addFieldToMap(f._exceptMap, http::fields::TRANSFER_ENCODING, "chunked");
    tests.push_back(f);
    f._exceptMap.clear();

    // 不正なTransfer-Encoding
    f._name = "不正なTransfer-Encoding";
    f._request = "POST / HTTP/1.1\r\nHost: sample\r\n"
                "Transfer-Encoding: invalid\r\n\r\n";
    f._isSuccessTest = false;  // 400 Bad Request
    tests.push_back(f);
    f._exceptMap.clear();

    // Transfer-Encodingが複数存在
    f._name = "Transfer-Encodingが複数存在";
    f._request = "POST / HTTP/1.1\r\nHost: sample\r\n"
                "Transfer-Encoding: chunked\r\n"
                "Transfer-Encoding: gzip\r\n\r\n";
    f._isSuccessTest = false;  // 400 Bad Request
    tests.push_back(f);
    f._exceptMap.clear();

    // chunkedが最後にない場合
    f._name = "chunkedが最後にない場合";
    f._request = "POST / HTTP/1.1\r\nHost: sample\r\n"
                "Transfer-Encoding: chunked, gzip\r\n\r\n";
    f._isSuccessTest = false;  // 400 Bad Request
    tests.push_back(f);
    f._exceptMap.clear();
}

/**
 * エンコーディング関連のヘッダーテスト
 * @param tests テストベクター
 */
void makeEncodingTests(TestVector& tests) {
    FieldTest f;

    // Accept-Encodingヘッダー処理
    f._name = "Accept-Encodingヘッダー処理";
    f._request = "GET / HTTP/1.1\r\nHost: sample\r\n"
                "Accept-Encoding: gzip, deflate\r\n\r\n";
    f._isSuccessTest = true;  // 200 OK
    addFieldToMap(f._exceptMap, http::fields::HOST, "sample");
    addFieldToMap(f._exceptMap, http::fields::ACCEPT_ENCODING, "gzip");
    addFieldToMap(f._exceptMap, http::fields::ACCEPT_ENCODING, "deflate");
    tests.push_back(f);
    f._exceptMap.clear();

    // Content-Encoding処理
    f._name = "Content-Encodingヘッダー処理";
    f._request = "POST / HTTP/1.1\r\nHost: sample\r\n"
                "Content-Encoding: gzip\r\n"
                "Content-Length: 10\r\n\r\n";
    f._isSuccessTest = true;  // 200 OK
    addFieldToMap(f._exceptMap, http::fields::HOST, "sample");
    addFieldToMap(f._exceptMap, http::fields::CONTENT_ENCODING, "gzip");
    addFieldToMap(f._exceptMap, http::fields::CONTENT_LENGTH, "10");
    tests.push_back(f);
    f._exceptMap.clear();
}

/**
 * サーバー設定とルーティングの検証
 * @param tests テストベクター
 */
void makeServerConfigTests(TestVector& tests) {
    FieldTest f;

    // サーバー名指定（server_name一致）
    f._name = "サーバー名指定（server_name一致）";
    f._request = "GET / HTTP/1.1\r\nHost: localhost\r\n\r\n";
    f._isSuccessTest = true;  // 200 OK
    addFieldToMap(f._exceptMap, http::fields::HOST, "localhost");
    tests.push_back(f);
    f._exceptMap.clear();

    // サーバー名指定（server_name不一致）
    f._name = "サーバー名指定（server_name不一致）";
    f._request = "GET / HTTP/1.1\r\nHost: unknown-server\r\n\r\n";
    f._isSuccessTest = true;  // 200 OK（デフォルトサーバーで処理）
    addFieldToMap(f._exceptMap, http::fields::HOST, "unknown-server");
    tests.push_back(f);
    f._exceptMap.clear();

    // サーバー名とポート指定
    f._name = "サーバー名とポート指定";
    f._request = "GET / HTTP/1.1\r\nHost: localhost:8080\r\n\r\n";
    f._isSuccessTest = true;  // 200 OK
    addFieldToMap(f._exceptMap, http::fields::HOST, "localhost:8080");
    tests.push_back(f);
    f._exceptMap.clear();
}

/**
 * Cookie関連のテスト
 * @param tests テストベクター
 */
void makeCookieTests(TestVector& tests) {
    FieldTest f;

    // Cookie設定検証
    f._name = "Cookie設定検証";
    f._request = "GET / HTTP/1.1\r\nHost: sample\r\n"
                "Cookie: session=abc123\r\n\r\n";
    f._isSuccessTest = true;  // 200 OK
    addFieldToMap(f._exceptMap, http::fields::HOST, "sample");
    addFieldToMap(f._exceptMap, http::fields::COOKIE, "session=abc123");
    tests.push_back(f);
    f._exceptMap.clear();

    // 複数のCookie設定
    f._name = "複数のCookie設定";
    f._request = "GET / HTTP/1.1\r\nHost: sample\r\n"
                "Cookie: session=abc123; user=john\r\n\r\n";
    f._isSuccessTest = true;  // 200 OK
    addFieldToMap(f._exceptMap, http::fields::HOST, "sample");
    addFieldToMap(f._exceptMap, http::fields::COOKIE, "session=abc123; user=john");
    tests.push_back(f);
    f._exceptMap.clear();
}

/**
 * CGI関連の検証
 * @param tests テストベクター
 */
void makeCGITests(TestVector& tests) {
    FieldTest f;

    // CGI実行用のContent-Typeテスト（PHP）
    f._name = "CGI実行用のContent-Typeテスト（PHP）";
    f._request = "POST /cgi-bin/script.php HTTP/1.1\r\nHost: sample\r\n"
                "Content-Type: application/x-www-form-urlencoded\r\n"
                "Content-Length: 9\r\n\r\nkey=value";
    f._isSuccessTest = true;  // 200 OK
    addFieldToMap(f._exceptMap, http::fields::HOST, "sample");
    addFieldToMap(f._exceptMap, http::fields::CONTENT_TYPE, 
                 "application/x-www-form-urlencoded");
    addFieldToMap(f._exceptMap, http::fields::CONTENT_LENGTH, "9");
    tests.push_back(f);
    f._exceptMap.clear();
    
    // マルチパートフォームデータ
    f._name = "マルチパートフォームデータ";
    f._request = "POST / HTTP/1.1\r\nHost: sample\r\n"
                "Content-Type: multipart/form-data; boundary=boundary\r\n"
                "Content-Length: 10\r\n\r\n";
    f._isSuccessTest = true;  // 200 OK
    addFieldToMap(f._exceptMap, http::fields::HOST, "sample");
    addFieldToMap(f._exceptMap, http::fields::CONTENT_TYPE, 
                 "multipart/form-data; boundary=boundary");
    addFieldToMap(f._exceptMap, http::fields::CONTENT_LENGTH, "10");
    tests.push_back(f);
    f._exceptMap.clear();
}

/**
 * Dateヘッダーの検証
 * @param tests テストベクター
 */
void makeDateHeaderTests(TestVector& tests) {
    FieldTest f;

    // 正しい形式のDateヘッダー
    f._name = "正しい形式のDateヘッダー";
    f._request = "GET / HTTP/1.1\r\nHost: sample\r\n"
                "Date: Fri, 15 May 2023 12:30:45 GMT\r\n\r\n";
    f._isSuccessTest = true;  // 200 OK
    addFieldToMap(f._exceptMap, http::fields::HOST, "sample");
    addFieldToMap(f._exceptMap, http::fields::DATE, "Fri, 15 May 2023 12:30:45 GMT");
    tests.push_back(f);
    f._exceptMap.clear();

    // 不正な形式のDateヘッダー
    f._name = "不正な形式のDateヘッダー";
    f._request = "GET / HTTP/1.1\r\nHost: sample\r\n"
                "Date: 2023-05-15 12:30:45\r\n\r\n";
    f._isSuccessTest = false;  // 400 Bad Request
    tests.push_back(f);
    f._exceptMap.clear();

    // 存在しない日付のDateヘッダー
    f._name = "存在しない日付のDateヘッダー";
    f._request = "GET / HTTP/1.1\r\nHost: sample\r\n"
                "Date: Fri, 32 May 2023 12:30:45 GMT\r\n\r\n";
    f._isSuccessTest = false;  // 400 Bad Request
    tests.push_back(f);
    f._exceptMap.clear();
}

/**
 * Serverヘッダーの検証
 * @param tests テストベクター
 */
void makeServerHeaderTests(TestVector& tests) {
    FieldTest f;

    // 基本的なServerヘッダー
    f._name = "基本的なServerヘッダー";
    f._request = "GET / HTTP/1.1\r\nHost: sample\r\n"
                "Server: Apache/2.4.41 (Unix)\r\n\r\n";
    f._isSuccessTest = true;  // 200 OK
    addFieldToMap(f._exceptMap, http::fields::HOST, "sample");
    addFieldToMap(f._exceptMap, http::fields::SERVER, "Apache/2.4.41 (Unix)");
    tests.push_back(f);
    f._exceptMap.clear();

    // 複雑なServerヘッダー
    f._name = "複雑なServerヘッダー";
    f._request = "GET / HTTP/1.1\r\nHost: sample\r\n"
                "Server: Apache/2.4.41 (Unix) OpenSSL/1.1.1d PHP/7.4.3\r\n\r\n";
    f._isSuccessTest = true;  // 200 OK
    addFieldToMap(f._exceptMap, http::fields::HOST, "sample");
    addFieldToMap(f._exceptMap, http::fields::SERVER, 
                 "Apache/2.4.41 (Unix) OpenSSL/1.1.1d PHP/7.4.3");
    tests.push_back(f);
    f._exceptMap.clear();
}

/**
 * 大きなリクエストとエンコーディングの検証
 * @param tests テストベクター
 */
void makeLargeRequestTests(TestVector& tests) {
    FieldTest f;

    // 非常に長いヘッダー値
    f._name = "非常に長いヘッダー値";
    {
        std::string longValue = "";
        for (int i = 0; i < 8192; ++i) {
            longValue += "a";
        }
        f._request = "GET / HTTP/1.1\r\nHost: " + longValue + "\r\n\r\n";
    }
    f._isSuccessTest = false;  // 413 Request Entity Too Large
    tests.push_back(f);
    f._exceptMap.clear();
}

void fieldTest() {
    TestVector tests;

    // makeFieldTest(tests);
    makeBasicFieldTests(tests);
    makeInvalidFieldTests(tests);
    makeHostHeaderTests(tests);
    makeSpecialCharacterTests(tests);
    makeDelimiterTests(tests);
    makeContentLengthTests(tests);
    makeTransferEncodingTests(tests);
    makeEncodingTests(tests);
    makeServerConfigTests(tests);
    makeCookieTests(tests);
    makeCGITests(tests);
    makeDateHeaderTests(tests);
    makeServerHeaderTests(tests);
    makeLargeRequestTests(tests);
    int pass = 0;
    for (std::size_t i = 0; i < tests.size(); ++i) {
        if (runTest(tests[i])) {
            ++pass;
        }
    }
    std::cout << pass << " / " << tests.size() << std::endl;
}

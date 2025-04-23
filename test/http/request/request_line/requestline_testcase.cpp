// Copyright 2025 Ideal Broccoli

#include <iostream>
#include <vector>
#include <string>

#include <cstdio>

#include "../../../toolbox/stepmark.hpp"
#include "../../../src/http/request/request_parser.hpp"
#include "requestline_test.hpp"

bool compareRequestLine(http::RequestParser& r, RequestLineTest& t) {
    // if (r.get().HttpStatus != t._httpStatus) {
    //     return false;
    // }
    if (r.get().method != t._exceptRequest.method ||
        r.get().uri.path != t._exceptRequest.path ||
        r.get().version != t._exceptRequest.version) {
            return false;
    }
    for (std::size_t i = 0; i < t._exceptRequest.queryVec.size(); ++i) {
        QueryMap::iterator it =
            r.get().uri.queryMap.find(t._exceptRequest.queryVec[i].first);
        if (it->second != t._exceptRequest.queryVec[i].second) {
            return false;
        }
    }
    return true;
}

void showRequestLine(http::RequestParser& r, RequestLineTest& t) {
    std::cout << "---- request -----" << std::endl;
    std::cout << t._request << std::endl;
    std::cout << "method" << std::endl;
    printf("webserv [%s]\nnginx [%s]\n",
        r.get().method.c_str(), t._exceptRequest.method.c_str());
    std::cout << "path" << std::endl;
    printf("webserv [%s]\nnginx[%s]\n",
        r.get().uri.path.c_str(), t._exceptRequest.path.c_str());
    std::cout << "version" << std::endl;
    printf("webserv [%s]\nnginx[%s]\n",
        r.get().version.c_str(), t._exceptRequest.version.c_str());
    std::cout << "query" << std::endl;
    for (std::size_t i = 0; i < t._exceptRequest.queryVec.size(); ++i) {
        QueryPair pair_t = t._exceptRequest.queryVec[i];
        QueryMap::iterator it_r = r.get().uri.queryMap.find(pair_t.first);
        printf("webserv %s: %s\nnginx %s: %s\n",
            pair_t.first.c_str(), pair_t.second.c_str(),
            it_r->first.c_str(), it_r->second.c_str());
    }
}

bool runTest(RequestLineTest& t) {
    toolbox::logger::StepMark::debug(t._name);
    http::RequestParser r;
    try {
        r.run(t._request);
        if (t._isSuccessTest && compareRequestLine(r, t)) {
            return true;
        } else {
            std::cout << "==== true -> false "
                << t._name << "====" << std::endl;
            showRequestLine(r, t);
            return false;
        }
    } catch (std::exception& e) {
        std::cout << e.what() << std::endl;
    }
    return false;
}

void makeMethodTests(TestVector& t) {
    RequestLineTest r;

    r._name = "基本的なGETリクエスト";
    r._request = "GET / HTTP/1.1\r\nHost: sample\r\n\r\n";
    r._httpStatus = 200;
    r._isSuccessTest = true;
    r._exceptRequest.method = "GET";
    r._exceptRequest.path = "/";
    r._exceptRequest.version = "HTTP/1.1";
    t.push_back(r);

    r._name = "基本的なHEADリクエスト";
    r._request = "HEAD /submit HTTP/1.1\r\nHost: sample\r\n\r\n";
    r._httpStatus = 200;
    r._isSuccessTest = true;
    r._exceptRequest.method = "HEAD";
    r._exceptRequest.path = "/submit";
    r._exceptRequest.version = "HTTP/1.1";
    t.push_back(r);

    r._name = "基本的なPOSTリクエスト";
    r._request = "POST /submit HTTP/1.1\r\nHost: sample\r\n\r\n";
    r._httpStatus = 200;
    r._isSuccessTest = true;
    r._exceptRequest.method = "POST";
    r._exceptRequest.path = "/submit";
    r._exceptRequest.version = "HTTP/1.1";
    t.push_back(r);

    r._name = "基本的なDELETEリクエスト";
    r._request = "DELETE /resource HTTP/1.1\r\nHost: sample\r\n\r\n";
    r._httpStatus = 200;
    r._isSuccessTest = true;
    r._exceptRequest.method = "DELETE";
    r._exceptRequest.path = "/resource";
    r._exceptRequest.version = "HTTP/1.1";
    t.push_back(r);

    r._name = "サポートされていないメソッド";
    r._request = "PATCH /resource HTTP/1.1\r\nHost: sample\r\n\r\n";
    r._httpStatus = 501;  // Not Implemented
    r._isSuccessTest = false;
    t.push_back(r);

    r._name = "大文字小文字混在メソッド";
    r._request = "gEt / HTTP/1.1\r\nHost: sample\r\n\r\n";
    r._httpStatus = 400;  // Bad Request
    r._isSuccessTest = false;
    t.push_back(r);

    r._name = "メソッド%エンコーディング";
    r._request = "%47%45%54 / HTTP/1.1\r\nHost: sample\r\n\r\n";
    r._httpStatus = 400;  // Bad Request
    r._isSuccessTest = false;
    t.push_back(r);

    r._name = "空のメソッド";
    r._request = " / HTTP/1.1\r\nHost: sample\r\n\r\n";
    r._httpStatus = 400;  // Bad Request
    r._isSuccessTest = false;
    t.push_back(r);
}

void makePathTests(TestVector& t) {
    RequestLineTest r;

    r._name = "基本的なルートパス";
    r._request = "GET / HTTP/1.1\r\nHost: sample\r\n\r\n";
    r._httpStatus = 200;
    r._isSuccessTest = true;
    r._exceptRequest.method = "GET";
    r._exceptRequest.path = "/";
    r._exceptRequest.version = "HTTP/1.1";
    t.push_back(r);

    r._name = "複雑なパス";
    r._request = "GET /path1/path2/path3/index.html HTTP/1.1\r\n"
                    "Host: sample\r\n\r\n";
    r._httpStatus = 200;
    r._isSuccessTest = true;
    r._exceptRequest.method = "GET";
    r._exceptRequest.path = "/path1/path2/path3/index.html";
    r._exceptRequest.version = "HTTP/1.1";
    t.push_back(r);

    r._name = "特殊文字を含むパス";
    r._request = "GET /file-name_with.special+chars HTTP/1.1\r\n"
                    "Host: sample\r\n\r\n";
    r._httpStatus = 200;
    r._isSuccessTest = true;
    r._exceptRequest.method = "GET";
    r._exceptRequest.path = "/file-name_with.special+chars";
    r._exceptRequest.version = "HTTP/1.1";
    t.push_back(r);

    r._name = "空のパス";
    r._request = "GET  HTTP/1.1\r\nHost: sample\r\n\r\n";
    r._httpStatus = 400;  // Bad Request
    r._isSuccessTest = false;
    t.push_back(r);

    std::string longPath = "/";
    for (int i = 0; i < 2000; ++i) {
        longPath += "a";
    }
    r._name = "非常に長いURI";  // 8kb8192made
    r._request = "GET " + longPath + " HTTP/1.1\r\nHost: sample\r\n\r\n";
    r._httpStatus = 414;  // URI Too Long
    r._isSuccessTest = false;
    t.push_back(r);
}

// ディレクトリトラバーサル攻撃に対するテストケース
void makeDirectoryTraversalTests(TestVector& t) {
    RequestLineTest r;

    r._name = "基本的なディレクトリトラバーサル";
    r._request = "GET /../etc/passwd HTTP/1.1\r\nHost: sample\r\n\r\n";
    r._httpStatus = 403;  // Forbidden
    r._isSuccessTest = false;
    t.push_back(r);

    r._name = "エンコードされたディレクトリトラバーサル";
    r._request = "GET /%2e%2e/etc/passwd HTTP/1.1\r\nHost: sample\r\n\r\n";
    r._httpStatus = 403;  // Forbidden
    r._isSuccessTest = false;
    t.push_back(r);

    r._name = "複数レベルのディレクトリトラバーサル";
    r._request = "GET /images/../../etc/passwd HTTP/1.1\r\n"
                    "Host: sample\r\n\r\n";
    r._httpStatus = 403;  // Forbidden
    r._isSuccessTest = false;
    t.push_back(r);

    r._name = "複雑なディレクトリトラバーサル";
    r._request = "GET /a/b/c/../../../etc/passwd HTTP/1.1\r\n"
                    "Host: sample\r\n\r\n";
    r._httpStatus = 403;  // Forbidden
    r._isSuccessTest = false;
    t.push_back(r);

    r._name = "バックスラッシュを使用したディレクトリトラバーサル";
    r._request = "GET /..\\..\\etc\\passwd HTTP/1.1\r\nHost: sample\r\n\r\n";
    r._httpStatus = 403;  // Forbidden
    r._isSuccessTest = false;
    t.push_back(r);

    r._name = "NULLバイトを使用したディレクトリトラバーサル";
    r._request = "GET /public/%00/../../../etc/passwd HTTP/1.1\r\n"
                    "Host: sample\r\n\r\n";
    r._httpStatus = 400;  // Bad Request
    r._isSuccessTest = false;
    t.push_back(r);

    r._name = "パスの途中にNULLバイトを含むディレクトリトラバーサル";
    r._request = "GET /../../etc/passwd%00.html HTTP/1.1\r\n"
                    "Host: sample\r\n\r\n";
    r._httpStatus = 400;  // Bad Request
    r._isSuccessTest = false;
    t.push_back(r);
}

void makePathNormalizationTests(TestVector& t) {
    RequestLineTest r;

    r._name = "カレントディレクトリ表記あり";
    r._request = "GET /./index.html HTTP/1.1\r\nHost: sample\r\n\r\n";
    r._httpStatus = 200;
    r._isSuccessTest = true;
    r._exceptRequest.method = "GET";
    r._exceptRequest.path = "/index.html";
    r._exceptRequest.version = "HTTP/1.1";
    t.push_back(r);

    r._name = "複数のカレントディレクトリ表記";
    r._request = "GET /./././index.html HTTP/1.1\r\nHost: sample\r\n\r\n";
    r._httpStatus = 200;
    r._isSuccessTest = true;
    r._exceptRequest.method = "GET";
    r._exceptRequest.path = "/index.html";
    r._exceptRequest.version = "HTTP/1.1";
    t.push_back(r);

    r._name = "親ディレクトリと現在のディレクトリの組み合わせ";
    r._request = "GET /a/b/../c/./d HTTP/1.1\r\nHost: sample\r\n\r\n";
    r._httpStatus = 200;
    r._isSuccessTest = true;
    r._exceptRequest.method = "GET";
    r._exceptRequest.path = "/a/c/d";
    r._exceptRequest.version = "HTTP/1.1";
    t.push_back(r);

    r._name = "複数のスラッシュ";
    r._request = "GET ////index.html HTTP/1.1\r\nHost: sample\r\n\r\n";
    r._httpStatus = 200;
    r._isSuccessTest = true;
    r._exceptRequest.method = "GET";
    r._exceptRequest.path = "/index.html";
    r._exceptRequest.version = "HTTP/1.1";
    t.push_back(r);

    r._name = "中間に複数のスラッシュ";
    r._request = "GET /path1////path2///path3 HTTP/1.1\r\nHost: sample\r\n\r\n";
    r._httpStatus = 200;
    r._isSuccessTest = true;
    r._exceptRequest.method = "GET";
    r._exceptRequest.path = "/path1/path2/path3";
    r._exceptRequest.version = "HTTP/1.1";
    t.push_back(r);

    r._name = "末尾のスラッシュ";
    r._request = "GET /path1/path2/ HTTP/1.1\r\nHost: sample\r\n\r\n";
    r._httpStatus = 200;
    r._isSuccessTest = true;
    r._exceptRequest.method = "GET";
    r._exceptRequest.path = "/path1/path2/";
    r._exceptRequest.version = "HTTP/1.1";
    t.push_back(r);

    r._name = "末尾の複数スラッシュ";
    r._request = "GET /path1/path2/// HTTP/1.1\r\nHost: sample\r\n\r\n";
    r._httpStatus = 200;
    r._isSuccessTest = true;
    r._exceptRequest.method = "GET";
    r._exceptRequest.path = "/path1/path2/";
    r._exceptRequest.version = "HTTP/1.1";
    t.push_back(r);

    r._name = "複雑なパス正規化";
    r._request = "GET /a/./b////../c/./d///.//e HTTP/1.1\r\n"
                    "Host: sample\r\n\r\n";
    r._httpStatus = 200;
    r._isSuccessTest = true;
    r._exceptRequest.method = "GET";
    r._exceptRequest.path = "/a/c/d/e";
    r._exceptRequest.version = "HTTP/1.1";
    t.push_back(r);

    r._name = "エンコードされたスラッシュと現在ディレクトリ";
    r._request = "GET /a%2F.%2Fb HTTP/1.1\r\nHost: sample\r\n\r\n";
    r._httpStatus = 200;
    r._isSuccessTest = true;
    r._exceptRequest.method = "GET";
    r._exceptRequest.path = "/a/.b";
    r._exceptRequest.version = "HTTP/1.1";
    t.push_back(r);
}

void makeMultipleDotPathTests(TestVector& t) {
    RequestLineTest r;

    r._name = "3つのドットを含むパス";
    r._request = "GET /... HTTP/1.1\r\nHost: sample\r\n\r\n";
    r._httpStatus = 200;
    r._isSuccessTest = true;
    r._exceptRequest.method = "GET";
    r._exceptRequest.path = "/...";
    r._exceptRequest.version = "HTTP/1.1";
    t.push_back(r);

    r._name = "ファイル名に3つのドットを含むパス";
    r._request = "GET /file... HTTP/1.1\r\nHost: sample\r\n\r\n";
    r._httpStatus = 200;
    r._isSuccessTest = true;
    r._exceptRequest.method = "GET";
    r._exceptRequest.path = "/file...";
    r._exceptRequest.version = "HTTP/1.1";
    t.push_back(r);

    r._name = "ディレクトリ名に3つのドットを含むパス";
    r._request = "GET /dir.../file.html HTTP/1.1\r\nHost: sample\r\n\r\n";
    r._httpStatus = 200;
    r._isSuccessTest = true;
    r._exceptRequest.method = "GET";
    r._exceptRequest.path = "/dir.../file.html";
    r._exceptRequest.version = "HTTP/1.1";
    t.push_back(r);

    r._name = "4つのドットを含むパス";
    r._request = "GET /.... HTTP/1.1\r\nHost: sample\r\n\r\n";
    r._httpStatus = 200;
    r._isSuccessTest = true;
    r._exceptRequest.method = "GET";
    r._exceptRequest.path = "/....";
    r._exceptRequest.version = "HTTP/1.1";
    t.push_back(r);

    r._name = "ドット3つと親ディレクトリ参照の混合";
    r._request = "GET /abc/.../.. HTTP/1.1\r\nHost: sample\r\n\r\n";
    r._httpStatus = 200;
    r._isSuccessTest = true;
    r._exceptRequest.method = "GET";
    r._exceptRequest.path = "/abc";
    r._exceptRequest.version = "HTTP/1.1";
    t.push_back(r);

    r._name = "拡張子に3つのドットを含むパス";
    r._request = "GET /file.... HTTP/1.1\r\nHost: sample\r\n\r\n";
    r._httpStatus = 200;
    r._isSuccessTest = true;
    r._exceptRequest.method = "GET";
    r._exceptRequest.path = "/file....";
    r._exceptRequest.version = "HTTP/1.1";
    t.push_back(r);

    r._name = "複数のドットを含む複雑なパス";
    r._request = "GET /a.../b.../c.../index.html HTTP/1.1\r\n"
                    "Host: sample\r\n\r\n";
    r._httpStatus = 200;
    r._isSuccessTest = true;
    r._exceptRequest.method = "GET";
    r._exceptRequest.path = "/a.../b.../c.../index.html";
    r._exceptRequest.version = "HTTP/1.1";
    t.push_back(r);

    r._name = "URLエンコードされた複数ドット";
    r._request = "GET /%2E%2E%2E HTTP/1.1\r\nHost: sample\r\n\r\n";
    r._httpStatus = 200;
    r._isSuccessTest = true;
    r._exceptRequest.method = "GET";
    r._exceptRequest.path = "/...";
    r._exceptRequest.version = "HTTP/1.1";
    t.push_back(r);
}

void makeQueryParameterTests(TestVector& t) {
    RequestLineTest r;

    r._name = "単一クエリパラメータ";
    r._request = "GET /search?q=test HTTP/1.1\r\nHost: sample\r\n\r\n";
    r._httpStatus = 200;
    r._isSuccessTest = true;
    r._exceptRequest.method = "GET";
    r._exceptRequest.path = "/search";
    r._exceptRequest.version = "HTTP/1.1";
    r._exceptRequest.queryVec.push_back(QueryPair("q", "test"));
    t.push_back(r);
    r._exceptRequest.queryVec.clear();

    r._name = "複数クエリパラメータ";
    r._request = "GET /search?q=test&page=1&limit=10 HTTP/1.1\r\n"
                    "Host: sample\r\n\r\n";
    r._httpStatus = 200;
    r._isSuccessTest = true;
    r._exceptRequest.method = "GET";
    r._exceptRequest.path = "/search";
    r._exceptRequest.version = "HTTP/1.1";
    r._exceptRequest.queryVec.push_back(QueryPair("q", "test"));
    r._exceptRequest.queryVec.push_back(QueryPair("page", "1"));
    r._exceptRequest.queryVec.push_back(QueryPair("limit", "10"));
    t.push_back(r);
    r._exceptRequest.queryVec.clear();

    r._name = "URLエンコード";
    r._request = "GET /search?q=hello%20world HTTP/1.1\r\nHost: sample\r\n\r\n";
    r._httpStatus = 200;
    r._isSuccessTest = true;
    r._exceptRequest.method = "GET";
    r._exceptRequest.path = "/search";
    r._exceptRequest.version = "HTTP/1.1";
    r._exceptRequest.queryVec.push_back(QueryPair("q", "hello world"));
    t.push_back(r);
    r._exceptRequest.queryVec.clear();

    r._name = "空の値を持つクエリパラメータ";
    r._request = "GET /search?q= HTTP/1.1\r\nHost: sample\r\n\r\n";
    r._httpStatus = 200;
    r._isSuccessTest = true;
    r._exceptRequest.method = "GET";
    r._exceptRequest.path = "/search";
    r._exceptRequest.version = "HTTP/1.1";
    r._exceptRequest.queryVec.push_back(QueryPair("q", ""));
    t.push_back(r);
    r._exceptRequest.queryVec.clear();

    r._name = "フラグメント";
    r._request = "GET /page#section1 HTTP/1.1\r\nHost: sample\r\n\r\n";
    r._httpStatus = 200;
    r._isSuccessTest = true;
    r._exceptRequest.method = "GET";
    r._exceptRequest.path = "/page";
    r._exceptRequest.version = "HTTP/1.1";
    t.push_back(r);

    r._name = "クエリパラメータとフラグメント";
    r._request = "GET /page?id=123#section1 HTTP/1.1\r\nHost: sample\r\n\r\n";
    r._httpStatus = 200;
    r._isSuccessTest = true;
    r._exceptRequest.method = "GET";
    r._exceptRequest.path = "/page";
    r._exceptRequest.version = "HTTP/1.1";
    r._exceptRequest.queryVec.push_back(QueryPair("id", "123"));
    t.push_back(r);
    r._exceptRequest.queryVec.clear();
}

void makeHttpVersionTests(TestVector& t) {
    RequestLineTest r;

    r._name = "HTTP/1.1";
    r._request = "GET / HTTP/1.1\r\nHost: sample\r\n\r\n";
    r._httpStatus = 200;
    r._isSuccessTest = true;
    r._exceptRequest.method = "GET";
    r._exceptRequest.path = "/";
    r._exceptRequest.version = "HTTP/1.1";
    t.push_back(r);

    r._name = "HTTP/1.0";
    r._request = "GET / HTTP/1.0\r\nHost: sample\r\n\r\n";
    r._httpStatus = 200;
    r._isSuccessTest = true;
    r._exceptRequest.method = "GET";
    r._exceptRequest.path = "/";
    r._exceptRequest.version = "HTTP/1.0";
    t.push_back(r);

    r._name = "不正なHTTPバージョン";
    r._request = "GET / HTTP/2.0\r\nHost: sample\r\n\r\n";
    r._httpStatus = 505;  // HTTP Version Not Supported
    r._isSuccessTest = false;
    t.push_back(r);

    r._name = "小文字HTTPバージョン";
    r._request = "GET / http/1.1\r\nHost: sample\r\n\r\n";
    r._httpStatus = 400;  // Bad Request
    r._isSuccessTest = false;
    t.push_back(r);

    r._name = "空のHTTPバージョン";
    r._request = "GET / \r\nHost: sample\r\n\r\n";
    r._httpStatus = 400;  // Bad Request
    r._isSuccessTest = false;
    t.push_back(r);
}

void makeRequestStructureTests(TestVector& t) {
    RequestLineTest r;

    r._name = "余分なスペース";
    r._request = "GET  /  HTTP/1.1\r\nHost: sample\r\n\r\n";
    r._httpStatus = 200;
    r._isSuccessTest = true;
    r._exceptRequest.method = "GET";
    r._exceptRequest.path = "/";
    r._exceptRequest.version = "HTTP/1.1";
    t.push_back(r);

    r._name = "LFのみの改行";
    r._request = "GET / HTTP/1.1\nHost: sample\n\n";
    r._httpStatus = 200;
    r._isSuccessTest = true;
    r._exceptRequest.method = "GET";
    r._exceptRequest.path = "/";
    r._exceptRequest.version = "HTTP/1.1";
    t.push_back(r);

    r._name = "不完全なリクエストライン";
    r._request = "GET /\r\nHost: sample\r\n\r\n";
    r._httpStatus = 400;  // Bad Request
    r._isSuccessTest = false;
    t.push_back(r);

    r._name = "タブ区切り";
    r._request = "GET\t/\tHTTP/1.1\r\nHost: sample\r\n\r\n";
    r._httpStatus = 400;  // Bad Request
    r._isSuccessTest = false;
    t.push_back(r);

    r._name = "余分な行続き";
    r._request = "GET / HTTP/1.1 extra\r\nHost: sample\r\n\r\n";
    r._httpStatus = 400;  // Bad Request
    r._isSuccessTest = false;
    t.push_back(r);
}

void requestLineTest() {
    TestVector tests;
    makeMethodTests(tests);
    makePathTests(tests);
    makeDirectoryTraversalTests(tests);
    makePathNormalizationTests(tests);
    makeMultipleDotPathTests(tests);
    makeQueryParameterTests(tests);
    makeHttpVersionTests(tests);
    makeRequestStructureTests(tests);

    int pass = 0;
    for (std::size_t i = 0; i < tests.size(); ++i) {
        if (runTest(tests[i])) {
            ++pass;
        }
    }
    std::cout << pass << " / " << tests.size() << std::endl;
}

int main(void) {
    toolbox::logger::StepMark::setLogFile("field_test.log");
    toolbox::logger::StepMark::setLevel(toolbox::logger::DEBUG);
    requestLineTest();
    return 0;
}

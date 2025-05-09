// Copyright 2025 Ideal Broccoli

#include <iostream>
#include <vector>
#include <string>

#include <cstdio>

#include "../../../toolbox/stepmark.hpp"
#include "requestline_test.hpp"
#include "../../../src/http/request/request_parser.hpp"

typedef std::vector<RequestLineTest> TestVector;

bool compareRequestLine(http::RequestParser& r, RequestLineTest& t) {
    if (r.get().httpStatus.get() != t._httpStatus.get()) {
        return false;
    }
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
    std::cout << "webserv status: " << r.get().httpStatus.get() << std::endl;
    std::cout << "nginx status  : " << t._httpStatus.get() << std::endl;
    std::cout << "***** method *****" << std::endl;
    printf("webserv [%s]\nnginx   [%s]\n",
        r.get().method.c_str(), t._exceptRequest.method.c_str());
    std::cout << "**** path *****" << std::endl;
    printf("webserv [%s]\nnginx   [%s]\n",
        r.get().uri.path.c_str(), t._exceptRequest.path.c_str());
    std::cout << "***** version *****" << std::endl;
    printf("webserv [%s]\nnginx   [%s]\n",
        r.get().version.c_str(), t._exceptRequest.version.c_str());
    std::cout << "***** query *****" << std::endl;
    for (std::size_t i = 0; i < t._exceptRequest.queryVec.size(); ++i) {
        QueryPair pair_t = t._exceptRequest.queryVec[i];
        QueryMap::iterator it_r = r.get().uri.queryMap.find(pair_t.first);
        printf("webserv %s: %s\nnginx   %s: %s\n",
            it_r->first.c_str(), it_r->second.c_str(),
            pair_t.first.c_str(), pair_t.second.c_str());
    }
}

bool runTest(RequestLineTest& t) {
    toolbox::logger::StepMark::debug(t._name);
    http::RequestParser r;
    try {
        r.run(t._request);
    } catch (std::exception& e) {
    }
    if (t._isSuccessTest && compareRequestLine(r, t)) {
        return true;
    } else if (t._isSuccessTest && !compareRequestLine(r, t)) {
        std::cout << "==== Failed: true->false " << t._name
                  << " ====" << std::endl;
        std::cout << t._request;
        showRequestLine(r, t);
        return false;
    } else if (!t._isSuccessTest && compareRequestLine(r, t)) {
        return true;
    } else {
        std::cout << "==== Failed: false->true " << t._name
                  << " ====" << std::endl;
        std::cout << t._request;
        showRequestLine(r, t);
        return false;
    }
    return false;
}

void clearUri(RequestLineTest& r) {
    r._exceptRequest.method.clear();
    r._exceptRequest.path.clear();
    r._exceptRequest.version.clear();
    r._exceptRequest.queryVec.clear();
}

void makeMethodTests(TestVector& t) {
    RequestLineTest r;

    r._name = "基本的なGETリクエスト";
    r._request = "GET / HTTP/1.1\r\nHost: sample\r\n\r\n";
    r._httpStatus.set(http::HttpStatus::OK);
    r._isSuccessTest = true;
    r._exceptRequest.method = "GET";
    r._exceptRequest.path = "/";
    r._exceptRequest.version = "HTTP/1.1";
    t.push_back(r);
    clearUri(r);

    r._name = "基本的なHEADリクエスト";
    r._request = "HEAD /submit HTTP/1.1\r\nHost: sample\r\n\r\n";
    r._httpStatus.set(http::HttpStatus::OK);
    r._isSuccessTest = true;
    r._exceptRequest.method = "HEAD";
    r._exceptRequest.path = "/submit";
    r._exceptRequest.version = "HTTP/1.1";
    t.push_back(r);
    clearUri(r);

    r._name = "基本的なPOSTリクエスト";
    r._request = "POST /submit HTTP/1.1\r\nHost: sample\r\n\r\n";
    r._httpStatus.set(http::HttpStatus::OK);
    r._isSuccessTest = true;
    r._exceptRequest.method = "POST";
    r._exceptRequest.path = "/submit";
    r._exceptRequest.version = "HTTP/1.1";
    t.push_back(r);
    clearUri(r);

    r._name = "基本的なDELETEリクエスト";
    r._request = "DELETE /resource HTTP/1.1\r\nHost: sample\r\n\r\n";
    r._httpStatus.set(http::HttpStatus::OK);
    r._isSuccessTest = true;
    r._exceptRequest.method = "DELETE";
    r._exceptRequest.path = "/resource";
    r._exceptRequest.version = "HTTP/1.1";
    t.push_back(r);
    clearUri(r);

    r._name = "大文字小文字混在メソッド";
    r._request = "gEt / HTTP/1.1\r\nHost: sample\r\n\r\n";
    r._httpStatus.set(http::HttpStatus::BAD_REQUEST);  // Bad Request
    r._isSuccessTest = false;
    r._exceptRequest.method = "gEt";
    r._exceptRequest.path = "/";
    r._exceptRequest.version = "HTTP/1.1";
    t.push_back(r);
    clearUri(r);

    r._name = "空のメソッド";
    r._request = " / HTTP/1.1\r\nHost: sample\r\n\r\n";
    r._httpStatus.set(http::HttpStatus::BAD_REQUEST);  // Bad Request
    r._isSuccessTest = false;
    r._exceptRequest.method = "";
    r._exceptRequest.path = "/";
    r._exceptRequest.version = "HTTP/1.1";
    t.push_back(r);
    clearUri(r);
}

void makePathTests(TestVector& t) {
    RequestLineTest r;

    r._name = "基本的なパス";
    r._request = "GET / HTTP/1.1\r\nHost: sample\r\n\r\n";
    r._httpStatus.set(http::HttpStatus::OK);
    r._isSuccessTest = true;
    r._exceptRequest.method = "GET";
    r._exceptRequest.path = "/";
    r._exceptRequest.version = "HTTP/1.1";
    t.push_back(r);
    clearUri(r);

    r._name = "複雑なパス";
    r._request = "GET /path1/path2/path3/index.html HTTP/1.1\r\n"
                    "Host: sample\r\n\r\n";
    r._httpStatus.set(http::HttpStatus::OK);
    r._isSuccessTest = true;
    r._exceptRequest.method = "GET";
    r._exceptRequest.path = "/path1/path2/path3/index.html";
    r._exceptRequest.version = "HTTP/1.1";
    t.push_back(r);
    clearUri(r);

    r._name = "特殊文字を含むパス";
    r._request = "GET /file-name_with.special+chars HTTP/1.1\r\n"
                    "Host: sample\r\n\r\n";
    r._httpStatus.set(http::HttpStatus::OK);
    r._isSuccessTest = true;
    r._exceptRequest.method = "GET";
    r._exceptRequest.path = "/file-name_with.special+chars";
    r._exceptRequest.version = "HTTP/1.1";
    t.push_back(r);
    clearUri(r);

    r._name = "空のパス";
    r._request = "GET  HTTP/1.1\r\nHost: sample\r\n\r\n";
    r._httpStatus.set(http::HttpStatus::BAD_REQUEST);  // Bad Request
    r._isSuccessTest = false;
    r._exceptRequest.method = "GET";
    r._exceptRequest.path = "HTTP/1.1";
    r._exceptRequest.version = "";
    t.push_back(r);
    clearUri(r);

    std::string longPath = "/";
    for (int i = 0; i < 8192; ++i) {
        longPath += "a";
    }
    r._name = "非常に長いURI";  // 8kb8192
    r._request = "GET " + longPath + " HTTP/1.1\r\nHost: sample\r\n\r\n";
    r._httpStatus.set(http::HttpStatus::URI_TOO_LONG);  // URI Too Long
    r._isSuccessTest = false;
    r._exceptRequest.method = "GET";
    r._exceptRequest.path = longPath;
    r._exceptRequest.version = "HTTP/1.1";
    t.push_back(r);
    clearUri(r);
}

void makeDirectoryTraversalTests(TestVector& t) {
    RequestLineTest r;

    r._name = "基本的なディレクトリトラバーサル";
    r._request = "GET /../etc/passwd HTTP/1.1\r\nHost: sample\r\n\r\n";
    r._httpStatus.set(http::HttpStatus::BAD_REQUEST);  // Bad Request
    r._isSuccessTest = false;
    r._exceptRequest.method = "GET";
    r._exceptRequest.path = "/../etc/passwd";
    r._exceptRequest.version = "HTTP/1.1";
    t.push_back(r);
    clearUri(r);

    r._name = "複数レベルのディレクトリトラバーサル";
    r._request = "GET /images/../../etc/passwd HTTP/1.1\r\n"
                    "Host: sample\r\n\r\n";
    r._httpStatus.set(http::HttpStatus::BAD_REQUEST);  // Bad Request
    r._isSuccessTest = false;
    r._exceptRequest.method = "GET";
    r._exceptRequest.path = "/images/../../etc/passwd";
    r._exceptRequest.version = "HTTP/1.1";
    t.push_back(r);
    clearUri(r);

    r._name = "複雑なディレクトリトラバーサル";
    r._request = "GET /a/b/c/../../../etc/passwd HTTP/1.1\r\n"
                    "Host: sample\r\n\r\n";
    r._httpStatus.set(http::HttpStatus::OK);  // 200
    r._isSuccessTest = true;
    r._exceptRequest.method = "GET";
    r._exceptRequest.path = "/etc/passwd";
    r._exceptRequest.version = "HTTP/1.1";
    t.push_back(r);
    clearUri(r);
}

void makePathNormalizationTests(TestVector& t) {
    RequestLineTest r;

    r._name = "カレントディレクトリ表記あり";
    r._request = "GET /./index.html HTTP/1.1\r\nHost: sample\r\n\r\n";
    r._httpStatus.set(http::HttpStatus::OK);
    r._isSuccessTest = true;
    r._exceptRequest.method = "GET";
    r._exceptRequest.path = "/index.html";
    r._exceptRequest.version = "HTTP/1.1";
    t.push_back(r);
    clearUri(r);

    r._name = "複数のカレントディレクトリ表記";
    r._request = "GET /./././index.html HTTP/1.1\r\nHost: sample\r\n\r\n";
    r._httpStatus.set(http::HttpStatus::OK);
    r._isSuccessTest = true;
    r._exceptRequest.method = "GET";
    r._exceptRequest.path = "/index.html";
    r._exceptRequest.version = "HTTP/1.1";
    t.push_back(r);
    clearUri(r);

    r._name = "親ディレクトリと現在のディレクトリの組み合わせ";
    r._request = "GET /a/b/../c/./d HTTP/1.1\r\nHost: sample\r\n\r\n";
    r._httpStatus.set(http::HttpStatus::OK);
    r._isSuccessTest = true;
    r._exceptRequest.method = "GET";
    r._exceptRequest.path = "/a/c/d";
    r._exceptRequest.version = "HTTP/1.1";
    t.push_back(r);
    clearUri(r);

    r._name = "複数のスラッシュ";
    r._request = "GET ////index.html HTTP/1.1\r\nHost: sample\r\n\r\n";
    r._httpStatus.set(http::HttpStatus::OK);
    r._isSuccessTest = true;
    r._exceptRequest.method = "GET";
    r._exceptRequest.path = "/index.html";
    r._exceptRequest.version = "HTTP/1.1";
    t.push_back(r);
    clearUri(r);

    r._name = "中間に複数のスラッシュ";
    r._request = "GET /path1////path2///path3 HTTP/1.1\r\nHost: sample\r\n\r\n";
    r._httpStatus.set(http::HttpStatus::OK);
    r._isSuccessTest = true;
    r._exceptRequest.method = "GET";
    r._exceptRequest.path = "/path1/path2/path3";
    r._exceptRequest.version = "HTTP/1.1";
    t.push_back(r);
    clearUri(r);

    r._name = "末尾のスラッシュ";
    r._request = "GET /path1/path2/ HTTP/1.1\r\nHost: sample\r\n\r\n";
    r._httpStatus.set(http::HttpStatus::OK);
    r._isSuccessTest = true;
    r._exceptRequest.method = "GET";
    r._exceptRequest.path = "/path1/path2/";
    r._exceptRequest.version = "HTTP/1.1";
    t.push_back(r);
    clearUri(r);

    r._name = "末尾の複数スラッシュ";
    r._request = "GET /path1/path2/// HTTP/1.1\r\nHost: sample\r\n\r\n";
    r._httpStatus.set(http::HttpStatus::OK);
    r._isSuccessTest = true;
    r._exceptRequest.method = "GET";
    r._exceptRequest.path = "/path1/path2/";
    r._exceptRequest.version = "HTTP/1.1";
    t.push_back(r);
    clearUri(r);

    r._name = "複雑なパス正規化";
    r._request = "GET /a/./b////../c/./d///.//e HTTP/1.1\r\n"
                    "Host: sample\r\n\r\n";
    r._httpStatus.set(http::HttpStatus::OK);
    r._isSuccessTest = true;
    r._exceptRequest.method = "GET";
    r._exceptRequest.path = "/a/c/d/e";
    r._exceptRequest.version = "HTTP/1.1";
    t.push_back(r);
    clearUri(r);
}

void makeMultipleDotPathTests(TestVector& t) {
    RequestLineTest r;

    r._name = "3つのドットを含むパス";
    r._request = "GET /... HTTP/1.1\r\nHost: sample\r\n\r\n";
    r._httpStatus.set(http::HttpStatus::OK);
    r._isSuccessTest = true;
    r._exceptRequest.method = "GET";
    r._exceptRequest.path = "/...";
    r._exceptRequest.version = "HTTP/1.1";
    t.push_back(r);
    clearUri(r);

    r._name = "ファイル名に3つのドットを含むパス";
    r._request = "GET /file... HTTP/1.1\r\nHost: sample\r\n\r\n";
    r._httpStatus.set(http::HttpStatus::OK);
    r._isSuccessTest = true;
    r._exceptRequest.method = "GET";
    r._exceptRequest.path = "/file...";
    r._exceptRequest.version = "HTTP/1.1";
    t.push_back(r);
    clearUri(r);

    r._name = "ディレクトリ名に3つのドットを含むパス";
    r._request = "GET /dir.../file.html HTTP/1.1\r\nHost: sample\r\n\r\n";
    r._httpStatus.set(http::HttpStatus::OK);
    r._isSuccessTest = true;
    r._exceptRequest.method = "GET";
    r._exceptRequest.path = "/dir.../file.html";
    r._exceptRequest.version = "HTTP/1.1";
    t.push_back(r);
    clearUri(r);

    r._name = "4つのドットを含むパス";
    r._request = "GET /.... HTTP/1.1\r\nHost: sample\r\n\r\n";
    r._httpStatus.set(http::HttpStatus::OK);
    r._isSuccessTest = true;
    r._exceptRequest.method = "GET";
    r._exceptRequest.path = "/....";
    r._exceptRequest.version = "HTTP/1.1";
    t.push_back(r);
    clearUri(r);

    r._name = "ドット3つと親ディレクトリ参照の混合";
    r._request = "GET /abc/.../.. HTTP/1.1\r\nHost: sample\r\n\r\n";
    r._httpStatus.set(http::HttpStatus::OK);
    r._isSuccessTest = true;
    r._exceptRequest.method = "GET";
    r._exceptRequest.path = "/abc";
    r._exceptRequest.version = "HTTP/1.1";
    t.push_back(r);
    clearUri(r);

    r._name = "拡張子に3つのドットを含むパス";
    r._request = "GET /file.... HTTP/1.1\r\nHost: sample\r\n\r\n";
    r._httpStatus.set(http::HttpStatus::OK);
    r._isSuccessTest = true;
    r._exceptRequest.method = "GET";
    r._exceptRequest.path = "/file....";
    r._exceptRequest.version = "HTTP/1.1";
    t.push_back(r);
    clearUri(r);

    r._name = "複数のドットを含む複雑なパス";
    r._request = "GET /a.../b.../c.../index.html HTTP/1.1\r\n"
                    "Host: sample\r\n\r\n";
    r._httpStatus.set(http::HttpStatus::OK);
    r._isSuccessTest = true;
    r._exceptRequest.method = "GET";
    r._exceptRequest.path = "/a.../b.../c.../index.html";
    r._exceptRequest.version = "HTTP/1.1";
    t.push_back(r);
    clearUri(r);
}

void makeQueryParameterTests(TestVector& t) {
    RequestLineTest r;

    r._name = "単一クエリパラメータ";
    r._request = "GET /search?q=test HTTP/1.1\r\nHost: sample\r\n\r\n";
    r._httpStatus.set(http::HttpStatus::OK);
    r._isSuccessTest = true;
    r._exceptRequest.method = "GET";
    r._exceptRequest.path = "/search";
    r._exceptRequest.version = "HTTP/1.1";
    r._exceptRequest.queryVec.push_back(QueryPair("q", "test"));
    t.push_back(r);
    clearUri(r);
    r._exceptRequest.queryVec.clear();

    r._name = "クエリパラメータ?連続";
    r._request = "GET /search???q=test HTTP/1.1\r\nHost: sample\r\n\r\n";
    r._httpStatus.set(http::HttpStatus::OK);
    r._isSuccessTest = true;
    r._exceptRequest.method = "GET";
    r._exceptRequest.path = "/search";
    r._exceptRequest.version = "HTTP/1.1";
    r._exceptRequest.queryVec.push_back(QueryPair("??q", "test"));
    t.push_back(r);
    clearUri(r);
    r._exceptRequest.queryVec.clear();

    r._name = "複数クエリパラメータ";
    r._request = "GET /search?q=test&page=1&limit=10 HTTP/1.1\r\n"
                    "Host: sample\r\n\r\n";
    r._httpStatus.set(http::HttpStatus::OK);
    r._isSuccessTest = true;
    r._exceptRequest.method = "GET";
    r._exceptRequest.path = "/search";
    r._exceptRequest.version = "HTTP/1.1";
    r._exceptRequest.queryVec.push_back(QueryPair("q", "test"));
    r._exceptRequest.queryVec.push_back(QueryPair("page", "1"));
    r._exceptRequest.queryVec.push_back(QueryPair("limit", "10"));
    t.push_back(r);
    clearUri(r);
    r._exceptRequest.queryVec.clear();

    r._name = "空の値を持つクエリパラメータ";
    r._request = "GET /search?q= HTTP/1.1\r\nHost: sample\r\n\r\n";
    r._httpStatus.set(http::HttpStatus::OK);
    r._isSuccessTest = true;
    r._exceptRequest.method = "GET";
    r._exceptRequest.path = "/search";
    r._exceptRequest.version = "HTTP/1.1";
    r._exceptRequest.queryVec.push_back(QueryPair("q", ""));
    t.push_back(r);
    clearUri(r);
    r._exceptRequest.queryVec.clear();

    r._name = "フラグメント";
    r._request = "GET /page#section1 HTTP/1.1\r\nHost: sample\r\n\r\n";
    r._httpStatus.set(http::HttpStatus::OK);
    r._isSuccessTest = true;
    r._exceptRequest.method = "GET";
    r._exceptRequest.path = "/page";
    r._exceptRequest.version = "HTTP/1.1";
    t.push_back(r);
    clearUri(r);

    r._name = "クエリパラメータとフラグメント";
    r._request = "GET /page?id=123#section1 HTTP/1.1\r\nHost: sample\r\n\r\n";
    r._httpStatus.set(http::HttpStatus::OK);
    r._isSuccessTest = true;
    r._exceptRequest.method = "GET";
    r._exceptRequest.path = "/page";
    r._exceptRequest.version = "HTTP/1.1";
    r._exceptRequest.queryVec.push_back(QueryPair("id", "123"));
    t.push_back(r);
    clearUri(r);
    r._exceptRequest.queryVec.clear();

    r._name = "フラグメントの後にクエリが存在する場合";
    r._request = "GET /page#section1?id=123 HTTP/1.1\r\nHost: sample\r\n\r\n";
    r._httpStatus.set(http::HttpStatus::OK);
    r._isSuccessTest = true;
    r._exceptRequest.method = "GET";
    r._exceptRequest.path = "/page";
    r._exceptRequest.version = "HTTP/1.1";
    t.push_back(r);
    clearUri(r);
    r._exceptRequest.queryVec.clear();
}

void makeHttpVersionTests(TestVector& t) {
    RequestLineTest r;

    r._name = "HTTP/1.1";
    r._request = "GET / HTTP/1.1\r\nHost: sample\r\n\r\n";
    r._httpStatus.set(http::HttpStatus::OK);
    r._isSuccessTest = true;
    r._exceptRequest.method = "GET";
    r._exceptRequest.path = "/";
    r._exceptRequest.version = "HTTP/1.1";
    t.push_back(r);
    clearUri(r);

    r._name = "HTTP/1.0";  // 対応するかどうか決める
    r._request = "GET / HTTP/1.0\r\nHost: sample\r\n\r\n";
    r._httpStatus.set(http::HttpStatus::OK);
    r._isSuccessTest = true;
    r._exceptRequest.method = "GET";
    r._exceptRequest.path = "/";
    r._exceptRequest.version = "HTTP/1.0";
    t.push_back(r);
    clearUri(r);

    r._name = "不正なHTTPバージョン";
    r._request = "GET / HTTP/2.0\r\nHost: sample\r\n\r\n";
    r._httpStatus.set(http::HttpStatus::HTTP_VERSION_NOT_SUPPORTED);
    r._isSuccessTest = false;
    r._exceptRequest.method = "GET";
    r._exceptRequest.path = "/";
    r._exceptRequest.version = "HTTP/2.0";
    t.push_back(r);
    clearUri(r);

    r._name = "小文字HTTPバージョン";
    r._request = "GET / http/1.1\r\nHost: sample\r\n\r\n";
    r._httpStatus.set(http::HttpStatus::BAD_REQUEST);  // Bad Request
    r._isSuccessTest = false;
    r._exceptRequest.method = "GET";
    r._exceptRequest.path = "/";
    r._exceptRequest.version = "http/1.1";
    t.push_back(r);
    clearUri(r);

    r._name = "空のHTTPバージョン";
    r._request = "GET / \r\nHost: sample\r\n\r\n";
    r._httpStatus.set(http::HttpStatus::BAD_REQUEST);  // OK
    r._isSuccessTest = false;
    r._exceptRequest.method = "GET";
    r._exceptRequest.path = "/";
    r._exceptRequest.version = "";
    t.push_back(r);
    clearUri(r);
}

void makeRequestStructureTests(TestVector& t) {
    RequestLineTest r;

    r._name = "余分なスペース";
    r._request = "GET  /  HTTP/1.1\r\nHost: sample\r\n\r\n";
    r._httpStatus.set(http::HttpStatus::OK);
    r._isSuccessTest = true;
    r._exceptRequest.method = "GET";
    r._exceptRequest.path = "/";
    r._exceptRequest.version = "HTTP/1.1";
    t.push_back(r);
    clearUri(r);

    r._name = "不完全なリクエストライン";
    r._request = "GET /\r\nHost: sample\r\n\r\n";
    r._httpStatus.set(http::HttpStatus::BAD_REQUEST);  // Bad Request
    r._isSuccessTest = false;
    r._exceptRequest.method = "GET";
    r._exceptRequest.path = "/";
    r._exceptRequest.version = "";
    t.push_back(r);
    clearUri(r);

    r._name = "タブ区切り";
    r._request = "GET\t/\tHTTP/1.1\r\nHost: sample\r\n\r\n";
    r._httpStatus.set(http::HttpStatus::BAD_REQUEST);  // Bad Request
    r._isSuccessTest = false;
    r._exceptRequest.method = "";
    r._exceptRequest.path = "";
    r._exceptRequest.version = "";
    t.push_back(r);
    clearUri(r);

    r._name = "余分な行続き";
    r._request = "GET / HTTP/1.1 extra\r\nHost: sample\r\n\r\n";
    r._httpStatus.set(http::HttpStatus::BAD_REQUEST);  // Bad Request
    r._isSuccessTest = false;
    r._exceptRequest.method = "GET";
    r._exceptRequest.path = "/";
    r._exceptRequest.version = "HTTP/1.1";
    t.push_back(r);
    clearUri(r);
}

void makePercentEncodingTests(TestVector& t) {
    RequestLineTest r;

    r._name = "基本的なパーセントエンコーディング";
    r._request = "GET /hello%20world HTTP/1.1\r\nHost: sample\r\n\r\n";
    r._httpStatus.set(http::HttpStatus::OK);
    r._isSuccessTest = true;
    r._exceptRequest.method = "GET";
    r._exceptRequest.path = "/hello world";
    r._exceptRequest.version = "HTTP/1.1";
    t.push_back(r);
    clearUri(r);

    r._name = "?パーセントエンコーディング";
    r._request = "GET /hello%3fworld HTTP/1.1\r\nHost: sample\r\n\r\n";
    r._httpStatus.set(http::HttpStatus::OK);
    r._isSuccessTest = true;
    r._exceptRequest.method = "GET";
    r._exceptRequest.path = "/hello?world";
    r._exceptRequest.version = "HTTP/1.1";
    t.push_back(r);
    clearUri(r);

    r._name = "エンコードされたGETメソッド";
    r._request = "%47%45%54 /index.html HTTP/1.1\r\nHost: sample\r\n\r\n";
    r._httpStatus.set(http::HttpStatus::BAD_REQUEST);
    r._exceptRequest.method = "%47%45%54";
    r._exceptRequest.path = "/index.html";
    r._exceptRequest.version = "HTTP/1.1";
    r._isSuccessTest = false;
    t.push_back(r);
    clearUri(r);

    r._name = "部分的にエンコードされたPOSTメソッド";
    r._request = "P%4FST /form.html HTTP/1.1\r\nHost: sample\r\n\r\n";
    r._httpStatus.set(http::HttpStatus::BAD_REQUEST);  // Bad Request
    r._isSuccessTest = false;
    r._exceptRequest.method = "P%4FST";
    r._exceptRequest.path = "/form.html";
    r._exceptRequest.version = "HTTP/1.1";
    t.push_back(r);
    clearUri(r);

    r._name = "空白文字をエンコードしたメソッド";
    r._request = "GET%20 /index.html HTTP/1.1\r\nHost: sample\r\n\r\n";
    r._httpStatus.set(http::HttpStatus::BAD_REQUEST);  // Bad Request
    r._isSuccessTest = false;
    r._exceptRequest.method = "GET%20";
    r._exceptRequest.path = "/index.html";
    r._exceptRequest.version = "HTTP/1.1";
    t.push_back(r);
    clearUri(r);

    r._name = "パーセントエンコーディングされた特殊文字";
    r._request = "GET /file%3Fname%3Dvalue%26id%3D123 HTTP/1.1\r\n"
                    "Host: sample\r\n\r\n";
    r._httpStatus.set(http::HttpStatus::OK);
    r._isSuccessTest = true;
    r._exceptRequest.method = "GET";
    r._exceptRequest.path = "/file?name=value&id=123";
    r._exceptRequest.version = "HTTP/1.1";
    t.push_back(r);
    clearUri(r);
    r._exceptRequest.queryVec.clear();


    r._name = "URLとクエリ文字列両方にエンコーディング";
    r._request = "GET /search%20page?q=hello%20world&lang=ja%2Den HTTP/1.1\r\n"
                    "Host: sample\r\n\r\n";
    r._httpStatus.set(http::HttpStatus::OK);
    r._isSuccessTest = true;
    r._exceptRequest.method = "GET";
    r._exceptRequest.path = "/search page";
    r._exceptRequest.version = "HTTP/1.1";
    r._exceptRequest.queryVec.push_back(QueryPair("q", "hello world"));
    r._exceptRequest.queryVec.push_back(QueryPair("lang", "ja-en"));
    t.push_back(r);
    clearUri(r);
    r._exceptRequest.queryVec.clear();

    r._name = "16進数の大文字小文字混在";
    r._request = "GET /test%2a%2A%2d%2D HTTP/1.1\r\nHost: sample\r\n\r\n";
    r._httpStatus.set(http::HttpStatus::OK);
    r._isSuccessTest = true;
    r._exceptRequest.method = "GET";
    r._exceptRequest.path = "/test**--";
    r._exceptRequest.version = "HTTP/1.1";
    t.push_back(r);
    clearUri(r);

    r._name = "複数バイト文字のエンコーディング";
    r._request = "GET /%E6%97%A5%E6%9C%AC%E8%AA%9E HTTP/1.1\r\n"
                    "Host: sample\r\n\r\n";
    r._httpStatus.set(http::HttpStatus::OK);
    r._isSuccessTest = true;
    r._exceptRequest.method = "GET";
    r._exceptRequest.path = "/日本語";
    r._exceptRequest.version = "HTTP/1.1";
    t.push_back(r);
    clearUri(r);

    r._name = "エンコーディングされたスラッシュ";
    r._request = "GET /path1%2Fpath2%2Ffile.html HTTP/1.1\r\n"
                    "Host: sample\r\n\r\n";
    r._httpStatus.set(http::HttpStatus::OK);
    r._isSuccessTest = true;
    r._exceptRequest.method = "GET";
    r._exceptRequest.path = "/path1/path2/file.html";
    r._exceptRequest.version = "HTTP/1.1";
    t.push_back(r);
    clearUri(r);

    r._name = "エンコーディングされたドット";
    r._request = "GET /path1%2E%2Epath2%2Efile%2Ehtml HTTP/1.1\r\n"
                    "Host: sample\r\n\r\n";
    r._httpStatus.set(http::HttpStatus::OK);
    r._isSuccessTest = true;
    r._exceptRequest.method = "GET";
    r._exceptRequest.path = "/path1..path2.file.html";
    r._exceptRequest.version = "HTTP/1.1";
    t.push_back(r);
    clearUri(r);

    r._name = "パーセント記号自体のエンコーディング";
    r._request = "GET /discount%2550%25 HTTP/1.1\r\nHost: sample\r\n\r\n";
    r._httpStatus.set(http::HttpStatus::OK);
    r._isSuccessTest = true;
    r._exceptRequest.method = "GET";
    r._exceptRequest.path = "/discount%50%";
    r._exceptRequest.version = "HTTP/1.1";
    t.push_back(r);
    clearUri(r);

    r._name = "不完全なパーセントエンコーディング（16進数1桁）";
    r._request = "GET /test%3 HTTP/1.1\r\nHost: sample\r\n\r\n";
    r._httpStatus.set(http::HttpStatus::BAD_REQUEST);  // Bad Request
    r._isSuccessTest = false;
    r._exceptRequest.method = "GET";
    r._exceptRequest.path = "/test%3";  // need check
    r._exceptRequest.version = "HTTP/1.1";
    t.push_back(r);
    clearUri(r);

    r._name = "パーセント記号の後に16進数以外の文字";
    r._request = "GET /test%XY HTTP/1.1\r\nHost: sample\r\n\r\n";
    r._httpStatus.set(http::HttpStatus::BAD_REQUEST);  // Bad Request
    r._isSuccessTest = false;
    r._exceptRequest.method = "GET";
    r._exceptRequest.path = "/test%XY";
    r._exceptRequest.version = "HTTP/1.1";
    t.push_back(r);
    clearUri(r);

    r._name = "エンコーディングされたNULL文字";
    r._request = "GET /test%00.html HTTP/1.1\r\nHost: sample\r\n\r\n";
    r._httpStatus.set(http::HttpStatus::BAD_REQUEST);  // Bad Request
    r._isSuccessTest = false;
    r._exceptRequest.method = "GET";
    r._exceptRequest.path = "/test%00.html";
    r._exceptRequest.version = "HTTP/1.1";
    t.push_back(r);
    clearUri(r);

    r._name = "エンコーディングされた制御文字";
    r._request = "GET /test%0A%0D.html HTTP/1.1\r\nHost: sample\r\n\r\n";
    r._httpStatus.set(http::HttpStatus::OK);  // will Not Found
    r._isSuccessTest = true;
    r._exceptRequest.method = "GET";
    r._exceptRequest.path = "/test%0A%0D.html";
    r._exceptRequest.version = "HTTP/1.1";
    t.push_back(r);
    clearUri(r);

    r._name = "エンコードされたディレクトリトラバーサル";
    r._request = "GET /%2e%2e/%2e%2e/etc/passwd HTTP/1.1\r\n"
                    "Host: sample\r\n\r\n";
    r._httpStatus.set(http::HttpStatus::BAD_REQUEST);  // Bad Request
    r._isSuccessTest = false;
    r._exceptRequest.method = "GET";
    r._exceptRequest.path = "/../../etc/passwd";
    r._exceptRequest.version = "HTTP/1.1";
    t.push_back(r);
    clearUri(r);

    r._name = "クエリ文字列内の+記号（スペースを表す）";
    r._request = "GET /search?q=hello+world HTTP/1.1\r\nHost: sample\r\n\r\n";
    r._httpStatus.set(http::HttpStatus::OK);
    r._isSuccessTest = true;
    r._exceptRequest.method = "GET";
    r._exceptRequest.path = "/search";
    r._exceptRequest.version = "HTTP/1.1";
    r._exceptRequest.queryVec.push_back(QueryPair("q", "hello+world"));
    t.push_back(r);
    clearUri(r);
    r._exceptRequest.queryVec.clear();

    r._name = "パス内の+記号（+記号そのもの）";
    r._request = "GET /file+name.html HTTP/1.1\r\nHost: sample\r\n\r\n";
    r._httpStatus.set(http::HttpStatus::OK);
    r._isSuccessTest = true;
    r._exceptRequest.method = "GET";
    r._exceptRequest.path = "/file+name.html";
    r._exceptRequest.version = "HTTP/1.1";
    t.push_back(r);
    clearUri(r);

    r._name = "エンコードされたURLスキーム";
    r._request = "GET /redirect?url=http%3A%2F%2Fexample.com HTTP/1.1\r\n"
                    "Host: sample\r\n\r\n";
    r._httpStatus.set(http::HttpStatus::OK);
    r._isSuccessTest = true;
    r._exceptRequest.method = "GET";
    r._exceptRequest.path = "/redirect";
    r._exceptRequest.version = "HTTP/1.1";
    r._exceptRequest.queryVec.push_back(QueryPair("url", "http://example.com"));
    t.push_back(r);
    clearUri(r);
    r._exceptRequest.queryVec.clear();

    std::string longEncodedPath = "/";
    for (int i = 0; i < 8192; ++i) {
        longEncodedPath += "%41";
    }
    r._name = "非常に長いエンコードされたパス";
    r._request = "GET " + longEncodedPath + " HTTP/1.1\r\nHost: sample\r\n\r\n";
    r._httpStatus.set(http::HttpStatus::URI_TOO_LONG);  // URI Too Long
    r._isSuccessTest = false;
    r._exceptRequest.method = "GET";
    r._exceptRequest.path = longEncodedPath;
    r._exceptRequest.version = "HTTP/1.1";
    t.push_back(r);
    clearUri(r);

    r._name = "エンコードされたリクエストラインの全要素";
    r._request = "%47%45%54 %2F%69%6E%64%65%78%2E%68%74%6D%6C "
                    "%48%54%54%50%2F%31%2E%31\r\nHost: sample\r\n\r\n";
    r._httpStatus.set(http::HttpStatus::BAD_REQUEST);  // Bad Request
    r._isSuccessTest = false;
    r._exceptRequest.method = "%47%45%54";
    r._exceptRequest.path = "%2F%69%6E%64%65%78%2E%68%74%6D%6C";
    r._exceptRequest.version = "%48%54%54%50%2F%31%2E%31";
    t.push_back(r);
    clearUri(r);

    r._name = "エンコードされた日本語のパス";
    r._request = "GET /%E3%83%95%E3%82%A1%E3%82%A4%E3%83%AB.html HTTP/1.1\r\n"
                    "Host: sample\r\n\r\n";
    r._httpStatus.set(http::HttpStatus::OK);
    r._isSuccessTest = true;
    r._exceptRequest.method = "GET";
    r._exceptRequest.path = "/ファイル.html";
    r._exceptRequest.version = "HTTP/1.1";
    t.push_back(r);
    clearUri(r);

    r._name = "パス中の特殊文字の複合エンコーディング";
    r._request = "GET /path/%3C%3E%22%27%60%7B%7D%5B%5D%5C%7C "
                    "HTTP/1.1\r\nHost: sample\r\n\r\n";
    r._httpStatus.set(http::HttpStatus::OK);
    r._isSuccessTest = true;
    r._exceptRequest.method = "GET";
    r._exceptRequest.path = "/path/<>\"'`{}[]\\|";
    r._exceptRequest.version = "HTTP/1.1";
    t.push_back(r);
    clearUri(r);

    r._name = "URIエンコードされたディレクトリ区切り文字の特殊ケース";
    r._request = "GET /dir%2F..%2Fsecret.txt HTTP/1.1\r\nHost: sample\r\n\r\n";
    r._httpStatus.set(http::HttpStatus::OK);  // will Not Found
    r._isSuccessTest = false;
    r._exceptRequest.method = "GET";
    r._exceptRequest.path = "/secret.txt";
    r._exceptRequest.version = "HTTP/1.1";
    t.push_back(r);
    clearUri(r);

    r._name = "マルチレベルエンコードURIのディレクトリトラバーサル";
    r._request = "GET /safe/..%2F..%2F..%2Fetc%2Fpasswd HTTP/1.1\r\n"
                    "Host: sample\r\n\r\n";
    r._httpStatus.set(http::HttpStatus::BAD_REQUEST);  // Bad Request
    r._isSuccessTest = false;
    r._exceptRequest.method = "GET";
    r._exceptRequest.path = "/safe/../../../etc/passwd";
    r._exceptRequest.version = "HTTP/1.1";
    t.push_back(r);
    clearUri(r);

    r._name = "Unicode正規化考慮のURI";
    r._request = "GET /caf%C3%A9.html HTTP/1.1\r\nHost: sample\r\n\r\n";
    r._httpStatus.set(http::HttpStatus::OK);
    r._isSuccessTest = true;
    r._exceptRequest.method = "GET";
    r._exceptRequest.path = "/café.html";
    r._exceptRequest.version = "HTTP/1.1";
    t.push_back(r);
    clearUri(r);

    r._name = "エンコードされたHTTPバージョン";
    r._request = "GET /index.html %48%54%54%50%2F%31%2E%31\r\n"
                    "Host: sample\r\n\r\n";
    r._httpStatus.set(http::HttpStatus::BAD_REQUEST);  // Bad Request
    r._isSuccessTest = false;
    r._exceptRequest.method = "GET";
    r._exceptRequest.path = "/index.html";
    r._exceptRequest.version = "%48%54%54%50%2F%31%2E%31";
    t.push_back(r);
    clearUri(r);

    r._name = "部分的にエンコードされたHTTPバージョン";
    r._request = "GET /index.html HTTP%2F1.1\r\nHost: sample\r\n\r\n";
    r._httpStatus.set(http::HttpStatus::BAD_REQUEST);  // Bad Request
    r._isSuccessTest = false;
    r._exceptRequest.method = "GET";
    r._exceptRequest.path = "/index.html";
    r._exceptRequest.version = "HTTP%2F1.1";
    t.push_back(r);
    clearUri(r);

    r._name = "数字をエンコードしたHTTPバージョン";
    r._request = "GET /index.html HTTP/%31.%31\r\nHost: sample\r\n\r\n";
    r._httpStatus.set(http::HttpStatus::BAD_REQUEST);  // Bad Request
    r._isSuccessTest = false;
    r._exceptRequest.method = "GET";
    r._exceptRequest.path = "/index.html";
    r._exceptRequest.version = "HTTP/%31.%31";
    t.push_back(r);
    clearUri(r);

    r._name = "複合的なエンコーディングケース";
    r._request = "GET /path%20with%20spaces.php?param=%22quoted%22&japanese"
            "=%E6%97%A5%E6%9C%AC%E8%AA%9E HTTP/1.1\r\nHost: sample\r\n\r\n";
    r._httpStatus.set(http::HttpStatus::OK);
    r._isSuccessTest = true;
    r._exceptRequest.method = "GET";
    r._exceptRequest.path = "/path with spaces.php";
    r._exceptRequest.version = "HTTP/1.1";
    r._exceptRequest.queryVec.push_back(QueryPair("param", "\"quoted\""));
    r._exceptRequest.queryVec.push_back(QueryPair("japanese", "日本語"));
    t.push_back(r);
    clearUri(r);
    r._exceptRequest.queryVec.clear();

    r._name = "混合エンコーディング queryの+をSP(対応しない)";
    r._request = "GET /path/with/mixed%20encoding+and+plus?"
                "q=test%20query+with+spaces HTTP/1.1\r\nHost: sample\r\n\r\n";
    r._httpStatus.set(http::HttpStatus::OK);
    r._isSuccessTest = true;
    r._exceptRequest.method = "GET";
    r._exceptRequest.path = "/path/with/mixed encoding+and+plus";
    r._exceptRequest.version = "HTTP/1.1";
    r._exceptRequest.queryVec.push_back(
                                QueryPair("q", "test query+with+spaces"));
    t.push_back(r);
    clearUri(r);
    r._exceptRequest.queryVec.clear();
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
    makePercentEncodingTests(tests);

    std::size_t pass = 0;
    for (std::size_t i = 0; i < tests.size(); ++i) {
        if (runTest(tests[i])) {
            ++pass;
        }
    }
    std::cout << "RequestLine testcase " << pass << " / "
        << tests.size() << std::endl;
}

#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include "../http_namespace.hpp"
#include "../status_code.hpp"
// #include "parse_request.hpp"
#include "request_parser.hpp"

/*
sample
*/

void showAll(HTTPRequest& r) {
    std::cout << "----- Request line ----" << std::endl;
    std::cout << "method: " << r.method << std::endl;
    std::cout << "path: " << r.uri.path << std::endl;
    std::cout << "version: " << r.version << std::endl;
    std::cout << "----- URI query map ----" << std::endl;
    for (std::map<std::string, std::string>::iterator it =
             r.uri.queryMap.begin();
         it != r.uri.queryMap.end(); ++it) {
        std::cout << it->first << ": " << it->second << std::endl;
    }
    std::cout << "----- Fields ----" << std::endl;
    for (std::map<std::string, std::vector<std::string>>::iterator it1 =
             r.fields.get().begin();
         it1 != r.fields.get().end(); ++it1) {
        if (it1->second.empty()) {
            continue;
        }
        std::cout << it1->first << " ->> ";
        for (std::vector<std::string>::iterator it2 = it1->second.begin();
             it2 != it1->second.end(); ++it2) {
            std::cout << *it2;
        }
        std::cout << std::endl;
    }
    std::cout << "----- recv body -----" << std::endl;
    std::cout << r.body.content << std::endl;
}

// テストケースを実行する関数
void runTest(const std::string& testName, const std::string& request) {
    std::cout << "===== テストケース: " << testName << " =====\n";
    std::cout << "リクエスト:\n" << request << "\n";
    RequestParser r;
    try {
        r.run(request);
        showAll(r.get());
    } catch (const std::exception& e) {
        std::cout << "エラー発生: " << e.what() << "\n\n";
    }
}

int main(void) {
    std::vector<std::pair<std::string, std::string>> tests;

    // テストケース1: 基本的なGETリクエスト（元の例）
    tests.push_back(std::make_pair(
        "基本的なGETリクエスト",
        "GET /index.html?id=5&date=2023-04-09&author=yooshima&limit=10 "
        "HTTP/1.1\r\nHost: example.com\r\nUser-Agent: "
        "Mozilla/5.0 (Windows NT 10.0; Win64; x64)\r\nAccept: "
        "text/html,application/xhtml+xml\r\nAccept-Language: "
        "en-US,en;q=0.9\r\nConnection: keep-alive\r\ncontent-length: "
        "10\r\n\r\nbodyy+++++"));

    // テストケース2: POSTリクエスト
    tests.push_back(
        std::make_pair("POSTリクエスト",
                       "POST /submit-form HTTP/1.1\r\n"
                       "Host: example.com\r\n"
                       "Content-Type: application/x-www-form-urlencoded\r\n"
                       "Content-Length: 28\r\n"
                       "\r\n"
                       "username=admin&password=1234"));

    // テストケース3: DELETEリクエスト
    tests.push_back(std::make_pair("DELETEリクエスト",
                                   "DELETE /api/resources/123 HTTP/1.1\r\n"
                                   "Host: example.com\r\n"
                                   "Authorization: Bearer token123\r\n"
                                   "\r\n"));

    tests.push_back(
        std::make_pair("ChunkEncodingリクエスト",
                       "POST /submit-form HTTP/1.1\r\n"
                       "Host: example.com\r\n"
                       "Content-Type: application/x-www-form-urlencoded\r\n"
                       "Transfer-Encoding: chunked\r\n"
                       "\r\n"
                       "1c\r\n"
                       "username=admin&password=1234"
                       "0\r\n"
                       "\r\n"));

    // テストケース1: URLにパーセントエンコードされたスペースを含むGETリクエスト
    tests.push_back(
        std::make_pair("パーセントエンコードされたスペースを含むURL",
                       "GET /hello%20world.html HTTP/1.1\r\n"
                       "Host: example.com\r\n"
                       "User-Agent: Mozilla/5.0\r\n"
                       "\r\n"));

    // テストケース2: URLにパーセントエンコードされた日本語を含むGETリクエスト
    tests.push_back(
        std::make_pair("パーセントエンコードされた日本語を含むURL",
                       "GET /%E6%97%A5%E6%9C%AC%E8%AA%9E.html HTTP/1.1\r\n"
                       "Host: example.com\r\n"
                       "User-Agent: Mozilla/5.0\r\n"
                       "\r\n"));

    // テストケース4: ヘッダーなしの無効なリクエスト
    tests.push_back(std::make_pair("無効なリクエスト（ヘッダーなし）",
                                   "GET / HTTP/1.1\r\n\r\n"));

    // テストケース5: 不正な形式のリクエストライン
    tests.push_back(std::make_pair("不正なリクエストライン",
                                   "INVALID REQUEST LINE\r\n"
                                   "Host: example.com\r\n"
                                   "\r\n"));

    // テストケース6: 複数のヘッダーを持つリクエスト
    tests.push_back(std::make_pair("複数ヘッダー付きリクエスト",
                                   "GET /api/data HTTP/1.1\r\n"
                                   "Host: example.com\r\n"
                                   "User-Agent: Mozilla/5.0\r\n"
                                   "Accept: application/json\r\n"
                                   "Accept-Encoding: gzip, deflate\r\n"
                                   "Accept-Language: ja-JP\r\n"
                                   "Cookie: session=abc123; user=guest\r\n"
                                   "Cache-Control: no-cache\r\n"
                                   "\r\n"));

    // テストケース7: クエリパラメータなしのGETリクエスト
    tests.push_back(std::make_pair("クエリパラメータなしのGETリクエスト",
                                   "GET /index.html HTTP/1.1\r\n"
                                   "Host: example.com\r\n"
                                   "\r\n"));

    tests.push_back(std::make_pair("fieldが重複したリクエスト",
                                   "GET /index.html HTTP/1.1\r\n"
                                   "Host: example.com\r\n"
                                   "Host: example.com\r\n"
                                   "\r\n"));

    // テストケース8: 大きなボディを持つPOSTリクエスト
    std::string largeBody = "";
    for (int i = 0; i < 1000; ++i) {
        largeBody +=
            "data" + std::to_string(i) + "=value" + std::to_string(i) + "&";
    }

    tests.push_back(
        std::make_pair("大きなボディを持つPOSTリクエスト",
                       "POST /upload HTTP/1.1\r\n"
                       "Host: example.com\r\n"
                       "Content-Type: application/x-www-form-urlencoded\r\n"
                       "Content-Length: " +
                           std::to_string(largeBody.size()) +
                           "\r\n"
                           "\r\n" +
                           largeBody));

    // テストケース9: マルチパートフォームデータ（ファイルアップロード）
    std::string boundary = "----WebKitFormBoundaryX7YA8HiPerfectBoundary";
    std::string multipartBody =
        "--" + boundary +
        "\r\n"
        "Content-Disposition: form-data; name=\"username\"\r\n"
        "\r\n"
        "admin\r\n"
        "--" +
        boundary +
        "\r\n"
        "Content-Disposition: form-data; name=\"file\"; "
        "filename=\"test.txt\"\r\n"
        "Content-Type: text/plain\r\n"
        "\r\n"
        "This is the content of the uploaded file.\r\n"
        "--" +
        boundary + "--\r\n";

    tests.push_back(
        std::make_pair("マルチパートフォームデータ",
                       "POST /upload HTTP/1.1\r\n"
                       "Host: example.com\r\n"
                       "Content-Type: multipart/form-data; boundary=" +
                           boundary +
                           "\r\n"
                           "Content-Length: " +
                           std::to_string(multipartBody.size()) +
                           "\r\n"
                           "\r\n" +
                           multipartBody));

    // テストケース10: 不完全なリクエスト（途中で切れている）
    tests.push_back(std::make_pair("不完全なリクエスト",
                                   "GET /index.html HTTP/1.1\r\n"
                                   "Host: example.com\r\n"
                                   "Content-Type: text/html\r\n"
                                   "Cont"));

    // 全てのテストケースを実行
    for (size_t i = 0; i < tests.size(); ++i) {
        runTest(tests[i].first, tests[i].second);
    }

    return 0;
}
#include <iostream>
#include <string>
#include <algorithm>
#include <cassert>
#include "../../../src/http/request/request_parser.hpp"

std::string statusToString(http::BaseParser::ParseStatus status);

void testRequestParser() {
    http::RequestParser parser;
    std::string request =
        "GET /index.html HTTP/1.1\r\n"
        "Host: example.com\r\n"
        "Content-Length: 13\r\n"
        "\r\n"
        "Hello, World!";
    const std::size_t BUFFER_SIZE = 10;

    std::size_t pos = 0;
    http::BaseParser::ParseStatus status = http::BaseParser::P_IN_PROGRESS;

    std::cout << "===== テスト開始 =====\n";

    while (pos < request.size() && status != http::BaseParser::P_ERROR) {
        std::size_t chunk_size = std::min(BUFFER_SIZE, request.size() - pos);
        std::string chunk = request.substr(pos, chunk_size);
        pos += chunk_size;

        status = parser.run(chunk);

        // std::cout << "チャンク処理: [" << chunk << "]\n";
        // std::cout << "ステータス: " << statusToString(status) << "\n";

        if (status == http::BaseParser::P_COMPLETED) {
            http::HTTPRequest& req = parser.get();
            std::cout << "パース完了:\n";
            std::cout << "  メソッド: " << req.method << "\n";
            std::cout << "  URI: " << req.uri.fullUri << "\n";
            std::cout << "  バージョン: " << req.version << "\n";
            std::cout << "  ホスト: " << req.fields.getFieldValue("Host")[0] << "\n";
            std::cout << "  ボディ: " << req.body.content << "\n";

            assert(req.method == "GET");
            assert(req.uri.fullUri == "/index.html");
            assert(req.version == "HTTP/1.1");
            assert(req.fields.getFieldValue("Host")[0] == "example.com");
            assert(req.body.content == "Hello, World!");
        } else if (status == http::BaseParser::P_ERROR) {
            std::cout << "エラー発生: HTTPステータス " 
                      << parser.get().httpStatus.get() << "\n";
        }
    }

    std::cout << "===== テスト終了 =====\n";
    if (status == http::BaseParser::P_COMPLETED) {
        std::cout << "テスト成功: リクエストが正しく解析されました\n";
    } else {
        std::cout << "テスト失敗: リクエストの解析が完了しませんでした\n";
    }
}

std::string statusToString(http::BaseParser::ParseStatus status) {
    switch (status) {
        case http::BaseParser::P_IN_PROGRESS:
            return "処理中";
        case http::BaseParser::P_NEED_MORE_DATA:
            return "データ不足";
        case http::BaseParser::P_COMPLETED:
            return "完了";
        case http::BaseParser::P_ERROR:
            return "エラー";
        default:
            return "不明";
    }
}

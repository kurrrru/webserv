#include <iostream>
#include <string>
#include <algorithm>
#include <cassert>
#include "../../../src/http/cgi/cgi_response_parser.hpp"
#include "../../../src/http/cgi/cgi_response.hpp"
#include "../../../src/http/request/http_fields.hpp"

std::string statusToString(http::BaseParser::ParseStatus status);

void testCgiBufSize() {
    http::CgiResponseParser parser;
    std::string request =
        "Content-type: text/html\r\n"
        "Content-Length: 13\n"
        "\n"
        "Hello, World!";
    const size_t BUFFER_SIZE = 10;

    size_t pos = 0;
    http::BaseParser::ParseStatus status = http::BaseParser::P_IN_PROGRESS;

    std::cout << "===== テスト開始 =====\n";

    while (pos < request.size() && status != http::BaseParser::P_ERROR) {
        size_t chunk_size = std::min(BUFFER_SIZE, request.size() - pos);
        std::string chunk = request.substr(pos, chunk_size);
        pos += chunk_size;

        status = parser.run(chunk);

        // std::cout << "チャンク処理: [" << chunk << "]\n";
        // std::cout << "ステータス: " << statusToString(status) << "\n";
    }
    if (status == http::BaseParser::P_NEED_MORE_DATA) {
        std::cout << "パース完了:\n";
        std::cout << "  content-type: " << parser.get().fields.getFieldValue(http::fields::CONTENT_TYPE).front() << "\n";
        std::cout << "  content-length: " << parser.get().fields.getFieldValue(http::fields::CONTENT_LENGTH).front() << "\n";
        std::cout << "  ボディ: " << parser.get().body << "\n";
        assert(parser.get().fields.getFieldValue(http::fields::CONTENT_TYPE).front() == "text/html");
        assert(parser.get().fields.getFieldValue(http::fields::CONTENT_LENGTH).front() == "13");
        assert(parser.get().body == "Hello, World!");
    } else if (status == http::BaseParser::P_ERROR) {
        std::cout << "エラー発生: HTTPステータス " 
                    << parser.get().httpStatus.get() << "\n";
    }
    std::cout << "===== テスト終了 =====\n";
    if (status == http::BaseParser::P_NEED_MORE_DATA) {
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

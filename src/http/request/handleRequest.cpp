#include <string>
#include <vector>

#include "../cgi/cgi_handler.hpp"
#include "../cgi/cgi_execute.hpp"
#include "request.hpp"

namespace http {
void Request::handleRequest(const Client& client) {
    /*
    CGIチェック
        CgiHandlerクラスのisCgiRequest関数を呼び出す　呼び出す際に以下の値が必要
            リクエストURLパス
            対象locationのcgi_extension
            対象locationのcgi_path
            対象locationのallowed_method
    1の結果、CGIの場合
        CgiHandlerクラスのhandleRequest関数を呼び出す　呼び出す際に以下の値が必要
            request
            response
            client
            対象location path or 対象location（検討中）
        CGI呼び出さない場合
            メソッドに対応するrun[メソッド]関数を呼び出す
    設定ファイルupload_storeに関して
    runPost関数の中でいい感じに値取得してどうこうする？
    // 可能であればPOSTの引数として渡したいが、、
        bool isCgiRequest(const std::string& targetPath,
                      const std::vector<std::string>& cgiExtensionVector,
                      const std::string& cgiPath) const;
    bool handleRequest(const HTTPRequest& request,
                       Response& Response,
                       const std::string& rootPath,
                       const std::vector<std::string>& cgiExtensionVector,
                       const std::string& cgiPath);
    void setRedirectCount(size_t count) { _redirectCount = count; }
*/
    CgiHandler cgiHandler;

    std::string targetPath = _parsedRequest.get().uri.path;
    const std::vector<std::string>& cgiExtensionVector = _config.getCgiExtensions();
    const std::string& cgiPath = _config.getCgiPath();

    if (cgiHandler.isCgiRequest(targetPath, cgiExtensionVector, cgiPath)) {
        // CGIリクエストの場合
        if (!cgiHandler.handleRequest(_parsedRequest.get(),
                                      _response,
                                      _config.getRoot(),
                                      cgiExtensionVector,
                                      cgiPath)) {
            // CGIハンドリングに失敗した場合、エラーレスポンスを設定
            _response.setStatus(HttpStatus::INTERNAL_SERVER_ERROR);
        }
    } else {
        /*
        メソッドの取得
        メソッドが現在のロケーションに割り当てられてるか
        ある
            run[メソッド]関数を呼び出す
        ない
            HTTPステータス405 Method Not Allowedを設定
            対象ロケーションのallowed_methodに応じて、Allowヘッダーを設定
            エラーレスポンスを返す
        */
    }
}

}  // namespace http

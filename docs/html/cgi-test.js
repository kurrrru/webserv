// CGI Test Functions
// このファイルはCGIスクリプトのテスト用関数を含みます

class CgiTester {
    constructor() {
        this.responseDiv = null;
        this.contentDiv = null;
    }

    init() {
        this.responseDiv = document.getElementById('cgiTestResponse');
        this.contentDiv = document.getElementById('cgiTestContent');
    }

    showResponse(content, isError = false) {
        if (!this.responseDiv || !this.contentDiv) {
            this.init();
        }
        this.responseDiv.style.display = 'block';
        this.contentDiv.innerHTML = content;
        this.contentDiv.style.color = isError ? '#dc3545' : '#212529';
    }

    showSuccess(content) {
        this.showResponse(content);
        this.contentDiv.style.color = '#28a745';
    }

    showError(content) {
        this.showResponse(content, true);
    }

    // 汎用CGIスクリプトテスト関数
    async testCgiScript(url, method = 'GET') {
        this.showResponse(`${method}リクエスト送信中...\nURL: ${url}`);
        
        const startTime = Date.now();
        
        try {
            const response = await fetch(url, {
                method: method,
                redirect: 'manual'
            });
            
            const endTime = Date.now();
            const responseTime = endTime - startTime;
            
            let responseInfo = '';
            responseInfo += `=== CGI ${method}リクエスト結果 ===\n`;
            responseInfo += `URL: ${url}\n`;
            responseInfo += `メソッド: ${method}\n`;
            responseInfo += `レスポンス時間: ${responseTime}ms\n\n`;
            
            // リダイレクト検出
            if (response.type === 'opaqueredirect') {
                responseInfo += '=== リダイレクト検出 ===\n';
                responseInfo += 'ステータス: リダイレクトレスポンス (opaque)\n';
                responseInfo += '注意: ブラウザのセキュリティ制限により、リダイレクトの詳細情報は取得できません\n';
                responseInfo += 'curlコマンドで詳細を確認してください: curl -v ' + window.location.origin + url + '\n\n';
                responseInfo += '=== 期待される動作 ===\n';
                responseInfo += 'サーバーはCGIスクリプトからのLocationヘッダーを受け取り、\n';
                responseInfo += '適切な302リダイレクトレスポンスを生成するはずです。\n';
                this.showSuccess(responseInfo);
                return;
            }
            
            // ステータス情報
            responseInfo += '=== ステータス ===\n';
            responseInfo += `ステータスコード: ${response.status}\n`;
            responseInfo += `ステータステキスト: ${response.statusText}\n`;
            responseInfo += `OK: ${response.ok}\n\n`;
            
            // ヘッダー情報
            responseInfo += '=== レスポンスヘッダー ===\n';
            const headers = [];
            for (let [key, value] of response.headers.entries()) {
                headers.push(`${key}: ${value}`);
            }
            
            if (headers.length > 0) {
                headers.sort();
                responseInfo += headers.join('\n') + '\n\n';
            } else {
                responseInfo += '(ヘッダーが見つかりません)\n\n';
            }
            
            // リダイレクト情報
            if (response.status >= 300 && response.status < 400) {
                const location = response.headers.get('Location');
                responseInfo += '=== リダイレクト情報 ===\n';
                responseInfo += `リダイレクト先: ${location || '(Location header not found)'}\n\n`;
            }
            
            // レスポンス内容
            if (response.status < 300 || response.status >= 400) {
                const responseText = await response.text();
                responseInfo += '=== レスポンス内容 ===\n';
                responseInfo += responseText + '\n';
            }
            
            // 特殊なテストケースの成功判定
            if (response.ok || response.status === 504 || response.status === 408 || 
                (response.status === 500 && (url.includes('loop1.py') || url.includes('loop2.py')))) {
                
                if (response.status === 504 || response.status === 408) {
                    responseInfo += '\n✓ タイムアウトテストが正常に動作しました！';
                } else if (response.status === 500 && (url.includes('loop1.py') || url.includes('loop2.py'))) {
                    responseInfo += '\n✓ リダイレクトループテストが正常に動作しました！';
                    responseInfo += '\n📋 stepmark.logで "[ERROR] CGI redirect loop detected" が確認できればOKです';
                }
                this.showSuccess(responseInfo);
            } else if (response.status >= 400) {
                this.showError(responseInfo);
            } else {
                this.showResponse(responseInfo);
            }
            
        } catch (error) {
            const endTime = Date.now();
            const responseTime = endTime - startTime;
            
            let errorInfo = '';
            errorInfo += `=== CGI ${method}リクエストエラー ===\n`;
            errorInfo += `URL: ${url}\n`;
            errorInfo += `メソッド: ${method}\n`;
            errorInfo += `エラー時間: ${responseTime}ms\n\n`;
            errorInfo += `エラー内容: ${error.message}\n\n`;
            errorInfo += '考えられる原因:\n';
            errorInfo += '- CGIスクリプトが存在しない\n';
            errorInfo += '- CGIスクリプトに実行権限がない\n';
            errorInfo += '- サーバーのCGI設定に問題がある\n';
            errorInfo += '- ネットワーク接続エラー';
            
            this.showError(errorInfo);
        }
    }

    // 大容量出力テスト
    async testLargeOutput() {
        this.showResponse('GETリクエスト送信中...\nURL: /large_output_test.py\n注意: 64KBの大容量出力のため時間がかかる場合があります');
        
        const startTime = Date.now();
        
        try {
            const controller = new AbortController();
            const timeoutId = setTimeout(() => controller.abort(), 60000);
            
            const response = await fetch('/large_output_test.py', {
                method: 'GET',
                signal: controller.signal,
                redirect: 'manual'
            });
            
            clearTimeout(timeoutId);
            const endTime = Date.now();
            const responseTime = endTime - startTime;
            
            let responseInfo = '';
            responseInfo += '=== 大容量出力テスト結果 ===\n';
            responseInfo += 'URL: /large_output_test.py\n';
            responseInfo += `レスポンス時間: ${responseTime}ms\n\n`;
            
            if (response.type === 'opaqueredirect') {
                responseInfo += '=== リダイレクト検出 ===\n';
                responseInfo += 'ステータス: リダイレクトレスポンス (opaque)\n';
                responseInfo += 'curlコマンドで詳細を確認してください: curl -v ' + window.location.origin + '/large_output_test.py\n';
                this.showSuccess(responseInfo);
                return;
            }
            
            responseInfo += '=== ステータス ===\n';
            responseInfo += `ステータスコード: ${response.status}\n`;
            responseInfo += `ステータステキスト: ${response.statusText}\n\n`;
            
            // ヘッダー情報
            responseInfo += '=== レスポンスヘッダー ===\n';
            const headers = [];
            for (let [key, value] of response.headers.entries()) {
                headers.push(`${key}: ${value}`);
            }
            
            if (headers.length > 0) {
                headers.sort();
                responseInfo += headers.join('\n') + '\n\n';
            }
            
            if (response.ok) {
                const responseText = await response.text();
                responseInfo += '=== レスポンス内容 ===\n';
                responseInfo += `レスポンスサイズ: ${responseText.length} 文字\n`;
                if (responseText.length > 500) {
                    responseInfo += '先頭500文字:\n';
                    responseInfo += responseText.substring(0, 500) + '\n\n...(残り省略)\n\n';
                    responseInfo += '末尾100文字:\n';
                    responseInfo += responseText.substring(responseText.length - 100);
                } else {
                    responseInfo += responseText;
                }
                this.showSuccess(responseInfo);
            } else {
                responseInfo += '=== エラー内容 ===\n';
                const errorText = await response.text();
                responseInfo += errorText;
                this.showError(responseInfo);
            }
            
        } catch (error) {
            const endTime = Date.now();
            const responseTime = endTime - startTime;
            
            let errorInfo = '';
            errorInfo += '=== 大容量出力テストエラー ===\n';
            errorInfo += 'URL: /large_output_test.py\n';
            errorInfo += `エラー時間: ${responseTime}ms\n\n`;
            
            if (error.name === 'AbortError') {
                errorInfo += 'エラー内容: タイムアウト (60秒)\n\n';
                errorInfo += '原因:\n';
                errorInfo += '- 64KBの大容量出力の処理に時間がかかりすぎている\n';
                errorInfo += '- サーバーの処理が遅い\n';
                errorInfo += '- ネットワークの問題\n\n';
                errorInfo += 'curlコマンドで確認してください:\n';
                errorInfo += `curl -v ${window.location.origin}/large_output_test.py`;
            } else {
                errorInfo += `エラー内容: ${error.message}\n\n`;
                errorInfo += '考えられる原因:\n';
                errorInfo += '- CGIスクリプトが存在しない\n';
                errorInfo += '- CGIスクリプトに実行権限がない\n';
                errorInfo += '- サーバーの大容量出力処理に問題がある\n';
                errorInfo += '- ネットワーク接続エラー';
            }
            
            this.showError(errorInfo);
        }
    }

    // 大容量POSTテスト
    async testLargePost() {
        this.showResponse('POSTリクエスト送信中...\nURL: /large_post_test.py\nデータサイズ: 1MB');
        
        try {
            const testData = 'TEST_DATA_' + 'X'.repeat(1024 * 1024 - 10);
            
            const response = await fetch('/large_post_test.py', {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/octet-stream'
                },
                body: testData
            });
            
            const responseText = await response.text();
            
            let responseInfo = '';
            responseInfo += '=== CGI POSTリクエスト結果 ===\n';
            responseInfo += 'URL: /large_post_test.py\n';
            responseInfo += 'メソッド: POST\n';
            responseInfo += 'データサイズ: 1MB\n\n';
            responseInfo += '=== ステータス ===\n';
            responseInfo += `ステータスコード: ${response.status}\n`;
            responseInfo += `ステータステキスト: ${response.statusText}\n\n`;
            responseInfo += '=== レスポンス内容 ===\n';
            responseInfo += responseText;
            
            if (response.ok) {
                this.showSuccess(responseInfo);
            } else {
                this.showError(responseInfo);
            }
        } catch (error) {
            this.showError(`エラー: ${error.message}`);
        }
    }

    // 書き込みタイムアウトテスト
    async testWriteTimeout() {
        this.showResponse('タイムアウトテスト開始...\nURL: /write_timeout_test.py\nデータサイズ: 2MB\n\n注意: このテストは最大60秒かかる場合があります');
        
        const startTime = Date.now();
        
        try {
            const testData = 'TIMEOUT_TEST_' + 'Y'.repeat(2 * 1024 * 1024 - 13);
            
            const response = await fetch('/write_timeout_test.py', {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/octet-stream'
                },
                body: testData
            });
            
            const endTime = Date.now();
            const responseTime = endTime - startTime;
            const responseText = await response.text();
            
            let responseInfo = '';
            responseInfo += '=== タイムアウトテスト結果 ===\n';
            responseInfo += 'URL: /write_timeout_test.py\n';
            responseInfo += `レスポンス時間: ${responseTime}ms\n\n`;
            responseInfo += '=== ステータス ===\n';
            responseInfo += `ステータスコード: ${response.status}\n`;
            responseInfo += `ステータステキスト: ${response.statusText}\n\n`;
            
            if (response.status === 504 || response.status === 408) {
                responseInfo += '✓ サーバーが正しくタイムアウトを検出しました\n\n';
            }
            
            responseInfo += '=== レスポンス内容 ===\n';
            responseInfo += responseText;
            
            if (response.status === 504 || response.status === 408) {
                this.showSuccess(responseInfo);
            } else {
                this.showError(responseInfo);
            }
        } catch (error) {
            const endTime = Date.now();
            const responseTime = endTime - startTime;
            
            this.showError(`タイムアウトテストエラー\n時間: ${responseTime}ms\nエラー: ${error.message}`);
        }
    }
}

// グローバルなCgiTesterインスタンス
const cgiTester = new CgiTester();

// グローバル関数（HTMLから呼び出し用）
function testCgiScript(url, method) {
    cgiTester.testCgiScript(url, method);
}

function testLargeOutput() {
    cgiTester.testLargeOutput();
}

function testCgiLargePost() {
    cgiTester.testLargePost();
}

function testCgiWriteTimeout() {
    cgiTester.testWriteTimeout();
}

// CGI GET request test (get current time)
async function performCgiGetRequest() {
    cgiTester.showResponse('CGI GETリクエスト送信中...\nURL: /script.py');
    
    try {
        const response = await fetch('/script.py', {
            method: 'GET',
            headers: {
                'Accept': 'text/plain'
            }
        });
        
        const responseText = await response.text();
        
        let responseInfo = '';
        responseInfo += '=== CGI GETリクエスト結果 ===\n';
        responseInfo += 'URL: /script.py\n';
        responseInfo += 'メソッド: GET\n\n';
        responseInfo += '=== ステータス ===\n';
        responseInfo += 'ステータスコード: ' + response.status + '\n';
        responseInfo += 'ステータステキスト: ' + response.statusText + '\n';
        responseInfo += 'OK: ' + response.ok + '\n\n';
        
        if (response.ok) {
            responseInfo += '=== レスポンス内容 ===\n';
            responseInfo += responseText + '\n\n';
        } else {
            responseInfo += '=== エラー内容 ===\n';
            responseInfo += responseText + '\n\n';
        }
        
        // Header info
        responseInfo += '=== レスポンスヘッダー ===\n';
        const headers = [];
        for (let [key, value] of response.headers.entries()) {
            headers.push(key + ': ' + value);
        }
        
        if (headers.length > 0) {
            headers.sort();
            responseInfo += headers.join('\n');
        } else {
            responseInfo += '(ヘッダーが見つかりません)';
        }
        
        if (response.ok) {
            cgiTester.showSuccess(responseInfo);
        } else {
            cgiTester.showError(responseInfo);
        }
    } catch (error) {
        let errorInfo = '';
        errorInfo += '=== CGI GETリクエストエラー ===\n';
        errorInfo += 'URL: /script.py\n';
        errorInfo += 'メソッド: GET\n\n';
        errorInfo += 'エラー内容: ' + error.message + '\n\n';
        errorInfo += '考えられる原因:\n';
        errorInfo += '- CGIスクリプトが存在しない\n';
        errorInfo += '- CGIスクリプトに実行権限がない\n';
        errorInfo += '- サーバーのCGI設定に問題がある\n';
        errorInfo += '- ネットワーク接続エラー';
        
        cgiTester.showError(errorInfo);
    }
}

function performCgiPostRequest() {
    const fileInput = document.getElementById('cgiFile');
    const filenameInput = document.getElementById('cgiFilename');
    
    if (!fileInput.files.length) {
        alert('ファイルを選択してください');
        return;
    }
    
    const file = fileInput.files[0];
    const customFilename = filenameInput.value.trim();
    
    cgiTester.showResponse(`ファイル "${file.name}" をCGI経由でアップロード中...`);
    
    (async () => {
        try {
            const formData = new FormData();
            if (customFilename) {
                const blob = new Blob([file], { type: file.type });
                formData.append('file', blob, customFilename);
            } else {
                formData.append('file', file);
            }
            
            const response = await fetch('/script.py', {
                method: 'POST',
                body: formData
            });
            
            const responseText = await response.text();
            
            let responseInfo = '';
            responseInfo += '=== CGI POSTリクエスト結果 ===\n';
            responseInfo += 'URL: /script.py\n';
            responseInfo += 'メソッド: POST\n';
            responseInfo += 'Content-Type: multipart/form-data\n\n';
            responseInfo += '=== アップロード情報 ===\n';
            responseInfo += 'ファイル名: ' + file.name + '\n';
            responseInfo += 'ファイルサイズ: ' + file.size + ' bytes\n';
            responseInfo += 'ファイルタイプ: ' + (file.type || 'unknown') + '\n';
            if (customFilename) {
                responseInfo += '指定ファイル名: ' + customFilename + '\n';
            }
            responseInfo += '\n';
            responseInfo += '=== ステータス ===\n';
            responseInfo += 'ステータスコード: ' + response.status + '\n';
            responseInfo += 'ステータステキスト: ' + response.statusText + '\n';
            responseInfo += 'OK: ' + response.ok + '\n\n';
            
            if (response.ok) {
                responseInfo += '=== アップロード結果 ===\n';
                responseInfo += responseText + '\n\n';
                fileInput.value = '';
                filenameInput.value = '';
                if (typeof loadFileList === 'function') {
                    loadFileList();
                }
                cgiTester.showSuccess(responseInfo);
            } else {
                responseInfo += '=== エラー内容 ===\n';
                responseInfo += responseText + '\n\n';
                cgiTester.showError(responseInfo);
            }
            
            // ヘッダー情報
            responseInfo += '=== レスポンスヘッダー ===\n';
            const headers = [];
            for (let [key, value] of response.headers.entries()) {
                headers.push(key + ': ' + value);
            }
            
            if (headers.length > 0) {
                headers.sort();
                responseInfo += headers.join('\n');
            } else {
                responseInfo += '(ヘッダーが見つかりません)';
            }
            
        } catch (error) {
            let errorInfo = '';
            errorInfo += '=== CGI POSTリクエストエラー ===\n';
            errorInfo += 'URL: /script.py\n';
            errorInfo += 'メソッド: POST\n\n';
            errorInfo += 'エラー内容: ' + error.message + '\n\n';
            errorInfo += '考えられる原因:\n';
            errorInfo += '- CGIスクリプトが存在しない\n';
            errorInfo += '- CGIスクリプトに実行権限がない\n';
            errorInfo += '- アップロードディレクトリに書き込み権限がない\n';
            errorInfo += '- ファイルサイズが上限を超えている\n';
            errorInfo += '- サーバーのCGI設定に問題がある\n';
            errorInfo += '- ネットワーク接続エラー';
            
            cgiTester.showError(errorInfo);
        }
    })();
}
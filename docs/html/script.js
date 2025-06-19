function openTab(tabName) {
    const contents = document.querySelectorAll('.tab-content');
    const buttons = document.querySelectorAll('.tab-button');
    
    contents.forEach(content => {
        content.classList.remove('active');
    });
    
    buttons.forEach(button => {
        button.classList.remove('active');
    });
    
    document.getElementById(tabName).classList.add('active');
    event.target.classList.add('active');
}

// Auto-load file list when page loads
document.addEventListener('DOMContentLoaded', function() {
    loadFileList();
});

// HEAD request function (for preset URLs)
function performHeadRequest(url) {
    if (!url) {
        const urlSelect = document.getElementById('headUrl');
        url = urlSelect ? urlSelect.value : '/';
    }
    executeHeadRequest(url);
}

// HEAD request function (for custom URLs)
function performCustomHeadRequest() {
    const url = document.getElementById('customHeadUrl').value.trim();
    if (!url) {
        alert('URLを入力してください');
        return;
    }
    executeHeadRequest(url);
}

// Execute HEAD request and display response
function executeHeadRequest(url) {
    const responseDiv = document.getElementById('headResponse');
    const contentDiv = document.getElementById('headResponseContent');
    
    // Show response display area
    responseDiv.style.display = 'block';
    contentDiv.innerHTML = 'HEADリクエスト送信中...\nURL: ' + url;

    const startTime = Date.now();
    
    fetch(url, {
        method: 'HEAD'
    })
    .then(response => {
        const endTime = Date.now();
        const responseTime = endTime - startTime;
        
        // Build response info
        let responseInfo = '';
        responseInfo += '=== HEADリクエスト結果 ===\n';
        responseInfo += 'URL: ' + url + '\n';
        responseInfo += 'メソッド: HEAD\n';
        responseInfo += 'レスポンス時間: ' + responseTime + 'ms\n\n';
        
        // Status info
        responseInfo += '=== ステータス ===\n';
        responseInfo += 'ステータスコード: ' + response.status + '\n';
        responseInfo += 'ステータステキスト: ' + response.statusText + '\n';
        responseInfo += 'OK: ' + response.ok + '\n\n';
        
        // Header info
        responseInfo += '=== レスポンスヘッダー ===\n';
        const headers = [];
        for (let [key, value] of response.headers.entries()) {
            headers.push(key + ': ' + value);
        }
        
        if (headers.length > 0) {
            headers.sort(); // Sort headers alphabetically
            responseInfo += headers.join('\n');
        } else {
            responseInfo += '(ヘッダーが見つかりません)';
        }
        contentDiv.innerHTML = responseInfo;
    })
    .catch(error => {
        const endTime = Date.now();
        const responseTime = endTime - startTime;
        
        let errorInfo = '';
        errorInfo += '=== HEADリクエストエラー ===\n';
        errorInfo += 'URL: ' + url + '\n';
        errorInfo += 'メソッド: HEAD\n';
        errorInfo += 'エラー時間: ' + responseTime + 'ms\n\n';
        errorInfo += 'エラー内容: ' + error.message;
        
        contentDiv.innerHTML = errorInfo;
        contentDiv.style.color = '#dc3545'; // Red color for errors
    });
}

function deleteFile(filepath) {
    if (confirm('本当にファイル "' + filepath + '" を削除しますか？')) {
        const responseDiv = document.getElementById('deleteResponse');
        const contentDiv = document.getElementById('deleteResponseContent');
        
        // Show response display area
        responseDiv.style.display = 'block';
        contentDiv.innerHTML = 'DELETEリクエスト送信中...\nファイル: ' + filepath;
        contentDiv.style.color = '#4a7c4a';

        const startTime = Date.now();
        
        fetch(filepath, {
            method: 'DELETE'
        })
        .then(async response => {
            const endTime = Date.now();
            const responseTime = endTime - startTime;
            
            const responseText = await response.text();
            
            let responseInfo = '';
            responseInfo += '=== DELETEリクエスト結果（ファイル削除） ===\n';
            responseInfo += 'URL: ' + filepath + '\n';
            responseInfo += 'メソッド: DELETE\n';
            responseInfo += 'レスポンス時間: ' + responseTime + 'ms\n\n';
            
            // Status info
            responseInfo += '=== ステータス ===\n';
            responseInfo += 'ステータスコード: ' + response.status + '\n';
            responseInfo += 'ステータステキスト: ' + response.statusText + '\n';
            responseInfo += 'OK: ' + response.ok + '\n\n';
            
            if (response.ok) {
                responseInfo += '=== レスポンス内容 ===\n';
                responseInfo += responseText + '\n\n';
                responseInfo += '✅ ファイルが正常に削除されました\n\n';
                contentDiv.style.color = '#4a7c4a';
                loadFileList(); // Update file list
            } else {
                responseInfo += '=== エラー内容 ===\n';
                responseInfo += responseText + '\n\n';
                responseInfo += '❌ 削除に失敗しました\n\n';
                contentDiv.style.color = '#dc3545';
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
            
            contentDiv.innerHTML = responseInfo;
        })
        .catch(error => {
            const endTime = Date.now();
            const responseTime = endTime - startTime;
            
            let errorInfo = '';
            errorInfo += '=== DELETEリクエストエラー（ファイル削除） ===\n';
            errorInfo += 'URL: ' + filepath + '\n';
            errorInfo += 'メソッド: DELETE\n';
            errorInfo += 'エラー時間: ' + responseTime + 'ms\n\n';
            errorInfo += 'エラー内容: ' + error.message + '\n\n';
            errorInfo += '考えられる原因:\n';
            errorInfo += '- ネットワーク接続エラー\n';
            errorInfo += '- サーバーが応答しない\n';
            errorInfo += '- ファイルが存在しない\n';
            errorInfo += '- 削除権限がない';
            
            contentDiv.innerHTML = errorInfo;
            contentDiv.style.color = '#dc3545';
        });
    }
}

function loadFileList() {
    // Show loading message
    document.getElementById('files').innerHTML = '<p style="color: #666; font-style: italic;">読み込み中...</p>';
    
    fetch('/uploads/8080/', {
        method: 'GET'
    })
    .then(response => {
        if (response.ok) {
            // Check if response is JSON
            const contentType = response.headers.get('content-type');
            if (contentType && contentType.includes('application/json')) {
                return response.json();
            } else {
                // Parse HTML directory listing
                return response.text().then(html => {
                    const parser = new DOMParser();
                    const doc = parser.parseFromString(html, 'text/html');
                    const links = doc.querySelectorAll('a');
                    const files = [];
                    links.forEach(link => {
                        const href = link.getAttribute('href');
                        // Skip parent directory, subdirectories, and hidden files
                        if (href && href !== '../' && href !== '.' && href !== '..' && 
                            !href.endsWith('/') && !href.startsWith('.')) {
                            // Remove URL encoding if present
                            const decodedHref = decodeURIComponent(href);
                            // Also check decoded name for hidden files
                            const filename = decodedHref.split('/').pop();
                            if (!filename.startsWith('.')) {
                                files.push('uploads/8080/' + filename);
                            }
                        }
                    });
                    return files;
                });
            }
        }
        throw new Error('ファイル一覧の取得に失敗しました (Status: ' + response.status + ')');
    })
    .then(files => {
        displayFileList(files);
    })
    .catch(error => {
        document.getElementById('files').innerHTML = '<p style="color: #dc3545;">エラー: ' + error.message + '</p>';
    });
}

function displayFileList(files) {
    const filesDiv = document.getElementById('files');
    if (files.length === 0) {
        filesDiv.innerHTML = '<p style="color: #666; font-style: italic;">アップロードされたファイルはありません</p>';
        return;
    }

    let html = '<table style="width: 100%; border-collapse: collapse;">';
    html += '<tr style="background-color: #f8f9fa; border-bottom: 1px solid #dee2e6;"><th style="padding: 8px; text-align: left;">ファイル名</th><th style="padding: 8px; text-align: center;">操作</th></tr>';
    
    files.forEach(file => {
        const filepath = file;
        const filename = file.split('/').pop(); // Extract only the filename
        html += '<tr style="border-bottom: 1px solid #dee2e6;">';
        html += '<td style="padding: 8px;"><a href="' + filepath + '" target="_blank" style="color: #007bff; text-decoration: none;">' + filename + '</a></td>';
        html += '<td style="padding: 8px; text-align: center;"><button onclick="deleteFile(\'' + filepath + '\')" style="background-color: #dc3545; color: white; padding: 4px 8px; border: none; border-radius: 3px; cursor: pointer; font-size: 12px;">削除</button></td>';
        html += '</tr>';
    });
    
    html += '</table>';
    filesDiv.innerHTML = html;
}

function deleteManualFile() {
    const filepath = document.getElementById('manualDeletePath').value.trim();
    if (!filepath) {
        alert('ファイルパスを入力してください');
        return;
    }
    deleteFile(filepath);
}

function deleteEmptyDir() {
    const dirpath = '/delete/';
    if (confirm('本当にディレクトリ "' + dirpath + '" を削除しますか？')) {
        const responseDiv = document.getElementById('deleteResponse');
        const contentDiv = document.getElementById('deleteResponseContent');
        
        // Show response display area
        responseDiv.style.display = 'block';
        contentDiv.innerHTML = 'DELETEリクエスト送信中...\nディレクトリ: ' + dirpath;
        contentDiv.style.color = '#4a7c4a';

        const startTime = Date.now();
        
        fetch(dirpath, {
            method: 'DELETE'
        })
        .then(async response => {
            const endTime = Date.now();
            const responseTime = endTime - startTime;
            
            const responseText = await response.text();
            
            let responseInfo = '';
            responseInfo += '=== DELETEリクエスト結果（ディレクトリ削除） ===\n';
            responseInfo += 'URL: ' + dirpath + '\n';
            responseInfo += 'メソッド: DELETE\n';
            responseInfo += 'レスポンス時間: ' + responseTime + 'ms\n\n';
            
            // Status info
            responseInfo += '=== ステータス ===\n';
            responseInfo += 'ステータスコード: ' + response.status + '\n';
            responseInfo += 'ステータステキスト: ' + response.statusText + '\n';
            responseInfo += 'OK: ' + response.ok + '\n\n';
            
            if (response.ok) {
                responseInfo += '=== レスポンス内容 ===\n';
                responseInfo += responseText + '\n\n';
                responseInfo += '✅ ディレクトリが正常に削除されました\n\n';
                contentDiv.style.color = '#4a7c4a';
            } else {
                responseInfo += '=== エラー内容 ===\n';
                responseInfo += responseText + '\n\n';
                if (response.status === 405) {
                    responseInfo += '⚠️ 405 Method Not Allowed: ディレクトリ削除が許可されていません\n\n';
                } else {
                    responseInfo += '❌ 削除に失敗しました\n\n';
                }
                contentDiv.style.color = '#dc3545';
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
            
            contentDiv.innerHTML = responseInfo;
        })
        .catch(error => {
            const endTime = Date.now();
            const responseTime = endTime - startTime;
            
            let errorInfo = '';
            errorInfo += '=== DELETEリクエストエラー（ディレクトリ削除） ===\n';
            errorInfo += 'URL: ' + dirpath + '\n';
            errorInfo += 'メソッド: DELETE\n';
            errorInfo += 'エラー時間: ' + responseTime + 'ms\n\n';
            errorInfo += 'エラー内容: ' + error.message;
            
            contentDiv.innerHTML = errorInfo;
            contentDiv.style.color = '#dc3545';
        });
    }
}

// File upload with fetch API
async function performFileUpload() {
    const fileInput = document.getElementById('file');
    const responseDiv = document.getElementById('postResponse');
    const contentDiv = document.getElementById('postResponseContent');
    
    if (!fileInput.files.length) {
        alert('ファイルを選択してください');
        return;
    }
    
    const file = fileInput.files[0];
    const formData = new FormData();
    formData.append('uploadFile', file);
    
    // Show response display area
    responseDiv.style.display = 'block';
    contentDiv.innerHTML = `ファイル "${file.name}" をアップロード中...`;
    contentDiv.style.color = '#4a7c4a';
    
    const startTime = Date.now();
    
    try {
        const response = await fetch('/uploads', {
            method: 'POST',
            body: formData
        });
        
        const endTime = Date.now();
        const responseTime = endTime - startTime;
        
        const responseText = await response.text();
        
        let responseInfo = '';
        responseInfo += '=== POSTリクエスト結果（ファイルアップロード） ===\n';
        responseInfo += 'URL: /uploads\n';
        responseInfo += 'メソッド: POST\n';
        responseInfo += 'Content-Type: multipart/form-data\n';
        responseInfo += 'レスポンス時間: ' + responseTime + 'ms\n\n';
        
        // Upload info
        responseInfo += '=== アップロード情報 ===\n';
        responseInfo += 'ファイル名: ' + file.name + '\n';
        responseInfo += 'ファイルサイズ: ' + file.size + ' bytes\n';
        responseInfo += 'ファイルタイプ: ' + (file.type || 'unknown') + '\n\n';
        
        // Status info
        responseInfo += '=== ステータス ===\n';
        responseInfo += 'ステータスコード: ' + response.status + '\n';
        responseInfo += 'ステータステキスト: ' + response.statusText + '\n';
        responseInfo += 'OK: ' + response.ok + '\n\n';
        
        if (response.ok) {
            responseInfo += '=== レスポンス内容 ===\n';
            responseInfo += responseText + '\n\n';
            contentDiv.style.color = '#4a7c4a';
            // Clear file input on successful upload
            fileInput.value = '';
            // Update file list in DELETE tab
            loadFileList();
        } else {
            responseInfo += '=== エラー内容 ===\n';
            responseInfo += responseText + '\n\n';
            contentDiv.style.color = '#dc3545';
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
        
        contentDiv.innerHTML = responseInfo;
    } catch (error) {
        const endTime = Date.now();
        const responseTime = endTime - startTime;
        
        let errorInfo = '';
        errorInfo += '=== POSTリクエストエラー（ファイルアップロード） ===\n';
        errorInfo += 'URL: /uploads\n';
        errorInfo += 'メソッド: POST\n';
        errorInfo += 'エラー時間: ' + responseTime + 'ms\n\n';
        errorInfo += 'エラー内容: ' + error.message;
        
        contentDiv.innerHTML = errorInfo;
        contentDiv.style.color = '#dc3545';
    }
}

// Message submit with fetch API
async function performMessageSubmit() {
    const username = document.getElementById('username').value.trim();
    const message = document.getElementById('message').value.trim();
    const responseDiv = document.getElementById('postResponse');
    const contentDiv = document.getElementById('postResponseContent');
    
    if (!username && !message) {
        alert('ユーザー名またはメッセージを入力してください');
        return;
    }
    
    const params = new URLSearchParams();
    params.append('username', username);
    params.append('message', message);
    
    // Show response display area
    responseDiv.style.display = 'block';
    contentDiv.innerHTML = 'メッセージを送信中...';
    contentDiv.style.color = '#4a7c4a';
    
    const startTime = Date.now();
    
    try {
        const response = await fetch('/uploads', {
            method: 'POST',
            headers: {
                'Content-Type': 'application/x-www-form-urlencoded'
            },
            body: params.toString()
        });
        
        const endTime = Date.now();
        const responseTime = endTime - startTime;
        
        const responseText = await response.text();
        
        let responseInfo = '';
        responseInfo += '=== POSTリクエスト結果（メッセージ送信） ===\n';
        responseInfo += 'URL: /uploads\n';
        responseInfo += 'メソッド: POST\n';
        responseInfo += 'Content-Type: application/x-www-form-urlencoded\n';
        responseInfo += 'レスポンス時間: ' + responseTime + 'ms\n\n';
        
        // Message info
        responseInfo += '=== メッセージ情報 ===\n';
        responseInfo += 'ユーザー名: ' + (username || '(空)') + '\n';
        responseInfo += 'メッセージ: ' + (message || '(空)') + '\n\n';
        
        // Status info
        responseInfo += '=== ステータス ===\n';
        responseInfo += 'ステータスコード: ' + response.status + '\n';
        responseInfo += 'ステータステキスト: ' + response.statusText + '\n';
        responseInfo += 'OK: ' + response.ok + '\n\n';
        
        if (response.ok) {
            responseInfo += '=== レスポンス内容 ===\n';
            responseInfo += responseText + '\n\n';
            contentDiv.style.color = '#4a7c4a';
            // Clear form on successful submit
            document.getElementById('username').value = '';
            document.getElementById('message').value = '';
        } else {
            responseInfo += '=== エラー内容 ===\n';
            responseInfo += responseText + '\n\n';
            contentDiv.style.color = '#dc3545';
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
        
        contentDiv.innerHTML = responseInfo;
    } catch (error) {
        const endTime = Date.now();
        const responseTime = endTime - startTime;
        
        let errorInfo = '';
        errorInfo += '=== POSTリクエストエラー（メッセージ送信） ===\n';
        errorInfo += 'URL: /uploads\n';
        errorInfo += 'メソッド: POST\n';
        errorInfo += 'エラー時間: ' + responseTime + 'ms\n\n';
        errorInfo += 'エラー内容: ' + error.message;
        
        contentDiv.innerHTML = errorInfo;
        contentDiv.style.color = '#dc3545';
    }
}

// File upload to no permission directory
async function performFileUploadToNoPermDir() {
    const fileInput = document.getElementById('file');
    const responseDiv = document.getElementById('postResponse');
    const contentDiv = document.getElementById('postResponseContent');
    
    if (!fileInput.files.length) {
        alert('ファイルを選択してください');
        return;
    }
    
    const file = fileInput.files[0];
    const formData = new FormData();
    formData.append('uploadFile', file);
    
    // Show response display area
    responseDiv.style.display = 'block';
    contentDiv.innerHTML = `ファイル "${file.name}" を権限なしディレクトリにアップロード中...`;
    contentDiv.style.color = '#4a7c4a';
    
    const startTime = Date.now();
    
    try {
        const response = await fetch('/no_perm_uploads', {
            method: 'POST',
            body: formData
        });
        
        const endTime = Date.now();
        const responseTime = endTime - startTime;
        
        const responseText = await response.text();
        
        let responseInfo = '';
        responseInfo += '=== POSTリクエスト結果（権限なしディレクトリ） ===\n';
        responseInfo += 'URL: /no_perm_uploads/\n';
        responseInfo += 'メソッド: POST\n';
        responseInfo += 'Content-Type: multipart/form-data\n';
        responseInfo += 'レスポンス時間: ' + responseTime + 'ms\n\n';
        
        // Upload info
        responseInfo += '=== アップロード情報 ===\n';
        responseInfo += 'ファイル名: ' + file.name + '\n';
        responseInfo += 'ファイルサイズ: ' + file.size + ' bytes\n';
        responseInfo += 'ファイルタイプ: ' + (file.type || 'unknown') + '\n\n';
        
        // Status info
        responseInfo += '=== ステータス ===\n';
        responseInfo += 'ステータスコード: ' + response.status + '\n';
        responseInfo += 'ステータステキスト: ' + response.statusText + '\n';
        responseInfo += 'OK: ' + response.ok + '\n\n';
        
        if (response.ok) {
            responseInfo += '=== レスポンス内容 ===\n';
            responseInfo += responseText + '\n\n';
            contentDiv.style.color = '#4a7c4a';
            // Clear file input on successful upload
            fileInput.value = '';
        } else {
            responseInfo += '=== エラー内容（権限エラーの可能性） ===\n';
            responseInfo += responseText + '\n\n';
            contentDiv.style.color = '#dc3545';
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
        
        contentDiv.innerHTML = responseInfo;
    } catch (error) {
        const endTime = Date.now();
        const responseTime = endTime - startTime;
        
        let errorInfo = '';
        errorInfo += '=== POSTリクエストエラー（権限なしディレクトリ） ===\n';
        errorInfo += 'URL: /no_perm_dir/\n';
        errorInfo += 'メソッド: POST\n';
        errorInfo += 'エラー時間: ' + responseTime + 'ms\n\n';
        errorInfo += 'エラー内容: ' + error.message;
        
        contentDiv.innerHTML = errorInfo;
        contentDiv.style.color = '#dc3545';
    }
}

// Delete file from no permission directory (file in no-perm directory)
function deleteNoPermFile() {
    const filepath = '/no_perm_dir/sample.html';
    if (confirm('本当にファイル "' + filepath + '" を削除しますか？（権限なしディレクトリ内のファイル）')) {
        const responseDiv = document.getElementById('deleteResponse');
        const contentDiv = document.getElementById('deleteResponseContent');
        
        // Show response display area
        responseDiv.style.display = 'block';
        contentDiv.innerHTML = 'DELETEリクエスト送信中...\nファイル: ' + filepath;
        contentDiv.style.color = '#4a7c4a';

        const startTime = Date.now();
        
        fetch(filepath, {
            method: 'DELETE'
        })
        .then(async response => {
            const endTime = Date.now();
            const responseTime = endTime - startTime;
            
            const responseText = await response.text();
            
            let responseInfo = '';
            responseInfo += '=== DELETEリクエスト結果（権限なしディレクトリ） ===\n';
            responseInfo += 'URL: ' + filepath + '\n';
            responseInfo += 'メソッド: DELETE\n';
            responseInfo += 'レスポンス時間: ' + responseTime + 'ms\n\n';
            
            // Status info
            responseInfo += '=== ステータス ===\n';
            responseInfo += 'ステータスコード: ' + response.status + '\n';
            responseInfo += 'ステータステキスト: ' + response.statusText + '\n';
            responseInfo += 'OK: ' + response.ok + '\n\n';
            
            if (response.ok) {
                responseInfo += '=== レスポンス内容 ===\n';
                responseInfo += responseText + '\n\n';
                responseInfo += '✅ ファイルが正常に削除されました\n\n';
                contentDiv.style.color = '#4a7c4a';
            } else {
                responseInfo += '=== エラー内容（権限エラーの可能性） ===\n';
                responseInfo += responseText + '\n\n';
                if (response.status === 403) {
                    responseInfo += '⚠️ 403 Forbidden: ファイル削除の権限がありません\n\n';
                } else if (response.status === 404) {
                    responseInfo += '⚠️ 404 Not Found: ファイルが見つかりません\n\n';
                } else {
                    responseInfo += '❌ 削除に失敗しました\n\n';
                }
                contentDiv.style.color = '#dc3545';
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
            
            contentDiv.innerHTML = responseInfo;
        })
        .catch(error => {
            const endTime = Date.now();
            const responseTime = endTime - startTime;
            
            let errorInfo = '';
            errorInfo += '=== DELETEリクエストエラー（権限なしディレクトリ） ===\n';
            errorInfo += 'URL: ' + filepath + '\n';
            errorInfo += 'メソッド: DELETE\n';
            errorInfo += 'エラー時間: ' + responseTime + 'ms\n\n';
            errorInfo += 'エラー内容: ' + error.message;
            
            contentDiv.innerHTML = errorInfo;
            contentDiv.style.color = '#dc3545';
        });
    }
}

// Delete file with no permission (file itself has no permission)
function deleteNoPermFileItself() {
    const filepath = '/no_perm.html';
    if (confirm('本当にファイル "' + filepath + '" を削除しますか？（権限なしファイル）')) {
        const responseDiv = document.getElementById('deleteResponse');
        const contentDiv = document.getElementById('deleteResponseContent');
        
        // Show response display area
        responseDiv.style.display = 'block';
        contentDiv.innerHTML = 'DELETEリクエスト送信中...\nファイル: ' + filepath;
        contentDiv.style.color = '#4a7c4a';

        const startTime = Date.now();
        
        fetch(filepath, {
            method: 'DELETE'
        })
        .then(async response => {
            const endTime = Date.now();
            const responseTime = endTime - startTime;
            
            const responseText = await response.text();
            
            let responseInfo = '';
            responseInfo += '=== DELETEリクエスト結果（権限なしファイル自体） ===\n';
            responseInfo += 'URL: ' + filepath + '\n';
            responseInfo += 'メソッド: DELETE\n';
            responseInfo += 'レスポンス時間: ' + responseTime + 'ms\n\n';
            
            // Status info
            responseInfo += '=== ステータス ===\n';
            responseInfo += 'ステータスコード: ' + response.status + '\n';
            responseInfo += 'ステータステキスト: ' + response.statusText + '\n';
            responseInfo += 'OK: ' + response.ok + '\n\n';
            
            if (response.ok) {
                responseInfo += '=== レスポンス内容 ===\n';
                responseInfo += responseText + '\n\n';
                responseInfo += '✅ ファイルが正常に削除されました\n\n';
                contentDiv.style.color = '#4a7c4a';
            } else {
                responseInfo += '=== エラー内容（権限エラーの可能性） ===\n';
                responseInfo += responseText + '\n\n';
                if (response.status === 403) {
                    responseInfo += '⚠️ 403 Forbidden: ファイル削除の権限がありません\n\n';
                } else if (response.status === 404) {
                    responseInfo += '⚠️ 404 Not Found: ファイルが見つかりません\n\n';
                } else {
                    responseInfo += '❌ 削除に失敗しました\n\n';
                }
                contentDiv.style.color = '#dc3545';
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
            
            contentDiv.innerHTML = responseInfo;
        })
        .catch(error => {
            const endTime = Date.now();
            const responseTime = endTime - startTime;
            
            let errorInfo = '';
            errorInfo += '=== DELETEリクエストエラー（権限なしファイル自体） ===\n';
            errorInfo += 'URL: ' + filepath + '\n';
            errorInfo += 'メソッド: DELETE\n';
            errorInfo += 'エラー時間: ' + responseTime + 'ms\n\n';
            errorInfo += 'エラー内容: ' + error.message;
            
            contentDiv.innerHTML = errorInfo;
            contentDiv.style.color = '#dc3545';
        });
    }
}
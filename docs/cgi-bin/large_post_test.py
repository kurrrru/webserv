#!/usr/bin/env python3

import sys
import os

def main():
    # CGI header
    print("Content-Type: text/html")
    print()
    
    # POSTデータのサイズを取得
    content_length = os.environ.get('CONTENT_LENGTH', '0')
    try:
        content_length = int(content_length)
    except ValueError:
        content_length = 0
    
    # POSTデータを読み取り
    if content_length > 0:
        post_data = sys.stdin.read(content_length)
    else:
        post_data = ""
    
    # レスポンス生成
    print("<html><body>")
    print("<h1>POST Data Test</h1>")
    print(f"<p>Content-Length: {content_length} bytes</p>")
    print(f"<p>Received data length: {len(post_data)} bytes</p>")
    
    if content_length > 0:
        # データの最初と最後の100文字を表示
        if len(post_data) > 200:
            start = post_data[:100]
            end = post_data[-100:]
            print(f"<p>First 100 chars: {start}</p>")
            print(f"<p>Last 100 chars: {end}</p>")
        else:
            print(f"<p>Full data: {post_data}</p>")
        
        # データ整合性チェック（期待されるパターンがあれば）
        if post_data.startswith("TEST_DATA_"):
            print("<p>✅ Data integrity check: PASSED</p>")
        else:
            print("<p>❌ Data integrity check: FAILED</p>")
    else:
        print("<p>No POST data received</p>")
    
    print("</body></html>")

if __name__ == "__main__":
    main()
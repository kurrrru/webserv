#!/usr/bin/env python3

import os
import time

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
    
    print("<html><body>")
    print("<h1>Write Timeout Test</h1>")
    print(f"<p>Content-Length: {content_length} bytes</p>")
    
    if content_length > 0:
        print("<p>CGI script will NOT read from stdin to force write timeout...</p>")
        print("<p>Server should timeout after 30 seconds when pipe buffer fills up.</p>")
        
        # 意図的にstdinから読み取らずに長時間待機
        # 大容量データ (1MB+) でパイプバッファが満杯になり、
        # "cat /proc/sys/fs/pipe-max-size"
        # サーバー側のwriteがブロックされてタイムアウト発生
        print("<p>Ignoring stdin completely and sleeping for 40 seconds...</p>")
        
        # stdinを全く読まずに待機
        time.sleep(60)  # 30秒のタイムアウトより長く待機
        
        print("<p>This message should not appear if write timeout works correctly</p>")
    else:
        print("<p>No POST data to test</p>")
    
    print("</body></html>")

if __name__ == "__main__":
    main()
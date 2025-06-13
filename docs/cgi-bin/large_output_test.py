#!/usr/bin/env python3

import sys
import os

def main():
    # 64KB (65536 bytes)
    data_size = 64 * 1024
    
    # Calculate exact content length
    html_start = "<html><body>"
    h1_tag = "<h1>Large Output Test</h1>"
    p1_tag = f"<p>Generating {data_size} bytes of test data...</p>"
    pre_start = "<pre>"
    prefix = "DATA_OUTPUT_"
    remaining = data_size - len(prefix)
    data_content = prefix + 'X' * remaining
    pre_end = "</pre>"
    p2_tag = f"<p>Output completed. Total data size: {data_size} bytes</p>"
    html_end = "</body></html>"
    
    total_content = html_start + h1_tag + p1_tag + pre_start + data_content + pre_end + p2_tag + html_end
    content_length = len(total_content)
    
    print("Content-Type: text/html")
    print(f"Content-Length: {content_length}")
    print()
    
    sys.stdout.write(html_start)
    sys.stdout.write(h1_tag)
    sys.stdout.write(p1_tag)
    sys.stdout.write(pre_start)
    sys.stdout.write(prefix)
    
    chunk_size = 1024
    full_chunks = remaining // chunk_size
    final_chunk = remaining % chunk_size
    
    chunk_data = 'X' * chunk_size
    for i in range(full_chunks):
        sys.stdout.write(chunk_data)
        sys.stdout.flush()

    if final_chunk > 0:
        sys.stdout.write('X' * final_chunk)
        sys.stdout.flush()
    
    sys.stdout.write(pre_end)
    sys.stdout.write(p2_tag)
    sys.stdout.write(html_end)
    sys.stdout.flush()

if __name__ == "__main__":
    main()
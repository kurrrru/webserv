#!/usr/bin/env python3

import sys
import os

def main():
    # 64KB (65536 bytes)
    data_size = 64 * 1024
    
    print("Content-Type: text/html")
    print(f"Content-Length: {data_size + 200}")
    print()
    
    sys.stdout.write("<html><body>")
    sys.stdout.write("<h1>Large Output Test</h1>")
    sys.stdout.write(f"<p>Generating {data_size} bytes of test data...</p>")
    sys.stdout.write("<pre>")
    
    prefix = "DATA_OUTPUT_"
    remaining = data_size - len(prefix)
    
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
    
    sys.stdout.write("</pre>")
    sys.stdout.write(f"<p>Output completed. Total data size: {data_size} bytes</p>")
    sys.stdout.write("</body></html>")
    sys.stdout.flush()

if __name__ == "__main__":
    main()
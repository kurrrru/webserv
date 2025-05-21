#!/usr/bin/env python3

import sys
import os
import datetime
import urllib.parse
import re

def sendResponse(status, headers=None):
    """
    Send HTTP response with status code and optional headers
    Args:
        status: HTTP status code (e.g., 200, 404, 500)
        headers: Dictionary of HTTP headers to include in response
    """
    print(f"Status: {status}\r\n")
    if headers:
        for key, value in headers.items():
            print(f"{key}: {value}\r\n")
    print("\r\n")


def sanitize_filename(filename):
    """
    Sanitize filename to prevent security issues
    - Remove path components
    - Replace special characters with underscore
    - Ensure filename is safe for filesystem
    """
    filename = os.path.basename(filename)
    filename = re.sub(r'[^\w\-\.]', '_', filename)
    return filename

def processFile(filename):
    """
    Process file upload request
    - Validate upload directory
    - Handle duplicate filenames
    - Write file content
    - Return appropriate status and headers
    """
    try:
        safeFilename = sanitize_filename(filename)
        uploadDir = os.getenv("UPLOAD_DIR")

        if not os.path.exists(uploadDir):
            return 404, {"Content-Type": "text/plain"}

        if not os.access(uploadDir, os.W_OK):
            return 403, {"Content-Type": "text/plain"}

        fullPath = os.path.join(uploadDir, safeFilename)
        if os.path.exists(fullPath):
            safeFilename = safeFilename + "_" + datetime.datetime.now().strftime('%Y%m%d%H%M%S')
            fullPath = os.path.join(uploadDir, safeFilename)

        contentLength = int(os.environ.get("CONTENT_LENGTH", 0))
        file = open(fullPath, "wb")
        file.write(sys.stdin.buffer.read(contentLength))
        file.close()

        location = f"/uploads/{safeFilename}"
        return 201, {"Location": location, "Content-Type": "text/plain"}
    except Exception as e:
        return 500, {"Content-Type": "text/plain"}

def getRequest():
    """
    Handle GET request
    Returns current timestamp in Japanese format
    """
    body = datetime.datetime.now().strftime('%Y年%m月%d日 %H:%M:%S')
    sendResponse(200, {"Content-Type": "text/plain"})

def postRequest():
    """
    Handle POST request
    - Parse query parameters for filename
    - Process file upload
    - Return appropriate response
    """
    query = os.environ.get("QUERY_STRING")
    queryDict = urllib.parse.parse_qs(query)

    status, headers, body = processFile(queryDict.get("filename", ["cgiPost"])[0])
    sendResponse(status, headers)

def otherRequest():
    """
    Handle unsupported HTTP methods
    Returns 501 Not Implemented
    """
    sendResponse(501, {"Content-Type": "text/plain"})

def main():
    """
    Main entry point
    Routes request to appropriate handler based on HTTP method
    """
    requestMethod = os.environ.get("REQUEST_METHOD")

    if requestMethod == "GET":
        getRequest()
    elif requestMethod == "POST":
        postRequest()
    else:
        otherRequest()

if __name__ == "__main__":
    main()
#!/usr/bin/env python3

import sys
import os
import datetime
import urllib.parse
import re

#application/octet-stream
def sendResponse(status, headers=None, body=None):
    print(f"Status: {status}\r\n")
    if headers:
        for key, value in headers.items():
            print(f"{key}: {value}\r\n")
    print("\r\n")
    if body:
        print(body)

def sanitize_filename(filename):
    # extract filename from path
    filename = os.path.basename(filename)
    # replace special characters with underscore
    filename = re.sub(r'[^\w\-\.]', '_', filename)
    return filename

def processFile(filename):
    try:
        safeFilename = sanitize_filename(filename)
        uploadDir = os.getenv("UPLOAD_DIR")

        if not os.path.exists(uploadDir):
            return 404, {"Content-Type": "text/plain"}, 'Directory not found'

        if not os.access(uploadDir, os.W_OK):
            return 403, {"Content-Type": "text/plain"}, 'Directory permissions error'

        fullPath = os.path.join(uploadDir, safeFilename)
        if os.path.exists(fullPath):
            safeFilename = safeFilename + "_" + datetime.datetime.now().strftime('%Y%m%d%H%M%S')
            fullPath = os.path.join(uploadDir, safeFilename)

        contentLength = int(os.environ.get("CONTENT_LENGTH"))

        file = open(fullPath, "wb")
        file.write(sys.stdin.buffer.read(contentLength))
        file.close()

        location = f"/uploads/{safeFilename}"
        return 201, {"Location": location, "Content-Type": "text/plain"}, f'{safeFilename} created'
    except Exception as e:
        return 500, {"Content-Type": "text/plain"}, f'File creation failed: {e}'

def getRequest():
    body = datetime.datetime.now().strftime('%Y年%m月%d日 %H:%M:%S')
    sendResponse(200, {"Content-Type": "text/plain"}, body)

def postRequest():
    query = os.environ.get("QUERY_STRING")
    queryDict = urllib.parse.parse_qs(query)

    status, headers, body = processFile(queryDict.get("filename", ["cgiPost"])[0])
    sendResponse(status, headers, body)

def otherRequest():
    sendResponse(501, {"Content-Type": "text/plain"}, "Not implemented")

def main():
    requestMethod = os.environ.get("REQUEST_METHOD")

    if requestMethod == "GET":
        getRequest()
    elif requestMethod == "POST":
        postRequest()
    else:
        otherRequest()

if __name__ == "__main__":
    main()
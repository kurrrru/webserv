#!/usr/bin/env python3

import sys
import os
import datetime
import urllib.parse
import re
import cgi

def sendResponse(status, headers=None, body=None):
    """
    Send HTTP response with status code and optional headers
    Args:
        status: HTTP status code (e.g., 200, 404, 500)
        headers: Dictionary of HTTP headers to include in response
        body: Optional response body content to print
    """
    print(f"Status: {status}")
    if headers:
        for key, value in headers.items():
            print(f"{key}: {value}")
    print()
    if body:
        print(body)

def sanitizeFilename(filename):
    """
    Sanitize filename to prevent security issues
    - Remove path components
    - Replace special characters with underscore
    - Ensure filename is safe for filesystem
    """
    filename = os.path.basename(filename)
    filename = re.sub(r'[<>:"/\\|?*\x00-\x1f]', '_', filename)
    return filename

def validateFilename(filename):
    """
    Validate filename length and content
    - Check if filename is empty or too long
    - Check if filename contains invalid characters
    """
    if not filename or len(filename) > 255:
        return ""
    safeFilename = sanitizeFilename(filename)
    return safeFilename

def generateUniqueFullPath(uploadDir, filename):
    """
    Generate a unique full path for the given filename
    - Check if file exists and append timestamp if necessary
    """
    fullPath = os.path.join(uploadDir, filename)
    if os.path.exists(fullPath):
        name, ext = os.path.splitext(filename)
        timestamp = datetime.datetime.now().strftime('%Y-%m-%d_%H-%M-%S')
        filename = f"{name}_{timestamp}{ext}"
        fullPath = os.path.join(uploadDir, filename)
    return fullPath

def getUploadDirectory():
    """
    Get and validate upload directory
    Returns the upload directory path or None if invalid
    """
    uploadDir = os.getenv("UPLOAD_DIR")
    # 500 Internal Server Error
    if not uploadDir:
        return 500
    # 403 Forbidden
    if not os.path.exists(uploadDir) or not os.access(uploadDir, os.W_OK):
        return 403
    # 200 OK
    return uploadDir

def handleMultipartUpload(uploadDir):
    """
    Handle multipart file upload
    - Use cgi.FieldStorage to parse the form data
    - Check if file exists and handle duplicates
    - Read data from uploaded files
    - Write files to upload directory
    - Return success response with uploaded file names
    Args:
        uploadDir: Directory to save the uploaded files
    """
    try:
        form = cgi.FieldStorage()
        uploaded_files = []

        for field in form:
            if hasattr(form[field], 'filename') and form[field].filename:
                filename = form[field].filename

                safeFilename = validateFilename(filename)
                if not safeFilename:
                    sendResponse(400, {"Content-Type": "text/plain"}, f"Invalid filename: {filename}")
                    return

                fullPath = generateUniqueFullPath(uploadDir, safeFilename)
                with open(fullPath, 'wb') as f:
                    f.write(form[field].file.read())
                uploaded_files.append(safeFilename)

        if uploaded_files:
            sendResponse(201, {"Content-Type": "text/plain"}, f"Files uploaded: {', '.join(uploaded_files)}")
        else:
            sendResponse(400, {"Content-Type": "text/plain"}, "No files uploaded")
    except Exception as e:
        sendResponse(500, {"Content-Type": "text/plain"}, f"Error: {str(e)}")

def handleRawUpload(uploadDir):
    """
    Handle raw file upload
    - Get filename from query string
    - Check if file exists and handle duplicates
    - Read data from stdin using CONTENT_LENGTH
    - Write to file
    - Return location in response
    Args:
        uploadDir: Directory to save the uploaded file
    """
    try:
        MAX_FILE_SIZE = 10 * 1024 * 1024  # 10MB
        contentLength = int(os.getenv("CONTENT_LENGTH") or 0)

        if contentLength > MAX_FILE_SIZE:
            sendResponse(413, {"Content-Type": "text/plain"}, f"File too large.")
            return

        if contentLength <= 0:
            sendResponse(400, {"Content-Type": "text/plain"}, "No content to upload")
            return

        query = os.getenv("QUERY_STRING")
        queryDict = urllib.parse.parse_qs(query or "")
        filename = queryDict.get("filename", ["uploaded_file"])[0]

        safeFilename = validateFilename(filename)
        if not safeFilename:
            sendResponse(400, {"Content-Type": "text/plain"}, "Invalid filename")
            return

        fullPath = generateUniqueFullPath(uploadDir, safeFilename)
        rawData = sys.stdin.buffer.read(contentLength)

        with open(fullPath, 'wb') as f:
            f.write(rawData)

        location = f"/uploads/{safeFilename}"
        sendResponse(201, {"Location": location, "Content-Type": "text/plain"}, f"File uploaded: {location}")
    except Exception as e:
        sendResponse(500, {"Content-Type": "text/plain"}, f"Error: {str(e)}")

def handleGetRequest():
    """
    Handle GET request
    Returns current timestamp in Japanese format
    """
    body = datetime.datetime.now().strftime('%Y年%m月%d日 %H時%M分%S秒')
    sendResponse(200, {"Content-Type": "text/plain"}, body)

def handlePostRequest():
    """
    Handle POST request
    - Get upload directory and validate it
    - Check Content-Type to determine upload method
    - Route to multipart or raw upload handler
    """
    uploadDir = getUploadDirectory()
    if uploadDir == 500:
        return sendResponse(500, {"Content-Type": "text/plain"}, "Internal Server Error")
    elif uploadDir == 403:
        return sendResponse(403, {"Content-Type": "text/plain"}, "Forbidden")
    contentType = os.getenv("CONTENT_TYPE")
    if contentType and "multipart/form-data" in contentType.lower():
        handleMultipartUpload(uploadDir)
    else:
        handleRawUpload(uploadDir)

def handleOtherRequest():
    """
    Handle unsupported HTTP methods
    Returns 501 Not Implemented
    """
    sendResponse(501, {"Content-Type": "text/plain"}, "Not Implemented")

def main():
    """
    Main entry point
    Routes request to appropriate handler based on HTTP method
    """
    requestMethod = os.getenv("REQUEST_METHOD")

    if requestMethod == "GET":
        handleGetRequest()
    elif requestMethod == "POST":
        handlePostRequest()
    else:
        handleOtherRequest()

if __name__ == "__main__":
    main()

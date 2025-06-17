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

def handleMultipartUpload(uploadDir, chunked_data=None):
    """
    Handle multipart file upload
    - Use cgi.FieldStorage to parse the form data
    - Check if file exists and handle duplicates
    - Read data from uploaded files
    - Write files to upload directory
    - Return success response with uploaded file names
    Args:
        uploadDir: Directory to save the uploaded files
        chunked_data: Pre-read chunked data (if Transfer-Encoding: chunked)
    """
    try:
        if chunked_data is not None:
            # Create a file-like object from chunked data for cgi.FieldStorage
            import io
            form = cgi.FieldStorage(fp=io.BytesIO(chunked_data))
        else:
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

def handleRawUpload(uploadDir, chunked_data=None):
    """
    Handle raw file upload
    - Get filename from query string
    - Check if file exists and handle duplicates
    - Read data from stdin using CONTENT_LENGTH or use chunked_data
    - Write to file
    Args:
        uploadDir: Directory to save the uploaded file
        chunked_data: Pre-read chunked data (if Transfer-Encoding: chunked)
    """
    try:
        MAX_FILE_SIZE = 10 * 1024 * 1024  # 10MB

        if chunked_data is not None:
            # Use pre-read chunked data
            rawData = chunked_data
            if len(rawData) > MAX_FILE_SIZE:
                sendResponse(413, {"Content-Type": "text/plain"}, f"File too large.")
                return
        else:
            # Use CONTENT_LENGTH to read data
            contentLength = int(os.getenv("CONTENT_LENGTH") or 0)

            if contentLength > MAX_FILE_SIZE:
                sendResponse(413, {"Content-Type": "text/plain"}, f"File too large.")
                return

            if contentLength <= 0:
                sendResponse(400, {"Content-Type": "text/plain"}, "No content to upload")
                return

            rawData = sys.stdin.buffer.read(contentLength)

        query = os.getenv("QUERY_STRING")
        queryDict = urllib.parse.parse_qs(query or "")
        filename = queryDict.get("filename", ["uploaded_file"])[0]

        safeFilename = validateFilename(filename)
        if not safeFilename:
            sendResponse(400, {"Content-Type": "text/plain"}, "Invalid filename")
            return

        fullPath = generateUniqueFullPath(uploadDir, safeFilename)

        with open(fullPath, 'wb') as f:
            f.write(rawData)

        sendResponse(201, {"Content-Type": "text/plain"}, f"File uploaded")
    except Exception as e:
        sendResponse(500, {"Content-Type": "text/plain"}, f"Error: {str(e)}")

def solveChunkedTransferEncoding(max_size=10*1024*1024):
    """
    Chunk format:
    chunk = chunk-size [ chunk-extension ] CRLF chunk-data CRLF
    last-chunk = "0" CRLF CRLF

    """
    chunks = []
    total_size = 0

    try:
        while True:
            # Read chunk-size line
            size_line = sys.stdin.buffer.readline()
            if not size_line:
                # Unexpected end of stream
                return b''

            # Remove CRLF and decode to ASCII
            size_line = size_line.rstrip(b'\r\n').decode('ascii', errors='replace')
            if not size_line:
                return b''

            # Parse chunk-size, ignore chunk-extension after semicolon
            # Example: "1a" or "1a;charset=utf-8" -> extract "1a"
            size_parts = size_line.split(';', 1)
            size_str = size_parts[0].strip()

            # Validate hexadecimal format
            if not re.match(r'^[0-9A-Fa-f]+$', size_str):
                # Invalid chunk size format
                return b''

            try:
                chunk_size = int(size_str, 16)
            except ValueError:
                # Failed to parse hex number
                return b''

            # Negative size check (shouldn't happen with hex, but safety)
            if chunk_size < 0:
                return b''

            # Check total size limit
            if total_size + chunk_size > max_size:
                # Content too large
                return b''

            # Last chunk (size 0)
            if chunk_size == 0:
                # Read final CRLF after last chunk
                final_crlf = sys.stdin.buffer.readline()
                if final_crlf != b'\r\n':
                    return b''
                break

            # Read chunk data
            chunk_data = sys.stdin.buffer.read(chunk_size)
            if len(chunk_data) != chunk_size:
                # Incomplete chunk data
                return b''

            chunks.append(chunk_data)
            total_size += chunk_size
            # Read trailing CRLF after chunk data
            trailing_crlf = sys.stdin.buffer.read(2)
            if trailing_crlf != b'\r\n':
                # Missing CRLF after chunk data
                return b''

        return b''.join(chunks)

    except Exception:
        return b''

def handleGetRequest():
    """
    Handle GET request
    Returns current timestamp in Japanese format
    """
    body = datetime.datetime.now().strftime('%Y年%m月%d日 %H時%M分%S秒')
    sendResponse(200, {"Content-Type": "text/plain; charset=utf-8"}, body)

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

    transferEncoding = os.getenv("HTTP_TRANSFER_ENCODING")
    chunked_data = None
    if transferEncoding and transferEncoding.lower() == "chunked":
        chunked_data = solveChunkedTransferEncoding()

    contentType = os.getenv("CONTENT_TYPE")
    if contentType and "multipart/form-data" in contentType.lower():
        handleMultipartUpload(uploadDir, chunked_data)
    else:
        handleRawUpload(uploadDir, chunked_data)

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

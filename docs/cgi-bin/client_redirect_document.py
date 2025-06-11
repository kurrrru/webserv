#!/usr/bin/env python3

# CLIENT_REDIRECT_DOCUMENT type
# Location with absolute URI, Content-Type, body, and redirect status
print("Status: 302 Found")
print("Location: http://www.example.com/")
print("Content-Type: text/html")
print()
print("<html>")
print("<head><title>Redirect</title></head>")
print("<body>")
print("<h1>Page Moved</h1>")
print("<p>This page has moved to <a href='http://www.example.com/'>http://www.example.com/</a></p>")
print("</body>")
print("</html>")
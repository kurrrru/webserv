#!/usr/bin/env python3
import os

print("Content-Type: text/plain\n")
print("Environment Variables:")
print("=" * 50)

# CGI specific environment variables
cgi_vars = [
    "AUTH_TYPE",
    "CONTENT_LENGTH",
    "CONTENT_TYPE",
    "GATEWAY_INTERFACE",
    "PATH_INFO",
    "PATH_TRANSLATED",
    "QUERY_STRING",
    "REMOTE_ADDR",
    "REMOTE_HOST",
    "REMOTE_IDENT",
    "REMOTE_USER",
    "REQUEST_METHOD",
    "SCRIPT_NAME",
    "SERVER_NAME",
    "SERVER_PORT",
    "SERVER_PROTOCOL",
    "SERVER_SOFTWARE"
]

print("\nCGI Variables:")
for var in cgi_vars:
    value = os.environ.get(var, "(not set)")
    print(f"{var}: {value}")

print("\nHTTP Headers (as environment variables):")
for key, value in os.environ.items():
    if key.startswith("HTTP_"):
        print(f"{key}: {value}")

print("\nAll Environment Variables:")
for key, value in sorted(os.environ.items()):
    print(f"{key}: {value}")
[Japanese version](README.ja.md)

# webserv

## Overview

`webserv` is a high-performance HTTP/1.1 web server implemented in C++98. This project utilizes a non-blocking, event-driven architecture with `epoll` to handle a large number of concurrent connections efficiently. The server is highly customizable through a flexible, Nginx-style configuration file, supporting virtual servers, location-specific routing, static content delivery, and dynamic content generation via the Common Gateway Interface (CGI).

---

## Technology Stack

* **Language**: C++98
* **Build System**: `make`
* **I/O Model**: Non-blocking I/O with `epoll` (Linux)
* **Protocols**: HTTP/1.1

---

## Key Features

* **Flexible Configuration**: Customize server behavior using an Nginx-like configuration file, including settings for multiple virtual servers.
* **Virtual Servers**: Host multiple domains or IP/port combinations from a single server instance.
* **Location-Based Routing**: Apply different rules and configurations for specific URL paths (locations).
* **HTTP/1.1 Methods**: Full support for `GET`, `HEAD`, `POST`, and `DELETE` requests.
* **Static File Serving**: Efficiently serves static files like HTML, CSS, images, and more.
* **CGI Execution**: Executes CGI scripts (e.g., Python, Bash) to generate dynamic web pages. The server correctly sets META variables and handles both `GET` and `POST` data streams.
* **File Uploads**: Manages file uploads via `POST` requests, with configurable body size limits and storage locations.
* **Directory Listing**: Automatically generates and displays a directory listing page if `autoindex` is enabled and an index file is not found.
* **Custom Error Pages**: Define custom HTML pages for specific HTTP error codes (e.g., 404, 500).
* **HTTP Redirection**: Supports permanent and temporary redirections using the `return` directive.

---

## Project Structure

The repository is organized as follows:

```
.
├── conf/                # Configuration files
│   ├── default.conf
│   └── ...
├── docs/                # Document root for web content
│   ├── cgi-bin/         # Example CGI scripts
│   └── html/            # Example HTML, CSS, and JS files
├── src/                 # Source code
│   ├── config/          # Configuration file parser and validation
│   ├── core/            # Main server logic, client handling
│   ├── event/           # Epoll event loop management
│   └── http/            # HTTP request/response parsing and handling
│       ├── cgi/
│       ├── parsing/
│       ├── request/
│       └── response/
├── toolbox/             # Utility functions
├── Makefile
└── webserv              # The compiled server executable
```

---

## Build and Run

### Prerequisites

* A C++98 compliant compiler (e.g., `c++`)
* `make`

### Build

To compile the project, run the following command from the root directory:

```bash
make
```

### Run

The server requires a configuration file as an argument to start.

```bash
./webserv [path/to/your_config_file.conf]
```

If no configuration file is provided, it will attempt to use `conf/default.conf`.

```bash
./webserv
```

---

## Configuration Directives

The server's behavior is controlled by directives in the configuration file. Below are some of the core directives available.

| Directive              | Context(s)                  | Description                                            | Example                               |
| ---------------------- | --------------------------- | ------------------------------------------------------ | ------------------------------------- |
| `listen`               | `server`                    | Specifies the port and optional IP address to listen on. | `listen 8080;`                        |
| `server_name`          | `server`                    | Defines the virtual server's name(s).                  | `server_name example.com www.example.com;` |
| `root`                 | `http`, `server`, `location`| Sets the root directory for requests.                  | `root /var/www/html;`                 |
| `index`                | `http`, `server`, `location`| Specifies the default file to serve.                   | `index index.html index.htm;`         |
| `allowed_methods`      | `http`, `server`, `location`| Restricts which HTTP methods are allowed.              | `allowed_methods GET POST;`           |
| `client_max_body_size` | `http`, `server`, `location`| Sets the maximum allowed size of the client request body.| `client_max_body_size 8M;`            |
| `error_page`           | `http`, `server`, `location`| Defines a custom page for a given error code.          | `error_page 404 /404.html;`           |
| `autoindex`            | `http`, `server`, `location`| Enables or disables directory listing.                 | `autoindex on;`                       |
| `cgi_path`             | `http`, `server`, `location`| Specifies the path to a CGI interpreter.               | `cgi_path /usr/bin/python3;`          |
| `cgi_extension`        | `http`, `server`, `location`| Associates a file extension with a CGI script.         | `cgi_extension .py;`                  |
| `upload_store`         | `http`, `server`, `location`| Defines the directory where uploaded files are stored.  | `upload_store /var/uploads;`          |
| `return`               | `server`, `location`        | Performs an HTTP redirection.                          | `return 301 http://new.example.com;`  |

---

## Usage Examples

Here are some `curl` commands to test the server's functionality (assuming the server is running on `localhost:8080` with the default configuration).

**Get the main page:**
```bash
curl http://localhost:8080/
```

**Test a CGI script (GET):**
```bash
curl http://localhost:8080/cgi-bin/script.py
```

**Test a CGI script (POST):**
```bash
curl -X POST -d "name=Gemini" http://localhost:8080/cgi-bin/script.py
```

**Upload a file:**
```bash
# First, create a dummy file
echo "This is a test file." > test.txt
# Then, upload it
curl -X POST --data-binary "@test.txt" http://localhost:8080/uploads/
```

**Delete an uploaded file:**
```bash
curl -X DELETE http://localhost:8080/upload/test.txt
```

---

## License

This project is licensed under the MIT License.

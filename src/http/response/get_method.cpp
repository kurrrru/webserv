#include <sys/stat.h>

#include <string>
#include <iostream>

bool isDirectory(const std::string& path) {
    struct stat statbuf;
    if (stat(path.c_str(), &statbuf) != 0) {
        return false;
    }
    return S_ISDIR(statbuf.st_mode);
}

bool isRegularFile(const std::string& path) {
    struct stat statbuf;
    if (stat(path.c_str(), &statbuf) != 0) {
        return false;
    }
    return S_ISREG(statbuf.st_mode);
}

void runGet(const std::string& path, std::string& responseBody,
    const std::string& indexPath, bool isAutoindex) {
    if (isDirectory(path)) {
        if (!indexPath.empty()) {
            
        }
    } else {

    }
}

int main(void) {
    std::string path;
    std::string responseBody;
    std::string indexPath;

    path = "/home/yooshima/42cursus/webserv/docs/html";
    indexPath = "index.html";
    runGet(path, responseBody, indexPath, true);
}
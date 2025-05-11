// Copyright 2025 Ideal Broccoli

#pragma once

#include <string>
#include <fstream>

namespace toolbox {

namespace logger {

class AccessLog {
 public:
    static void setLogFile(const std::string& file);
    static void log(
        const std::string& remote_addr,
        const std::string& remote_user,
        const std::string& request,
        int status,
        int body_bytes_sent,
        const std::string& http_referer,
        const std::string& http_user_agent);

 private:
    std::string _logFileName;
    std::ofstream _logFile;

    static AccessLog& getInstance();

    AccessLog();
    ~AccessLog();
    AccessLog(const AccessLog&);
    AccessLog& operator=(const AccessLog&);
    std::string getTimeStamp();
};

}  // namespace logger

}  // namespace toolbox

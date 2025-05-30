// Copyright 2025 Ideal Broccoli

/*
AccessLog class does not throw exceptions
because the log is not critical to the program's execution and
throwing exceptions would require additional error handling
which is not needed in this case.
The class is designed to be simple and lightweight,
allowing for easy logging without the overhead of exception handling.
The class uses a singleton pattern to ensure that only one instance of the logger exists.
The class is not thread-safe, so it should not be used in a multi-threaded environment.
*/

#include "access.hpp"

#include <iostream>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <string>

namespace {
    const std::size_t IO_BUFFER_SIZE = 16 * 1024; // 16 KB
}

namespace toolbox {

/*
 * @brief AccessLog constructor.
 * @note The reason for not opening the log file in the constructor is to allow
 *       the user to set the log file name before opening it.
 */
logger::AccessLog::AccessLog() : _logFileName("access.log") {
}

logger::AccessLog::~AccessLog() {
    if (_logFile.is_open()) {
        _logFile.close();
    }
}

void logger::AccessLog::setLogFile(const std::string& file) {
    logger::AccessLog& instance = getInstance();
    if (instance._logFile.is_open()) {
        instance._logFile.close();
    }
    instance._logFileName = file;
    instance.openLogFile();
}

logger::AccessLog& logger::AccessLog::getInstance() {
    static AccessLog instance;
    return instance;
}

/*
 * @brief Logs an access request to the log file.
 * @param remote_addr The remote address of the client.
 * @param remote_user The remote user (if any).
 * @param request The HTTP request line.
 * @param status The HTTP status code.
 * @param body_bytes_sent The size of the response body in bytes.
 * @param http_referer The HTTP referer header.
 * @param http_user_agent The HTTP user agent header.
 * @note This function will create the log file if it does not exist.
 */
void logger::AccessLog::log(
    const std::string& remote_addr,
    const std::string& remote_user,
    const std::string& request,
    int status,
    int body_bytes_sent,
    const std::string& http_referer,
    const std::string& http_user_agent
) {
    logger::AccessLog& instance = getInstance();
    if (!instance._logFile.is_open()) {
        instance.openLogFile();
    }
    if (instance._logFile.is_open()) {
        instance._logFile << remote_addr << " "
                << "- "
                << remote_user << " "
                << "[" << instance.getTimeStamp() << "] "
                << "\"" << request << "\" "
                << status << " "
                << body_bytes_sent << " "
                << "\"" << http_referer << "\" "
                << "\"" << http_user_agent << "\""
                << std::endl;
    } else {
        std::cerr << "Error: Log file is not open." << std::endl;
    }
}

std::string logger::AccessLog::getTimeStamp() {
    std::time_t now = std::time(NULL);
    std::tm* localTime = std::localtime(&now);
    char buffer[100];
    std::strftime(buffer, sizeof(buffer), "%d/%b/%Y:%H:%M:%S %z", localTime);
    return std::string(buffer);
}

void logger::AccessLog::openLogFile() {
    _logFile.open(_logFileName.c_str(), std::ios::app);
    if (!_logFile.is_open()) {
        std::cerr << "Error opening log file: " << _logFileName
                << ". Defaulting to access.log." << std::endl;
        _logFileName = "access.log";
        _logFile.open(_logFileName.c_str(), std::ios::app);
    }
    if (_logFile.is_open()) {
        _logFile << "[" << getTimeStamp() << "] "
                << "Log file opened: " << _logFileName
                << std::endl;
    }
}

}  // namespace toolbox

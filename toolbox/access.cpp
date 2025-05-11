// Copyright 2025 Ideal Broccoli

#include "access.hpp"

#include <iostream>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <string>

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
    instance._logFile.open(instance._logFileName.c_str(), std::ios::app);
    if (!instance._logFile.is_open()) {
        throw std::runtime_error("Failed to open log file: "
            + instance._logFileName);
    }
    instance._logFile << "[" << instance.getTimeStamp() << "] "
                      << "Log file opened: " << instance._logFileName
                      << std::endl;
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
 * @throws std::runtime_error if the log file cannot be opened.
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
        instance._logFile.open(instance._logFileName.c_str(), std::ios::app);
        if (!instance._logFile.is_open()) {
            throw std::runtime_error("Failed to open log file: "
                + instance._logFileName);
        }
        instance._logFile << "[" << instance.getTimeStamp() << "] "
                          << "Log file opened: " << instance._logFileName
                          << std::endl;
    }

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
}

std::string logger::AccessLog::getTimeStamp() {
    std::time_t now = std::time(nullptr);
    std::tm* tm_now = std::gmtime(&now);
    std::ostringstream oss;
    oss << std::put_time(tm_now, "%d/%b/%Y:%H:%M:%S %z");
    return oss.str();
}

}  // namespace toolbox

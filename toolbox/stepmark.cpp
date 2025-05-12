#include "stepmark.hpp"

#include <iostream>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <string>

namespace toolbox {

/*
 * @brief StepMark constructor.
 * @note The reason for not opening the log file in the constructor is to allow
 *       the user to set the log file name before opening it.
 */
logger::StepMark::StepMark()
    : _level(INFO), _logFileName("stepmark.log") {
}

logger::StepMark::~StepMark() {
    if (_logFile.is_open()) {
        _logFile.close();
    }
}

void logger::StepMark::setLevel(StepmarkLevel level) {
    logger::StepMark& instance = getInstance();
    instance._level = level;
}

void logger::StepMark::setLogFile(const std::string& file) {
    logger::StepMark& instance = getInstance();
    if (instance._logFile.is_open()) {
        instance._logFile.close();
    }
    instance._logFileName = file;
    instance._logFile.open(instance._logFileName.c_str(), std::ios::app);
    if (!instance._logFile.is_open()) {
        throw std::runtime_error("Failed to open log file: "
            + instance._logFileName);
    }
}

logger::StepMark& logger::StepMark::getInstance() {
    static StepMark instance;
    return instance;
}

/*
 * @brief Logs a message to the log file.
 * @param level The log level.
 * @param message The message to log.
 * @throws std::runtime_error if the log file cannot be opened.
 * @note This function will create the log file if it does not exist.
 */
void logger::StepMark::log(StepmarkLevel level, const std::string& message) {
    const char* levelStr[] = {
        "DEBUG",
        "INFO",
        "WARNING",
        "ERROR",
        "CRITICAL"
    };

    logger::StepMark& instance = getInstance();
    if (!instance._logFile.is_open()) {
        instance._logFile.open(instance._logFileName.c_str(), std::ios::app);
        if (!instance._logFile.is_open()) {
            throw std::runtime_error("Failed to open log file: "
                + instance._logFileName);
        }
    }
    if (level >= instance._level) {
        instance._logFile << instance.getTimeStamp()
            << " [" << levelStr[level] << "] " << message << std::endl;
    }
}

void logger::StepMark::debug(const std::string& message) {
    log(DEBUG, message);
}

void logger::StepMark::info(const std::string& message) {
    log(INFO, message);
}

void logger::StepMark::warning(const std::string& message) {
    log(WARNING, message);
}

void logger::StepMark::error(const std::string& message) {
    log(ERROR, message);
}

void logger::StepMark::critical(const std::string& message) {
    log(CRITICAL, message);
}

std::string logger::StepMark::getTimeStamp() {
    std::time_t now = std::time(NULL);
    std::tm* localTime = std::localtime(&now);
    char buffer[20];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", localTime);
    return std::string(buffer);
}

}  // namespace toolbox

#pragma once

#include <string>
#include <fstream>

namespace toolbox {

namespace logger {

enum StepmarkLevel {
    DEBUG = 0,
    INFO = 1,
    WARNING = 2,
    ERROR = 3,
    CRITICAL = 4
};

class StepMark {
 public:
    static void setLevel(StepmarkLevel level);
    static void setLogFile(const std::string& file);
    static void log(StepmarkLevel level, const std::string& message);

    static void debug(const std::string& message);
    static void info(const std::string& message);
    static void warning(const std::string& message);
    static void error(const std::string& message);
    static void critical(const std::string& message);

 private:
    StepmarkLevel _level;
    std::string _logFileName;
    std::ofstream _logFile;

    static StepMark& getInstance();

    StepMark();
    ~StepMark();
    StepMark(const StepMark&);
    StepMark& operator=(const StepMark&);
    std::string getTimeStamp();
};

}  // namespace logger

}  // namespace toolbox

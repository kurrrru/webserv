#pragma once

#include <string>
#include <map>
#include <vector>
#include <unistd.h>
#include <sys/types.h>
#include "../request/http_request.hpp"

namespace http {
/**
 * Handles the execution of CGI (Common Gateway Interface) scripts.
 * 
 * This class manages the complete lifecycle of CGI execution including:
 * - Environment variable setup according to CGI/1.1 specification
 * - Process forking and execution of CGI scripts
 * - Input/output handling through pipes
 * - Timeouts and error management
 * - Child process monitoring and cleanup
 * 
 * It supports both direct script execution and interpreter-based execution
 * (for scripts like PHP, Python, etc.) and handles POST requests with
 * request bodies of any size.
 * 
 * Usage example:
 * 
 *     CgiExecute cgi;
 *     cgi.setTimeout(30);  // Set timeout to 30 seconds
 *     std::string output;
 *     CgiExecute::ExecuteResult result = cgi.execute(
 *         "/path/to/script.php", "/usr/bin/php", request, output);
 *     
 *     if (result == CgiExecute::EXECUTE_SUCCESS) {
 *         // Process the output
 *     }
 */
class CgiExecute {
 public:
    enum ExecuteResult {
        EXECUTE_SUCCESS,
        EXECUTE_PATH_ERROR,
        EXECUTE_FORK_ERROR,
        EXECUTE_EXEC_ERROR,
        EXECUTE_IO_ERROR,
        EXECUTE_TIMEOUT
    };
    CgiExecute();
    ~CgiExecute();

    ExecuteResult execute(const std::string& scriptPath,
                        const std::string& interpreter,
                        const HTTPRequest& request,
                        std::string& output);
    void setTimeout(unsigned int seconds) {
        _timeoutSeconds = seconds;
    }

 private:
    CgiExecute(const CgiExecute& other);
    CgiExecute& operator=(const CgiExecute& other);

    void setupEnvironmentVariables(const HTTPRequest& request,
                                const std::string& scriptPath);
    void setServerVariables(const HTTPRequest& request);
    void setClientVariables();
    void setRequestVariables(const HTTPRequest& request);
    void setPathVariables(const HTTPRequest& request,
                        const std::string& scriptPath);
    void convertHeadersToEnv(const HTTPRequest& request);
    void preparePostBody(const HTTPRequest& request);
    ExecuteResult createPipes();
    bool forkAndExecute(const std::string& scriptPath,
                        const std::string& interpreter);
    bool createChildProcess();
    void executeChildProcess(const std::string& scriptPath,
                        const std::string& interpreter);
    void setupChildIORedirection();
    void setupNullStdin();
    void closeChildPipeEnds();
    std::vector<char*> prepareEnvironmentVariables();
    void executeScript(const std::string& scriptPath,
                    const std::string& interpreter,
                    const std::vector<char*>& envp);
    void closeUnusedPipeEnds();
    ExecuteResult processData(const HTTPRequest& request,
                            std::string& output);
    bool readChildOutput(std::string& output);
    bool isChildProcessEnded();
    int readDataFromPipe(char* buffer, size_t maxSize);
    bool shouldTimeout(bool processEnded,
                    time_t startReadTime, int readTimeout);
    std::string extractScriptName(const std::string& requestPath,
                                const std::string& scriptPath) const;
    bool writeRequestBody(const HTTPRequest& request);
    bool waitForChildProcess();
    bool handleProcessExit(int status);
    bool handleWaitpidError();
    bool validateScriptPath(const std::string& scriptPath) const;
    void terminateChildProcess();
    void cleanupPipes();
    bool hasTimedOut() const {
        return (time(NULL) - _startTime) > _timeoutSeconds;
    }
    pid_t _childPid;
    int _inputPipe[2];
    int _outputPipe[2];
    unsigned int _timeoutSeconds;
    time_t _startTime;
    std::map<std::string, std::string> _environment;
    bool _hasPostBody;
    bool _isTimeOut;
    bool _isExecveError;
    std::vector<std::string> _envStrings;
};

}  // namespace http

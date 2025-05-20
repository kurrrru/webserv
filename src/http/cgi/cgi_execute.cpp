#include <cstring>
#include <iostream>
#include <cstdlib>
#include <cctype>
#include <map>
#include <vector>
#include <string>

#include <fcntl.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <signal.h>

#include "cgi_execute.hpp"

#include "../../toolbox/stepmark.hpp"
#include "../../toolbox/string.hpp"

namespace http {

CgiExecute::CgiExecute() :
_childPid(-1),
_timeoutSeconds(http::cgi::TIMEOUT),
_hasPostBody(false),
_isTimeOut(false),
_isExecveError(false) {
    _inputPipe[0] = -1;
    _inputPipe[1] = -1;
    _outputPipe[0] = -1;
    _outputPipe[1] = -1;
}

CgiExecute::~CgiExecute() {
    cleanupPipes();
}

/**
 * Main method for executing CGI scripts.
 * 
 * @param scriptPath Path to the CGI script to execute
 * @param interpreter Path to the interpreter (empty if directly executable)
 * @param request HTTP request information
 * @param output String to store CGI script output (output parameter)
 * @return Enumeration value of ExecuteResult indicating execution result
 * 
 * Process flow:
 * 1. Script path validation - Checks if executable
 * 2. Environment variables setup - Prepares HTTP request and server information
 * 3. POST body preparation - Prepares request body data processing
 * 4. Pipe creation - Establishes communication channels between parent and child processes
 * 5. Execution start time recording - For timeout management
 * 6. Child process creation and execution - Runs CGI script in a separate process
 * 7. Data processing - Sends POST data and reads CGI output
 * 8. Resource cleanup - Releases used pipes and other resources
 * 
 * Each step terminates early with appropriate error code if an error occurs
 */
CgiExecute::ExecuteResult CgiExecute::execute(const std::string& scriptPath,
                                            const std::string& interpreter,
                                            const HTTPRequest& request,
                                            std::string& output) {
    if (!validateScriptPath(scriptPath)) {
        toolbox::logger::StepMark::error(
            "Invalid CGI script path: " + scriptPath);
        return EXECUTE_PATH_ERROR;
    }
    setupEnvironmentVariables(request, scriptPath);
    preparePostBody(request);
    ExecuteResult result = createPipes();
    if (result != EXECUTE_SUCCESS) {
        return result;
    }
    _startTime = time(NULL);
    if (!forkAndExecute(scriptPath, interpreter)) {
        cleanupPipes();
        return EXECUTE_FORK_ERROR;
    }
    result = processData(request, output);
    if (result != EXECUTE_SUCCESS) {
        return result;
    }
    cleanupPipes();
    return EXECUTE_SUCCESS;
}

bool CgiExecute::validateScriptPath(const std::string& scriptPath) const {
    if (scriptPath.find("..") != std::string::npos) {
        return false;
    }
    struct stat st;
    if (stat(scriptPath.c_str(), &st) != 0) {
        return false;
    }
    if (!S_ISREG(st.st_mode)) {
        return false;
    }
    if (!(st.st_mode & S_IXUSR)) {
        return false;
    }
    return true;
}

void CgiExecute::setupEnvironmentVariables(const HTTPRequest& request,
                                        const std::string& scriptPath) {
    _environment.clear();
    _environment[http::cgi::meta::AUTH_TYPE] = "";
    setServerVariables(request);
    setClientVariables();
    setRequestVariables(request);
    setPathVariables(request, scriptPath);
    convertHeadersToEnv(request);
}

void CgiExecute::setServerVariables(const HTTPRequest& request) {
    _environment[http::cgi::meta::SERVER_SOFTWARE] = http::cgi::SERVER_SOFTWARE;
    _environment[http::cgi::meta::SERVER_NAME] = "";
    _environment[http::cgi::meta::SERVER_PORT] = "";
    _environment[http::cgi::meta::SERVER_PROTOCOL] = request.version;
    _environment[http::cgi::meta::GATEWAY_INTERFACE] =
                                        http::cgi::GATEWAY_INTERFACE;
}

void CgiExecute::setClientVariables() {
    _environment[http::cgi::meta::REMOTE_ADDR] = "";
    _environment[http::cgi::meta::REMOTE_HOST] = "";
    _environment[http::cgi::meta::REMOTE_IDENT] = "";
    _environment[http::cgi::meta::REMOTE_USER] = "";
}

void CgiExecute::setRequestVariables(const HTTPRequest& request) {
    _environment[http::cgi::meta::REQUEST_METHOD] = request.method;
    std::string query = request.uri.fullQuery;
    if (!query.empty()) {
        _environment[http::cgi::meta::QUERY_STRING] = query;
    } else {
        _environment[http::cgi::meta::QUERY_STRING] = "";
    }
    const HTTPFields::FieldValue& lengthValues =
        request.fields.getFieldValue(http::fields::CONTENT_LENGTH);
    if (!lengthValues.empty()) {
        const std::string& contentLength = lengthValues.front();
        if (!contentLength.empty()) {
            _environment[http::cgi::meta::CONTENT_LENGTH] = contentLength;
        } else {
            _environment[http::cgi::meta::CONTENT_LENGTH] =
                toolbox::to_string(request.body.contentLength);
        }
    } else if (request.body.contentLength > 0) {
        _environment[http::cgi::meta::CONTENT_LENGTH] =
            toolbox::to_string(request.body.contentLength);
    }
    if (request.body.contentLength > 0) {
        const HTTPFields::FieldValue& typeValues =
            request.fields.getFieldValue(http::fields::CONTENT_TYPE);
        if (!typeValues.empty()) {
            const std::string& contentType = typeValues.front();
            if (!contentType.empty()) {
                _environment[http::cgi::meta::CONTENT_TYPE] = contentType;
            } else {
                _environment[http::cgi::meta::CONTENT_TYPE] = "";
            }
        } else {
            _environment[http::cgi::meta::CONTENT_TYPE] = "";
        }
    }
}

void CgiExecute::setPathVariables(const HTTPRequest& request,
                                const std::string& scriptPath) {
    std::string scriptName = extractScriptName(request.uri.path, scriptPath);
    _environment[http::cgi::meta::SCRIPT_NAME] = scriptName;
    std::string pathInfo = request.uri.path;
    if (!pathInfo.empty()) {
        _environment[http::cgi::meta::PATH_INFO] = pathInfo;
        // Should actually be rootPath + pathInfo
        _environment[http::cgi::meta::PATH_TRANSLATED] = pathInfo;
    }
}

void CgiExecute::convertHeadersToEnv(const HTTPRequest& request) {
    const HTTPFields::FieldMap& fieldsMap = request.fields.get();
    for (HTTPFields::FieldMap::const_iterator it = fieldsMap.begin();
        it != fieldsMap.end(); ++it) {
        std::string name = it->first;
        if (name == http::fields::CONTENT_TYPE
            || name == http::fields::CONTENT_LENGTH
            || name == http::fields::AUTHORIZATION) {
            continue;
        }
        std::string envName = http::cgi::ENV_PREFIX;
        for (size_t i = 0; i < name.size(); i++) {
            char c = name[i];
            if (c == '-') {
                envName += '_';
            } else {
                envName += std::toupper(c);
            }
        }
        const HTTPFields::FieldValue& values = it->second;
        if (!values.empty()) {
            const std::string& value = values.front();
            _environment[envName] = value;
        }
    }
}

void CgiExecute::preparePostBody(const HTTPRequest& request) {
    _hasPostBody = (request.body.contentLength != 0 &&
                    request.method == http::method::POST);
}

CgiExecute::ExecuteResult CgiExecute::createPipes() {
    if (pipe(_outputPipe) == -1) {
        toolbox::logger::StepMark::error(
            "Failed to create output pipe: " + std::string(strerror(errno)));
        return EXECUTE_IO_ERROR;
    }
    if (_hasPostBody) {
        if (pipe(_inputPipe) == -1) {
            cleanupPipes();
            toolbox::logger::StepMark::error(
                "Failed to create input pipe: " + std::string(strerror(errno)));
            return EXECUTE_IO_ERROR;
        }
    }
    return EXECUTE_SUCCESS;
}

bool CgiExecute::forkAndExecute(const std::string& scriptPath,
                                const std::string& interpreter) {
    if (!createChildProcess()) {
        return false;
    }
    if (_childPid == 0) {
        executeChildProcess(scriptPath, interpreter);
        toolbox::logger::StepMark::error(
            "Child process failed to execute CGI script: " +
            std::string(strerror(errno)));
        std::exit(127);
    }
    closeUnusedPipeEnds();
    return true;
}

bool CgiExecute::createChildProcess() {
    _childPid = fork();
    if (_childPid == -1) {
        toolbox::logger::StepMark::error(
            "Fork failed: " + std::string(strerror(errno)));
        return false;
    }
    return true;
}

void CgiExecute::executeChildProcess(const std::string& scriptPath,
                                const std::string& interpreter) {
    setupChildIORedirection();
    std::vector<char*> envp = prepareEnvironmentVariables();
    executeScript(scriptPath, interpreter, envp);
}

void CgiExecute::setupChildIORedirection() {
    if (_hasPostBody) {
        dup2(_inputPipe[0], STDIN_FILENO);
    } else {
        setupNullStdin();
    }
    dup2(_outputPipe[1], STDOUT_FILENO);
    closeChildPipeEnds();
}

void CgiExecute::setupNullStdin() {
    int nullfd = open("/dev/null", O_RDONLY);
    if (nullfd != -1) {
        dup2(nullfd, STDIN_FILENO);
        close(nullfd);
    }
}

void CgiExecute::closeChildPipeEnds() {
    if (_hasPostBody && _inputPipe[0] != -1) {
        close(_inputPipe[0]);
    }
    if (_hasPostBody && _inputPipe[1] != -1) {
        close(_inputPipe[1]);
    }
    if (_outputPipe[0] != -1) {
        close(_outputPipe[0]);
    }
    if (_outputPipe[1] != -1) {
        close(_outputPipe[1]);
    }
}

std::vector<char*> CgiExecute::prepareEnvironmentVariables() {
    _envStrings.clear();
    for (std::map<std::string, std::string>::const_iterator it =
        _environment.begin();
        it != _environment.end(); ++it) {
        _envStrings.push_back(it->first + "=" + it->second);
    }
    std::vector<char*> envp;
    for (size_t i = 0; i < _envStrings.size(); ++i) {
        envp.push_back(const_cast<char*>(_envStrings[i].c_str()));
    }
    envp.push_back(NULL);
    return envp;
}

void CgiExecute::executeScript(const std::string& scriptPath,
                            const std::string& interpreter,
                            const std::vector<char*>& envp) {
    if (!interpreter.empty()) {
        char* argv[3];
        argv[0] = const_cast<char*>(interpreter.c_str());
        argv[1] = const_cast<char*>(scriptPath.c_str());
        argv[2] = NULL;
        execve(interpreter.c_str(), argv, const_cast<char**>(&envp[0]));
    } else {
        char* argv[2];
        argv[0] = const_cast<char*>(scriptPath.c_str());
        argv[1] = NULL;
        execve(scriptPath.c_str(), argv, const_cast<char**>(&envp[0]));
    }
}

void CgiExecute::closeUnusedPipeEnds() {
    if (_outputPipe[1] != -1) {
        close(_outputPipe[1]);
        _outputPipe[1] = -1;
    }
    if (_hasPostBody && _inputPipe[0] != -1) {
        close(_inputPipe[0]);
        _inputPipe[0] = -1;
    }
}

CgiExecute::ExecuteResult CgiExecute::processData(
    const HTTPRequest& request, std::string& output) {
    if (_hasPostBody) {
        if (!writeRequestBody(request)) {
            terminateChildProcess();
            cleanupPipes();
            return EXECUTE_IO_ERROR;
        }
        if (_inputPipe[1] != -1) {
            close(_inputPipe[1]);
            _inputPipe[1] = -1;
        }
    }
    if (!readChildOutput(output)) {
        terminateChildProcess();
        cleanupPipes();
        if (_isTimeOut) {
            return EXECUTE_TIMEOUT;
        }
        return EXECUTE_IO_ERROR;
    }
    if (!waitForChildProcess()) {
        terminateChildProcess();
        cleanupPipes();
        return EXECUTE_TIMEOUT;
    }
    if (_isExecveError) {
        cleanupPipes();
        return EXECUTE_EXEC_ERROR;
    }
    return EXECUTE_SUCCESS;
}

bool CgiExecute::writeRequestBody(const HTTPRequest& request) {
    try {
        const std::string& body = request.body.content;
        size_t totalBytes = 0;
        const char* data = body.c_str();
        size_t remaining = body.size();
        while (remaining > 0) {
            ssize_t written = write(_inputPipe[1],
                                    data + totalBytes,
                                    remaining);
            if (written <= 0) {
                if (errno == EINTR) {
                    continue;
                }
                toolbox::logger::StepMark::error(
                                "Failed to write request body: "
                                + std::string(strerror(errno)));
                return false;
            }
            totalBytes += written;
            remaining -= written;
        }
        return true;
    } catch (const std::exception& e) {
        toolbox::logger::StepMark::error(
            "Exception writing request body: " + std::string(e.what()));
        return false;
    }
}

bool CgiExecute::readChildOutput(std::string& output) {
    char buffer[http::cgi::READ_BUFFER_SIZE];
    time_t startReadTime = time(NULL);
    const int READ_TIMEOUT = http::cgi::READ_TIMEOUT_SEC;
    fcntl(_outputPipe[0], F_SETFL, O_NONBLOCK);

    while (true) {
        bool processEnded = isChildProcessEnded();
        int readStatus =
                    readDataFromPipe(buffer, http::cgi::READ_BUFFER_SIZE - 1);
        if (readStatus > 0) {
            int bytesRead = readStatus;
            buffer[bytesRead] = '\0';
            output.append(buffer, bytesRead);
            startReadTime = time(NULL);
        } else if (readStatus == 0) {
            break;
        } else if (readStatus == -1) {
            return false;
        }
        if (shouldTimeout(processEnded, startReadTime, READ_TIMEOUT)) {
            return false;
        }
    }
    return true;
}

bool CgiExecute::isChildProcessEnded() {
    if (_childPid <= 0) {
        return true;
    }
    int status;
    pid_t result = waitpid(_childPid, &status, WNOHANG);
    return (result == _childPid);
}

int CgiExecute::readDataFromPipe(char* buffer, size_t maxSize) {
    ssize_t bytes = read(_outputPipe[0], buffer, maxSize);
    if (bytes > 0) {
        return bytes;
    } else if (bytes == 0) {
        return 0;
    } else {
        if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR) {
            return -2;
        } else {
            toolbox::logger::StepMark::error(
                "Failed to read CGI output: " + std::string(strerror(errno)));
            return -1;
        }
    }
}

bool CgiExecute::shouldTimeout(bool processEnded,
                            time_t startReadTime, int readTimeout) {
    if (hasTimedOut()) {
        _isTimeOut = true;
        toolbox::logger::StepMark::error("CGI execution timed out");
        return true;
    }
    if (processEnded && (time(NULL) - startReadTime > readTimeout)) {
        return true;
    }
    return false;
}

bool CgiExecute::waitForChildProcess() {
    int status;
    while (true) {
        pid_t result = waitpid(_childPid, &status, WNOHANG);
        if (result == _childPid) {
            return handleProcessExit(status);
        }
        if (result == -1) {
            return handleWaitpidError();
        }
        if (hasTimedOut()) {
            _isTimeOut = true;
            toolbox::logger::StepMark::error("CGI execution timed out");
            return false;
        }
    }
}

bool CgiExecute::handleProcessExit(int status) {
    if (WIFEXITED(status)) {
        int exitCode = WEXITSTATUS(status);
        if (exitCode != 0) {
            toolbox::logger::StepMark::warning(
                "CGI exited with status: " + toolbox::to_string(exitCode));
            if (exitCode == 127) {
                _isExecveError = true;
            }
        }
    } else if (WIFSIGNALED(status)) {
        toolbox::logger::StepMark::warning(
                "CGI terminated by signal: "
                + toolbox::to_string(WTERMSIG(status)));
    }
    _childPid = -1;
    return true;
}

bool CgiExecute::handleWaitpidError() {
    if (errno == EINTR) {
        return true;
    }
    if (errno == ECHILD) {
        _childPid = -1;
        return true;
    }
    toolbox::logger::StepMark::error(
        "Waitpid error: " + std::string(strerror(errno)));
    return false;
}

void CgiExecute::terminateChildProcess() {
    if (_childPid > 0) {
        kill(_childPid, SIGTERM);
        kill(_childPid, SIGKILL);
        waitpid(_childPid, NULL, 0);
        _childPid = -1;
    }
}

void CgiExecute::cleanupPipes() {
    if (_inputPipe[0] != -1) {
        close(_inputPipe[0]);
        _inputPipe[0] = -1;
    }
    if (_inputPipe[1] != -1) {
        close(_inputPipe[1]);
        _inputPipe[1] = -1;
    }
    if (_outputPipe[0] != -1) {
        close(_outputPipe[0]);
        _outputPipe[0] = -1;
    }
    if (_outputPipe[1] != -1) {
        close(_outputPipe[1]);
        _outputPipe[1] = -1;
    }
}

std::string CgiExecute::extractScriptName(const std::string& requestPath,
                                        const std::string& scriptPath) const {
    std::string filename;
    size_t scriptLastSlash = scriptPath.find_last_of('/');
    if (scriptLastSlash != std::string::npos) {
        filename = scriptPath.substr(scriptLastSlash + 1);
    } else {
        filename = scriptPath;
    }
    size_t pos = requestPath.find(filename);
    if (pos != std::string::npos) {
        return requestPath.substr(0, pos + filename.length());
    }
    return requestPath;
}

}  // namespace http

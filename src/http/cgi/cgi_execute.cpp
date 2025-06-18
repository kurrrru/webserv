#include <cstring>
#include <iostream>
#include <cstdlib>
#include <cctype>
#include <map>
#include <vector>
#include <string>
#include <ctime>

#include <fcntl.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <signal.h>

#include "cgi_execute.hpp"
#include "../request/request.hpp"
#include "../response/method_utils.hpp"
#include "../../core/constant.hpp"
#include "../../event/epoll.hpp"
#include "../../../toolbox/stepmark.hpp"
#include "../../../toolbox/string.hpp"

namespace http {

CgiExecute::CgiExecute() :
_childPid(-1),
_timeoutSeconds(http::cgi::TIMEOUT),
_startTime(0),
_hasPostBody(false),
_isTimeOut(false),
_isExecveError(false),
_writeState(WRITE_IDLE),
_totalBytes(0),
_bytesWritten(0),
_writeBuffer(),
_readState(READ_IDLE),
_parser(),
_readStartTime(0),
_lastReadTime(0) {
    _inputPipe[0] = -1;
    _inputPipe[1] = -1;
    _outputPipe[0] = -1;
    _outputPipe[1] = -1;
}

CgiExecute::~CgiExecute() {
    cleanupPipes();
}

void CgiExecute::reset() {
    cleanupPipes();
    _childPid = -1;
    _timeoutSeconds = http::cgi::TIMEOUT;
    _startTime = 0;
    _hasPostBody = false;
    _isTimeOut = false;
    _isExecveError = false;
    _writeState = WRITE_IDLE;
    _totalBytes = 0;
    _bytesWritten = 0;
    _writeBuffer.clear();
    _readState = READ_IDLE;
    _parser.reset();
    _readStartTime = 0;
    _lastReadTime = 0;
    _lastWriteTime = 0;
    _inputPipe[0] = -1;
    _inputPipe[1] = -1;
    _outputPipe[0] = -1;
    _outputPipe[1] = -1;
    _environment.clear();
    _envStrings.clear();
    _client.reset();
}

CgiExecute::ExecuteResult CgiExecute::execute(
                                const std::string& scriptPath,
                                const std::string& interpreter,
                                const HTTPRequest& request,
                                const toolbox::SharedPtr<Client>& client,
                                const config::LocationConfig& locationConfig) {
    _client = client;
    if (!validateScriptPath(scriptPath)) {
        toolbox::logger::StepMark::error(
            "Invalid CGI script path: " + scriptPath);
        return EXECUTE_PATH_ERROR;
    }
    setupEnvironmentVariables(request, scriptPath, client, locationConfig);
    preparePostBody(request);
    ExecuteResult result = createPipes();
    if (result != EXECUTE_SUCCESS) {
        return result;
    }
    _startTime = std::time(NULL);
    if (!forkAndExecute(scriptPath, interpreter)) {
        cleanupPipes();
        return EXECUTE_FORK_ERROR;
    }
    result = processData(request);
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
                                const std::string& scriptPath,
                                const toolbox::SharedPtr<Client>& client,
                                const config::LocationConfig& locationConfig) {
    _environment.clear();
    _environment[http::cgi::meta::AUTH_TYPE] = "";
    setServerVariables(request, client);
    setClientVariables(client);
    setRequestVariables(request);
    setPathVariables(request, scriptPath, locationConfig.getRoot());
    convertHeadersToEnv(request);
}

void CgiExecute::setServerVariables(const HTTPRequest& request,
                                    const toolbox::SharedPtr<Client>& client) {
    _environment[http::cgi::meta::SERVER_SOFTWARE] = http::cgi::SERVER_SOFTWARE;
    const HTTPFields::FieldValue& hostValues =
                        request.fields.getFieldValue(http::fields::HOST);
    if (hostValues.empty()) {
        _environment[http::cgi::meta::SERVER_NAME] = "";
    } else {
        _environment[http::cgi::meta::SERVER_NAME] = hostValues.front();
    }
    size_t serverPort = client->getServerPort();
    if (serverPort == 0) {
        _environment[http::cgi::meta::SERVER_PORT] = "";
    } else {
        _environment[http::cgi::meta::SERVER_PORT] =
                                toolbox::to_string(serverPort);
    }
    _environment[http::cgi::meta::SERVER_PROTOCOL] = request.version;
    _environment[http::cgi::meta::GATEWAY_INTERFACE] =
                                        http::cgi::GATEWAY_INTERFACE;
}

void CgiExecute::setClientVariables(const toolbox::SharedPtr<Client>& client) {
    _environment[http::cgi::meta::REMOTE_ADDR] = client->getIp();
    _environment[http::cgi::meta::REMOTE_HOST] = "";
    _environment[http::cgi::meta::REMOTE_IDENT] = "";
    _environment[http::cgi::meta::REMOTE_USER] = "";
}

void CgiExecute::setRequestVariables(const HTTPRequest& request) {
    _environment[http::cgi::meta::REQUEST_METHOD] = request.method;
    std::string query = request.uri.fullQuery;
    if (!query.empty() && query[0] == '?') {
        query = query.substr(1);
    }
    _environment[http::cgi::meta::QUERY_STRING] = query;
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
    } else if (request.body.isChunked) {
        _environment[http::cgi::meta::CONTENT_LENGTH] =
            toolbox::to_string(request.body.content.size());
    }
    if (request.body.content.size() > 0) {
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
                                const std::string& scriptPath,
                                const std::string& rootPath) {
    std::string scriptName = extractScriptName(request.uri.path, scriptPath);
    _environment[http::cgi::meta::SCRIPT_NAME] = scriptName;
    _environment[http::cgi::meta::UPLOAD_DIR] = _client->getRequest()->getUploadPath();
    std::string pathInfo = request.uri.path;
    if (!pathInfo.empty()) {
        _environment[http::cgi::meta::PATH_INFO] = pathInfo;
        _environment[http::cgi::meta::PATH_TRANSLATED] =
                                http::joinPath(rootPath, pathInfo);
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
        toolbox::logger::StepMark::error("Failed to create output pipe");
        return EXECUTE_IO_ERROR;
    }
    if (!setNonBlocking(_outputPipe[0])) {
        cleanupPipes();
        toolbox::logger::StepMark::error(
            "Failed to set output pipe read end to non-blocking");
        return EXECUTE_IO_ERROR;
    }
    if (_hasPostBody) {
        if (pipe(_inputPipe) == -1) {
            cleanupPipes();
            toolbox::logger::StepMark::error("Failed to create input pipe");
            return EXECUTE_IO_ERROR;
        }
        if (!setNonBlocking(_inputPipe[1])) {
            cleanupPipes();
            toolbox::logger::StepMark::error(
                "Failed to set input pipe write end to non-blocking");
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
        std::size_t lastSlashPos = scriptPath.find_last_of('/');
        if (lastSlashPos != std::string::npos && chdir(scriptPath.substr(0, lastSlashPos).c_str()) != 0) {
            toolbox::logger::StepMark::error(
                "Failed to change directory to CGI script directory: "
                + scriptPath.substr(0, lastSlashPos));
            std::exit(127);
        }
        std::string newPath = scriptPath.substr(lastSlashPos + 1);
        executeChildProcess(newPath, interpreter);
        toolbox::logger::StepMark::error(
            "Child process failed to execute CGI script");
        std::exit(127);
    }
    closeUnusedPipeEnds();
    return true;
}

bool CgiExecute::createChildProcess() {
    _childPid = fork();
    if (_childPid == -1) {
        toolbox::logger::StepMark::error("Fork failed");
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
    if (_hasPostBody) {
        if (_inputPipe[1] != STDOUT_FILENO && _inputPipe[1] != STDIN_FILENO) {
            wrapClose(_inputPipe[1]);
        }
    }
    if (_outputPipe[0] != STDOUT_FILENO && _outputPipe[0] != STDIN_FILENO) {
        wrapClose(_outputPipe[0]);
    }
    if (_outputPipe[1] != STDOUT_FILENO && _outputPipe[1] != STDIN_FILENO) {
        wrapClose(_outputPipe[1]);
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
    wrapClose(_outputPipe[1]);
    if (_hasPostBody) {
        wrapClose(_inputPipe[0]);
    }
}

CgiExecute::ExecuteResult CgiExecute::processData(
    const HTTPRequest& request) {
    if (_hasPostBody && _writeState != WRITE_COMPLETED) {
        if (!initWriteRequestBody(request)) {
            if (_writeState == WRITE_ERROR) {
                terminateChildProcess();
                cleanupPipes();
                return EXECUTE_IO_ERROR;
            }
            return EXECUTE_WRITE_PENDING;
        }
    }
    if (_readState == READ_IDLE) {
        if (!initReadOutput()) {
            if (_readState == READ_ERROR) {
                terminateChildProcess();
                cleanupPipes();
                if (_isTimeOut) {
                    return EXECUTE_TIMEOUT;
                }
                return EXECUTE_IO_ERROR;
            }
            return EXECUTE_READ_PENDING;
        }
    } else if (_readState != READ_COMPLETED) {
        return EXECUTE_READ_PENDING;
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


bool CgiExecute::initWriteRequestBody(const HTTPRequest& request) {
    if (!_hasPostBody) {
        _writeState = WRITE_COMPLETED;
        return true;
    }
    _writeState = WRITE_IN_PROGRESS;
    _writeBuffer = request.body.content;
    _totalBytes = _writeBuffer.size();
    _bytesWritten = 0;
    return false;
}

bool CgiExecute::continueWriteRequestBody() {
    if (_writeState != WRITE_IN_PROGRESS) {
        return _writeState == WRITE_COMPLETED;
    }
    if (hasTimedOut()) {
        _isTimeOut = true;
        toolbox::logger::StepMark::error("CGI write operation timed out");
        _writeState = WRITE_ERROR;
        return false;
    }
    time_t currentTime = std::time(NULL);
    if (_lastWriteTime != 0 && (currentTime - _lastWriteTime) < 1) {
        return false;
    }
    size_t remaining = _totalBytes - _bytesWritten;
    size_t writeSize = remaining;
    if (writeSize > core::IO_BUFFER_SIZE) {
        writeSize = core::IO_BUFFER_SIZE;
    }
    ssize_t written = write(_inputPipe[1],
                            _writeBuffer.c_str() + _bytesWritten,
                            writeSize);
    toolbox::logger::StepMark::info(
        "continueWriteRequestBody: write returned "
        + toolbox::to_string(written) + " bytes");
    if (written > 0) {
        _bytesWritten += written;
        _lastWriteTime = 0;
        if (_bytesWritten >= _totalBytes) {
            _writeState = WRITE_COMPLETED;
            wrapClose(_inputPipe[1]);
            toolbox::logger::StepMark::debug(
                "POST data write completed, pipe closed");
            return true;
        }
        return false;
    } else if (written == -1) {
        // Non-blocking write might return -1 when it would block
        // Set timer for next retry attempt
        _lastWriteTime = currentTime;
        return false;
    } else {
        _writeState = WRITE_ERROR;
        return false;
    }
    return false;
}

bool CgiExecute::initReadOutput() {
    if (_readState != READ_IDLE) {
        return _readState == READ_COMPLETED;
    }
    if (isChildProcessEnded()) {
        toolbox::logger::StepMark::warning(
            "Child process already ended before reading output");
    }
    _readState = READ_IN_PROGRESS;
    _readStartTime = std::time(NULL);
    return false;
}

bool CgiExecute::continueReadOutput() {
    if (_readState != READ_IN_PROGRESS) {
        return _readState == READ_COMPLETED;
    }
    if (hasTimedOut()) {
        _isTimeOut = true;
        toolbox::logger::StepMark::error("CGI read operation timed out");
        _readState = READ_ERROR;
        return false;
    }
    time_t currentTime = std::time(NULL);
    time_t elapsed = currentTime - _readStartTime;
    if (elapsed < 1) {
        return false;
    }
    if (_lastReadTime != 0 && (currentTime - _lastReadTime) < 1) {
        return false;
    }
    char buffer[core::IO_BUFFER_SIZE];
    ssize_t bytes = read(_outputPipe[0], buffer, sizeof(buffer) - 1);
    toolbox::logger::StepMark::info(
        "continueReadOutput: read returned "
        + toolbox::to_string(bytes) + " bytes");
    if (bytes > 0) {
        _lastReadTime = 0;
        return processReadBytes(buffer, bytes);
    } else if (bytes == 0) {
        return processEndOfFile();
    } else {
        _lastReadTime = currentTime;
        return handleReadError();
    }
}

bool CgiExecute::processReadBytes(const char* buffer, size_t bytes) {
    BaseParser::ParseStatus status = _parser.run(std::string(buffer, bytes));
    if (status == BaseParser::P_COMPLETED) {
        _readState = READ_COMPLETED;
        _parser.get().identifyCgiType();
        return true;
    } else if (status == BaseParser::P_ERROR) {
        toolbox::logger::StepMark::error("CGI response parsing error");
        _readState = READ_ERROR;
        return false;
    }
    // P_NEED_MORE_DATA or P_IN_PROGRESS - continue reading
    return false;
}

bool CgiExecute::processEndOfFile() {
    BaseParser::ParseStatus status = _parser.run("");
    if (status == BaseParser::P_ERROR) {
        toolbox::logger::StepMark::error("CGI response parsing error at EOF");
        _readState = READ_ERROR;
        return false;
    }
    _parser.get().identifyCgiType();
    _readState = READ_COMPLETED;
    return true;
}

bool CgiExecute::handleReadError() {
    if (hasTimedOut()) {
        _isTimeOut = true;
        toolbox::logger::StepMark::error("CGI execution timed out");
        _readState = READ_ERROR;
        return false;
    }
    bool childEnded = isChildProcessEnded();
    if (childEnded) {
        const int READ_TIMEOUT = http::cgi::READ_TIMEOUT_SEC;
        if ((std::time(NULL) - _readStartTime) > READ_TIMEOUT) {
            _readState = READ_ERROR;
            return false;
        }
    }
    return false;
}

bool CgiExecute::isChildProcessEnded() {
    if (_childPid <= 0) {
        return true;
    }
    int status;
    pid_t result = waitpid(_childPid, &status, WNOHANG);
    return (result == _childPid);
}

bool CgiExecute::waitForChildProcess() {
    int status;
    while (true) {
        pid_t result = waitpid(_childPid, &status, WNOHANG);
        if (result == _childPid) {
            return handleProcessExit(status);
        }
        if (result == -1) {
            _childPid = -1;
            return true;
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


bool CgiExecute::hasTimedOut() const {
    time_t currentTime = std::time(NULL);
    time_t elapsed = currentTime - _startTime;
    if (elapsed > _timeoutSeconds) {
        return true;
    }
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
    wrapClose(_inputPipe[0]);
    wrapClose(_inputPipe[1]);
    wrapClose(_outputPipe[0]);
    wrapClose(_outputPipe[1]);
}

void CgiExecute::wrapClose(int& fd) {
    if (fd != -1) {
        close(fd);
        fd = -1;
    }
}

bool CgiExecute::setNonBlocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) {
        toolbox::logger::StepMark::error("Failed to get file descriptor flags");
        return false;
    }
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        toolbox::logger::StepMark::error("Failed to set non-blocking mode");
        return false;
    }
    return true;
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

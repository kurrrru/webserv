#pragma once

#include <string>
#include <map>
#include <vector>
#include <unistd.h>
#include <sys/types.h>

#include "../request/http_request.hpp"
#include "../../config/config_base.hpp"
#include "../../config/config_http.hpp"
#include "../../config/config_server.hpp"
#include "../../config/config_location.hpp"
#include "../../../toolbox/shared.hpp"
#include "cgi_response.hpp"
#include "cgi_response_parser.hpp"
#include "../../core/client.hpp"

namespace http {
class CgiExecute {
 public:
    enum ExecuteResult {
        EXECUTE_SUCCESS,
        EXECUTE_PATH_ERROR,
        EXECUTE_FORK_ERROR,
        EXECUTE_EXEC_ERROR,
        EXECUTE_IO_ERROR,
        EXECUTE_TIMEOUT,
        EXECUTE_WRITE_PENDING,
        EXECUTE_READ_PENDING
    };
    enum WriteState {
        WRITE_IDLE,
        WRITE_IN_PROGRESS,
        WRITE_COMPLETED,
        WRITE_ERROR
    };
    enum ReadState {
        READ_IDLE,
        READ_IN_PROGRESS,
        READ_COMPLETED,
        READ_ERROR
    };
    CgiExecute();
    ~CgiExecute();

    ExecuteResult execute(const std::string& scriptPath,
                        const std::string& interpreter,
                        const HTTPRequest& request,
                        const Client* client,
                        const config::LocationConfig& locationConfig);
    bool initWriteRequestBody(const HTTPRequest& request);
    bool continueWriteRequestBody();
    bool isWriteComplete() const { return _writeState == WRITE_COMPLETED; }
    bool hasWriteError() const { return _writeState == WRITE_ERROR; }

    bool initReadOutput();
    bool continueReadOutput();
    ReadState getReadState() const { return _readState; }
    bool isReadComplete() const { return _readState == READ_COMPLETED; }
    bool hasReadError() const { return _readState == READ_ERROR; }
    CgiResponse& getResponse() { return _parser.get(); }
    void reset();
    void cleanupPipes();
    bool hasTimedOut() const;

 private:
    CgiExecute(const CgiExecute& other);
    CgiExecute& operator=(const CgiExecute& other);

    bool processReadBytes(const char* buffer, size_t bytes);
    bool processEndOfFile();
    bool handleReadError();

    void setupEnvironmentVariables(const HTTPRequest& request,
                                const std::string& scriptPath,
                                const Client* client,
                                const config::LocationConfig& locationConfig);
    void setServerVariables(const HTTPRequest& request,
                        const Client* client);
    void setClientVariables(const Client* client);
    void setRequestVariables(const HTTPRequest& request);
    void setPathVariables(const HTTPRequest& request,
                        const std::string& scriptPath,
                        const std::string& rootPath);
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
    ExecuteResult processData(const HTTPRequest& request);
    bool isChildProcessEnded();
    std::string extractScriptName(const std::string& requestPath,
                                const std::string& scriptPath) const;
    bool waitForChildProcess();
    bool handleProcessExit(int status);
    bool validateScriptPath(const std::string& scriptPath) const;
    void terminateChildProcess();
    void wrapClose(int& fd);
    bool setNonBlocking(int fd);

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
    WriteState _writeState;
    size_t _totalBytes;
    size_t _bytesWritten;
    std::string _writeBuffer;
    ReadState _readState;
    CgiResponseParser _parser;
    time_t _readStartTime;
    time_t _lastReadTime;
    time_t _lastWriteTime;
    const Client* _client;
};

}  // namespace http

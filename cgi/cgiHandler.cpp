#include "CGIHandler.hpp"
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/wait.h>
#include <cstring>
#include <cstdlib>
#include <cerrno>

CGIHandler::CGIHandler(const std::string& scriptPath,
                        const std::string& interpreterPath,
                        const std::vector<std::string>& env,
                        const std::string& requestBody)
    : _scriptPath(scriptPath),
      _interpreterPath(interpreterPath),
      _env(env),
      _requestBody(requestBody),
      _writeOffset(0),
      _pid(-1),
      _state(CGI_WRITING),
      _startTime(0)
{
    _stdinPipe[0] = -1; _stdinPipe[1] = -1;
    _stdoutPipe[0] = -1; _stdoutPipe[1] = -1;
}

CGIHandler::~CGIHandler()
{
    closeAllFds();
}

static char** vectorToCharArray(const std::vector<std::string>& v)
{
    char** arr = new char*[v.size() + 1];
    for (size_t i = 0; i < v.size(); ++i) {
        arr[i] = new char[v[i].size() + 1];
        std::strcpy(arr[i], v[i].c_str());
    }
    arr[v.size()] = NULL;
    return arr;
}

static void freeCharArray(char** arr)
{
    for (size_t i = 0; arr[i] != NULL; ++i)
        delete[] arr[i];
    delete[] arr;
}

static bool setNonBlocking(int fd)
{
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1)
        return false;
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK) != -1;
}

bool CGIHandler::start()
{
    if (pipe(_stdinPipe) == -1)
        return false;
    if (pipe(_stdoutPipe) == -1) {
        close(_stdinPipe[0]);
        close(_stdinPipe[1]);
        return false;
    }

    _pid = fork();
    if (_pid < 0) {
        closeAllFds();
        return false;
    }

    if (_pid == 0) {
        dup2(_stdinPipe[0], STDIN_FILENO);
        dup2(_stdoutPipe[1], STDOUT_FILENO);

        close(_stdinPipe[0]);
        close(_stdinPipe[1]);
        close(_stdoutPipe[0]);
        close(_stdoutPipe[1]);

        char** envArr = vectorToCharArray(_env);
        char* argv[3];
        argv[0] = const_cast<char*>(_interpreterPath.c_str());
        argv[1] = const_cast<char*>(_scriptPath.c_str());
        argv[2] = NULL;

        execve(_interpreterPath.c_str(), argv, envArr);

        std::exit(1);
    }

    closeUnusedFdsInParent();

    if (!setNonBlocking(_stdinPipe[1]) || !setNonBlocking(_stdoutPipe[0])) {
        closeAllFds();
        return false;
    }

    _startTime = time(NULL);
    _state = _requestBody.empty() ? CGI_READING : CGI_WRITING;

    if (_requestBody.empty()) {
        close(_stdinPipe[1]);
        _stdinPipe[1] = -1;
    }

    return true;
}

void CGIHandler::closeUnusedFdsInParent()
{
    close(_stdinPipe[0]);
    _stdinPipe[0] = -1;
    close(_stdoutPipe[1]);
    _stdoutPipe[1] = -1;
}

void CGIHandler::closeAllFds()
{
    if (_stdinPipe[0] != -1) close(_stdinPipe[0]);
    if (_stdinPipe[1] != -1) close(_stdinPipe[1]);
    if (_stdoutPipe[0] != -1) close(_stdoutPipe[0]);
    if (_stdoutPipe[1] != -1) close(_stdoutPipe[1]);
    _stdinPipe[0] = _stdinPipe[1] = -1;
    _stdoutPipe[0] = _stdoutPipe[1] = -1;
}

int CGIHandler::getStdinFd() const  { return _stdinPipe[1]; }
int CGIHandler::getStdoutFd() const { return _stdoutPipe[0]; }

void CGIHandler::tryWrite()
{
    if (_stdinPipe[1] == -1 || _state != CGI_WRITING)
        return;

    size_t remaining = _requestBody.size() - _writeOffset;
    ssize_t n = write(_stdinPipe[1],
                       _requestBody.c_str() + _writeOffset,
                       remaining);

    if (n > 0) {
        _writeOffset += static_cast<size_t>(n);
        if (_writeOffset >= _requestBody.size()) {
            close(_stdinPipe[1]);
            _stdinPipe[1] = -1;
            _state = CGI_READING;
        }
    } else if (n == 0) {
        close(_stdinPipe[1]);
        _stdinPipe[1] = -1;
        _state = CGI_READING;
    } else {
        _state = CGI_ERROR;
    }
}

void CGIHandler::tryRead()
{
    if (_stdoutPipe[0] == -1 || _state != CGI_READING)
        return;

    char buf[4096];
    ssize_t n = read(_stdoutPipe[0], buf, sizeof(buf));

    if (n > 0) {
        _outputBuffer.append(buf, static_cast<size_t>(n));
    } else if (n == 0) {
        close(_stdoutPipe[0]);
        _stdoutPipe[0] = -1;
        _state = CGI_DONE;
    } else {
        _state = CGI_ERROR;
    }
}

void CGIHandler::checkChildStatus()
{
    if (_pid <= 0)
        return;
    int status;
    pid_t r = waitpid(_pid, &status, WNOHANG);
    if (r == _pid) {
        _pid = -1;
    }
}

bool CGIHandler::isTimedOut(time_t now) const
{
    const time_t CGI_TIMEOUT_SECONDS = 5;
    return (now - _startTime) > CGI_TIMEOUT_SECONDS;
}

void CGIHandler::killChild()
{
    if (_pid > 0) {
        kill(_pid, SIGKILL);
        _state = CGI_TIMEOUT;
    }
}

CGIState CGIHandler::getState() const { return _state; }
const std::string& CGIHandler::getOutputRaw() const { return _outputBuffer; }
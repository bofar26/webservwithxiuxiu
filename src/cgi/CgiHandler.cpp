/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiHandler.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xzhen <xzhen@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/13 00:00:00 by xzhen             #+#    #+#             */
/*   Updated: 2026/08/13 21:43:43 by xzhen            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CgiHandler.hpp"

#include <cstdlib>
#include <cstring>

#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/wait.h>

namespace{
	const time_t	cgiTimeout = 5;//a script that runs longer than this will be killed
	const size_t	cgiReadLimit = 4096;//limit of one read from CGI stdout
}

CgiHandler::CgiHandler()
	: _pid(-1), _inFd(-1), _outFd(-1), _body(), _bodySent(0),
	  _output(), _state(RUNNING), _start(time(NULL)){}

//call killChild to recall the sources
CgiHandler::~CgiHandler()
{
	if (_pid > 0)//if child process is alive
		killChild();
	closeFds();
}
//close two pipe fds
void	CgiHandler::closeFds()
{
	if (_inFd != -1)
	{
		close(_inFd);
		_inFd = -1;
	}
	if (_outFd != -1)
	{
		close(_outFd);
		_outFd = -1;
	}
}
//a transfrom to prepare parameters for execve()
static char	**buildArray(const std::vector<std::string>& values)
{
	char	**array = new char*[values.size() + 1];
	//std::vector<std::string>& values -> char**array;
	for (size_t i = 0; i < values.size(); i++)
	{
		array[i] = new char[values[i].size() + 1];
		std::strcpy(array[i], values[i].c_str());
	}
	array[values.size()] = NULL;
	return (array);
}
//because there's a new in buildArray();
static void	freeArray(char **array)
{
	for (size_t i = 0; array[i] != NULL; i++)
		delete[] array[i];
	delete[] array;
}
//splits "./www/cgi-bin/hello.py" into dir="./www/cgi-bin" and file="hello.py"
//we should split it, because we execve "python3 hello.py" instead of "python3 ./www/cgi-bin/hello.py"
static void	splitScriptPath(const std::string& script, std::string& dir, std::string& file)
{
	size_t	slash = script.find_last_of('/');

	if (slash == std::string::npos)
	{
		dir = ".";
		file = script;
		return ;
	}
	dir = script.substr(0, slash);
	file = script.substr(slash + 1);
}
//pipe -> fork -> child -> exec CGI -> parent
bool	CgiHandler::start(const std::string& interpreter, const std::string& script,
			const std::vector<std::string>& env, const std::string& body)
{
	int	inPipe[2];//create first pipe
	int	outPipe[2];

	if (pipe(inPipe) == -1)
		return (false);
	if (pipe(outPipe) == -1)
	{
		close(inPipe[0]);
		close(inPipe[1]);
		return (false);
	}

	_body = body;
	_bodySent = 0;
	_start = time(NULL);

	std::string	dir;
	std::string	file;
	splitScriptPath(script, dir, file);
	//create the child process with fork
	_pid = fork();
	if (_pid < 0)//failure of fork
	{
		close(inPipe[0]);
		close(inPipe[1]);
		close(outPipe[0]);
		close(outPipe[1]);
		_state = FAILED;
		return (false);
	}

	if (_pid == 0)//we are in child process now
	{
		dup2(inPipe[0], STDIN_FILENO);//connect the inpipe for reading to stdin
		dup2(outPipe[1], STDOUT_FILENO);//connect the outpipe for writing to stdout
		close(inPipe[0]);//we needn't any more pipe[0] or pipe[1]
		close(inPipe[1]);//beacause we had stdin->pipe<-stdout
		close(outPipe[0]);
		close(outPipe[1]);

		if (chdir(dir.c_str()) == -1)
			_exit(1);//kill the actual process with _exit

		std::vector<std::string>	args;
		args.push_back(interpreter);
		args.push_back(file);//we are inside dir now, the bare name is enough

		char	**argv = buildArray(args);
		char	**envp = buildArray(env);
		
		execve(interpreter.c_str(), argv, envp);
		//only reached when execve failed
		freeArray(argv);
		freeArray(envp);
		_exit(1);
	}
	//parent:close all pipes that it needn't
	close(inPipe[0]);
	close(outPipe[1]);
	_inFd = inPipe[1];
	_outFd = outPipe[0];	//these fds won't block actual process
	if (fcntl(_inFd, F_SETFL, O_NONBLOCK) == -1
		|| fcntl(_outFd, F_SETFL, O_NONBLOCK) == -1)
	{//if fail to set mode non-block
		killChild();
		closeFds();
		_state = FAILED;
		return (false);
	}
	//no body to push: close stdin now so the script sees EOF straight away
	if (_body.empty())
	{
		close(_inFd);
		_inFd = -1;
	}
	return (true);
}

int	CgiHandler::getReadFd() const
{
	return (_outFd);
}

int	CgiHandler::getWriteFd() const
{
	return (_inFd);
}

//to read the stdout of CGI
void	CgiHandler::onReadable()
{
	char	buffer[cgiReadLimit];
	ssize_t	byteread = read(_outFd, buffer, sizeof(buffer));

	if (byteread > 0)//sucess to read
	{	//pushback byteread to _output
		_output.append(buffer, static_cast<size_t>(byteread));
		return ;
	}
	close(_outFd);
	_outFd = -1;
	if (byteread == 0)//EOF, finished to read
		_state = FINISHED;
	else
		_state = FAILED;//byteread < 0, error
	if (_pid > 0)
	{
		waitpid(_pid, NULL, 0);//collect the child pid, no zombie left behind
		_pid = -1;
	}
}

//to send http body/CHI->server
void	CgiHandler::onWritable()
{	//the bytes really sent	//send to _inFd	   //send from here
	ssize_t	bytesent = write(_inFd, _body.c_str() + _bodySent,
						_body.size() - _bodySent);//try to write the left
	//error in last read
	if (bytesent <= 0)
	{
		close(_inFd);//broken pipe: stop feeding
		_inFd = -1;
		return ;
	}
	_bodySent += static_cast<size_t>(bytesent);
	if (_bodySent >= _body.size())//all of body has been sent
	{
		close(_inFd);
		_inFd = -1;
	}
}

bool	CgiHandler::isRunning() const
{
	return (_state == RUNNING);
}

bool	CgiHandler::isFailed() const
{
	return (_state == FAILED);
}

bool	CgiHandler::isTimedOut(time_t now) const
{
	return (_state == RUNNING && now - _start > cgiTimeout);
}
//to kill CGI
void	CgiHandler::killChild()
{
	if (_pid > 0)
	{
		kill(_pid, SIGKILL);
		waitpid(_pid, NULL, 0);
		_pid = -1;
	}
	_state = FAILED;
}
//to get the final output of CGI 
const std::string&	CgiHandler::getOutput() const
{
	return (_output);
}

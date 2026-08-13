/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiHandler.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xzhen <xzhen@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/13 00:00:00 by xzhen             #+#    #+#             */
/*   Updated: 2026/08/13 21:58:13 by xzhen            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CGIHANDLER_HPP
#define CGIHANDLER_HPP

#include <string>
#include <vector>
#include <ctime>
#include <cstddef>
#include <sys/types.h>

//a CgiHandler handles a child process, a pipe for stdin, a pipe for stdout

class CgiHandler{
	public:
	enum State{
		RUNNING,//child still working
		FINISHED,//child closed its stdout; output is complete
		FAILED//fork/pipe/execve went wrong, or the child was killed
	};

	private:
	pid_t		_pid;//PID of child process
	int			_inFd;//pipe to enter CGI stdin, -1 once closed
	int			_outFd;//pipe to read CGI stdout, -1 once closed
	std::string	_body;//http request body
	std::size_t	_bodySent;//how much of _body was already sent
	std::string	_output;//everything the child printed so far
	State		_state;//to save state of CGI:RUNNING,FINISHED;FAILED
	time_t		_start;//time of start of CGI
	//can't be copy
	CgiHandler(const CgiHandler& other);
	CgiHandler& operator=(const CgiHandler& other);
	//after closeFds, _inFd=-1, _outFd=-1
	void	closeFds();

	public:
	CgiHandler();
	~CgiHandler();

	//create CGI and execve CGI script
	bool	start(const std::string& interpreter, const std::string& script,
				const std::vector<std::string>& env, const std::string& body);
	
	int		getReadFd() const;//server should listen which fd to wait the stdout of CGI? -1 when there is nothing left to read
	int		getWriteFd() const;//server should listen which fd to give the body to CGI?-1 when the body has been fully sent

	void	onReadable();//called only after poll() reported POLLIN/CGI pipe is ready to be read.
	void	onWritable();//called only after poll() reported POLLOUT/CGI pipe is ready to be writen something.

	bool	isRunning() const;
	bool	isFailed() const;
	bool	isTimedOut(time_t now) const;
	void	killChild();//kill the child process and close it

	const std::string&	getOutput() const;
};

#endif

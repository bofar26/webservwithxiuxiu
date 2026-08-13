/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Connection.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xzhen <xzhen@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/06 11:04:21 by xzhen             #+#    #+#             */
/*   Updated: 2026/08/12 22:15:16 by xzhen            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Connection.hpp"
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "Router.hpp"

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <vector>
#include <cstdlib>
#include <unistd.h>
#include <sys/socket.h>

namespace{
	const time_t	connectionTimeout = 30;//out if no activity for 30 seconds
	const size_t	readLimit = 4096;//server can read 4096 bytes maximum
}

Connection::Connection(int fd, const ServerConfig& config)
	: _fd(fd), _state(READING), _config(config),
	  _requestReceived(), _responseToSend(), _responseSent(0), _lastActivity(time(NULL)),
	  _cgi(NULL), _logMethod(), _logPath()
{}

//deleting the handler kills the child and closes both pipes, so a client that
//disconnects mid script never leaves a process or an fd behind
Connection::~Connection()
{
	delete _cgi;
	if (_fd != -1)//-1 means socket isn't created
		close(_fd);
}
//--------------getters----------------------------
int	Connection::getFd() const
{
	return (_fd);
}

bool	Connection::getRead() const//true if reading
{
	return (_state == READING);
}

bool	Connection::getWrite() const
{
	return (_state == WRITING);
}

bool	Connection::getDone() const
{
	return (_state == DONE);
}
//----------------------------------------------
//true if no activity for more than connectionTimeout seconds
//a runaway script gets its own, much shorter deadline
bool	Connection::isTimedOut(time_t now) const
{
	if (_cgi != NULL && _cgi->isTimedOut(now))
		return (true);
	return (now - _lastActivity > connectionTimeout);
}
//
void	Connection::toRead()
{
	char	buffer[readLimit];
	ssize_t	byteread;//how many bytes received
	byteread = recv(_fd, buffer, sizeof(buffer), 0);
	if (byteread <= 0)
	{
		_state = DONE;
		return ;
	}
	_lastActivity = time(NULL);//append the received request to _requestReceived. If the request is complete, process it.
	_requestReceived.append(buffer, static_cast<size_t>(byteread));
	//reject an oversized body
	if (isBodyTooLarge())
	{
		queueError(413, "Payload Too Large");
		return ;
	}
	if(isRequestComplete())
		processRequest();
}

void	Connection::toWrite()
{
	ssize_t	bytesent;
	//begin to send from the place (_responseSent_responseToSend.c_str() + _responseSent)
	//we have still (_responseToSend.size() - _responseSent) to send
	bytesent = send(_fd, _responseToSend.c_str() + _responseSent, _responseToSend.size() - _responseSent, 0);
	if (bytesent <= 0)
	{
		_state = DONE;
		return ;
	}
	_lastActivity = time(NULL);//refresh the time of this activity
	_responseSent += static_cast<size_t>(bytesent);
	if (_responseSent >= _responseToSend.size())
		_state = DONE;
}
//for POST if we use '\r\n\r\n' to judge the end of request, maybe we miss the body of request
bool	Connection::isRequestComplete() const
{
	size_t	headerEnd = _requestReceived.find("\r\n\r\n");

	if (headerEnd == std::string::npos)//didn't find a "\r\n\r\n"
		return (false);

	size_t		bodyStart = headerEnd + 4;//skip the empty line of post
	std::string	header = _requestReceived.substr(0, headerEnd);//cut from 0 to headerEnd
	size_t		pos = header.find("Content-Length:");

	if (pos == std::string::npos)//request is complete
		return (true);

	std::string			value = header.substr(pos + 15);//skip "Content-Length:"
	std::istringstream	iss(value);
	size_t				length = 0;

	iss >> length;//>> skips the spaces and stops at the first non digit
	//
	size_t	bodyReceived = _requestReceived.size() - bodyStart;
	return (bodyReceived >= length);//processRequest util bodyReceived completely
}
//
bool	Connection::isBodyTooLarge() const
{
	size_t	headerEnd = _requestReceived.find("\r\n\r\n");

	if (headerEnd == std::string::npos)
		return (false);

	std::string	header = _requestReceived.substr(0, headerEnd);
	size_t		pos = header.find("Content-Length:");

	if (pos == std::string::npos)
		return (false);

	std::istringstream	iss(header.substr(pos + 15));
	size_t				length = 0;

	iss >> length;
	return (length > _config.getClientMaxBodySize());
}

//prepare a response when body is to large
void	Connection::queueError(int code, const std::string& text)
{
	HttpResponse	response;

	response.setStatus(code);
	response.setHeader("Content-Type", "text/plain");
	response.setBody(text + "\n");
	std::cout << "[" << _config.getPort() << "] - - -> " << code << std::endl;
	_responseToSend = response.toString();
	_responseSent = 0;
	_state = WRITING;
}

//1.create a HttpRequest from _requestReceived,
//2.then create a Router with _config, 
//3.and call handleRequest to get a HttpResponse. 
//4.Then convert the HttpResponse to string and store it in _responseToSend. 
//5.Finally, set _state to WRITING.
void	Connection::processRequest()
{
	HttpResponse	response;
	std::string		method = "-";
	std::string		path = "-";

	try
	{
		HttpRequest	request(_requestReceived);
		Router		router(_config);

		method = request.getMethod();
		path = request.getPath();
		_logMethod = method;
		_logPath = path;
		//a CGI cannot answer immediately: start it and let poll() drive the
		//pipes. The response is built later, in finishCgi().
		if (startCgi(request, router))
			return ;
		//http request -> router -> http response
		response = router.handleRequest(request);
	}
	catch (const std::exception& e)
	{
		response.setStatus(400);
		response.setHeader("Content-Type", "text/plain");
		response.setBody(std::string("Bad Request: ") + e.what() + "\n");
	}				//response here is convert from request, it's valid
	queueResponse(response, response.getStatusCode());
}

//the single place where a finished response enters the WRITING state
void	Connection::queueResponse(const HttpResponse& response, int code)
{
	//access log: one line per request, port + method + path + status
	std::cout << "[" << _config.getPort() << "] "
		<< (_logMethod.empty() ? "-" : _logMethod) << " "
		<< (_logPath.empty() ? "-" : _logPath)
		<< " -> " << code << std::endl;
	_responseToSend = response.toString();
	_responseSent = 0;
	_state = WRITING;
}

//only GET and POST reach a script; the Router already refused anything the
//location does not allow, so here we just look at the extension
bool	Connection::startCgi(const HttpRequest& request, const Router& router)
{
	std::string	interpreter;
	std::string	script;
	std::string	query;

	if (!router.isCgiRequest(request, interpreter, script, query))
		return (false);

	std::vector<std::string>	env;
	std::ostringstream			port;
	std::ostringstream			length;

	port << _config.getPort();
	length << request.getBody().size();

	//the variables RFC 3875 asks a server to hand over to the script
	env.push_back("GATEWAY_INTERFACE=CGI/1.1");
	env.push_back("SERVER_PROTOCOL=HTTP/1.1");
	env.push_back("SERVER_SOFTWARE=webserv/1.0");
	env.push_back("SERVER_PORT=" + port.str());
	env.push_back("REQUEST_METHOD=" + request.getMethod());
	env.push_back("QUERY_STRING=" + query);
	env.push_back("SCRIPT_NAME=" + request.getPath());
	env.push_back("SCRIPT_FILENAME=" + script);
	env.push_back("CONTENT_LENGTH=" + length.str());
	env.push_back("CONTENT_TYPE=" + request.getHeader("Content-Type"));
	env.push_back("HTTP_HOST=" + request.getHeader("Host"));
	env.push_back("REDIRECT_STATUS=200");//php-cgi refuses to run without it
	env.push_back("PATH=/usr/local/bin:/usr/bin:/bin");

	_cgi = new CgiHandler();
	if (!_cgi->start(interpreter, script, env, request.getBody()))
	{
		delete _cgi;
		_cgi = NULL;
		queueError(500, "Internal Server Error");
		return (true);//handled: an error response is already queued
	}
	_state = CGI;
	return (true);
}

//the script prints its own headers, an empty line, then the body:
//    Content-Type: text/html
//
//    <html>...
//so we split on that empty line and copy its headers into our response.
//When it prints no headers at all, everything is treated as the body.
void	Connection::finishCgi()
{
	HttpResponse	response;

	if (_cgi == NULL || _cgi->isFailed())
	{
		response.setStatus(502);//the gateway (our CGI) misbehaved
		response.setHeader("Content-Type", "text/plain");
		response.setBody("Bad Gateway\n");
		delete _cgi;
		_cgi = NULL;
		queueResponse(response, 502);
		return ;
	}

	const std::string&	output = _cgi->getOutput();
	size_t				headerEnd = output.find("\r\n\r\n");
	size_t				skip = 4;

	if (headerEnd == std::string::npos)
	{
		headerEnd = output.find("\n\n");//scripts often use bare newlines
		skip = 2;
	}

	response.setStatus(200);
	if (headerEnd == std::string::npos)
	{
		response.setHeader("Content-Type", "text/html");
		response.setBody(output);
	}
	else
	{
		std::istringstream	headers(output.substr(0, headerEnd));
		std::string			line;

		while (std::getline(headers, line))
		{
			if (!line.empty() && line[line.size() - 1] == '\r')
				line.erase(line.size() - 1);
			size_t	colon = line.find(':');
			if (colon == std::string::npos)
				continue ;
			std::string	key = line.substr(0, colon);
			std::string	value = line.substr(colon + 1);
			if (!value.empty() && value[0] == ' ')
				value.erase(0, 1);
			//"Status: 404 Not Found" is how a CGI changes the status code
			if (key == "Status")
				response.setStatus(atoi(value.c_str()));
			else
				response.setHeader(key, value);
		}
		//setBody recomputes Content-Length, so a stale one from the script
		//cannot desynchronise the answer
		response.setBody(output.substr(headerEnd + skip));
	}

	delete _cgi;
	_cgi = NULL;
	queueResponse(response, response.getStatusCode());
}

int	Connection::getCgiReadFd() const
{
	if (_cgi == NULL)
		return (-1);
	return (_cgi->getReadFd());
}

int	Connection::getCgiWriteFd() const
{
	if (_cgi == NULL)
		return (-1);
	return (_cgi->getWriteFd());
}

//poll() reported the script's stdout is readable
void	Connection::onCgiReadable()
{
	if (_cgi == NULL)
		return ;
	_lastActivity = time(NULL);
	_cgi->onReadable();
	if (!_cgi->isRunning())//the script closed its stdout: output is complete
		finishCgi();
}

//poll() reported the script's stdin accepts more of the request body
void	Connection::onCgiWritable()
{
	if (_cgi == NULL)
		return ;
	_lastActivity = time(NULL);
	_cgi->onWritable();
}

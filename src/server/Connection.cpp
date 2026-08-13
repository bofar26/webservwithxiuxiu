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
#include <unistd.h>
#include <sys/socket.h>

namespace{
	const time_t	connectionTimeout = 30;//out if no activity for 30 seconds
	const size_t	readLimit = 4096;//server can read 4096 bytes maximum
}

Connection::Connection(int fd, const ServerConfig& config)
	: _fd(fd), _state(READING), _config(config),
	  _requestReceived(), _responseToSend(), _responseSent(0), _lastActivity(time(NULL))
{}

Connection::~Connection()
{
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
bool	Connection::isTimedOut(time_t now) const
{
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
		//http request -> router -> http response
		response = router.handleRequest(request);
	}
	catch (const std::exception& e)
	{
		response.setStatus(400);
		response.setHeader("Content-Type", "text/plain");
		response.setBody(std::string("Bad Request: ") + e.what() + "\n");
	}				//response here is convert from request, it's valid
	//access log: one line per request, port + method + path + status
	std::cout << "[" << _config.getPort() << "] " << method << " " << path
		<< " -> " << response.getStatusCode() << std::endl;
	_responseToSend = response.toString();
	_responseSent = 0;
	_state = WRITING;
}

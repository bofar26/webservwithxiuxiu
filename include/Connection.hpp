/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Connection.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xzhen <xzhen@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/05 00:00:00 by xzhen             #+#    #+#             */
/*   Updated: 2026/08/13 00:00:00 by xzhen            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONNECTION_HPP
#define CONNECTION_HPP

#include <string>
#include <ctime>
#include <cstddef>
#include "ServerConfig.hpp"
#include "CgiHandler.hpp"

//only pointers and references to these are used in the declarations below,
//so a forward declaration is enough and the headers stay decoupled
class HttpRequest;
class HttpResponse;
class Router;

//it represents a connected client
//it remembers the state of the connection:
// READING: receiving a request
// CGI    : a script is running, we are waiting on its pipes, not on the client
// WRITING: sending a response
// DONE: finished, ready to be closed

class Connection{
	public:
	enum State{//most crucial
		READING,//0
		CGI,//1 waiting for a child process
		WRITING,//2
		DONE//3
	};

	private:
	int				_fd;//socket of this client
	State			_state;//current state
	ServerConfig	_config;//of server
	std::string		_requestReceived;//request received
	std::string		_responseToSend;//response ready to send
	std::size_t		_responseSent;//response which has been sent
	time_t			_lastActivity;//last time of activity
	CgiHandler		*_cgi;//NULL unless a script is running for this client
	std::string		_logMethod;//kept for the access log once the CGI is over
	std::string		_logPath;

	Connection(const Connection& other);
	Connection& operator=(const Connection& other);

	//RFC 7230: header names are case insensitive and the value may be padded
	//with spaces or tabs. These two do the lookup properly.
	static std::string	toLower(const std::string& text);
	static std::string	getHeaderValue(const std::string& headers,
							const std::string& name);

	bool	isRequestComplete() const;
	//true when the client announced Transfer-Encoding: chunked
	bool	isChunked() const;
	//true when the terminating "0\r\n\r\n" chunk has arrived
	bool	hasChunkedEnd() const;
	//rebuilds the whole request with the chunk framing removed
	void	unchunkBody();
	//true when the announced Content-Length is over client_max_body_size
	bool	isBodyTooLarge() const;
	//queues a ready made response without going through the Router
	void	queueError(int code, const std::string& text);
	void	processRequest();
	//tries to start a CGI for this request, false when it is not a CGI URL
	bool	startCgi(const HttpRequest& request, const Router& router);
	//turns what the script printed into a real HTTP response
	void	finishCgi();
	//shared tail of processRequest / queueError / finishCgi
	void	queueResponse(const HttpResponse& response, int code);

	public:
	Connection(int fd, const ServerConfig& config);
	~Connection();

	void	toRead();//called when the socket is readable
	void	toWrite();//called when the socket is writable

	bool	getRead() const;//return true if _state == READING
	bool	getWrite() const;//return true if _state == WRITING
	bool	getDone() const;//return true if _state == DONE
	bool	isTimedOut(time_t now) const;
	bool	hasCgiTimedOut(time_t now) const;
	void	abortCgi();

	//the event loop needs these to put the CGI pipes in poll()
	int		getCgiReadFd() const;//-1 when no script is running
	int		getCgiWriteFd() const;//-1 when there is no body left to push
	void	onCgiReadable();//poll said the script printed something
	void	onCgiWritable();//poll said the script can take more body

	int		getFd() const;
};

#endif

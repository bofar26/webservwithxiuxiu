/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Connection.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xzhen <xzhen@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/05 00:00:00 by xzhen             #+#    #+#             */
/*   Updated: 2026/08/11 18:32:47 by xzhen            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONNECTION_HPP
#define CONNECTION_HPP

#include <string>
#include <ctime>
#include <cstddef>
#include "ServerConfig.hpp"

//it represents a connected client
//it remembers the state of the connection:
// READING: receiving a request
// WRITING: sending a response
// DONE: finished, ready to be closed

class Connection{
	public:
	enum State{//most crucial
		READING,//0
		WRITING,//1
		DONE//2
	};

	private:
	int				_fd;//socket of this client
	State			_state;//current state
	ServerConfig	_config;//of server
	std::string		_requestReceived;//request received
	std::string		_responseToSend;//response ready to send
	std::size_t		_responseSent;//response which has been sent
	time_t			_lastActivity;//last time of activity

	Connection(const Connection& other);
	Connection& operator=(const Connection& other);

	bool	isRequestComplete() const;
	//true when the announced Content-Length is over client_max_body_size
	bool	isBodyTooLarge() const;
	//queues a ready made response without going through the Router
	void	queueError(int code, const std::string& text);
	void	processRequest();

	public:
	Connection(int fd, const ServerConfig& config);
	~Connection();

	void	toRead();//called when the socket is readable
	void	toWrite();//called when the socket is writable

	bool	getRead() const;//return true if _state == READING
	bool	getWrite() const;//return true if _state == WRITING
	bool	getDone() const;//return true if _state == DONE
	bool	isTimedOut(time_t now) const;

	int		getFd() const;
};

#endif

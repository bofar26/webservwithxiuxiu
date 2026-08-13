/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xzhen <xzhen@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/09 17:01:38 by mipang            #+#    #+#             */
/*   Updated: 2026/08/10 12:18:59 by xzhen            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

#include <iostream>
#include <string>
#include <cstring>
#include <cerrno>
#include <csignal>
#include <sstream>
#include <stdexcept>

#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>

namespace{
	const int	POLL_TIMEOUT_MS = 1000;//close inactive connections every POLL_TIMEOUT_MS ms
	const int	maximumConnections = 128;//listen socket can wait au maximum maximumConnections client to connect
}
//tell to server to stop when g_stop=1, so that it can clean up and close sockets.
volatile sig_atomic_t	g_stop = 0;

static void	handleSigint(int)
{
	g_stop = 1;
}

Server::Server():_configs(), _listenFds(), _connections(), _pollFds(){}

Server::Server(const std::vector<ServerConfig>& configs)
	: _configs(configs), _listenFds(), _connections(), _pollFds(){}
//clean up all connections and close all listen fd(listening sockets).
Server::~Server()
{
	for (std::map<int, Connection *>::iterator it = _connections.begin();
		it != _connections.end(); ++it)
		delete it->second;
	for (std::map<int, size_t>::iterator it = _listenFds.begin();
		it != _listenFds.end(); ++it)
		close(it->first);
}

//create a listening socket for the given port
int	Server::createListenSocket(int port)
{
	struct sockaddr_in	address;
	int					fd;
	int					opt = 1;

	fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd == -1)
	{
		std::cerr << "socket error: " << std::strerror(errno) << std::endl;
		throw std::runtime_error("socket() failed.");
	}	//setsocketopt allows to reuse the same port most quikly
	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
	{
		std::cerr << "setsockopt error: " << std::strerror(errno) << std::endl;
		close(fd);
		throw std::runtime_error("setsockopt() failed.");
	}	//fcntl sets the socket to non-blocking mode, don't wait for a client to send data if no activity
	if (fcntl(fd, F_SETFL, O_NONBLOCK) == -1)
	{
		std::cerr << "fcntl error: " << std::strerror(errno) << std::endl;
		close(fd);
		throw std::runtime_error("fcntl() failed.");
	}
	//struct sockaddr_in is used for IPv4 addresses, it contains the address family, port and IP address.
	memset(&address, 0, sizeof(address));//initialize the address structure to zero
	address.sin_family = AF_INET;//ipv4
	address.sin_addr.s_addr = INADDR_ANY;//bind to all available network interfaces
	address.sin_port = htons(port);//set the port number, htons converts port number to network number
	//bind associates the socket with port and ip, eg:fd=3 -> 127.0.0.1:8080
	if (bind(fd, reinterpret_cast<struct sockaddr *>(&address), sizeof(address)) == -1)
	{
		std::cerr << "bind error on port " << port << ": "
			<< std::strerror(errno) << std::endl;
		close(fd);
		throw std::runtime_error("bind() failed.");
	}
	//listen marks the socket as a listening socket, "i'm waiting for clients to connect"
	if (listen(fd, maximumConnections) == -1)
	{
		std::cerr << "listen error on port " << port << ": "
			<< std::strerror(errno) << std::endl;
		close(fd);
		throw std::runtime_error("listen() failed.");
	}
	return (fd);
}
//create a listen socket with createListenSocket based on _configs
void	Server::setListenSocket()
{
	if (_configs.empty())
		throw std::runtime_error("No server configuration to listen with.");
	for (size_t i = 0; i < _configs.size(); i++)
	{
		int	fd = createListenSocket(_configs[i].getPort());

		_listenFds[fd] = i;
	}
}
//what should poll monitor
void	Server::buildPollSet()
{
	struct pollfd	entry;

	_pollFds.clear();
	//add all listening sockets to the poll
	for (std::map<int, size_t>::iterator it = _listenFds.begin();
		it != _listenFds.end(); ++it)
	{
		entry.fd = it->first;//the listening socket fd
		entry.events = POLLIN;//tell me if there's a new client to accept
		entry.revents = 0;
		_pollFds.push_back(entry);//add this listening socket to the poll
	}
	//add all client connections to the poll
	for (std::map<int, Connection *>::iterator it = _connections.begin();
		it != _connections.end(); ++it)
	{
		entry.fd = it->first;
		entry.events = 0;
		if (it->second->getRead())//this client fd is ready to read
			entry.events |= POLLIN;
		if (it->second->getWrite())
			entry.events |= POLLOUT;
		entry.revents = 0;
		_pollFds.push_back(entry);//add this client connection to the poll
	}
}
//attribute tasks to each fd; if it's a listening socket, accept the new client connection, if it's a client connection, read or write data.
void	Server::dispatch()
{
	std::vector<int>	finished;

	for (size_t i = 0; i < _pollFds.size(); i++)
	{
		short	events = _pollFds[i].revents;//events:what i monitor
		int		fd = _pollFds[i].fd;		//revents:what happened

		if (events == 0)//nothing happened, skip this fd
			continue ;
		//if this fd is a listening socket, accept
		if (_listenFds.find(fd) != _listenFds.end())
		{
			if (events & POLLIN)
				acceptNew(fd);
			continue ;
		}
		//if this fd is a client connection, read or write
		std::map<int, Connection *>::iterator	it = _connections.find(fd);
		if (it == _connections.end())
			continue ;
		//there's an error on this connection, close it;
		if (events & (POLLERR | POLLHUP | POLLNVAL))
		{
			finished.push_back(fd);
			continue ;
		}
		//server fin a client is readable
		if (events & POLLIN)
			it->second->toRead();
		else if (events & POLLOUT)//or writable
			it->second->toWrite();

		if (it->second->getDone())
			finished.push_back(fd);
	}
	//close all finished connections
	for (size_t i = 0; i < finished.size(); i++)
		closeConnection(finished[i]);
}

//the listening fd tells us which server block this client belongs to
void	Server::acceptNew(int listenFd)
{
	std::map<int, size_t>::iterator	it = _listenFds.find(listenFd);

	if (it == _listenFds.end())
		return ;
	//accept client and create a connection
	int	clientFd = accept(listenFd, NULL, NULL);
	if (clientFd == -1)
		return ;
	//set the client socket to non-blocking mode, so that we don't wait for the client to send data if no activity
	if (fcntl(clientFd, F_SETFL, O_NONBLOCK) == -1)
	{
		close(clientFd);
		return ;
	}
	//create a new Connection for this client and store it in _connections
	_connections[clientFd] = new Connection(clientFd, _configs[it->second]);
}
//close the connection and remove it from _connections
void	Server::closeConnection(int fd)
{
	std::map<int, Connection *>::iterator	it = _connections.find(fd);

	if (it == _connections.end())
		return ;
	delete it->second;//the destructor closes the socket
	_connections.erase(it);
}
//close inactive connections
void	Server::cleanConnectionInactive()
{
	std::vector<int>	stale;
	time_t				now = time(NULL);

	for (std::map<int, Connection *>::iterator it = _connections.begin();
		it != _connections.end(); ++it)
	{    
		if (it->second->isTimedOut(now))
			stale.push_back(it->first);
	}
	for (size_t i = 0; i < stale.size(); i++)
		closeConnection(stale[i]);
}

void	Server::start()
{
	setListenSocket();

	std::signal(SIGINT, handleSigint);
	std::signal(SIGPIPE, SIG_IGN);

	std::cout << "webserv listening on";
	for (std::map<int, size_t>::iterator it = _listenFds.begin();
		it != _listenFds.end(); ++it)
		std::cout << " " << _configs[it->second].getPort()
				  << " (" << _configs[it->second].getRoot() << ")";
	std::cout << std::endl;

	while (!g_stop)
	{
		buildPollSet();
		//poll please help me monitor all fds in _pollFds, tell me which fd is readable or writable or has an error, wait for POLL_TIMEOUT_MS ms
		int	ready = poll(&_pollFds[0], _pollFds.size(), POLL_TIMEOUT_MS);
		//there are sockets are ready
		if (ready > 0)
			dispatch();
		cleanConnectionInactive();
	}
	std::cout << "shutting down" << std::endl;
}

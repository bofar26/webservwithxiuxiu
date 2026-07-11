/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mipang <mipang@student.42.fr>              #+#  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026-07-09 17:01:38 by mipang            #+#    #+#             */
/*   Updated: 2026-07-09 17:01:38 by mipang           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include "HttpResponse.hpp"

#include <iostream>
#include <string>
#include <cstring>
#include <stdexcept>

#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

Server::Server():_port(8080), _serverFd(-1){}
Server::Server(int port):_port(port), _serverFd(-1){}
Server::~Server()
{
	if (_serverFd != -1)
		close(_serverFd);
}

Server::Server(const Server& other)
{
	*this = other;
}
Server& Server::operator=(const Server& other)
{
	if (this != &other)
	{
		_port = other._port;
		_serverFd = other._serverFd;
	}
	return (*this);
}

void	Server::createServer()
{
	_serverFd = socket(AF_INET, SOCK_STREAM, 0);
	if (_serverFd == -1)
		throw std::runtime_error("socket() failed.");
}
void	Server::bindServer()
{
	struct sockaddr_in	address;

	memset(&address, 0, sizeof(address));
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(_port);
	if (bind(_serverFd, reinterpret_cast<struct sockaddr *>(&address), sizeof(address)) == -1)
		throw std::runtime_error("bind() failed.");
}

void	Server::listenServer()
{
	if (listen(_serverFd, 128) == -1)
		throw std::runtime_error("listen() failed.");
}
void	Server::handleClient(int clientFd)
{
	std::string	request;
	char	buffer[4096];

	while (true)
	{
		memset(buffer, 0, sizeof(buffer));
		int byteread = recv(clientFd, buffer, sizeof(buffer), 0);
		if (byteread == -1)
		{
			close(clientFd);
			throw std::runtime_error("rev() failed.");
		}
		if (byteread == 0)
			break ;
		request.append(buffer, byteread);
		if (request.find("\r\n\r\n") != std::string::npos)
			break ;
	}
		//for test
		HttpResponse	response;
		response.setStatus(200);
		response.setHeader("Content-Type", "text/plain");
		response.setBody("test");

	std::string	Rawresponse = response.toString();
	if (send(clientFd, Rawresponse.c_str(), Rawresponse.size(), 0))
	{
		close(clientFd);
		throw std::runtime_error("send() failed.");
	}

	close(clientFd);
}

void	Server::start()
{
	createServer();
	bindServer();
	listenServer();

	while(true)
	{
		int	clientFd = accept(_serverFd, NULL, NULL);
		if (clientFd == -1)
			throw std::runtime_error("accept() failed.");
		handleClient(clientFd);
	}
}

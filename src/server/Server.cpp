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
#include "HttpRequest.hpp"
#include "Router.hpp"

#include <iostream>
#include <string>
#include <cstring>
#include <cerrno>
#include <stdexcept>

#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

Server::Server():_config(), _serverFd(-1){}
Server::Server(ServerConfig& config):_config(config), _serverFd(-1){}
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
		_config = other._config;
		_serverFd = other._serverFd;
	}
	return (*this);
}

void	Server::createServer()
{
	int	opt = 1;

	_serverFd = socket(AF_INET, SOCK_STREAM, 0);
	if (_serverFd == -1)
	{
		std::cerr << "send errno: " << errno << std::endl;
		std::cerr << "send error: " << std::strerror(errno) << std::endl;
		throw std::runtime_error("socket() failed.");
	}
	if (setsockopt(_serverFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
	{
		std::cerr << "send errno: " << errno << std::endl;
		std::cerr << "send error: " << std::strerror(errno) << std::endl;
		throw std::runtime_error("setsockopt() failed.");
	}
}
void	Server::bindServer()
{
	struct sockaddr_in	address;

	memset(&address, 0, sizeof(address));
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(_config.getPort());
	if (bind(_serverFd, reinterpret_cast<struct sockaddr *>(&address), sizeof(address)) == -1)
	{
		std::cerr << "send errno: " << errno << std::endl;
		std::cerr << "send error: " << std::strerror(errno) << std::endl;
		throw std::runtime_error("bind() failed.");
	}
}

void	Server::listenServer()
{
	if (listen(_serverFd, 128) == -1)
	{
		std::cerr << "send errno: " << errno << std::endl;
		std::cerr << "send error: " << std::strerror(errno) << std::endl;
		throw std::runtime_error("listen() failed.");
	}
}
void	Server::handleClient(int clientFd)
{
	std::string	rawRequest;
	char	buffer[4096];

	while (true)
	{
		memset(buffer, 0, sizeof(buffer));
		int byteread = recv(clientFd, buffer, sizeof(buffer), 0);
		if (byteread == -1)
		{
			std::cerr << "send errno: " << errno << std::endl;
			std::cerr << "send error: " << std::strerror(errno) << std::endl;
			close(clientFd);
			throw std::runtime_error("rev() failed.");
		}
		if (byteread == 0)
			break ;
		rawRequest.append(buffer, byteread);
		if (rawRequest.find("\r\n\r\n") != std::string::npos)
			break ;
	}

	HttpResponse response;

	try
	{
		HttpRequest request(rawRequest);
		Router router(_config);

		response = router.handleRequest(request);

	}
	catch (const std::exception& e)
	{
		response.setStatus(400);
		response.setHeader("Content-Type", "text/plain");
		response.setBody(std::string("Bad Request: ") + e.what() + "\n");
	}

	std::string	Rawresponse = response.toString();
	if (send(clientFd, Rawresponse.c_str(), Rawresponse.size(), 0) == -1)
	{
		std::cerr << "send errno: " << errno << std::endl;
		std::cerr << "send error: " << std::strerror(errno) << std::endl;
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
		{
			std::cerr << "send errno: " << errno << std::endl;
			std::cerr << "send error: " << std::strerror(errno) << std::endl;
			throw std::runtime_error("accept() failed.");
		}
		handleClient(clientFd);
	}
}

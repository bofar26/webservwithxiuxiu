/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xzhen <xzhen@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/09 17:00:01 by mipang            #+#    #+#             */
/*   Updated: 2026/08/10 11:46:33 by xzhen            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
#define SERVER_HPP

#include <string>
#include <map>
#include <vector>
#include <poll.h>

#include "ServerConfig.hpp"
#include "Connection.hpp"


class Server{
	private:
	std::vector<ServerConfig>	_configs;//_configs[i]
	std::map<int, size_t>		_listenFds;//key=_listenFds[fd] value=_configs[index];
	std::map<int, Connection *>	_connections;//this client fd corresponds to which client connection, _connections[fd] = Connection[index];
	std::vector<struct pollfd>	_pollFds;//all fds to poll, including listening fds and client fds
	
	//copying a Server is not allowed, fds couldn't be copied
	Server(const Server& other);
	Server& operator=(const Server& other);

	int		createListenSocket(int port);//will be called in setListenSocket
	void	setListenSocket();

	void	buildPollSet();//put all fds into poll()
	void	dispatch();//decide what to do with each fd, accept if a listen fd, connection if a client fd
	void	acceptNew(int listenFd);
	void	closeConnection(int fd);
	void	cleanConnectionInactive();

	public:
	Server();
	Server(const std::vector<ServerConfig>& configs);
	~Server();

	void	start();
};

#endif

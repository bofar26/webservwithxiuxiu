/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mipang <mipang@student.42.fr>              #+#  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026-07-09 17:00:01 by mipang            #+#    #+#             */
/*   Updated: 2026-07-09 17:00:01 by mipang           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
#define SERVER_HPP

class Server{
	private:
	int	_port;
	int	_serverFd;

	void	createServer();
	void	bindServer();
	void	listenServer();
	void	handleClient(int clientFd);

	public:
	Server();
	Server(int port);
	~Server();
	Server(const Server& other);
	Server& operator=(const Server& other);

	void	start();
};

#endif

/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mipang <mipang@student.42.fr>              #+#  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026-07-23 16:23:04 by mipang            #+#    #+#             */
/*   Updated: 2026-07-23 16:23:04 by mipang           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVERCONFIG_HPP
#define SERVERCONFIG_HPP

#include "LocationConfig.hpp"
#include <string>

class ServerConfig{
	private:
	int	_port;
	std::string	_root;
	std::string	_index;
	std::vector<LocationConfig> _locations;
	public:
	ServerConfig();
	~ServerConfig();
	ServerConfig(const ServerConfig& other);
	ServerConfig& operator=(const ServerConfig& other);
	void	setPort(int port);
	void	setRoot(std::string root);
	void	setIndex(std::string index);
	void	addLocation(const LocationConfig& _location);

	int	getPort() const;
	std::string	getRoot() const;
	std::string	getIndex() const;
	const std::vector<LocationConfig>& getLocation() const;

};

# endif

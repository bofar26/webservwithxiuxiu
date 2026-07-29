/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mipang <mipang@student.42.fr>              #+#  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026-07-24 14:52:26 by mipang            #+#    #+#             */
/*   Updated: 2026-07-24 14:52:26 by mipang           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ServerConfig.hpp"


ServerConfig::ServerConfig():_port(8080),_root("./www"),_index("index.html"){}
ServerConfig::~ServerConfig(){};
ServerConfig::ServerConfig(const ServerConfig& other)
{
	*this = other;
}
ServerConfig& ServerConfig::operator=(const ServerConfig& other)
{
	if (this != &other)
	{
		_port = other._port;
		_root = other._root;
		_index = other._index;
		_locations = other._locations;
	}
	return (*this);
}
void	ServerConfig::setPort(int port)
{
	_port = port;
}
void	ServerConfig::setRoot(std::string root)
{
	_root = root;
}
void	ServerConfig::setIndex(std::string index)
{
	_index = index;
}

void	ServerConfig::addLocation(const LocationConfig& location)
{
	_locations.push_back(location);
}

int	ServerConfig::getPort() const
{
	return (_port);
}

std::string	ServerConfig::getRoot() const
{
	return (_root);
}

std::string	ServerConfig::getIndex() const
{
	return (_index);
}

const std::vector<LocationConfig>& ServerConfig::getLocation() const
{
	return (_locations);
}


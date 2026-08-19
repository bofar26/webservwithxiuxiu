/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xzhen <xzhen@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/24 14:52:26 by mipang            #+#    #+#             */
/*   Updated: 2026/08/12 16:56:48 by xzhen            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ServerConfig.hpp"


ServerConfig::ServerConfig()
	: _host("0.0.0.0"), _port(8080), _root("./www"), _index("index.html"), _locations(),
	  _clientMaxBodySize(1048576), _errorPages(){}//1 MiB by default
ServerConfig::~ServerConfig(){};
ServerConfig::ServerConfig(const ServerConfig& other)
{
	*this = other;
}
ServerConfig& ServerConfig::operator=(const ServerConfig& other)
{
	if (this != &other)
	{
		_host = other._host;
		_port = other._port;
		_root = other._root;
		_index = other._index;
		_locations = other._locations;
		_clientMaxBodySize = other._clientMaxBodySize;
		_errorPages = other._errorPages;
	}
	return (*this);
}
void	ServerConfig::setPort(int port)
{
	_port = port;
}

void	ServerConfig::setHost(const std::string& host)
{
	_host = host;
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

std::string	ServerConfig::getHost() const
{
	return (_host);
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

void	ServerConfig::setClientMaxBodySize(std::size_t size)//
{
	_clientMaxBodySize = size;
}

std::size_t	ServerConfig::getClientMaxBodySize() const//
{
	return (_clientMaxBodySize);
}

void	ServerConfig::addErrorPage(int code, const std::string& path)//
{
	_errorPages[code] = path;
}

std::string	ServerConfig::getErrorPage(int code) const//
{
	std::map<int, std::string>::const_iterator	it = _errorPages.find(code);

	if (it == _errorPages.end())
		return ("");
	return (it->second);
}

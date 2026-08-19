/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xzhen <xzhen@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/15 13:20:03 by mipang            #+#    #+#             */
/*   Updated: 2026/08/12 16:43:49 by xzhen            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVERCONFIG_HPP
#define SERVERCONFIG_HPP

#include "LocationConfig.hpp"
#include <string>
#include <map>
#include <cstddef>

//new:
//private: _clientMaxBodySize + _errorPages
//setClientMaxBodySize 
//getClientMaxBodySize 
//addErrorPage 
//getErrorPage

class ServerConfig{
	private:
	std::string					_host;
	int							_port;
	std::string					_root;
	std::string					_index;
	std::vector<LocationConfig>	_locations;
	std::size_t					_clientMaxBodySize;//biggest request body accepted
	std::map<int, std::string>	_errorPages;//status code -> file to serve

	public:
	ServerConfig();
	~ServerConfig();
	ServerConfig(const ServerConfig& other);
	ServerConfig& operator=(const ServerConfig& other);

	void	setPort(int port);
	void	setHost(const std::string& host);
	void	setRoot(std::string root);
	void	setIndex(std::string index);
	void	addLocation(const LocationConfig& _location);
	void	setClientMaxBodySize(std::size_t size);//
	void	addErrorPage(int code, const std::string& path);//

	std::string	getHost() const;
	int			getPort() const;
	std::string	getRoot() const;
	std::string	getIndex() const;
	const std::vector<LocationConfig>&	getLocation() const;
	std::size_t	getClientMaxBodySize() const;//
	std::string	getErrorPage(int code) const;//
};

# endif

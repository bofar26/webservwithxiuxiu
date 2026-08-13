/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   LocationConfig.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xzhen <xzhen@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/27 15:37:18 by mipang            #+#    #+#             */
/*   Updated: 2026/08/12 15:56:42 by xzhen            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "LocationConfig.hpp"

LocationConfig::LocationConfig()
	: _path("/"), _root("./www"), _index("index.html"),
	  _allowedMethods(), _autoindex(false), _uploadStore(""), _redirect(""){}

LocationConfig::~LocationConfig(){}

LocationConfig::LocationConfig(const LocationConfig& other){
	*this = other;
}

LocationConfig& LocationConfig::operator=(const LocationConfig& other)
{
	if (this != &other)
	{
		_path = other._path;
		_root = other._root;
		_index = other._index;
		_allowedMethods = other._allowedMethods;
		_autoindex = other._autoindex;
		_uploadStore = other._uploadStore;
		_redirect = other._redirect;
	}
	return (*this);
}

std::string LocationConfig::getPath() const
{
	return (_path);
}
std::string LocationConfig::getRoot() const
{
	return (_root);
}
std::string LocationConfig::getIndex() const
{
	return (_index);
}
const std::vector<std::string>&	LocationConfig::getAllowedMethods() const
{
	return (_allowedMethods);
}
bool	LocationConfig::getAutoindex() const
{
	return (_autoindex);
}
std::string	LocationConfig::getUploadStore() const
{
	return (_uploadStore);
}
std::string	LocationConfig::getRedirect() const
{
	return (_redirect);
}

void	LocationConfig::setPath(const std::string& path)
{
	_path = path;
}
void	LocationConfig::setRoot(const std::string& root)
{
	_root = root;
}
void	LocationConfig::setIndex(const std::string& index)
{
	_index = index;
}
void	LocationConfig::setAllowedMethods(const std::vector<std::string>& methods)
{
	_allowedMethods = methods;
}
void	LocationConfig::setAutoindex(bool autoindex)
{
	_autoindex = autoindex;
}
void	LocationConfig::setUploadStore(const std::string& uploadStore)
{
	_uploadStore = uploadStore;
}
void	LocationConfig::setRedirect(const std::string& redirect)
{
	_redirect = redirect;
}

bool	LocationConfig::isMethodAllowed(const std::string& method) const{
	if (_allowedMethods.empty())
		return (true);//allow all methods by default
	for (size_t i = 0; i < _allowedMethods.size(); i++){
		if (_allowedMethods[i] == method)
			return (true);
	}
	return (false);
}

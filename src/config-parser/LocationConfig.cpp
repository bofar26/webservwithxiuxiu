/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   LocationParser.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mipang <mipang@student.42.fr>              #+#  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026-07-27 15:37:18 by mipang            #+#    #+#             */
/*   Updated: 2026-07-27 15:37:18 by mipang           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "LocationParser.hpp"

LocationConfig::LocationConfig():_path("/"), _root("./www"), _index("index.html"){}
LocationConfig::~LocationConfig(){}
LocationConfig::LocationConfig(const LocationConfig& other)
{
	*this = other;
}
LocationConfig& LocationConfig::operator=(const LocationConfig& other)
{
	if (this != &other)
	{
		_path = other._path;
		_root = other._root;
		_index = other._index;
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

/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RouterFileUtils.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xzhen <xzhen@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/15 12:26:44 by mipang            #+#    #+#             */
/*   Updated: 2026/08/13 13:27:30 by xzhen            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Router.hpp"

bool	Router::fileExists(const std::string& filePath) const
{
	struct stat	info;

	return (stat(filePath.c_str(), &info) == 0 && S_ISREG(info.st_mode));
}

std::string	Router::buildFilePath(const std::string& requestPath) const
{
	std::string	root;
	std::string	index;
	const LocationConfig* location;
	std::string	relativePath;

	root = _config.getRoot();
	index = _config.getIndex();
	location = findLocation(requestPath);
	relativePath = requestPath;

	if (location != NULL)
	{
		if (location->getRoot() != "")
			root = location->getRoot();
		if (location->getIndex() != "")
			index = location->getIndex();

		relativePath = relativePath.substr(location->getPath().length());
		if (relativePath == "")
			relativePath = "/";
	}
	if (relativePath == "/")
		return (root + "/" + index);
	return (root + relativePath);
}

//find/calcul the real directory
std::string	Router::buildDirPath(const std::string& requestPath) const
{
	std::string				root = _config.getRoot();
	const LocationConfig	*location = findLocation(requestPath);
	std::string				relativePath = requestPath;

	if (location != NULL)
	{
		if (location->getRoot() != "")
			root = location->getRoot();
		relativePath = relativePath.substr(location->getPath().length());
	}
	if (relativePath == "" || relativePath == "/")
		return (root);
	return (root + relativePath);
}

//write file to disk/upload_store
bool	Router::writeFile(const std::string& filePath, const std::string& data) const
{											//to write		//in binary		//delete if there's already a file
	std::ofstream	file(filePath.c_str(), std::ios::out | std::ios::binary | std::ios::trunc);
	//if failed to open file
	if (!file.is_open())
		return (false);
	//write filedata in file 
	file.write(data.c_str(), static_cast<std::streamsize>(data.size()));
	return (file.good());
}

std::string	Router::readFile(const std::string& filePath) const
{
	std::ifstream file(filePath.c_str(), std::ios::in | std::ios::binary);
	std::ostringstream	content;

	if (!file.is_open())
		throw std::runtime_error("Could not open file\n");
	content << file.rdbuf();
	return (content.str());
}

std::string Router::getContentType(const std::string& filePath) const
{
	if (filePath.size() >= 5 && filePath.substr(filePath.size() - 5) == ".html")
		return ("text/html");

	if (filePath.size() >= 4 && filePath.substr(filePath.size() - 4) == ".css")
		return ("text/css");

	if (filePath.size() >= 3 && filePath.substr(filePath.size() - 3) == ".js")
		return ("application/javascript");

	if (filePath.size() >= 4 && filePath.substr(filePath.size() - 4) == ".png")
		return ("image/png");

	if (filePath.size() >= 4 && filePath.substr(filePath.size() - 4) == ".jpg")
		return ("image/jpeg");

	if (filePath.size() >= 5 && filePath.substr(filePath.size() - 5) == ".jpeg")
		return ("image/jpeg");

	return ("text/plain");
}

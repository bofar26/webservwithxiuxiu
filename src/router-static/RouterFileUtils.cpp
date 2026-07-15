/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RouterFileUtils.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mipang <mipang@student.42.fr>              #+#  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026-07-15 12:26:44 by mipang            #+#    #+#             */
/*   Updated: 2026-07-15 12:26:44 by mipang           ###   ########.fr       */
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
	std::string root;

	root = "./www";
	if (requestPath == "/")
		return (root + "/index.html");
	return (root + requestPath);
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

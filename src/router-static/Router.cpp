/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Router.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mipang <mipang@student.42.fr>              #+#  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026-07-12 16:48:59 by mipang            #+#    #+#             */
/*   Updated: 2026-07-12 16:48:59 by mipang           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Router.hpp"
#include <sys/stat.h>
#include <fstream>
#include <stdexcept>

Router::Router(){}

Router::~Router(){}

HttpResponse	Router::handleRequest(const HttpRequest& request) const
{
	if (!isSupportedVersion(request))
		return (buildTextResponse(505, "HTTP Version Not Supported\n"));
	if (!isSupportedMethod(request))
		return (buildTextResponse(405, "Method Not Allowed\n"));
	if (request.getMethod() == "GET")
		return (handleGet(request));
	return (buildTextResponse(404, "Not Found\n"));
}

HttpResponse	Router::buildTextResponse(int statusCode, const std::string& body) const
{
	HttpResponse	response;

	response.setStatus(statusCode);
	response.setHeader("Content-Type", "text/plain");
	response.setBody(body);

	return (response);
}

bool	Router::isSupportedVersion(const HttpRequest& request) const
{
	return (request.getVersion() == "HTTP/1.1");
}

bool	Router::isSupportedMethod(const HttpRequest& request) const
{
	return (request.getMethod() == "GET" || request.getMethod() == "POST" || request.getMethod() == "DELETE");
}

bool	Router::fileExists(const std::string& filePath) const
{
	struct stat	info;

	return (stat(filePath.c_str(), &info) == 0 && S_ISREG(info.st_mode));
}

HttpResponse Router::handleGet(const HttpRequest& request) const
{
	std::string		filePath;

	filePath = buildFilePath(request.getPath());
	if (!fileExists(filePath))
		return (buildTextResponse(404, "Not Found\n"));
	return (buildFileResponse(filePath));
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

HttpResponse	Router::buildFileResponse(const std::string& filePath) const
{
	HttpResponse response;

	response.setStatus(200);
	response.setHeader("Content-Type", getContentType(filePath));
	response.setBody(readFile(filePath));

	return (response);
}


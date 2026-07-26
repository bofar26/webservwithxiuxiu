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

Router::Router():_root(), _index(){}
Router::Router(std::string root, std::string index):_root(root),_index(index){}
Router::~Router(){}

HttpResponse	Router::handleRequest(const HttpRequest& request) const
{
	if (!isSupportedVersion(request))
		return (buildTextResponse(505, "HTTP Version Not Supported\n"));
	if (!isSupportedMethod(request))
		return (buildTextResponse(405, "Method Not Allowed\n"));
	if (request.getMethod() == "GET")
	{
		std::cout << "METHOD = [" << request.getMethod() << "]" << std::endl;
		return (handleGet(request));
	}
	if (request.getMethod() == "DELETE")
	{
		std::cout << "METHOD = [" << request.getMethod() << "]" << std::endl;
		return (handleDelete(request));
	}
	return (buildTextResponse(404, "Not Found\n"));
}

bool	Router::isSupportedVersion(const HttpRequest& request) const
{
	return (request.getVersion() == "HTTP/1.1");
}

bool	Router::isSupportedMethod(const HttpRequest& request) const
{
	return (request.getMethod() == "GET" || request.getMethod() == "POST" || request.getMethod() == "DELETE");
}

bool	Router::isDirectory(const std::string& filePath) const
{
	struct stat	info;

	if (stat(filePath.c_str(), &info) != 0)
		return (false);
	return (S_ISDIR(info.st_mode));
}

HttpResponse	Router::buildTextResponse(int statusCode, const std::string& body) const
{
	HttpResponse	response;

	response.setStatus(statusCode);
	response.setHeader("Content-Type", "text/plain");
	response.setBody(body);

	return (response);
}

std::string	Router::getRoot() const
{
	return(_root);
}

std::string	Router::getIndex() const
{
	return(_index);
}




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

Router::Router(){}

Router::~Router(){}

HttpResponse	Router::handleRequest(const HttpRequest& request) const
{
	if (!isSupportedVersion(request))
		return (buildTextResponse(505, "HTTP Version Not Supported\n"));
	if (!isSupportedMethod(request))
		return (buildTextResponse(405, "Method Not Allowed\n"));
	if (request.getMethod() == "GET" && request.getPath() == "/") //need to work on path after
		return (buildTextResponse(200, "Welcome to webserv\n"));
	return (buildTextResponse(404, "Not Found\n"));
}

HttpResponse	Router::buildTextResponse(int statusCode, const std::string& body) const
{
	HttpResponse	reponse;

	reponse.setStatus(statusCode);
	reponse.setHeader("Content-Type", "text/plain");
	reponse.setBody(body);

	return (reponse);
}

bool	Router::isSupportedVersion(const HttpRequest& request) const
{
	return (request.getVersion() == "HTTP/1.1");
}

bool	Router::isSupportedMethod(const HttpRequest& request) const
{
	return (request.getMethod() == "GET" || request.getMethod() == "POST" || request.getMethod() == "DELETE");
}



/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RouterStatic.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mipang <mipang@student.42.fr>              #+#  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026-07-15 12:27:51 by mipang            #+#    #+#             */
/*   Updated: 2026-07-15 12:27:51 by mipang           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Router.hpp"

HttpResponse Router::handleGet(const HttpRequest& request) const
{
	std::string		filePath;

	filePath = buildFilePath(request.getPath());
	if (!fileExists(filePath))
		return (buildTextResponse(404, "Not Found\n"));
	return (buildFileResponse(filePath));
}

HttpResponse Router::handleDelete(const HttpRequest& request) const
{
	HttpResponse	response;
	std::string		filePath;

	filePath = buildFilePath(request.getPath());
	if (isDirectory(filePath))
		return (buildTextResponse(403, "Forbidden\n"));
	if (!fileExists(filePath))
		return (buildTextResponse(404, "Not Found\n"));
	if (unlink(filePath.c_str()) != 0)
		return (buildTextResponse(500, "Internal Server Error\n"));

	response.setStatus(204);
	response.setBody("");
	return (response);
}

HttpResponse	Router::buildFileResponse(const std::string& filePath) const
{
	HttpResponse response;

	response.setStatus(200);
	response.setHeader("Content-Type", getContentType(filePath));
	response.setBody(readFile(filePath));

	return (response);
}

/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Router.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xzhen <xzhen@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/12 16:39:54 by mipang            #+#    #+#             */
/*   Updated: 2026/08/12 18:08:45 by xzhen            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef ROUTER_HPP
#define ROUTER_HPP

#include "HttpResponse.hpp"
#include "HttpRequest.hpp"
#include "ServerConfig.hpp"
#include <sys/stat.h>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <unistd.h>
#include <dirent.h>//opendir / readdir / closedir, used by autoindex

class Router{
	private:
	ServerConfig	_config;

	bool	isSupportedVersion(const HttpRequest& request) const;
	bool	isSupportedMethod(const HttpRequest& request) const;
	bool	isDirectory(const std::string& filePath) const;
	bool	isPathSafe(const std::string& requestPath) const;//new

	bool	fileExists(const std::string& filePath) const;
	std::string	buildFilePath(const std::string& requestPath) const;
	std::string	readFile(const std::string& filePath) const;
	std::string getContentType(const std::string& filePath) const;

	HttpResponse handleGet(const HttpRequest& request) const;
	HttpResponse handleDelete(const HttpRequest& request) const;
	//saves the request body into the directory named by upload_store
	HttpResponse handlePost(const HttpRequest& request) const;
	//multipart/form-data: pull the file content out of the browser envelope
	bool	extractMultipart(const std::string& body, const std::string& contentType,
				std::string& fileName, std::string& fileData) const;
	//writes data to disk, returns false when the file cannot be created
	bool	writeFile(const std::string& filePath, const std::string& data) const;

	HttpResponse	buildFileResponse(const std::string& filePath) const;
	HttpResponse	buildTextResponse(int statusCode, const std::string& body) const;
	//Response when error
	HttpResponse	buildErrorResponse(int statusCode) const;
	//
	HttpResponse	buildRedirectResponse(const std::string& target) const;
	//HTML page listing a directory, used when autoindex is on
	HttpResponse	buildAutoindexResponse(const std::string& dirPath,
						const std::string& urlPath) const;
	//same mapping as buildFilePath but stops before appending the index file
	std::string		buildDirPath(const std::string& requestPath) const;
	const	LocationConfig* findLocation(const std::string& path) const;

	public:
	Router();
	Router(ServerConfig& config);
	~Router();
	ServerConfig	getConfig() const;
	HttpResponse handleRequest(const HttpRequest& request) const;
	//Connection asks this before calling handleRequest: a CGI cannot be
	//answered on the spot, it has to be started and polled like a socket.
	//Fills interpreter / scriptPath / queryString and returns true when the
	//URL maps to a script the config declared with cgi_ext.
	bool	isCgiRequest(const HttpRequest& request, std::string& interpreter,
				std::string& scriptPath, std::string& queryString) const;
};

# endif

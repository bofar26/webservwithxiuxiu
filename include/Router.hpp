/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Router.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mipang <mipang@student.42.fr>              #+#  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026-07-12 16:39:54 by mipang            #+#    #+#             */
/*   Updated: 2026-07-12 16:39:54 by mipang           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef ROUTER_HPP
#define ROUTER_HPP

#include "HttpResponse.hpp"
#include "HttpRequest.hpp"
#include "ServerConfig.hpp"
#include <sys/stat.h>
#include <fstream>
#include <stdexcept>
#include <unistd.h>

class Router{
	private:
	ServerConfig	_config;

	bool	isSupportedVersion(const HttpRequest& request) const;
	bool	isSupportedMethod(const HttpRequest& request) const;
	bool	isDirectory(const std::string& filePath) const;

	bool	fileExists(const std::string& filePath) const;
	std::string	buildFilePath(const std::string& requestPath) const;
	std::string	readFile(const std::string& filePath) const;
	std::string getContentType(const std::string& filePath) const;

	HttpResponse handleGet(const HttpRequest& request) const;
	HttpResponse handleDelete(const HttpRequest& request) const;

	HttpResponse	buildFileResponse(const std::string& filePath) const;
	HttpResponse	buildTextResponse(int statusCode, const std::string& body) const;
	const	LocationConfig* findLocation(const std::string& path) const;

	public:
	Router();
	Router(ServerConfig& config);
	~Router();
	Router(std::string root, std::string index);
	ServerConfig	getConfig() const;
	HttpResponse handleRequest(const HttpRequest& request) const;
};

# endif

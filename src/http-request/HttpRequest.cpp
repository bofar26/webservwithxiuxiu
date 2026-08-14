/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mipang <mipang@student.42.fr>              #+#  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026-07-11 15:25:41 by mipang            #+#    #+#             */
/*   Updated: 2026-07-11 15:25:41 by mipang           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpRequest.hpp"
#include <sstream>
#include <stdexcept>
#include <iostream>
#include <cctype>

HttpRequest::HttpRequest():_method(""), _path(""), _version(""), _headers(), _body(""){}
HttpRequest::HttpRequest(const std::string& rawRequest):_method(""), _path(""), _version(""), _headers(), _body("")
{
	parse(rawRequest);
}
HttpRequest::~HttpRequest(){}
HttpRequest::HttpRequest(const HttpRequest& other)
{
	*this = other;
}
HttpRequest& HttpRequest::operator=(const HttpRequest& other)
{
	if (this != &other)
	{
		_method = other._method;
		_path = other._path;
		_version = other._version;
		_headers = other._headers;
		_body = other._body;
	}
	return (*this);
}

std::string HttpRequest::getMethod() const
{
	return (_method);
}

std::string HttpRequest::getPath() const
{
	return (_path);
}

std::string HttpRequest::getVersion() const
{
	return (_version);
}

//RFC 7230: header names are case insensitive. parse() stores every name in
//lower case, so the lookup lowers the key too and callers can keep writing
//getHeader("Content-Type") or getHeader("content-type") as they prefer.
static std::string	lowerKey(const std::string& text)
{
	std::string	result = text;

	for (size_t i = 0; i < result.size(); i++)
		result[i] = std::tolower(static_cast<unsigned char>(result[i]));
	return (result);
}

std::string HttpRequest::getHeader(const std::string key) const
{
	std::map<std::string, std::string>::const_iterator it = _headers.find(lowerKey(key));
	if (it == _headers.end())
		return ("");
	return (it->second);
}

std::string HttpRequest::getBody() const
{
	return (_body);
}

void	HttpRequest::parse(const std::string& rawRequest)
{
	std::size_t	headerend = rawRequest.find("\r\n\r\n");
	if (headerend == std::string::npos)
		throw std::runtime_error("Bad request: headers not complete");
	std::string headerpart = rawRequest.substr(0, headerend);
	_body = rawRequest.substr(headerend + 4);

	std::istringstream	stream(headerpart);
	std::string	line;

	if (!std::getline(stream, line))
		throw std::runtime_error("Bad request: empty request");
	if (!line.empty() && line[line.size() - 1] == '\r')
		line.erase(line.size() - 1);
	std::istringstream	Requestline(line);
	if (!(Requestline >> _method >> _path >> _version))
		throw std::runtime_error("Bad request: invalid request line");

	while (std::getline(stream, line))
	{
		if (!line.empty() && line[line.size() - 1] == '\r')
			line.erase(line.size() - 1);
		size_t split = line.find(':');
		if (split == std::string::npos)
			throw std::runtime_error("Bad request: invalid header");
		std::string key = line.substr(0, split);
		std::string value = line.substr(split + 1);

		//the value may be padded with several spaces or tabs, not just one
		size_t	start = value.find_first_not_of(" \t");
		if (start == std::string::npos)
			value = "";
		else
		{
			size_t	end = value.find_last_not_of(" \t");
			value = value.substr(start, end - start + 1);
		}
		_headers[lowerKey(key)] = value;//stored lowered, see getHeader()
	}
}

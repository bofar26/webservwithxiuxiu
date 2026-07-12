/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mipang <mipang@student.42.fr>              #+#  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026-07-09 13:19:10 by mipang            #+#    #+#             */
/*   Updated: 2026-07-09 13:19:10 by mipang           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpResponse.hpp"

HttpResponse::HttpResponse()
	: _statusCode(200), _statusText("OK"), _body("")
{
	_headers["Server"] = "webserv/1.0";
	_headers["Connection"] = "close";
	_headers["Content-Type"] = "text/plain";
}

HttpResponse::~HttpResponse(){}

HttpResponse::HttpResponse(const HttpResponse& other)
{
	*this = other;
}
HttpResponse& HttpResponse::operator=(const HttpResponse& other)
{
	if (this != &other)
	{
		_statusCode = other._statusCode;
		_statusText = other._statusText;
		_headers = other._headers;
		_body = other._body;
	}
	return (*this);
}

int	HttpResponse::getStatusCode() const
{
	return (_statusCode);
}

std::string	HttpResponse::getStatusText() const
{
	return (_statusText);
}

std::string	HttpResponse::getHeader(const std::string& key) const
{
	std::map<std::string, std::string>::const_iterator it = _headers.find(key);
	if (it == _headers.end())
		return ("");
	return (it->second);
}

std::string	HttpResponse::getBody() const
{
	return (_body);
}

void HttpResponse::setStatus(int code)
{
	_statusCode = code;
	_statusText = resolveStatusText(code);
}

void HttpResponse::setHeader(const std::string& key, const std::string& value)
{
	_headers[key] = value;
}

void HttpResponse::setBody(const std::string& body)
{
	_body = body;

	std::ostringstream oss;
	oss << _body.size();
	_headers["Content-Length"] = oss.str();
}

void HttpResponse::setDefaultErrorPage(int code)
{
	setStatus(code);

	std::ostringstream html;
	html << "<html><head><title>" << code << " " << _statusText << "</title></head>"
		<< "<body><center><h1>" << code << " " << _statusText << "</h1></center>"
		<< "<hr><center>webserv</center></body></html>";

	setHeader("Content-Type", "text/html");
	setBody(html.str());
}

std::string HttpResponse::toString() const
{
	std::ostringstream response;

	response << "HTTP/1.1 " << _statusCode << " " << _statusText << "\r\n";

	for (std::map<std::string, std::string>::const_iterator it = _headers.begin();
		it != _headers.end(); ++it)
	{
		response << it->first << ": " << it->second << "\r\n";
	}

	response << "\r\n";

	response << _body;

	return response.str();
}

std::string HttpResponse::resolveStatusText(int code) const
{
	switch (code)
	{
		case 200: return "OK";
		case 201: return "Created";
		case 204: return "No Content";
		case 301: return "Moved Permanently";
		case 302: return "Found";
		case 400: return "Bad Request";
		case 403: return "Forbidden";
		case 404: return "Not Found";
		case 405: return "Method Not Allowed";
		case 413: return "Payload Too Large";
		case 500: return "Internal Server Error";
		case 501: return "Not Implemented";
		case 505: return "HTTP Version Not Supported";
		default:  return "Unknown";
	}
}

#ifndef HTTPRESPONSE_HPP
#define HTTPRESPONSE_HPP

#include <string>
#include <map>
#include <sstream>
#include <iostream>


class HttpResponse
{
	private:
	int _statusCode;
	std::string _statusText;
	std::map<std::string, std::string> _headers;
	std::string	_body;
	std::string resolveStatusText(int code) const;

	public:
	HttpResponse();
	~HttpResponse();
	HttpResponse(const HttpResponse& other);
	HttpResponse& operator=(const HttpResponse& other);
	int	getStatusCode() const;
	std::string	getStatusText() const;
	std::string	getHeader(const std::string& key) const;
	std::string	getBody() const;
	void setStatus(int code);
	void setHeader(const std::string& key, const std::string& value);
	void setBody(const std::string& body);
	void setDefaultErrorPage(int code);

	std::string toString() const;
};

#endif

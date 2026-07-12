/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mipang <mipang@student.42.fr>              #+#  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026-07-11 15:25:21 by mipang            #+#    #+#             */
/*   Updated: 2026-07-11 15:25:21 by mipang           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPREQUEST_HPP
#define HTTPREQUEST_HPP

#include <string>
#include <map>

class HttpRequest {
	private:
	std::string	_method;
	std::string	_path;
	std::string	_version;
	std::map<std::string, std::string> _headers;
	std::string	_body;
	public:
	HttpRequest();
	HttpRequest(const std::string& rawRequest);
	~HttpRequest();
	HttpRequest(const HttpRequest& other);
	HttpRequest& operator=(const HttpRequest& other);
	std::string	getMethod() const;
	std::string	getPath() const;
	std::string	getVersion() const;
	std::string	getHeader(const std::string key) const;
	std::string	getBody() const;

	void	parse(const std::string& rawRequest);
};

# endif

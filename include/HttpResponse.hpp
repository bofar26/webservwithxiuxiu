/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mipang <mipang@student.42.fr>              #+#  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026-07-08 13:24:28 by mipang            #+#    #+#             */
/*   Updated: 2026-07-08 13:24:28 by mipang           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPRESPONSE_HPP
#define HTTPRESPONSE_HPP

#include <string>
#include <iostream>
#include <sstream>

class HttpResponse{
	private:
	int _statusCode;
	std::string _statusMessage;
	std::string _contentType;
	std::string _body;
	public:
	HttpResponse();
	~HttpResponse();
	HttpResponse(const HttpResponse& other);
	HttpResponse& operator=(const HttpResponse &other);

	int getStatusCode() const;
	std::string getStatusMessage() const;
	std::string getContentType() const;
	std::string getBody() const;
	void setStatus(int statusCode, const std::string& statusMessage);
	void setContentType(const std::string& contentType);
	void setBody(const std::string& body);

	std::string toString() const;
};

#endif

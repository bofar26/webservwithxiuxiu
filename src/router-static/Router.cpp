/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Router.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xzhen <xzhen@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/12 16:48:59 by mipang            #+#    #+#             */
/*   Updated: 2026/08/13 12:37:40 by xzhen            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Router.hpp"
#include <sstream>

Router::Router():_config(){}
Router::Router(ServerConfig& config):_config(config){}
Router::~Router(){}

//decide how to manage the request received
HttpResponse	Router::handleRequest(const HttpRequest& request) const
{
	if (!isPathSafe(request.getPath()))//a safe path?
		return (buildErrorResponse(400));
	if (!isSupportedVersion(request))//a correct version?
		return (buildErrorResponse(505));
	if (!isSupportedMethod(request))//a method supported?
		return (buildErrorResponse(405));
								//find the real location in request
	const LocationConfig	*location = findLocation(request.getPath());
	//a shunt
	//redirect:"don't sereach something here but otherwhere": location{return /new}
	//if there's a "return" in this location
	if (location != NULL && location->getRedirect() != "")
		return (buildRedirectResponse(location->getRedirect()));//return(buildRedirectResponse("/new"));
	//405 method not allowed
	if (location != NULL && !location->isMethodAllowed(request.getMethod()))
		return (buildErrorResponse(405));
	if (request.getMethod() == "GET")//3 methods
		return (handleGet(request));
	if (request.getMethod() == "DELETE")
		return (handleDelete(request));
	if (request.getMethod() == "POST")
		return (handlePost(request));
	return (buildErrorResponse(404));
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

//generate a response when receive 404,400,405
HttpResponse	Router::buildErrorResponse(int statusCode) const
{	//there's a Errorpage default in config maybe
	std::string	page = _config.getErrorPage(statusCode);
	//yes there's a Errorpage default in config , 
		//and  this file of errorpage exixts
	if (page != "" && fileExists(page))
	{
		try//just in case of failure to read the file of errorpage
		{
			HttpResponse	response;

			response.setStatus(statusCode);
			response.setHeader("Content-Type", getContentType(page));
			response.setBody(readFile(page));
			return (response);
		}
		catch (const std::exception&)
		{}
	}

	HttpResponse	response;//error response default

	response.setStatus(statusCode);
	response.setHeader("Content-Type", "text/plain");
	response.setBody(response.getStatusText() + "\n");
	return (response);
}

//301: tell the server to go anther URL 
HttpResponse	Router::buildRedirectResponse(const std::string& target) const
{
	HttpResponse	response;

	response.setStatus(301);
	response.setHeader("Location", target);
	response.setBody("");
	return (response);
}

//read the real directory, generate a html page based on the real filepath
//autoindex on:when there's no page default
HttpResponse	Router::buildAutoindexResponse(const std::string& dirPath,
					const std::string& urlPath) const
{
	DIR	*dir = opendir(dirPath.c_str());

	if (dir == NULL)
		return (buildErrorResponse(403));

	//make sure the prefix ends with exactly one '/' so links are well formed
	std::string	base = urlPath;
	if (base.empty() || base[base.size() - 1] != '/')
		base += "/";

	std::ostringstream	page;
	page << "<!DOCTYPE html><html><head><meta charset=\"utf-8\">"
		<< "<title>Index of " << urlPath << "</title></head><body>"
		<< "<h1>Index of " << urlPath << "</h1><ul>";

	struct dirent	*entry;
	while ((entry = readdir(dir)) != NULL)
	{
		std::string	name = entry->d_name;

		if (name == ".")//"." would just link to the page we are on
			continue ;
		page << "<li><a href=\"" << base << name << "\">" << name << "</a></li>";
	}
	closedir(dir);//readdir keeps an open descriptor, it must be released

	page << "</ul></body></html>";

	HttpResponse	response;
	response.setStatus(200);
	response.setHeader("Content-Type", "text/html");
	response.setBody(page.str());
	return (response);
}

ServerConfig	Router::getConfig() const
{
	return (_config);
}
//
bool	Router::isPathSafe(const std::string& requestPath) const{//new
	std::istringstream stream(requestPath);
	std::string segment;
	int depth=0;
	
	while(std::getline(stream, segment, '/')){
		if((segment == "") || (segment == "."))
			continue;
		else if(segment==".."){
			depth--;
			if(depth<0) 
				return false;
		}
		else
			depth++;
	}
	return true;
}

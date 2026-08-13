/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParser.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xzhen <xzhen@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/15 13:21:19 by mipang            #+#    #+#             */
/*   Updated: 2026/08/12 17:48:09 by xzhen            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ConfigParser.hpp"
#include <stdexcept>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <stdexcept>
#include <cstdlib>


ConfigParser::ConfigParser(){}
ConfigParser::~ConfigParser(){}

std::vector<std::string> ConfigParser::tokenize(const std::string content) const
{
	std::vector<std::string> tokens;
	std::string	current;
	size_t	i = 0;

	while (i < content.size())
	{
		if (isspace(static_cast<unsigned char>(content[i])))
		{
			if (!current.empty())
			{
				tokens.push_back(current);
				current.clear();
			}
		}
		else if (content[i] == '{' || content[i] == '}' || content[i] == ';')
		{
			if (!current.empty())
			{
				tokens.push_back(current);
				current.clear();
			}
			tokens.push_back(std::string(1, content[i]));
		}
		else
		{
			current += content[i];
		}
		i ++;
	}
	if (!current.empty())
		tokens.push_back(current);
	return (tokens);
}

//return the server configs,eg:ServerConfig(port=8080);ServerConfig(port=8081)
std::vector<ServerConfig>	ConfigParser::parseConfigFile(const std::string& filePath) const
{
	std::vector<ServerConfig>	servers;
	size_t						pos = 0;
	std::vector<std::string>	tokens;
	std::ifstream				file(filePath.c_str(), std::ios::in | std::ios::binary);
	std::ostringstream			content;

	if (!file.is_open())
		throw std::runtime_error("Could not open config file: " + filePath);
	content << file.rdbuf();
	tokens = tokenize(content.str());

	while (pos < tokens.size())//one server by one server to analyse
	{
		ServerConfig	config_server;

		parseServerBlock(tokens, pos, config_server);
		servers.push_back(config_server);
	}
	if (servers.empty())
		throw std::runtime_error("No server block found in config file");
	checkDuplicatePorts(servers);//shouldn't listen the same port for each server
	return (servers);
}

void	ConfigParser::parseServerBlock(const std::vector<std::string>& tokens, size_t& pos, ServerConfig& config) const
{
	if (pos >= tokens.size() || tokens[pos] != "server")
		throw std::runtime_error("Expected server block");
	pos ++;
	if (pos >= tokens.size() || tokens[pos] != "{")
		throw std::runtime_error("Expected { after server");
	pos ++;
	while (pos < tokens.size() && tokens[pos] != "}")
		parseInsideBlock(tokens, pos, config);
	if (pos >= tokens.size() || tokens[pos] != "}")
		throw std::runtime_error("Expected } at end of server block");
	pos ++;
}
//analyse config inside server{...}
void	ConfigParser::parseInsideBlock(const std::vector<std::string>& tokens, size_t& pos, ServerConfig& config) const
{
	std::string	directive;

	if (pos >= tokens.size())
		throw std::runtime_error("Unexpected end of config");
	directive = tokens[pos];
	pos ++;
	if (directive == "location")
	{
		parseLocationBlock(tokens, pos, config);
		return;
	}
	if (pos >= tokens.size())
		throw std::runtime_error("Expected value after " + directive);
	if (directive == "listen")
	{
		config.setPort(atoi(tokens[pos].c_str()));
		pos++;
	}
	else if (directive == "root")
	{
		config.setRoot(tokens[pos]);
		pos++;
	}
	else if (directive == "index")
	{
		config.setIndex(tokens[pos]);
		pos++;
	}
	else if (directive == "client_max_body_size")//
	{
		config.setClientMaxBodySize(parseSize(tokens[pos]));
		pos++;
	}
	else if (directive == "error_page")//
	{
		//error_page <code> <file>;
		int	code = atoi(tokens[pos].c_str());

		pos++;
		if (pos >= tokens.size())
			throw std::runtime_error("Expected a file after error_page");
		config.addErrorPage(code, tokens[pos]);
		pos++;
	}
	else
		throw std::runtime_error("Unknown directive in server: " + directive);
	if (pos >= tokens.size() || tokens[pos] != ";")
		throw std::runtime_error("Expected ; after " + directive);
	pos++;
}
//analyse config inside location{...}
void	ConfigParser::parseInsideBlock(const std::vector<std::string>& tokens, size_t& pos, LocationConfig& config) const
{
	std::string	directive;

	if (pos >= tokens.size())
		throw std::runtime_error("Unexpected end of config");
	directive = tokens[pos];
	pos ++;
	if (pos >= tokens.size())
		throw std::runtime_error("Expected value after " + directive);
	if (directive == "root")
	{
		config.setRoot(tokens[pos]);
		pos++;
	}
	else if (directive == "index")
	{
		config.setIndex(tokens[pos]);
		pos++;
	}
	else if (directive == "upload_store")
	{
		config.setUploadStore(tokens[pos]);
		pos++;
	}
	else if (directive == "return")
	{
		config.setRedirect(tokens[pos]);
		pos++;
	}
	else if (directive == "autoindex")//
	{
		if (tokens[pos] != "on" && tokens[pos] != "off")
			throw std::runtime_error("autoindex expects on or off");
		config.setAutoindex(tokens[pos] == "on");
		pos++;
	}
	else if (directive == "cgi_ext")
	{
		//cgi_ext <extension> <interpreter>;   e.g. cgi_ext .py /usr/bin/python3;
		std::string	extension = tokens[pos];

		pos++;
		if (pos >= tokens.size())
			throw std::runtime_error("Expected an interpreter after cgi_ext");
		config.addCgi(extension, tokens[pos]);
		pos++;
	}
	else if (directive == "allowed_methods")//
	{
		//this one takes several values: read tokens until the ;
		std::vector<std::string>	methods;

		while (pos < tokens.size() && tokens[pos] != ";")
		{
			methods.push_back(tokens[pos]);
			pos++;
		}
		if (methods.empty())
			throw std::runtime_error("allowed_methods expects at least one method");
		config.setAllowedMethods(methods);
	}
	else
		throw std::runtime_error("Unknown directive in location: " + directive);
	if (pos >= tokens.size() || tokens[pos] != ";")
		throw std::runtime_error("Expected ; after " + directive);
	pos++;
}
//create a LocationConfig 
void	ConfigParser::parseLocationBlock(const std::vector<std::string>& tokens, size_t& pos, ServerConfig& config) const
{
	LocationConfig	location;

	location.setPath(tokens[pos]);
	pos ++;
	if (pos >= tokens.size() || tokens[pos] != "{")
		throw std::runtime_error("Expected { after location");
	pos ++;
	while (pos < tokens.size() && tokens[pos] != "}")
		parseInsideBlock(tokens, pos, location);
	if (pos >= tokens.size() || tokens[pos] != "}")
		throw std::runtime_error("Expected } at end of location block");
	pos ++;
	config.addLocation(location);
}
//transform the string to its real fonction
//"10M"   → 10485760
//"512K"  → 524288
//"2048"  → 2048
//"2G"    → 2147483648
std::size_t	ConfigParser::parseSize(const std::string& text) const
{
	std::istringstream	iss(text);
	std::size_t			value = 0;
	char				unit = 0;

	iss >> value;
	if (iss.fail())
		throw std::runtime_error("Invalid size in config: " + text);
	iss >> unit;
	if (unit == 'k' || unit == 'K')
		value *= 1024;
	else if (unit == 'm' || unit == 'M')
		value *= 1024 * 1024;
	else if (unit == 'g' || unit == 'G')
		value *= 1024 * 1024 * 1024;
	else if (unit != 0)
		throw std::runtime_error("Unknown size unit in config: " + text);
	return (value);
}
//check if servers listen the same port
void	ConfigParser::checkDuplicatePorts(const std::vector<ServerConfig>& servers) const
{
	for (size_t i = 0; i < servers.size(); i++)
	{
		for (size_t j = i + 1; j < servers.size(); j++)
		{
			if (servers[i].getPort() == servers[j].getPort())
			{
				std::ostringstream	message;

				message << "Duplicate listen port in config: " << servers[i].getPort();
				throw std::runtime_error(message.str());
			}
		}
	}
}



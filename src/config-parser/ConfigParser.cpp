/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParser.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mipang <mipang@student.42.fr>              #+#  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026-07-15 13:21:19 by mipang            #+#    #+#             */
/*   Updated: 2026-07-15 13:21:19 by mipang           ###   ########.fr       */
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
	else
		throw std::runtime_error("Unknown directive in server: " + directive);
	if (pos >= tokens.size() || tokens[pos] != ";")
		throw std::runtime_error("Expected ; after " + directive);
	pos++;
}

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
	{	config.setRoot(tokens[pos]);
		pos++;
	}
	if (directive == "index")
	{
		config.setIndex(tokens[pos]);
		pos ++;
	}
	if (pos >= tokens.size() || tokens[pos] != ";")
		throw std::runtime_error("Expected ; after " + directive);
	pos++;
}

ServerConfig	ConfigParser::parseServerConfig(const std::string filePath) const
{
	ServerConfig	config_server;

	size_t	pos = 0;
	std::vector<std::string>	tokens;
	std::ifstream file(filePath.c_str(), std::ios::in | std::ios::binary);
	std::ostringstream	content;

	if (!file.is_open())
		throw std::runtime_error("Could not open config file\n");
	content << file.rdbuf();
	tokens = tokenize(content.str());
	parseServerBlock(tokens, pos, config_server);
	if (pos != tokens.size())
		throw std::runtime_error("Unexpected token after server block");
	return (config_server);
}

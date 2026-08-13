/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParser.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xzhen <xzhen@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/15 13:19:17 by mipang            #+#    #+#             */
/*   Updated: 2026/08/12 17:02:26 by xzhen            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIGPARSER_HPP
#define CONFIGPARSER_HPP
#include "ServerConfig.hpp"
#include "LocationConfig.hpp"
#include <vector>
#include <string>

class	ConfigParser{
	private:
	std::vector<std::string> tokenize(const std::string content) const;
	void	parseServerBlock(const std::vector<std::string>& tokens, size_t& pos, ServerConfig& config) const;
	void	parseLocationBlock(const std::vector<std::string>& tokens, size_t& pos, ServerConfig& config) const;
	void	parseInsideBlock(const std::vector<std::string>& tokens, size_t& pos, ServerConfig& config) const;
	void	parseInsideBlock(const std::vector<std::string>& tokens, size_t& pos, LocationConfig& config) const;

	void	checkDuplicatePorts(const std::vector<ServerConfig>& servers) const;
	//transform .config string to her size 
	std::size_t	parseSize(const std::string& text) const;

	public:
	ConfigParser();
	~ConfigParser();
	//each .conf block becomes a ServerConfig
	std::vector<ServerConfig>	parseConfigFile(const std::string& filePath) const;
};

#endif

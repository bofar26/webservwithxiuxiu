/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParser.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mipang <mipang@student.42.fr>              #+#  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026-07-15 13:19:17 by mipang            #+#    #+#             */
/*   Updated: 2026-07-15 13:19:17 by mipang           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIGPARSER_HPP
#define CONFIGPARSER_HPP
#include "ServerConfig.hpp"
#include <vector>
#include <string>

class	ConfigParser{
	private:
	std::vector<std::string> tokenize(const std::string content) const;
	void	parseServerBlock(const std::vector<std::string>& token, size_t& pos, ServerConfig& config) const;
	void	parseInsideBlock(const std::vector<std::string>& token, size_t& pos, ServerConfig& config) const;

	public:
	ConfigParser();
	~ConfigParser();
	ServerConfig	parseServerConfig(const std::string filePath) const;
};

#endif

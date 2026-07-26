/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mipang <mipang@student.42.fr>              #+#  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026-07-09 16:27:09 by mipang            #+#    #+#             */
/*   Updated: 2026-07-09 16:27:09 by mipang           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include "ConfigParser.hpp"
#include "ServerConfig.hpp"
#include "Server.hpp"

int	main(int argc, char **argv)
{
	std::string		configPath;
	ConfigParser	parser;
	ServerConfig	config;

	if (argc > 2)
	{
		std::cerr << "Usage: ./webserv [config_file]" << std::endl;
		return (1);
	}

	if (argc == 2)
		configPath = argv[1];
	else
		configPath = "configs/default.conf";

	try
	{
		config = parser.parseServerConfig(configPath);

		Server server(config.getPort(), config.getRoot(), config.getIndex());
		server.start();
	}
	catch (const std::exception& e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
		return (1);
	}

	return (0);
}

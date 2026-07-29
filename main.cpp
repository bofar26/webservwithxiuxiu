/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mipang <mipang@student.42.fr>              #+#  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026-07-29 16:52:50 by mipang            #+#    #+#             */
/*   Updated: 2026-07-29 16:52:50 by mipang           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <vector>
#include "ConfigParser.hpp"
#include "ServerConfig.hpp"
#include "LocationParser.hpp"

int	main(int argc, char **argv)
{
	std::string	configPath;

	if (argc == 2)
		configPath = argv[1];
	else
		configPath = "configs/default.conf";

	try
	{
		ConfigParser	parser;
		ServerConfig	config;

		config = parser.parseServerConfig(configPath);

		std::cout << "===== SERVER CONFIG =====" << std::endl;
		std::cout << "port  = [" << config.getPort() << "]" << std::endl;
		std::cout << "root  = [" << config.getRoot() << "]" << std::endl;
		std::cout << "index = [" << config.getIndex() << "]" << std::endl;

		std::cout << std::endl;
		std::cout << "===== LOCATIONS =====" << std::endl;

		const std::vector<LocationConfig>& parsedLocations = config.getLocation();

		std::cout << "location count = [" << parsedLocations.size() << "]" << std::endl;

		for (size_t i = 0; i < parsedLocations.size(); i++)
		{
			std::cout << "--- location " << i << " ---" << std::endl;
			std::cout << "path  = [" << parsedLocations[i].getPath() << "]" << std::endl;
			std::cout << "root  = [" << parsedLocations[i].getRoot() << "]" << std::endl;
			std::cout << "index = [" << parsedLocations[i].getIndex() << "]" << std::endl;
		}
	}
	catch (const std::exception& e)
	{
		std::cerr << "Config error: " << e.what() << std::endl;
		return (1);
	}

	return (0);
}

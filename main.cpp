/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mipang <mipang@student.42.fr>              #+#  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026-07-30 13:45:12 by mipang            #+#    #+#             */
/*   Updated: 2026-07-30 13:45:12 by mipang           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <string>
#include "ConfigParser.hpp"
#include "ServerConfig.hpp"
#include "Server.hpp"

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
		ServerConfig	config = parser.parseServerConfig(configPath);

		Server	server(config);
		server.start();
	}
	catch (const std::exception& e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
		return (1);
	}
	return (0);
}

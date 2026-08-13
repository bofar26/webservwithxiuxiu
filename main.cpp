/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xzhen <xzhen@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/30 13:45:12 by mipang            #+#    #+#             */
/*   Updated: 2026/08/06 20:29:42 by xzhen            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <string>
#include <vector>
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
		ConfigParser				parser;
		std::vector<ServerConfig>	configs = parser.parseConfigFile(configPath);

		Server	server(configs);
		server.start();
	}
	catch (const std::exception& e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
		return (1);
	}
	return (0);
}

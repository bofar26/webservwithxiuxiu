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

int main()
{
	try
	{
		ConfigParser parser;
		ServerConfig config = parser.parserServerConfig("configs/default.conf");

		std::cout << "port: " << config.getPort() << std::endl;
		std::cout << "root: " << config.getRoot() << std::endl;
		std::cout << "index: " << config.getindex() << std::endl;
	}
	catch (const std::exception& e)
	{
		std::cerr << "Config error: " << e.what() << std::endl;
		return 1;
	}

	return 0;
}

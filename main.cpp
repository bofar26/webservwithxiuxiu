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

#include "HttpResponse.hpp"
#include <iostream>

#include "Server.hpp"

#include <iostream>
#include <exception>

int	main(void)
{
	try
	{
		Server server(8080);

		std::cout << "Starting webserv on port 8080..." << std::endl;
		server.start();
	}
	catch (const std::exception& e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
		return (1);
	}

	return (0);
}

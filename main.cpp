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

int main(void)
{
	HttpResponse response;

	response.setStatus(200);
	response.setHeader("Content-Type", "text/plain");
	response.setBody("Hello Webserv");

	std::cout << response.toString() << std::endl;

	return (0);
}

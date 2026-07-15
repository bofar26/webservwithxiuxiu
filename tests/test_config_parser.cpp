#include "ConfigParser.hpp"
#include <iostream>
#include <exception>

static void	printLocation(const LocationConfig& loc)
{
	std::cout << "    location " << loc.path << " {" << std::endl;
	std::cout << "      root         = " << loc.root << std::endl;
	std::cout << "      index        = " << loc.index << std::endl;
	std::cout << "      autoindex    = " << (loc.autoindex ? "on" : "off") << std::endl;

	std::cout << "      methods      = ";
	for (std::size_t i = 0; i < loc.allowedMethods.size(); ++i)
		std::cout << loc.allowedMethods[i] << " ";
	std::cout << std::endl;

	if (!loc.uploadStore.empty())
		std::cout << "      upload_store = " << loc.uploadStore << std::endl;

	if (loc.redirectCode != 0)
		std::cout << "      redirect     = " << loc.redirectCode << " -> " << loc.redirectTarget << std::endl;

	for (std::map<std::string, std::string>::const_iterator it = loc.cgiExtensions.begin();
		 it != loc.cgiExtensions.end(); ++it)
		std::cout << "      cgi          = " << it->first << " -> " << it->second << std::endl;

	std::cout << "    }" << std::endl;
}

int	main(int argc, char** argv)
{
	std::string	configPath = "conf/webserv.conf";

	if (argc > 1)
		configPath = argv[1];

	try
	{
		ConfigParser				parser;
		std::vector<ServerConfig>	servers = parser.parseFile(configPath);

		std::cout << "Parsed " << servers.size() << " server block(s) from "
				   << configPath << std::endl << std::endl;

		for (std::size_t i = 0; i < servers.size(); ++i)
		{
			const ServerConfig&	server = servers[i];

			std::cout << "server " << i << " {" << std::endl;
			std::cout << "  host  = " << server.host << std::endl;
			std::cout << "  port  = " << server.port << std::endl;

			std::cout << "  names = ";
			for (std::size_t n = 0; n < server.serverNames.size(); ++n)
				std::cout << server.serverNames[n] << " ";
			std::cout << std::endl;

			std::cout << "  client_max_body_size = " << server.clientMaxBodySize << " bytes" << std::endl;

			for (std::map<int, std::string>::const_iterator it = server.errorPages.begin();
				 it != server.errorPages.end(); ++it)
				std::cout << "  error_page " << it->first << " -> " << it->second << std::endl;

			for (std::size_t l = 0; l < server.locations.size(); ++l)
				printLocation(server.locations[l]);

			std::cout << "}" << std::endl << std::endl;
		}

		// ---- sanity check findLocation() prefix matching ----
		std::cout << "===== findLocation() check =====" << std::endl;
		const char*	testPaths[] = { "/", "/index.html", "/uploads/photo.png",
									"/cgi-bin/hello.py", "/old-page", "/does-not-exist" };

		for (std::size_t p = 0; p < 6; ++p)
		{
			const LocationConfig* loc = servers[0].findLocation(testPaths[p]);
			std::cout << testPaths[p] << "  -->  ";
			if (loc)
				std::cout << "matched location \"" << loc->path << "\"" << std::endl;
			else
				std::cout << "no location matched" << std::endl;
		}
	}
	catch (const std::exception& e)
	{
		std::cerr << "Config error: " << e.what() << std::endl;
		return (1);
	}

	return (0);
}

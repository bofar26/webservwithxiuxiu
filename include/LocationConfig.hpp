/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   LocationConfig.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xzhen <xzhen@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/27 15:36:52 by mipang            #+#    #+#             */
/*   Updated: 2026/08/15 17:10:33 by xzhen            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LOCATIONCONFIG_HPP
#define LOCATIONCONFIG_HPP

#include <string>
#include <vector>
#include <map>
#include <cstddef>

//new:
//_allowedMethods, _autoindex, _uploadStore, _redirect 
//+ 7 getter/setter 
//+ isMethodAllowed()
class LocationConfig{
	private:
	std::string					_path;//after location 
	std::string					_root;//of page
	std::string					_index;//by default
	std::vector<std::string>	_allowedMethods;//GET POST 
	bool						_autoindex;//true or false
	std::string					_uploadStore;//./public
	std::string					_redirect;//
	std::map<std::string, std::string>	_cgi;//".py" -> "/usr/bin/python3"
	std::size_t					_maxBodySize;//0 means: keep the server value

	public:
	LocationConfig();
	~LocationConfig();
	LocationConfig(const LocationConfig& other);
	LocationConfig& operator=(const LocationConfig& other);

	std::string	getPath() const;
	std::string	getRoot() const;
	std::string	getIndex() const;
	const std::vector<std::string>&	getAllowedMethods() const;
	bool		getAutoindex() const;
	std::string	getUploadStore() const;
	std::string	getRedirect() const;
	std::size_t	getClientMaxBodySize() const;

	void	setPath(const std::string& path);
	void	setRoot(const std::string& root);
	void	setIndex(const std::string& index);
	void	setAllowedMethods(const std::vector<std::string>& methods);
	void	setAutoindex(bool autoindex);
	void	setUploadStore(const std::string& uploadStore);
	void	setRedirect(const std::string& redirect);
	void	setClientMaxBodySize(std::size_t size);
	
	bool	isMethodAllowed(const std::string& method) const;

	void	addCgi(const std::string& extension, const std::string& interpreter);
	//returns "" when this extension is not configured as a CGI here
	std::string	getCgiInterpreter(const std::string& extension) const;
};

# endif

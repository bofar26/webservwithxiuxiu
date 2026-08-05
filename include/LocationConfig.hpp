/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Locationconfig.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mipang <mipang@student.42.fr>              #+#  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026-07-27 15:28:16 by mipang            #+#    #+#             */
/*   Updated: 2026-07-27 15:28:16 by mipang           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LOCATIONCONFIG_HPP
#define LOCATIONCONFIG_HPP

#include <string>
#include <vector>

class LocationConfig{
	private:
	std::string	_path;
	std::string	_root;
	std::string	_index;
	public:
	LocationConfig();
	~LocationConfig();
	LocationConfig(const LocationConfig& other);
	LocationConfig& operator=(const LocationConfig& other);

	std::string getPath() const;
	std::string getRoot() const;
	std::string getIndex() const;

	void	setPath(const std::string& path);
	void	setRoot(const std::string& root);
	void	setIndex(const std::string& index);

};

# endif

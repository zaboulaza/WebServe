/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lenakach <lenakach@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/03 17:53:10 by zaboulaza         #+#    #+#             */
/*   Updated: 2026/04/12 10:40:00 by lenakach         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Server.hpp"

class Config {

	public:

		Config() {}
		~Config() {}
		Config(const Config &c);
		Config &operator=(const Config &c);

		// Parse le fichier conf (ou une liste de ports si pas de conf).
		int parse(char **av, int ac);

		std::vector<Server> &get_servers() { return _servers; }

	private:

		std::vector<Server> _servers;

		int is_number(char *av);
		int is_empty(std::string str);
		std::string trim(std::string str);
		std::vector<std::string> split(std::string str, char delimiter);
		int is_word(std::string str);
		int is_word2(std::string str);
		void set_values_server(Server &serve, std::vector<std::string> line);
		void set_values_location(Location &loc, std::vector<std::string> line);
		Server creat_serve(std::vector<std::string> vec, size_t &i, Server serve);
		Location creat_location(std::vector<std::string> vec, size_t &i);
};

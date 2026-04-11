/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Epoll_utils.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lenakach <lenakach@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/03 17:53:10 by zaboulaza         #+#    #+#             */
/*   Updated: 2026/04/08 17:22:05 by lenakach         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../hpp/Epoll.hpp"

// VU
int Epoll::is_number(char *av)
{
	for (int i = 0; av[i]; i++)
	{
		if (!(av[i] >= '0' && av[i] <= '9'))
			return (0);
	}
	return (1);
}

// VU
int Epoll::is_empty(std::string str)
{
	if (str.empty())
		return (1);
	return (0);
}

// VU
std::string Epoll::trim(std::string str)
{
	size_t	start;
	size_t	end;

	start = str.find_first_not_of(" \t\n\r\f\v\"");
	if (start == std::string::npos)
		return (""); // Chaîne vide si uniquement des espaces
	end = str.find_last_not_of(" \t\n\r\f\v\"");
	return (str.substr(start, end - start + 1));
}

// VU
std::vector<std::string> Epoll::split(std::string str, char delimiter)
{
	size_t	pos;

	std::vector<std::string> res;
	while ((pos = str.find(delimiter)) != std::string::npos)
	{
		std::string token = str.substr(0, pos);
		if (!token.empty())
			res.push_back(token);
		str.erase(0, pos + 1);
	}
	if (!str.empty())
		res.push_back(str);
	return (res);
}

// VU
int Epoll::is_word(std::string str)
{
	std::vector<std::string> words;
	words.push_back("listen");
	words.push_back("root");
	words.push_back("index");
	words.push_back("allowed_methods");
	words.push_back("auto_index");
	words.push_back("cgi");
	words.push_back("error_page");
	words.push_back("upload_folder");
	words.push_back("redirect");
	words.push_back("location");
	words.push_back("client_max_body_size");
	for (size_t i = 0; i < words.size(); i++)
	{
		if (str == words[i])
			return (1);
	}
	return (-1);
}

// VU
void Epoll::set_values_server(Server &serve, std::vector<std::string> line)
{
	int	code;

	if (line[0] == "listen")
	{
		if (line.size() != 2)
		{
			serve = Server();
			return ;
		}
		serve.set_port(line[1]);
	}
	else if (line[0] == "root")
	{
		if (line.size() != 2)
		{
			serve = Server();
			return ;
		}
		serve.set_root(line[1]);
	}
	else if (line[0] == "index")
	{
		if (line.size() != 2)
		{
			serve = Server();
			return ;
		}
		serve.set_index(line[1]);
	}
	else if (line[0] == "allowed_methods")
	{
		std::set<std::string> methods;
		for (size_t j = 1; j < line.size(); j++)
		{
			if (line[j] != "GET" && line[j] != "POST" && line[j] != "DELETE")
			{
				serve = Server();
				return ;
			}
			methods.insert(line[j]);
		}
		serve.set_allowed_methods(methods);
	}
	else if (line[0] == "auto_index")
	{
		if (line.size() != 2)
		{
			serve = Server();
			return ;
		}
		if (line[1] == "on")
			serve.set_auto_index(true);
		else
			serve.set_auto_index(false);
	}
	else if (line[0] == "cgi")
	{
		if (line.size() != 3)
		{
			serve = Server();
			return ;
		}
		std::map<std::string, std::string> cgi_map;
		cgi_map[line[1]] = line[2];
		serve.set_cgi(cgi_map);
	}
	else if (line[0] == "error_page")
	{
		if (line.size() != 3)
		{
			serve = Server();
			return ;
		}
		std::map<int, std::string> error_pages = serve.get_error_pages();
		code = std::atoi(line[1].c_str());
		error_pages[code] = line[2];
		serve.set_error_pages(error_pages);
	}
	else if (line[0] == "upload_folder")
	{
		if (line.size() != 2)
		{
			serve = Server();
			return ;
		}
		serve.set_upload_folder(line[1]);
	}
	else if (line[0] == "redirect")
	{
		if (line.size() != 2)
		{
			serve = Server();
			return ;
		}
		serve.set_redirect(line[1]);
	}
	else if (line[0] == "client_max_body_size")
	{
		if (line.size() != 2)
		{
			serve = Server();
			return ;
		}
		serve.set_client_max_body_size((size_t)std::atol(line[1].c_str()));
	}
}

// VU
int Epoll::is_word2(std::string str)
{
	std::vector<std::string> words;
	words.push_back("root");
	words.push_back("index");
	words.push_back("allowed_methods");
	words.push_back("auto_index");
	words.push_back("cgi");
	words.push_back("error_page");
	words.push_back("upload_folder");
	words.push_back("redirect");
	for (size_t i = 0; i < words.size(); i++)
	{
		if (str == words[i])
			return (1);
	}
	return (-1);
}

// VU
void Epoll::set_values_location(Location &loc, std::vector<std::string> line)
{
	int	code;

	if (line[0] == "root")
	{
		if (line.size() != 2)
		{
			loc.set_is_good(false);
			loc = Location();
			return ;
		}
		loc.set_root(line[1]);
	}
	else if (line[0] == "index")
	{
		if (line.size() != 2)
		{
			loc.set_is_good(false);
			loc = Location();
			return ;
		}
		loc.set_index(line[1]);
	}
	else if (line[0] == "allowed_methods")
	{
		std::set<std::string> methods;
		for (size_t j = 1; j < line.size(); j++)
		{
			methods.insert(line[j]);
		}
		loc.set_allowed_methods(methods);
	}
	else if (line[0] == "auto_index")
	{
		if (line.size() != 2)
		{
			loc.set_is_good(false);
			loc = Location();
			return ;
		}
		if (line[1] == "on")
			loc.set_auto_index(true);
		else
			loc.set_auto_index(false);
	}
	else if (line[0] == "cgi")
	{
		if (line.size() != 3 || line[1].size() < 2 || line[1][0] != '.')
		{
			loc.set_is_good(false);
			loc = Location();
			return ;
		}
		// Fusion avec les CGI déjà enregistrés (corrige l'écrasement)
		std::map<std::string, std::string> cgi_map = loc.get_cgi();
		cgi_map[line[1]] = line[2];
		loc.set_cgi(cgi_map);
	}
	else if (line[0] == "error_page")
	{
		if (line.size() != 3)
		{
			loc.set_is_good(false);
			loc = Location();
			return ;
		}
		std::map<int, std::string> error_pages;
		code = std::atoi(line[1].c_str());
		error_pages[code] = line[2];
		loc.set_error_pages(error_pages);
	}
	else if (line[0] == "upload_folder")
	{
		if (line.size() != 2)
		{
			loc.set_is_good(false);
			loc = Location();
			return ;
		}
		loc.set_upload_folder(line[1]);
	}
	else if (line[0] == "redirect")
	{
		if (line.size() != 2)
		{
			loc.set_is_good(false);
			loc = Location();
			return ;
		}
		loc.set_redirect(line[1]);
	}
}

// VU
Location Epoll::creat_location(std::vector<std::string> vec, size_t &i)
{
	Location	loc;

	loc = Location();
	loc.set_is_good(true);
	i++;
	if (i >= vec.size() || vec[i] != "{")
	{
		loc = Location();
		std::cout << "fail 1" << std::endl;
		loc.set_is_good(false);
		return (loc);
	}
	i++;
	while (i < vec.size() && vec[i] != "}")
	{
		std::vector<std::string> line = split(vec[i], ' ');
		if (line.size() < 2)
		{
			loc = Location();
			loc.set_is_good(false);
			std::cout << "fail 2" << std::endl;
			return (loc);
		}
		if (is_word2(line[0]) == -1)
		{
			loc = Location();
			loc.set_is_good(false);
			std::cout << "fail 3" << std::endl;
			return (loc);
		}
		set_values_location(loc, line);
		i++;
	}
	if (i >= vec.size() || vec[i] != "}")
	{
		loc = Location();
		loc.set_is_good(false);
		std::cout << "fail 4" << std::endl;
		return (loc);
	}
	else
		i++;
	return (loc);
}


std::string trimBIS(const std::string &str)
{
    size_t start = 0;
    size_t end = str.length();

    // Trim à gauche
    while (start < end && (std::isspace(str[start]) || str[start] == ';'))
        start++;

    // Trim à droite
    while (end > start && (std::isspace(str[end - 1]) || str[end - 1] == ';'))
        end--;

    return str.substr(start, end - start);
}

bool fill_map(std::vector<std::string> line, Server & server)
{
    std::map<std::string, std::string> tmp;
    std::string extension;
    std::string path;
    if (line.size() != 3)
    {
        std::cout << "CGI PARSING : TOO MANY ARGUMENT IN CONFIG FILE" << std::endl;
        return false ;
    }
    //Ici je dois verifier que apres le cgi il y aun bon format
    //check des extensions
    if ( line[1].size() < 2 || line[1][0] != '.')
        return false; 
    extension = trimBIS(line[1]);
    path = trimBIS(line[2]);
    tmp = server.get_cgi();
    tmp[extension] = path; 
    server.set_cgi(tmp);
    return true ;
}

// VU
Server Epoll::creat_serve(std::vector<std::string> vec, size_t &i, Server serve)
{
	Location	loc;

	serve.set_is_good(true);
	i++;
	if (i >= vec.size() || vec[i] != "{")
	{
		std::cout << "1" << std::endl;
		serve.set_is_good(false);
		return (serve);
	}
	i++;
	while (i < vec.size() && vec[i] != "}")
	{
		while (i < vec.size() && is_empty(vec[i]))
			i++;
		if (i >= vec.size() || vec[i] == "}")
			break;
		std::vector<std::string> line = split(vec[i], ' ');
		if (line.size() < 2)
		{
			std::cout << "2" << std::endl;
			serve.set_is_good(false);
			return (serve);
		}
		if (is_word(line[0]) == -1)
		{
			std::cout << "3" << std::endl;
			serve.set_is_good(false);
			return (serve);
		}
		else
		{
			if (line[0] == "location")
			{
				if (line.size() != 2)
				{
					std::cout << "4" << std::endl;
					serve.set_is_good(false);
					return (serve);
				}
				loc = creat_location(vec, i);
				if (loc.get_is_good() == false)
				{
					std::cout << "5" << std::endl;
					serve.set_is_good(false);
					return (serve);
				}
				// Stocker le préfixe URL de ce bloc location (ex: "/uploads")
				loc.set_path(line[1]);
				std::vector<Location> locations = serve.get_locations();
				locations.push_back(loc);
				serve.set_locations(locations);
			}
			// TEST D'AJOUT DE PARSING CGI ICI
			else if (line[0] == "cgi")
			{
				if (fill_map(line, serve) == false)
                {
                    std::cout << "PROBLEME AVEC CGI PARSING" << std::endl;
                    serve.set_is_good(false);
                    return (serve);
                }
				i++;
				// REMPLIR STD::MAP _CGI DANS SERVER
			}
			else
			{
				set_values_server(serve, line);
				i++;
			}
		}
		while (i < vec.size() && is_empty(vec[i]))
		{
			i++;
		}
	}
	if (i >= vec.size() || vec[i] != "}")
	{
		std::cout << "6" << std::endl;
		serve.set_is_good(false);
		return (serve);
	}
	else
		i++;
	return (serve);
}

// VU
int Epoll::set_ports(char **av, int ac)
{
	Server	serve;
			Server server;

	if (ac == 2 && !is_number(av[1]))
	{
		std::ifstream ifs;
		ifs.open(av[1], std::ifstream::in);
		if (!ifs)
		{
			std::cerr << "error : open fail" << std::endl;
			return (-1);
		}
		std::string str;
		std::vector<std::string> vec;
		while (getline(ifs, str))
		{
			str = trim(str);
			vec.push_back(str);
		}
		for (size_t i = 0; i < vec.size();)
		{
			while (i < vec.size() && is_empty(vec[i]))
				i++;
			if (i < vec.size() && vec[i] == "server")
			{
				serve = Server();
				serve = creat_serve(vec, i, serve);
				if (serve.get_is_good() == false)
				{
					std::cerr << "error : server config" << std::endl;
					return (-1);
				}
				_servers.push_back(serve);
			}
			else if (i < vec.size() && !is_empty(vec[i]) && vec[i] != "server")
			{
				std::cerr << "error : config file" << std::endl;
				return (-1);
			}
		}
	}
	else
	{
		for (int i = 1; i < ac; ++i)
		{
			server.set_port(av[i]);
			_servers.push_back(server);
		}
	}
	return (1);
}

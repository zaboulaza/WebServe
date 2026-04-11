/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lenakach <lenakach@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/15 00:55:12 by zaboulaza         #+#    #+#             */
/*   Updated: 2026/04/08 21:33:31 by lenakach         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <stack>
#include <map>
#include <cstring>
#include <sys/types.h>   // pour certains types réseau
#include <sys/socket.h>  // pour socket(), bind(), etc.
#include <netdb.h>       // pour getaddrinfo(), struct addrinfo, gai_strerror()
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <sys/wait.h>
#include <cerrno>
#include <string>
#include <fstream>
#include <map>
#include <set>
#include <string>
#include <vector>
#include <cstdlib>
#include <sstream>
#include <fstream>
#define BACKLOG 200
#define MAXEPOLLSIZE 500

class Request {

    public:

        Request() : _content_length(-1) {};
        ~Request() {};
        Request(const Request &request);
        Request &operator=(const Request &request);
        std::vector<std::string> split(std::string str, char delimiter);
        std::vector<std::string> split_first(std::string str, char delimiter);
        int parse_header(std::string head);
        bool set_first_line(std::string str);
        std::string trim(std::string str);
        bool pars_head(std::string str);
        // max_body_size : client_max_body_size du serveur (défaut 10 Mo)
        int validate_header(std::set<std::string> allowed_methods,
                            size_t max_body_size = 10 * 1024 * 1024);

        std::string get_method() const { return _method; }
        int get_content_length() const { return _content_length; }

        void set_body(std::string body) { _body = body; }
        std::string get_version() const {return _version;};

        std::string get_path() const { return _path; }
        std::string get_body() const { return _body; }

        // Retourne la valeur d'un en-tête (ex: "Host", "Cookie").
        // Retourne "" si l'en-tête n'existe pas. Utile pour les bonus (cookies).
        std::string get_header(const std::string &key) const {
            std::map<std::string, std::string>::const_iterator it = _header.find(key);
            if (it != _header.end()) return it->second;
            return "";
        }

    private:

        std::map<std::string, std::string> _header;
        std::string _method;
        std::string _path;
        std::string _version;
        int _content_length;
        std::string _body;
        std::vector<std::string> _vec_header;
};
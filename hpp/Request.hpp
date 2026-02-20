/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zaboulaza <zaboulaza@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/15 00:55:12 by zaboulaza         #+#    #+#             */
/*   Updated: 2026/02/20 16:17:35 by zaboulaza        ###   ########.fr       */
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
#include <cerrno>
#include <fstream>
#include <map>
#include <set>
#include <string>
#include <vector>
#include <cstdlib>
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
        int validate_header();

        std::string get_method() const { return _method; }
        int get_content_length() const { return _content_length; }

        void set_body(std::string body) { _body = body; }

    private:

        std::map<std::string, std::string> _header;
        std::string _method;
        std::string _path;
        std::string _version;
        int _content_length;
        std::string _body;
        std::vector<std::string> _vec_header;
};
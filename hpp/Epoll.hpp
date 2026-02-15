/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Epoll.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zaboulaza <zaboulaza@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/29 03:31:10 by zaboulaza         #+#    #+#             */
/*   Updated: 2026/02/15 02:34:40 by zaboulaza        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Server.hpp"

class Epoll {

    public:

        Epoll() {};
        ~Epoll() {};
        Epoll(const Epoll &epoll);
        Epoll &operator=(const Epoll &epoll);

        int init_epoll_servers();
        int create_and_bind_socket(Server &server);
        int set_non_blocking(int socket_fd);
        int setup_epoll();
        int accept_new_client(Server &server);
        int handle_client_event(int client_fd);
        int is_word2(std::string str);


        int set_ports(char **av, int ac);
        int is_number(char *av);
        std::string trim(std::string str);
        int is_empty(std::string str);
        int is_word(std::string str);
        void set_values_server(Server &serve, std::vector<std::string> line);
        void set_values_location(Location &serve, std::vector<std::string> line);
        std::vector<std::string> split(std::string str, char delimiter);
        Server creat_serve(std::vector<std::string> vec, size_t &i, Server serve);
        Location creat_location(std::vector<std::string> vec, size_t &i);
        
    private:

        std::vector<Server> _servers;
        int _epoll_g;
        int _nb_sockets;

};


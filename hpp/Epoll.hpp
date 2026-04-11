/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Epoll.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zaboulaza <zaboulaza@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/29 03:31:10 by zaboulaza         #+#    #+#             */
/*   Updated: 2026/04/11 00:00:00 by zaboulaza        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Server.hpp"

#define CGI_TIMEOUT 10  // secondes avant de tuer un CGI bloqué

class Epoll {

    public:

        Epoll() {}
        ~Epoll() {}
        Epoll(const Epoll &epoll);
        Epoll &operator=(const Epoll &epoll);

        int init_epoll_servers();
        int create_and_bind_socket(Server &server);
        int set_non_blocking(int socket_fd);
        int setup_epoll();
        int accept_new_client(Server &server);

        // Gère un événement sur un fd client (EPOLLIN, EPOLLOUT, EPOLLERR…).
        int handle_client_event(int client_fd, uint32_t events);

        // Gère un événement sur un pipe de sortie CGI.
        void handle_cgi_pipe_event(int pipe_fd, uint32_t events);

        // Vérifie les CGI en timeout et les tue.
        void check_cgi_timeouts();

        int set_ports(char **av, int ac);
        int is_number(char *av);
        std::string trim(std::string str);
        int is_empty(std::string str);
        int is_word(std::string str);
        int is_word2(std::string str);
        void set_values_server(Server &serve, std::vector<std::string> line);
        void set_values_location(Location &loc, std::vector<std::string> line);
        std::vector<std::string> split(std::string str, char delimiter);
        Server creat_serve(std::vector<std::string> vec, size_t &i, Server serve);
        Location creat_location(std::vector<std::string> vec, size_t &i);

    private:

        std::vector<Server> _servers;
        int _epoll_g;
        int _nb_sockets;

        // CGI : associe un pipe_out fd à son client fd et à son index de serveur.
        std::map<int, int>    _pipe_to_client;  // pipe_out fd → client fd
        std::map<int, size_t> _pipe_to_server;  // pipe_out fd → index dans _servers

        // Supprime proprement un client : epoll_ctl DEL + close + remove_client.
        void cleanup_client(size_t server_idx, int client_fd);
};

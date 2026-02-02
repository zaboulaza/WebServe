/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Epoll.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zaboulaza <zaboulaza@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/29 03:31:10 by zaboulaza         #+#    #+#             */
/*   Updated: 2026/02/02 03:13:51 by zaboulaza        ###   ########.fr       */
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

        void set_ports(char **av, int ac);

    private:

        std::vector<Server> _servers;
        int _epoll_g;
        int _nb_sockets;
    
};

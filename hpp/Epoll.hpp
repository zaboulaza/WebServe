/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Epoll.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zaboulaza <zaboulaza@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/29 03:31:10 by zaboulaza         #+#    #+#             */
/*   Updated: 2026/01/29 20:08:03 by zaboulaza        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
// #include "Client.hpp"
#include "Server.hpp"
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

class Epoll {

    public:

        Epoll() {};
        ~Epoll() {};
        Epoll(const Epoll &epoll);
        Epoll &operator=(const Epoll &epoll);
        int set_port(char **av, int ac);

        int serv_init();
        int create_and_bind_socket();
        int set_non_blocking(int sockfd);
        int setup_epoll();
        int accept_new_client(Server &serve);
        int handle_client_event(int n);

        struct epoll_event *events; // epoll vas metre dedant les socket qui on declencher un event (epoll_wait)

    private:

        int _epoll_g;           // epoll qui surveille tout le monde
        int _nb_socket;         // nombre de socket surveiller par epoll
        int _nb_event;          // nombre de socket qui on declancher une action

        std::vector<Server> _serve;
        std::map<int, Server*> _fd_to_server;
        // std::vector<int>    _port;
        // std::vector<char *> _port;
};

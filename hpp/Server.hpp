/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zaboulaza <zaboulaza@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/26 15:15:51 by zaboulaza         #+#    #+#             */
/*   Updated: 2026/01/28 17:04:00 by zaboulaza        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include "Client.hpp"
#include <iostream>
#include <string>
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

#define BACKLOG 200 // nombre de demande de conextion en file d'attente possible
#define MAXEPOLLSIZE 500 // nombre de conection possible au total 

class Server{
    
    public:

        Server() {};
        ~Server() {};    
        Server(const Server &serv);
        Server &operator=(const Server &serv);

        void set_port(char *port) {this->_port = port;};

        int serv_init();
        int set_non_blocking(int sockfd);
        int creat_and_bind_socket();
        int setup_epoll();
        int accept_new_client();
        int handle_client_event(int n);

        struct addrinfo hints;      // les regle a respecter genre
        struct addrinfo *servinfo;  // stock toute mes info (le port, IP, etc...)
        struct addrinfo *tmp;       // tmp pour boucler sur servinfo
        struct epoll_event ev;      // dit a epoll quoi surveiller et quoi renvoyer 
        struct epoll_event *events; // epoll vas metre dedant les socket qui on declencher un event (epoll_wait)
        struct sockaddr_storage client_addr; // peux contenir nimporte quelle IP 4 ou 6

    private:    

        char *_port;            // av[1]
        int _status;            // utile pour print err -> gai_strerror(status)
        int _sockfd;            // socket server 
        int _epoll_g;           // epoll qui surveille tout le monde
        int _nb_socket;         // nombre de socket surveiller par epoll
        int _nb_event;          // nombre de socket qui on declancher une action
        socklen_t _addr_size;   // taille de la structure ou il ya l'adress du client 
        int _new_fd;            // le nouveau fd client
        std::map<int, Client> _clients; // stock les client dans un std::map pour chaque socket il y a son client
        
};

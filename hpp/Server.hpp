/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zaboulaza <zaboulaza@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/26 15:15:51 by zaboulaza         #+#    #+#             */
/*   Updated: 2026/01/26 20:34:19 by zaboulaza        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include "Client.hpp"
#include <iostream>
#include <string>
#include <stack>
#include <cstring>
#include <sys/types.h>   // pour certains types réseau
#include <sys/socket.h>  // pour socket(), bind(), etc.
#include <netdb.h>       // pour getaddrinfo(), struct addrinfo, gai_strerror()

class Server{
    
    public:

        Server() {};
        ~Server() {};    
        Server(const Server &serv);
        Server &operator=(const Server &serv);

        void set_port(char *port) {this->_port = port;};

        void serv_init();

    private:    

        char *_port; // av[1]
        int _status; // utile pour print err -> gai_strerror(status)
        
};

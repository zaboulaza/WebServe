/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zaboulaza <zaboulaza@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/26 15:15:51 by zaboulaza         #+#    #+#             */
/*   Updated: 2026/02/02 03:12:38 by zaboulaza        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Client.hpp"

class Server{
    
    public:
    
        Server() {};
        ~Server() {};
        Server(const Server &server);
        Server &operator=(const Server &server);
        
        void add_client(int clientfd);
        void remove_client(int clientfd);
                
        void set_port(char *port) { _port = port; };
        char *get_port() { return _port; };
    
        void set_socketfd(int socket_fd) { _socketfd = socket_fd; };
        int get_socketfd() { return _socketfd; };

    private:

        std::map<int , Client> _clients_map;
        char *_port;
        int _socketfd;
};

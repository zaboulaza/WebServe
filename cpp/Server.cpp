/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zaboulaza <zaboulaza@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/27 20:28:17 by zaboulaza         #+#    #+#             */
/*   Updated: 2026/02/02 03:12:12 by zaboulaza        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../hpp/Server.hpp"

Server::Server(const Server &server) {
    *this = server;
}

Server &Server::operator=(const Server &server) {
    if (this != &server) {
        this->_port = server._port;
        this->_socketfd = server._socketfd;
        this->_clients_map = server._clients_map;
    }
    return *this;
}

void Server::add_client(int clientfd) {
   this->_clients_map[clientfd] = Client(clientfd);
}

void Server::remove_client(int clientfd) {
    this->_clients_map.erase(clientfd);
}
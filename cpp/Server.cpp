/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zaboulaza <zaboulaza@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/27 20:28:17 by zaboulaza         #+#    #+#             */
/*   Updated: 2026/02/15 05:30:32 by zaboulaza        ###   ########.fr       */
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
        this->_root = server._root;
        this->_index = server._index;
        this->_allowed_methods = server._allowed_methods;
        this->_auto_index = server._auto_index;
        this->_cgi = server._cgi;
        this->_error_pages = server._error_pages;
        this->_upload_folder = server._upload_folder;
        this->_redirect = server._redirect;
        this->_is_good = server._is_good;
        this->_locations = server._locations;
    }
    return *this;
}

void Server::add_client(int clientfd) {
   this->_clients_map[clientfd] = Client(clientfd);
}

void Server::remove_client(int clientfd) {
    this->_clients_map.erase(clientfd);
}

bool Server::has_client(int clientfd){
    std::map<int, Client>::iterator it;
    it = _clients_map.find(clientfd);
    if (it == _clients_map.end())
        return (false);
    return (true);
}

Client &Server::get_client(int clientfd){
    return (_clients_map[clientfd]);
}
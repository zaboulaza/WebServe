/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zaboulaza <zaboulaza@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/26 15:16:21 by zaboulaza         #+#    #+#             */
/*   Updated: 2026/01/26 22:02:56 by zaboulaza        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

Server::Server(const Server &serv){
    *this = serv;
}

Server &Server::operator=(const Server &serv){
    if (this != &serv){
        
    }
    return (*this);
}

void Server::serv_init(){
    
    struct addrinfo hints;
    struct addrinfo *servinfo;
    
    memset(&hints, 0, sizeof(hints));   // verif que hints est bien vide
    hints.ai_family = AF_UNSPEC;        // prend IPv4 ou IPv6
    hints.ai_socktype = SOCK_STREAM;    // respect TCP
    hints.ai_flags = AI_PASSIVE;        // IP elle est pour un server je crois
    
    if ((this->_status = getaddrinfo(NULL, this->_port, &hints, &servinfo)) != 0) {
        std::cerr << "getaddrinfo error: " << gai_strerror(this->_status) << std::endl;
        return;
    }

    
}

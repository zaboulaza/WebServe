/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zaboulaza <zaboulaza@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/26 15:16:56 by zaboulaza         #+#    #+#             */
/*   Updated: 2026/02/15 21:11:55 by zaboulaza        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../hpp/Client.hpp"

Client::Client(const Client &client) {
    *this = client;
}

Client &Client::operator=(const Client &client) {
    if (this != &client) {
        this->_socket_fd = client._socket_fd;
    }
    return *this;
}

int Client::recv_request(){

    std::string str;
    char tmp[5000];
    int bytes;

    while (str.find("\r\n\r\n") == std::string::npos){

        bytes = recv(_socket_fd, tmp, 5000, 0);
        if (bytes <= 0){
            std::cerr << "error bad header http" << std::endl;
            return (-1);
        }
        str.append(tmp, bytes);
    }
    std::cout << "SUCESS" << std::endl;
    std::cout << str << std::endl;

    if (this->_request.parse_header(str) == -1){
        std::cerr << "error bas header http" << std::endl;   
        return (-1);
    }

    return (1);
}

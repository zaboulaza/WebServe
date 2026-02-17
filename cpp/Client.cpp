/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zaboulaza <zaboulaza@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/26 15:16:56 by zaboulaza         #+#    #+#             */
/*   Updated: 2026/02/17 20:02:30 by zaboulaza        ###   ########.fr       */
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
    std::string _header;
    std::string _body;
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
    
    _header = str.substr(0, str.find("\r\n\r\n") + 4);
    _body = str.substr(str.find("\r\n\r\n") + 4);

    std::cout << str << std::endl;

    if (this->_request.parse_header(_header) == -1){
        std::cerr << "error bas header http" << std::endl;   
        return (-1);
    }
    else if (need_to_recup_body(_header)){
        if (/*header_good_for_body(_header) == false*/){
            // std::cerr << err << std::endl;
            // return (-1);
        }
        // ici je sais pas trop comment recupere le body et voir si j'ai deja une partie ou pas ?
        // _request.set_body(body);
    }

    return (1);
}

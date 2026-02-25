/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zaboulaza <zaboulaza@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/26 15:16:56 by zaboulaza         #+#    #+#             */
/*   Updated: 2026/02/25 18:10:12 by zaboulaza        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../hpp/Client.hpp"
#include "../hpp/Server.hpp"

Client::Client(const Client &client) {
    *this = client;
}

Client &Client::operator=(const Client &client) {
    if (this != &client) {
        this->_socket_fd = client._socket_fd;
    }
    return *this;
}

std::string Client::get_body(std::string body){
    char tmp[5000];
    
    while (body.size() < _request.get_content_length()){
        int bytes = recv(_socket_fd, tmp, 5000, 0);
        if (bytes <= 0)
            break;
        body.append(tmp, bytes);
    }
    return (body);
}

int Client::recv_request(Server &server){

    std::string str;
    std::string header;
    std::string body;
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
    
    header = str.substr(0, str.find("\r\n\r\n") + 4);
    body = str.substr(str.find("\r\n\r\n") + 4);

    std::cout << str << std::endl;

    int validate = 1;
    if (this->_request.parse_header(header) == -1){
        validate = 400;   
    }
    if (validate == 1)
        validate = _request.validate_header(server.get_allowed_methods());
    Response responce;
    responce.set_socket_client(this->_socket_fd);
    if (validate != 1){
        responce.handle_erreur_response(validate);
        return (-1);
    }
    else if (_request.get_method() == "POST"){
        std::string tmp_body = get_body(body);
        _request.set_body(tmp_body);
    }

    responce.response_http(server, _request);
    
    return (1);
}

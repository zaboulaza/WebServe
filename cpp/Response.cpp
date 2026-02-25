/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zaboulaza <zaboulaza@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/24 09:11:29 by zaboulaza         #+#    #+#             */
/*   Updated: 2026/02/25 03:18:55 by zaboulaza        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../hpp/Response.hpp"

void Response::handel_erreur_responce(int socket_fd ,int err_code){

    std::string err_message;
    
    if (err_code == 411)
        err_message = "HTTP/1.1 411 Length Required\r\n\r\n";
    else if (err_code == 413)
        err_message = "HTTP/1.1 413 Payload Too Large\r\n\r\n";
    else if (err_code == 400){
        std::cout << "test" << std::endl;
        err_message = "HTTP/1.1 400 Bad Request\r\n\r\n";
    }
    else if (err_code == 404)
        err_message = "HTTP/1.1 404 Not Found\r\n\r\n";
    
    send(socket_fd, err_message.c_str(), err_message.size(), 0);
}

void Response::response_http(std::string _version){
    
    std::string response;
    
    response += _version;
    response += " 200 OK\r\n";
    response += "Content-Length: 11\r\n";
    response += "Content-Type: text/html\r\n";
    response += "\r\n";
    response += "Hello world";

    send(_socket_client, response.c_str(), response.size(), 0);
}
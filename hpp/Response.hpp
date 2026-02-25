/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zaboulaza <zaboulaza@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/24 09:12:01 by zaboulaza         #+#    #+#             */
/*   Updated: 2026/02/25 18:09:46 by zaboulaza        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Request.hpp"

class Server;
class Request;

class Response {
  
    public:

        Response(){};
        ~Response(){};
        Response(const Response &response);
        Response &operator=(const Response &response);

        void handle_erreur_response(int err_code);
        void response_http(Server &server, Request &request);
        int handle_GET_response(Server &server, Request &request);
        

        void set_socket_client(int socket) {this->_socket_client = socket;};

    private:

        int _socket_client;
    
};

/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zaboulaza <zaboulaza@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/24 09:12:01 by zaboulaza         #+#    #+#             */
/*   Updated: 2026/02/25 03:06:09 by zaboulaza        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Request.hpp"

class Response {
  
    public:

        Response(){};
        ~Response(){};
        Response(const Response &response);
        Response &operator=(const Response &response);

        void handel_erreur_responce(int socket_fd ,int err_code);
        void response_http(std::string _version);

        void set_socket_client(int socket) {this->_socket_client = socket;};

    private:

        int _socket_client;
    
};

/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zaboulaza <zaboulaza@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/27 20:27:49 by zaboulaza         #+#    #+#             */
/*   Updated: 2026/02/25 16:04:18 by zaboulaza        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Request.hpp"
#include "Response.hpp"

class Server;

class Client {

    public:

        Client() {};
        ~Client() {};
        Client(const Client &client);
        Client &operator=(const Client &client);
        Client(int fd) : _socket_fd(fd) {};

        int recv_request(Server &server);
        bool recup_body_or_not(std::string header);
        std::string get_body(std::string body);

    private:

        int _socket_fd;
        Request _request;

};


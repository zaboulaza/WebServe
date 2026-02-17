/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zaboulaza <zaboulaza@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/27 20:27:49 by zaboulaza         #+#    #+#             */
/*   Updated: 2026/02/17 19:53:11 by zaboulaza        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Request.hpp"

class Client {

    public:

        Client() {};
        ~Client() {};
        Client(const Client &client);
        Client &operator=(const Client &client);
        Client(int fd) : _socket_fd(fd) {};

        int recv_request();
        bool need_to_recup_body(std::string header);

    private:

        int _socket_fd;
        Request _request;

};
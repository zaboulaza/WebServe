/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zaboulaza <zaboulaza@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/27 20:27:49 by zaboulaza         #+#    #+#             */
/*   Updated: 2026/02/05 04:39:22 by zaboulaza        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <stack>
#include <map>
#include <cstring>
#include <sys/types.h>   // pour certains types réseau
#include <sys/socket.h>  // pour socket(), bind(), etc.
#include <netdb.h>       // pour getaddrinfo(), struct addrinfo, gai_strerror()
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <cerrno>
#include <fstream>
#include <map>
#include <set>
#include <string>
#include <vector>
#include <cstdlib>

#define BACKLOG 200
#define MAXEPOLLSIZE 500

class Client {

    public:

        Client() {};
        ~Client() {};
        Client(const Client &client);
        Client &operator=(const Client &client);
        
        Client(int fd) : _socket_fd(fd) {};

    private:

        int _socket_fd;

};
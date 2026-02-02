/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Epoll.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zaboulaza <zaboulaza@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/29 03:31:00 by zaboulaza         #+#    #+#             */
/*   Updated: 2026/02/02 03:13:25 by zaboulaza        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../hpp/Epoll.hpp"

Epoll::Epoll(const Epoll &epoll) {
    *this = epoll;
}

Epoll &Epoll::operator=(const Epoll &epoll) {
    if (this != &epoll) {
        // Copy member variables here when they are added
    }
    return *this;
}

void Epoll::set_ports(char **av, int ac) {
    
    // check if we had config files
    // if we had config files parse them and set values in servers vector

    for (int i = 1; i < ac; ++i) {        
        Server server;
        server.set_port(av[i]);
        _servers.push_back(server);
    }
}

int Epoll::set_non_blocking(int socket_fd) {
    int flags = fcntl(socket_fd, F_GETFL, 0);
    if (flags == -1) {
        std::cerr << "fcntl F_GETFL error: " << strerror(errno) << std::endl;
        return (-1);
    }
    if (fcntl(socket_fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        std::cerr << "fcntl F_SETFL error: " << strerror(errno) << std::endl;
        return (-1);
    }
    return 0;
}

int Epoll::create_and_bind_socket(Server &server) {
 
    struct addrinfo hints; // les rigles a respecter
    struct addrinfo *servinfo; // stock toute les infos retounées par getaddrinfo
    struct addrinfo *tmp; // pour parcourir la liste
    int status;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC; // prend IPv4 ou IPv6
    hints.ai_socktype = SOCK_STREAM; // respect TCP
    hints.ai_flags = AI_PASSIVE; // utilise l'IP de la machine automatiquement

    if ((status = getaddrinfo(NULL, server.get_port(), &hints, &servinfo)) != 0) {
        std::cerr << "getaddrinfo error: " << gai_strerror(status) << std::endl;
        return (-1);
    }
    
    for (tmp = servinfo; tmp != NULL; tmp = tmp->ai_next) {
        int sockfd = socket(tmp->ai_family, tmp->ai_socktype, tmp->ai_protocol);
        if (sockfd == -1) {
            std::cerr << "socket failed: " << strerror(errno) << std::endl;
            continue;
        }
        if( set_non_blocking(sockfd) == -1) {
            close(sockfd);
            continue;
        }
        if (bind(sockfd, tmp->ai_addr, tmp->ai_addrlen) == -1) {
            close(sockfd);
            std::cerr << "bind failed: " << strerror(errno) << std::endl;
            continue;
        }
        server.set_socketfd(sockfd);
        break;
    }
    
    if (tmp == NULL) {
        std::cerr << "Failed to bind socket on port " << server.get_port() << std::endl;
        close(server.get_socketfd());
        freeaddrinfo(servinfo);
        return (-1);
    }
    freeaddrinfo(servinfo);
    
    if (listen(server.get_socketfd(), BACKLOG) == -1) {
        std::cerr << "listen failed: " << strerror(errno) << std::endl;
        close(server.get_socketfd());
        return (-1);
    }
    return (1);
}

int Epoll::setup_epoll(){
    
    struct epoll_event ev; // dit a epoll quoi surveiller et renvoyer
    this->_epoll_g = epoll_create(MAXEPOLLSIZE);
    this->_nb_sockets = 0;
    
    if (this->_servers.empty()) {
        std::cerr << "No servers available to set up epoll." << std::endl;
        return (-1);
    }
    for (std::vector<Server>::iterator it = _servers.begin(); it != _servers.end();) {
        
        ev.events = EPOLLIN; // surveille les entrées en mode bordure
        ev.data.fd = it->get_socketfd();
        if (epoll_ctl(this->_epoll_g, EPOLL_CTL_ADD, it->get_socketfd(), &ev) == -1) {
            std::cerr << "epoll_ctl error: " << strerror(errno) << std::endl;
            it = _servers.erase(it);
            continue;
        }
        else{
            std::cout << "success insert socket into epoll" << std::endl;
            this->_nb_sockets++;
            it++;
        }
    }
    if (this->_servers.empty()) {
        std::cerr << "Failed to set up epoll for any server." << std::endl;
        return (-1);
    }
    return (1);
}

int Epoll::accept_new_client(Server &server) {
    
    struct sockaddr_storage client_addr; // stock l'ip du client
    struct epoll_event ev; // dit a epoll quoi surveiller et renvoyer
    socklen_t addr_size = sizeof(client_addr);
    
    int client_fd = accept(server.get_socketfd(), (struct sockaddr *)&client_addr, &addr_size);
    if (client_fd == -1) {
        if ((errno != EAGAIN) || (errno != EWOULDBLOCK)) {
            return (0);
        }
        else{
            std::cerr << "accept error: " << strerror(errno) << std::endl;
            return (-1);
        }
    }
    set_non_blocking(client_fd);
    server.add_client(client_fd);
    std::cout << "New client connected with fd: " << client_fd << std::endl;
    ev.events = EPOLLIN; // surveille les entrées
    ev.data.fd = client_fd;
    if (epoll_ctl(this->_epoll_g, EPOLL_CTL_ADD, client_fd, &ev) == -1) {
        std::cerr << "epoll_ctl error: " << strerror(errno) << std::endl;
        close(client_fd);
        server.remove_client(client_fd);
        return (-1);
    }
    else
        this->_nb_sockets++;
    return (1);
}

int Epoll::handle_client_event(int client_fd){
    struct epoll_event ev;

    if (send(client_fd, "Hello world!", 13, 0) == -1) {
        std::cerr << "send error: " << strerror(errno) << std::endl;
        return (-1);
    }
    // ev.data.fd = client_fd;
    if (epoll_ctl(this->_epoll_g, EPOLL_CTL_DEL, client_fd, &ev) == -1) {
        std::cerr << "epoll_ctl DEL error: " << strerror(errno) << std::endl;
    }
    else
        this->_nb_sockets--;
    close(client_fd);
    return (1);
}

int Epoll::init_epoll_servers() {

    for (std::vector<Server>::iterator it = _servers.begin(); it != _servers.end();) {
        
        if (create_and_bind_socket(*it) == -1) {
            // std::cerr << "Error: Failed to create and bind socket for server on port " << it->get_port() << std::endl;
            it = _servers.erase(it);
            continue;
        }
        it++;
    }
    std::cout << "Servers waiting for connections..." << std::endl;
    if( setup_epoll() == -1) {
        return (-1);
    }

    struct epoll_event events[MAXEPOLLSIZE];
    while (1){
        
        int nb_events = epoll_wait(this->_epoll_g, events, MAXEPOLLSIZE, -1);
        if (nb_events == -1) {
            std::cerr << "epoll_wait error: " << strerror(errno) << std::endl;
            return (-1);
        }
        for(int i = 0; i < nb_events; i++) {
            int fd = events[i].data.fd;
            bool is_server_socket = false;
            for (std::vector<Server>::iterator it = _servers.begin(); it != _servers.end(); ++it) {
                if (fd == it->get_socketfd()) {
                    accept_new_client(*it);
                    is_server_socket = true;
                    break;
                }
            }
            if (!is_server_socket) {
                handle_client_event(fd);
            }
        }
    }
    for (std::vector<Server>::iterator it = _servers.begin(); it != _servers.end(); ++it) {
        close(it->get_socketfd());
    }
    close(this->_epoll_g);
    return (1);
}
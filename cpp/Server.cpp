/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zaboulaza <zaboulaza@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/27 20:28:17 by zaboulaza         #+#    #+#             */
/*   Updated: 2026/01/28 17:06:41 by zaboulaza        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../hpp/Server.hpp"

Server::Server(const Server &serv){
    *this = serv;
}

Server &Server::operator=(const Server &serv){
    if (this != &serv){
        
    }
    return (*this);
}

int Server::set_non_blocking(int sockfd){
    int flags, s;
    flags = fcntl(sockfd, F_GETFL, 0);
    if(flags == -1){
        std::cerr << "fcntl failed: " << strerror(errno) << std::endl;
        return (-1);
    }
    flags = flags | O_NONBLOCK;
    s = fcntl(sockfd, F_SETFL, flags);
    if (s == -1){
        std::cerr << "fcntl failed: " << strerror(errno) << std::endl;
        return(-1);
    }
    return(0);
}

int Server::creat_and_bind_socket(){
    
    memset(&hints, 0, sizeof(hints));   // verif que hints est bien vide
    hints.ai_family = AF_UNSPEC;        // prend IPv4 ou IPv6
    hints.ai_socktype = SOCK_STREAM;    // respect TCP
    hints.ai_flags = AI_PASSIVE;        // IP elle est pour un server je crois
    
    if ((this->_status = getaddrinfo(NULL, this->_port, &hints, &servinfo)) != 0) {
        std::cerr << "getaddrinfo error: " << gai_strerror(this->_status) << std::endl;
        return(0);
    }
    
    for (tmp = servinfo; tmp != 0; tmp = tmp->ai_next){
        if ((this->_sockfd = socket(tmp->ai_family, tmp->ai_socktype, tmp->ai_protocol)) == -1){
            std::cerr << "socket failed: " << strerror(errno) << std::endl;;
            continue;
        }
        set_non_blocking(this->_sockfd);
        if ((bind(this->_sockfd, tmp->ai_addr, tmp->ai_addrlen)) == -1) {
            close(this->_sockfd);
            std::cerr << "bind failed: " << strerror(errno) << std::endl;
            continue;
        }
        break;
    }
    
    if(tmp == NULL){
        std::cerr << "server: failed to bind" << std::endl;
        freeaddrinfo(servinfo);
        return(0);
    }
    freeaddrinfo(servinfo); // free la liste chainer
    
    if (listen(this->_sockfd, BACKLOG) == -1){ // cette socket est prete a recevoir des dmd de conextion
        std::cerr << "listen failed: " << strerror(errno) << std::endl;
       return (0);
    }
    return (1);
}

int Server::setup_epoll(){
    
    this->_epoll_g = epoll_create(MAXEPOLLSIZE); // cree le epoll qui vas tout gere
    ev.events = EPOLLIN | EPOLLET;  // EPOLLIN notif if -> donner a lire / EPOLLET edge-triggered -> une seul notif
    ev.data.fd = this->_sockfd; // quand cette event ^ arrive donne moi ce fd
    if (epoll_ctl(this->_epoll_g, EPOLL_CTL_ADD, this->_sockfd, &ev) < 0){
        std::cerr << "error : epoll fail to insert" << std::endl;
        return(0);
    }
    else
        std::cout << "success insert socket into epoll" << std::endl;
    
    events = new epoll_event[MAXEPOLLSIZE](); // alloue la place et vide
    this->_nb_socket = 1;
    return (1);
}

int Server::accept_new_client(){
    
    this->_addr_size = sizeof(client_addr);
    _new_fd = accept(this->_sockfd, (struct sockaddr *)&client_addr, &_addr_size); // cree une socket client _new_fd
    if (_new_fd == -1){
        if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) // fausse erreur EAGAIN = "reessaie plus tard" / EWOULDBLOCK = "si j'etais bloquant j'att"
            return (0);
        else{
            std::cerr << "accept failed: " << strerror(errno) << std::endl;
            return (0);
        }
    }
    set_non_blocking(_new_fd);
    this->_clients[_new_fd] = Client(_new_fd);
    std::cout << "Server: connection etablished..." << std::endl;
    ev.events = EPOLLIN | EPOLLET;
    ev.data.fd = _new_fd;
    
    if (epoll_ctl(_epoll_g, EPOLL_CTL_ADD, _new_fd, &ev) < 0)
        std::cerr << "error : epoll fail to insert socket" << std::endl;
    else 
        this->_nb_socket++;
    return (1);
}

int Server::handle_client_event(int n){
    
    if(send(events[n].data.fd, "Hello world!", 13, 0) == -1){
        std::cerr << "send failed: " << strerror(errno) << std::endl;
        return (0);
    }
    if(epoll_ctl(this->_epoll_g, EPOLL_CTL_DEL, events[n].data.fd, &ev) < 0)
        std::cerr << "error : epoll fail to remove socket" << std::endl;
    else
        this->_nb_socket--;
    close(events[n].data.fd);
    _clients.erase(events[n].data.fd);
    return (1);
}

int Server::serv_init(){

    if (!creat_and_bind_socket())
        return (0);

    std::cout << "server: waiting for connections..." << std::endl;
    
    if (!setup_epoll())
        return (1);
    
    while(1){ // boucle pour accepter les prochaine conextion je crois / action
        
        this->_nb_event = epoll_wait(this->_epoll_g, events, _nb_socket, -1);
        if(this->_nb_event == -1){
            std::cerr << "epoll_wait failed: " << strerror(errno) << std::endl;
            delete[] events;
            break;
        }
        for (int n = 0; n < this->_nb_event; n++){ // verif tout les event qu'il ya eu
            if(events[n].data.fd == this->_sockfd){
                if(!accept_new_client())
                    break;
            }
            else{
                if(!handle_client_event(n))
                    break;
            }
        }
    }
    close(this->_sockfd);
    return(1);
}

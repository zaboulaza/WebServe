/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zaboulaza <zaboulaza@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/26 15:16:21 by zaboulaza         #+#    #+#             */
/*   Updated: 2026/01/27 20:24:16 by zaboulaza        ###   ########.fr       */
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

int Server::serv_init(){
    
    struct addrinfo hints; // les regle a respecter genre
    struct addrinfo *servinfo; // stock toute mes info (le port, IP, etc...)
    struct addrinfo *tmp; // tmp pour boucler sur servinfo
    struct epoll_event ev; // dit a epoll quoi surveiller et quoi renvoyer 
	struct epoll_event *events; // epoll vas metre dedant les socket qui on declencher un event (epoll_wait)
	struct sockaddr_storage client_addr; // peux contenir nimporte quelle IP 4 ou 6

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

    std::cout << "server: waiting for connections..." << std::endl;
    
    this->_epoll_g = epoll_create(MAXEPOLLSIZE); // cree le epoll qui vas tout gere
    ev.events = EPOLLIN | EPOLLET; // EPOLLIN notif if -> donner a lire / EPOLLET edge-triggered -> une seul notif
    ev.data.fd = this->_sockfd; // quand cette event ^ arrive donne moi ce fd
    if (epoll_ctl(this->_epoll_g, EPOLL_CTL_ADD, this->_sockfd, &ev) < 0){
        std::cerr << "error : epoll fail to insert" << std::endl;
        return(0);
    }
    else{
        std::cout << "success insert socket into epoll" << std::endl;
    }
    events = new epoll_event[MAXEPOLLSIZE](); // alloue la place et vide
    this->_nb_socket = 1;
    
    while(1){ // boucle pour accepter les prochaine conextion je crois / action
        
        this->_nb_event = epoll_wait(this->_epoll_g, events, _nb_socket, -1);
        if(this->_nb_event == -1){
            std::cerr << "epoll_wait failed: " << strerror(errno) << std::endl;
            delete[] events;
            break;
        }
        for (int n = 0; n < this->_nb_event; n++){ // verif tout les event qu'il ya eu
            if(events[n].data.fd == this->_sockfd){
                this->_addr_size = sizeof(client_addr);
                _new_fd = accept(this->_sockfd, (struct sockaddr *)&client_addr, &_addr_size); // cree une socket client _new_fd
                if (_new_fd == -1){
                    if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) // fausse erreur EAGAIN = "reessaie plus tard" / EWOULDBLOCK = "si j'etais bloquant j'att"
                        break;
                    else{
                        std::cerr << "accept failed: " << strerror(errno) << std::endl;
                        break;
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
                }
                else{
                    if(send(events[n].data.fd, "Hello world!", 13, 0) == -1){
                       std::cerr << "send failed: " << strerror(errno) << std::endl;
                        break;
                    }
                    if(epoll_ctl(this->_epoll_g, EPOLL_CTL_DEL, events[n].data.fd, &ev) < 0)
                        std::cerr << "error : epoll fail to remove socket" << std::endl;
                    else
                        this->_nb_socket--;
                    close(events[n].data.fd);
                    _clients.erase(events[n].data.fd);
            }
        }
    }
    close(this->_sockfd);
    return(1);
}

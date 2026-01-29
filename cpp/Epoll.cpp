/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Epoll.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zaboulaza <zaboulaza@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/29 03:31:00 by zaboulaza         #+#    #+#             */
/*   Updated: 2026/01/29 05:12:59 by zaboulaza        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../hpp/Epoll.hpp"

Epoll::Epoll(const Epoll &epoll){
    *this = epoll;
}

Epoll &Epoll::operator=(const Epoll &epoll){
    if (this != &epoll){
        this->_serve = epoll._serve;
    }
    return (*this);
}

int Epoll::set_port(char **av, int ac){

    if (this->_serve.size() > 0)
        return (1);

    for (int i = 1; i < ac; i++){
        Server serv;
        serv.set_port(av[i]);
        _serve.push_back(serv);
    }
    return (1);
}

int Epoll::set_non_blocking(int sockfd){
    int flags, s;
    flags = fcntl(sockfd, F_GETFL, 0);
    if(flags == -1){
        std::cerr << "fcntl failed: " << strerror(errno) << std::endl;
        return (0);
    }
    flags = flags | O_NONBLOCK;
    s = fcntl(sockfd, F_SETFL, flags);
    if (s == -1){
        std::cerr << "fcntl failed: " << strerror(errno) << std::endl;
        return(0);
    }
    return(1);
}

int Epoll::create_and_bind_socket(){
    
    struct addrinfo hints;      // les regle a respecter genre
    struct addrinfo *servinfo;  // stock toute mes info (le port, IP, etc...)
    struct addrinfo *tmp;       // tmp pour boucler sur servinfo
    int status;

    memset(&hints, 0, sizeof(hints));   // verif que hints est bien vide
    hints.ai_family = AF_UNSPEC;        // prend IPv4 ou IPv6
    hints.ai_socktype = SOCK_STREAM;    // respect TCP
    hints.ai_flags = AI_PASSIVE;        // IP elle est pour un server je crois
    
    for (std::vector<Server>::iterator it = _serve.begin(); it != _serve.end(); ){
        
        if ((status = getaddrinfo(NULL, it->get_port(), &hints, &servinfo)) != 0) {
            std::cerr << "getaddrinfo error: " << gai_strerror(status) << std::endl;
            it = _serve.erase(it);
            continue;
        }
        
        for (tmp = servinfo; tmp != 0; tmp = tmp->ai_next){
            int sockfd = socket(tmp->ai_family, tmp->ai_socktype, tmp->ai_protocol);
            if (sockfd == -1){
                std::cerr << "socket failed: " << strerror(errno) << std::endl;;
                continue;
            }
            if (!set_non_blocking(sockfd)){
                close(sockfd);
                continue;
            }
            if ((bind(sockfd, tmp->ai_addr, tmp->ai_addrlen)) == -1) {
                close(sockfd);
                std::cerr << "bind failed: " << strerror(errno) << std::endl;
                continue;
            }
            it->set_sockfd(sockfd);
            break;
        }
        
        if(tmp == NULL){
            std::cerr << "server: failed to bind" << std::endl;
            freeaddrinfo(servinfo);
            it = _serve.erase(it);
            continue;
        }
        freeaddrinfo(servinfo); // free la liste chainer
        
        if (listen(it->get_sockfd(), BACKLOG) == -1){ // cette socket est prete a recevoir des dmd de conextion
            std::cerr << "listen failed: " << strerror(errno) << std::endl;
            it = _serve.erase(it);
            continue;
        }
        it++;
    }
    return (1);
}

int Epoll::setup_epoll(){
    
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

int Epoll::accept_new_client(){
    
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

int Epoll::handle_client_event(int n){
    
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

int Epoll::serv_init(){

    if (!create_and_bind_socket())
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
            // while (serve[i]){
            if(events[n].data.fd == this->_sockfd){
                if(!accept_new_client())
                    break;
            }
            // }
            else{
                if(!handle_client_event(n))
                    break;
            }
        }
    }
    close(this->_sockfd);
    return(1);
}
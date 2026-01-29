/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Epoll.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zaboulaza <zaboulaza@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/29 03:31:00 by zaboulaza         #+#    #+#             */
/*   Updated: 2026/01/29 20:18:15 by zaboulaza        ###   ########.fr       */
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
        return (0);

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
    this->_nb_socket = 0;
    struct epoll_event ev;      // dit a epoll quoi surveiller et quoi renvoyer 
    for (std::vector<Server>::iterator it = _serve.begin(); it != _serve.end(); ){

        ev.events = EPOLLIN;  // EPOLLIN notif if -> donner a lire / EPOLLET edge-triggered -> une seul notif
        ev.data.fd = it->get_sockfd(); // quand cette event ^ arrive donne moi ce fd
        if (epoll_ctl(this->_epoll_g, EPOLL_CTL_ADD, it->get_sockfd(), &ev) < 0){
            std::cerr << "error : epoll fail to insert" << std::endl;
            it = _serve.erase(it);
            continue;
        }
        else{
            std::cout << "success insert socket into epoll" << std::endl;
            this->_nb_socket++;
            it++;
        }
    }
    return (1);
}

int Epoll::accept_new_client(Server &serve){
    
    struct sockaddr_storage client_addr; // peux contenir nimporte quelle IP 4 ou 6
    socklen_t addr_size = sizeof(client_addr);
    struct epoll_event ev;      // dit a epoll quoi surveiller et quoi renvoyer 
    
    serve.set_new_fd(accept(serve.get_sockfd(), (struct sockaddr *)&client_addr, &addr_size)); // cree une socket client _new_fd
    if (serve.get_new_fd() == -1){
        if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) // fausse erreur EAGAIN = "reessaie plus tard" / EWOULDBLOCK = "si j'etais bloquant j'att"
            return (0);
        else{
            std::cerr << "accept failed: " << strerror(errno) << std::endl;
            return (0);
        }
    }
    set_non_blocking(serve.get_new_fd());
    serve.add_client(serve.get_new_fd());
    _fd_to_server[serve.get_new_fd()] = &serve;
    std::cout << "Server: connection established (fd=" << serve.get_new_fd() << ")" << std::endl;
    ev.events = EPOLLIN;
    ev.data.fd = serve.get_new_fd();
    
    if (epoll_ctl(_epoll_g, EPOLL_CTL_ADD, serve.get_new_fd(), &ev) < 0)
        std::cerr << "error : epoll fail to insert socket" << std::endl;
    else 
        this->_nb_socket++;
    return (1);
}

int Epoll::handle_client_event(int n){

    struct epoll_event ev;

    if(send(events[n].data.fd, "Hello world!", 13, 0) == -1){
        std::cerr << "send failed: " << strerror(errno) << std::endl;
        return (0);
    }
    if(epoll_ctl(this->_epoll_g, EPOLL_CTL_DEL, events[n].data.fd, &ev) < 0)
        std::cerr << "error : epoll fail to remove socket" << std::endl;
    else
        this->_nb_socket--;
    close(events[n].data.fd);
    return (1);
}

int Epoll::serv_init(){

    if (!create_and_bind_socket())
        return (0);

    std::cout << "server: waiting for connections..." << std::endl;
    
    if (!setup_epoll())
        return (0);
        
    events = new epoll_event[MAXEPOLLSIZE](); // alloue la place et vide
    while(1){ // boucle pour accepter les prochaine conextion je crois / action
    
        this->_nb_event = epoll_wait(this->_epoll_g, events, MAXEPOLLSIZE, -1);
        if(this->_nb_event == -1){
            std::cerr << "epoll_wait failed: " << strerror(errno) << std::endl;
            delete[] events;
            break;
        }
        for (int n = 0; n < _nb_event; n++) {
            int fd = events[n].data.fd;
            bool is_server = false;

            for (std::vector<Server>::iterator it = _serve.begin(); it != _serve.end(); ++it) {
                if (fd == it->get_sockfd()) {
                    accept_new_client(*it);
                    is_server = true;
                    break;
                }
            }

            if (!is_server) {
                handle_client_event(n);
            }
        }
    }
    for (std::vector<Server>::iterator it = _serve.begin(); it != _serve.end(); ++it) {
        close(it->get_sockfd());
    }
    return(1);
}
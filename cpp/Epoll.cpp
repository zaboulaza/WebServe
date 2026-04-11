/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Epoll.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zaboulaza <zaboulaza@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/29 03:31:00 by zaboulaza         #+#    #+#             */
/*   Updated: 2026/04/11 00:00:00 by zaboulaza        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../hpp/Epoll.hpp"
#include <csignal>

Epoll::Epoll(const Epoll &epoll) {
    *this = epoll;
}

Epoll &Epoll::operator=(const Epoll &epoll) {
    if (this != &epoll) {
        _servers         = epoll._servers;
        _epoll_g         = epoll._epoll_g;
        _nb_sockets      = epoll._nb_sockets;
        _pipe_to_client  = epoll._pipe_to_client;
        _pipe_to_server  = epoll._pipe_to_server;
    }
    return *this;
}

int Epoll::set_non_blocking(int socket_fd) {
    int flags = fcntl(socket_fd, F_GETFL, 0);
    if (flags == -1) {
        std::cerr << "fcntl F_GETFL: " << strerror(errno) << std::endl;
        return -1;
    }
    if (fcntl(socket_fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        std::cerr << "fcntl F_SETFL: " << strerror(errno) << std::endl;
        return -1;
    }
    return 0;
}

int Epoll::create_and_bind_socket(Server &server) {
    struct addrinfo hints;
    struct addrinfo *servinfo;
    struct addrinfo *tmp;
    int status;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family   = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags    = AI_PASSIVE;

    if ((status = getaddrinfo(NULL, server.get_port().c_str(), &hints, &servinfo)) != 0) {
        std::cerr << "getaddrinfo: " << gai_strerror(status) << std::endl;
        return -1;
    }

    for (tmp = servinfo; tmp != NULL; tmp = tmp->ai_next) {
        int sockfd = socket(tmp->ai_family, tmp->ai_socktype, tmp->ai_protocol);
        if (sockfd == -1)
            continue;

        int opt = 1;
        setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

        if (set_non_blocking(sockfd) == -1) {
            close(sockfd);
            continue;
        }
        if (bind(sockfd, tmp->ai_addr, tmp->ai_addrlen) == -1) {
            close(sockfd);
            continue;
        }
        server.set_socketfd(sockfd);
        break;
    }

    if (tmp == NULL) {
        std::cerr << "bind impossible sur le port " << server.get_port() << std::endl;
        freeaddrinfo(servinfo);
        return -1;
    }
    freeaddrinfo(servinfo);

    if (listen(server.get_socketfd(), BACKLOG) == -1) {
        std::cerr << "listen: " << strerror(errno) << std::endl;
        close(server.get_socketfd());
        return -1;
    }
    return 1;
}

int Epoll::setup_epoll() {
    struct epoll_event ev;

    _epoll_g    = epoll_create(MAXEPOLLSIZE);
    _nb_sockets = 0;

    if (_servers.empty()) {
        std::cerr << "Aucun serveur à démarrer." << std::endl;
        return -1;
    }
    for (std::vector<Server>::iterator it = _servers.begin(); it != _servers.end();) {
        ev.events  = EPOLLIN;
        ev.data.fd = it->get_socketfd();
        if (epoll_ctl(_epoll_g, EPOLL_CTL_ADD, it->get_socketfd(), &ev) == -1) {
            std::cerr << "epoll_ctl ADD serveur: " << strerror(errno) << std::endl;
            it = _servers.erase(it);
            continue;
        }
        std::cout << "Serveur en écoute sur le port " << it->get_port() << std::endl;
        _nb_sockets++;
        ++it;
    }
    if (_servers.empty()) {
        std::cerr << "Aucun serveur n'a pu démarrer." << std::endl;
        return -1;
    }
    return 1;
}

int Epoll::accept_new_client(Server &server) {
    struct sockaddr_storage client_addr;
    struct epoll_event ev;
    socklen_t addr_size = sizeof(client_addr);

    int client_fd = accept(server.get_socketfd(),
                           (struct sockaddr *)&client_addr, &addr_size);
    if (client_fd == -1) {
        if (errno != EAGAIN && errno != EWOULDBLOCK)
            std::cerr << "accept: " << strerror(errno) << std::endl;
        return 0;
    }
    set_non_blocking(client_fd);
    server.add_client(client_fd);

    // On surveille EPOLLIN pour lire la requête
    ev.events  = EPOLLIN;
    ev.data.fd = client_fd;
    if (epoll_ctl(_epoll_g, EPOLL_CTL_ADD, client_fd, &ev) == -1) {
        std::cerr << "epoll_ctl ADD client: " << strerror(errno) << std::endl;
        close(client_fd);
        server.remove_client(client_fd);
        return -1;
    }
    _nb_sockets++;
    return 1;
}

// Supprime proprement un client : retire d'epoll, ferme le fd, supprime de la map.
void Epoll::cleanup_client(size_t server_idx, int client_fd) {
    struct epoll_event ev; // nécessaire sur certains kernels < 2.6.9
    epoll_ctl(_epoll_g, EPOLL_CTL_DEL, client_fd, &ev);
    _nb_sockets--;
    close(client_fd);
    _servers[server_idx].remove_client(client_fd);
}

// Gère un événement sur un fd client.
// Délègue à la machine à états du Client, puis met à jour l'abonnement epoll si besoin.
// Retourne 2 si un CGI vient d'être démarré (le pipe doit être enregistré dans epoll).
int Epoll::handle_client_event(int client_fd, uint32_t events) {
    for (size_t i = 0; i < _servers.size(); i++) {
        if (!_servers[i].has_client(client_fd))
            continue;

        Client &client = _servers[i].get_client(client_fd);

        // Erreur ou déconnexion → on nettoie directement
        if (events & (EPOLLERR | EPOLLHUP)) {
            cleanup_client(i, client_fd);
            return -1;
        }

        int result = client.handle_event(_servers[i], events);

        if (result == 2) {
            // Le client a démarré un CGI : on enregistre son pipe dans epoll.
            int pipe_fd = client.get_cgi_pipe();
            fcntl(pipe_fd, F_SETFL, O_NONBLOCK);

            struct epoll_event ev;
            ev.events  = EPOLLIN;
            ev.data.fd = pipe_fd;
            if (epoll_ctl(_epoll_g, EPOLL_CTL_ADD, pipe_fd, &ev) == -1) {
                std::cerr << "epoll_ctl ADD pipe CGI: " << strerror(errno) << std::endl;
                // En cas d'échec, on termine directement le CGI en lisant tout
                // de façon bloquante n'est pas souhaitable — on envoie un 500.
                close(pipe_fd);
                Response resp;
                // On ne peut pas facilement préparer l'envoi ici sans accéder
                // aux internals du client ; on se contente de fermer la connexion.
                cleanup_client(i, client_fd);
                return -1;
            }
            _nb_sockets++;
            _pipe_to_client[pipe_fd] = client_fd;
            _pipe_to_server[pipe_fd] = i;
        } else if (client.get_state() == DONE || result == -1) {
            cleanup_client(i, client_fd);
        } else if (client.get_state() == SENDING) {
            // La réponse n'a pas pu être envoyée en entier → on attend EPOLLOUT
            struct epoll_event ev;
            ev.events  = EPOLLOUT;
            ev.data.fd = client_fd;
            epoll_ctl(_epoll_g, EPOLL_CTL_MOD, client_fd, &ev);
        }
        break;
    }
    return 1;
}

// Gère un événement EPOLLIN/EPOLLHUP/EPOLLERR sur un pipe de sortie CGI.
// Lit les données disponibles ou détecte la fin du CGI, puis finalise la réponse.
void Epoll::handle_cgi_pipe_event(int pipe_fd, uint32_t events) {
    std::map<int, int>::iterator    cit = _pipe_to_client.find(pipe_fd);
    std::map<int, size_t>::iterator sit = _pipe_to_server.find(pipe_fd);

    if (cit == _pipe_to_client.end() || sit == _pipe_to_server.end())
        return; // ne devrait pas arriver

    int    client_fd  = cit->second;
    size_t server_idx = sit->second;
    Client &client    = _servers[server_idx].get_client(client_fd);

    bool cgi_done  = false;
    bool timed_out = false;

    if (events & (EPOLLERR | EPOLLHUP)) {
        // Le pipe a été fermé ou une erreur s'est produite.
        // On lit quand même ce qui reste avant de déclarer la fin.
        client.read_cgi_chunk();
        cgi_done = true;
    } else {
        int result = client.read_cgi_chunk();
        if (result == -1)
            cgi_done = true;
    }

    // Vérification du timeout CGI
    if (!cgi_done && difftime(time(NULL), client.get_cgi_start()) > CGI_TIMEOUT) {
        cgi_done  = true;
        timed_out = true;
    }

    if (cgi_done) {
        // Retirer le pipe d'epoll et le fermer
        epoll_ctl(_epoll_g, EPOLL_CTL_DEL, pipe_fd, NULL);
        close(pipe_fd);
        _nb_sockets--;
        _pipe_to_client.erase(pipe_fd);
        _pipe_to_server.erase(pipe_fd);

        // Construire + envoyer la réponse HTTP
        int send_result = client.finish_cgi(_servers[server_idx], timed_out);

        if (client.get_state() == DONE || send_result == -1) {
            cleanup_client(server_idx, client_fd);
        } else if (send_result == 1) {
            // Envoi partiel → on attend EPOLLOUT sur le fd du client
            struct epoll_event ev;
            ev.events  = EPOLLOUT;
            ev.data.fd = client_fd;
            epoll_ctl(_epoll_g, EPOLL_CTL_MOD, client_fd, &ev);
        }
    }
}

// Parcourt tous les pipes CGI enregistrés et tue ceux qui dépassent le timeout.
// Appelé à chaque itération de la boucle epoll_wait si le timeout est actif.
void Epoll::check_cgi_timeouts() {
    time_t now = time(NULL);
    std::vector<int> to_kill;

    for (std::map<int, int>::iterator it = _pipe_to_client.begin();
         it != _pipe_to_client.end(); ++it) {
        int    pipe_fd    = it->first;
        size_t server_idx = _pipe_to_server[pipe_fd];
        int    client_fd  = it->second;
        Client &client    = _servers[server_idx].get_client(client_fd);

        if (difftime(now, client.get_cgi_start()) > CGI_TIMEOUT)
            to_kill.push_back(pipe_fd);
    }

    for (size_t i = 0; i < to_kill.size(); i++)
        handle_cgi_pipe_event(to_kill[i], EPOLLERR); // force la fin avec timeout
}

int Epoll::init_epoll_servers() {
    for (std::vector<Server>::iterator it = _servers.begin(); it != _servers.end();) {
        if (create_and_bind_socket(*it) == -1) {
            it = _servers.erase(it);
            continue;
        }
        ++it;
    }

    if (setup_epoll() == -1)
        return -1;

    struct epoll_event events[MAXEPOLLSIZE];
    while (1) {
        // Timeout de 5 s pour vérifier les CGI en cours
        int nb_events = epoll_wait(_epoll_g, events, MAXEPOLLSIZE,
                                   _pipe_to_client.empty() ? -1 : 5000);
        if (nb_events == -1) {
            if (errno == EINTR) continue; // signal interrompu, on réessaie
            std::cerr << "epoll_wait: " << strerror(errno) << std::endl;
            return -1;
        }

        // Si epoll_wait a expiré (nb_events == 0) et qu'il y a des CGI → vérifier
        if (nb_events == 0 && !_pipe_to_client.empty()) {
            check_cgi_timeouts();
            continue;
        }

        for (int i = 0; i < nb_events; i++) {
            int fd = events[i].data.fd;

            // Est-ce un pipe CGI ?
            if (_pipe_to_client.find(fd) != _pipe_to_client.end()) {
                handle_cgi_pipe_event(fd, events[i].events);
                continue;
            }

            // Est-ce un socket serveur (accept) ?
            bool is_server_socket = false;
            for (std::vector<Server>::iterator it = _servers.begin();
                 it != _servers.end(); ++it) {
                if (fd == it->get_socketfd()) {
                    accept_new_client(*it);
                    is_server_socket = true;
                    break;
                }
            }
            if (!is_server_socket)
                handle_client_event(fd, events[i].events);
        }

        // Vérification des timeouts CGI à chaque tour de boucle
        if (!_pipe_to_client.empty())
            check_cgi_timeouts();
    }

    for (std::vector<Server>::iterator it = _servers.begin();
         it != _servers.end(); ++it) {
        close(it->get_socketfd());
    }
    close(_epoll_g);
    return 1;
}

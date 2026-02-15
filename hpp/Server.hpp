/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zaboulaza <zaboulaza@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/26 15:15:51 by zaboulaza         #+#    #+#             */
/*   Updated: 2026/02/15 05:30:17 by zaboulaza        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Client.hpp"
#include "Location.hpp"

class Server{
    
    public:
    
        Server() {};
        ~Server() {};
        Server(const Server &server);
        Server &operator=(const Server &server);
        
        void add_client(int clientfd);
        void remove_client(int clientfd);
        bool has_client(int clientfd);
        Client &get_client(int clientfd);
    

        void set_port(std::string port) { _port = port; };
        std::string get_port() { return _port; };
        
        void set_root(std::string root) { _root = root; };
        std::string get_root() { return _root; };

        void set_index(std::string index) { _index = index; };
        std::string get_index() { return _index; };

        void set_allowed_methods(std::set<std::string> allowed_methods) { _allowed_methods = allowed_methods; };
        std::set<std::string> get_allowed_methods() { return _allowed_methods; };

        void set_auto_index(bool auto_index) { _auto_index = auto_index; };
        bool get_auto_index() { return _auto_index; };

        void set_cgi(std::map<std::string, std::string> cgi) { _cgi = cgi; };
        std::map<std::string, std::string> get_cgi() { return _cgi; };

        void set_error_pages(std::map<int, std::string> error_pages) { _error_pages = error_pages; };
        std::map<int, std::string> get_error_pages() { return _error_pages; };

        void set_upload_folder(std::string upload_folder) { _upload_folder = upload_folder; };
        std::string get_upload_folder() { return _upload_folder; };

        void set_redirect(std::string redirect) { _redirect = redirect; };
        std::string get_redirect() { return _redirect; };
    
        void set_socketfd(int socket_fd) { _socketfd = socket_fd; };
        int get_socketfd() { return _socketfd; };
        
        bool get_is_good() { return _is_good; };
        void set_is_good(bool is_good) { _is_good = is_good; };
        
        std::vector<Location> get_locations() { return _locations; };
        void set_locations(std::vector<Location> locations) { _locations = locations; };

    private:

        std::map<int , Client> _clients_map;
        int _socketfd;


        std::string _port;                                  // Le port sur lequel ce server écoute
        std::string _root;                            // Dossier racine des fichiers
        std::string _index;                           // Fichier index par défaut (index.html)
        std::set<std::string> _allowed_methods;      // Méthodes HTTP autorisées (GET, POST, DELETE…)
        bool _auto_index;                             // Active l'affichage du listing des dossiers
        std::map<std::string, std::string> _cgi;     // Extension → chemin du binaire CGI (.py, .php…)
        std::map<int, std::string> _error_pages;     // Code HTTP → chemin de la page d’erreur personnalisée
        std::string _upload_folder;
        std::string _redirect;
        bool _is_good;
        std::vector<Location> _locations;                   // Liste des blocs location associés à ce server
};

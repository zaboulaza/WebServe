/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zaboulaza <zaboulaza@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/26 15:15:51 by zaboulaza         #+#    #+#             */
/*   Updated: 2026/04/11 00:00:00 by zaboulaza        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Client.hpp"
#include "Location.hpp"

class Server {

    public:

        Server();
        ~Server() {}
        Server(const Server &server);
        Server &operator=(const Server &server);

        void add_client(int clientfd);
        void remove_client(int clientfd);
        bool has_client(int clientfd);
        Client &get_client(int clientfd);

        // Retourne le bloc location dont le préfixe correspond le mieux au path.
        // Retourne NULL si aucune location ne correspond.
        Location *find_location(const std::string &path);

        void set_port(const std::string &p)                     { _port = p; }
        std::string get_port() const                            { return _port; }

        void set_root(const std::string &r)                     { _root = r; }
        std::string get_root() const                            { return _root; }

        void set_index(const std::string &i)                    { _index = i; }
        std::string get_index() const                           { return _index; }

        void set_allowed_methods(std::set<std::string> m)       { _allowed_methods = m; }
        std::set<std::string> get_allowed_methods() const       { return _allowed_methods; }

        void set_auto_index(bool v)                             { _auto_index = v; }
        bool get_auto_index() const                             { return _auto_index; }

        void set_cgi(std::map<std::string, std::string> cgi)    { _cgi = cgi; }
        std::map<std::string, std::string> get_cgi() const      { return _cgi; }

        void set_error_pages(std::map<int, std::string> ep)     { _error_pages = ep; }
        std::map<int, std::string> get_error_pages() const      { return _error_pages; }

        void set_upload_folder(const std::string &uf)           { _upload_folder = uf; }
        std::string get_upload_folder() const                   { return _upload_folder; }

        void set_redirect(const std::string &r)                 { _redirect = r; }
        std::string get_redirect() const                        { return _redirect; }

        void set_socketfd(int fd)                               { _socketfd = fd; }
        int get_socketfd() const                                { return _socketfd; }

        void set_is_good(bool v)                                { _is_good = v; }
        bool get_is_good() const                                { return _is_good; }

        // Taille max du corps d'une requête (défaut 10 Mo)
        void set_client_max_body_size(size_t s)                 { _client_max_body_size = s; }
        size_t get_client_max_body_size() const                 { return _client_max_body_size; }

        std::vector<Location> &get_locations()                  { return _locations; }
        void set_locations(const std::vector<Location> &locs)   { _locations = locs; }

        std::map<int, Client> &get_clients()                    { return _clients_map; }

    private:

        std::map<int, Client>            _clients_map;
        int                              _socketfd;

        std::string                      _port;
        std::string                      _root;
        std::string                      _index;
        std::set<std::string>            _allowed_methods;
        bool                             _auto_index;
        std::map<std::string, std::string> _cgi;
        std::map<int, std::string>       _error_pages;
        std::string                      _upload_folder;
        std::string                      _redirect;
        bool                             _is_good;
        size_t                           _client_max_body_size;
        std::vector<Location>            _locations;
};

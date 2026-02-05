/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Location.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zaboulaza <zaboulaza@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/04 16:41:52 by zaboulaza         #+#    #+#             */
/*   Updated: 2026/02/05 04:10:44 by zaboulaza        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Client.hpp"

class Location {
    
    public:
    
        Location() {};
        ~Location() {};
        Location(const Location &Location);
        Location &operator=(const Location &Location);
            
        void set_is_good(bool is_good) { _is_good = is_good; };
        bool get_is_good() { return _is_good; };

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
        
        
    private:
    
        std::string _root;                            // Dossier racine des fichiers
        std::string _index;                           // Fichier index par défaut (index.html)
        std::set<std::string> _allowed_methods;      // Méthodes HTTP autorisées (GET, POST, DELETE…)
        bool _auto_index;                             // Active l'affichage du listing des dossiers
        std::map<std::string, std::string> _cgi;     // Extension → chemin du binaire CGI (.py, .php…)
        std::map<int, std::string> _error_pages;     // Code HTTP → chemin de la page d’erreur personnalisée
        std::string _upload_folder;
        std::string _redirect;
        bool _is_good;
};
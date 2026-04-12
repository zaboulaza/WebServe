/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Location.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zaboulaza <zaboulaza@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/04 16:41:52 by zaboulaza         #+#    #+#             */
/*   Updated: 2026/04/11 00:00:00 by zaboulaza        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Client.hpp"

class Location {

    public:

        Location() : _auto_index(false), _is_good(false) {}
        ~Location() {}
        Location(const Location &loc);
        Location &operator=(const Location &loc);

        void set_is_good(bool v) { _is_good = v; }
        bool get_is_good() const { return _is_good; }

        // prefixe URL du bloc location (ex: "/uploads")
        void set_path(const std::string &path) { _path = path; }
        std::string get_path() const { return _path; }

        void set_root(const std::string &root) { _root = root; }
        std::string get_root() const { return _root; }

        void set_index(const std::string &index) { _index = index; }
        std::string get_index() const { return _index; }

        void set_allowed_methods(std::set<std::string> m) { _allowed_methods = m; }
        std::set<std::string> get_allowed_methods() const { return _allowed_methods; }

        void set_auto_index(bool v) { _auto_index = v; }
        bool get_auto_index() const { return _auto_index; }

        void set_cgi(std::map<std::string, std::string> cgi) { _cgi = cgi; }
        std::map<std::string, std::string> get_cgi() const { return _cgi; }

        void set_error_pages(std::map<int, std::string> ep) { _error_pages = ep; }
        std::map<int, std::string> get_error_pages() const { return _error_pages; }

        void set_upload_folder(const std::string &uf) { _upload_folder = uf; }
        std::string get_upload_folder() const { return _upload_folder; }

        void set_redirect(const std::string &r) { _redirect = r; }
        std::string get_redirect() const { return _redirect; }

    private:

        std::string                      _path;            // préfixe URL (ex: "/uploads")
        std::string                      _root;
        std::string                      _index;
        std::set<std::string>            _allowed_methods;
        bool                             _auto_index;
        std::map<std::string, std::string> _cgi;
        std::map<int, std::string>       _error_pages;
        std::string                      _upload_folder;
        std::string                      _redirect;
        bool                             _is_good;
};

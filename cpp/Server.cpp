/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zaboulaza <zaboulaza@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/27 20:28:17 by zaboulaza         #+#    #+#             */
/*   Updated: 2026/04/11 00:00:00 by zaboulaza        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../hpp/Server.hpp"

Server::Server()
    : _socketfd(-1),
      _root("./www"),
      _index("index.html"),
      _auto_index(false),
      _upload_folder("./www/uploads"),
      _is_good(false),
      _client_max_body_size(10 * 1024 * 1024) // 10 Mo par défaut
{
    _allowed_methods.insert("GET");
    _allowed_methods.insert("POST");
    _allowed_methods.insert("DELETE");
}

Server::Server(const Server &server) {
    *this = server;
}

Server &Server::operator=(const Server &server) {
    if (this != &server) {
        _port                 = server._port;
        _socketfd             = server._socketfd;
        _clients_map          = server._clients_map;
        _root                 = server._root;
        _index                = server._index;
        _allowed_methods      = server._allowed_methods;
        _auto_index           = server._auto_index;
        _cgi                  = server._cgi;
        _error_pages          = server._error_pages;
        _upload_folder        = server._upload_folder;
        _redirect             = server._redirect;
        _is_good              = server._is_good;
        _client_max_body_size = server._client_max_body_size;
        _locations            = server._locations;
    }
    return *this;
}

void Server::add_client(int clientfd) {
    _clients_map[clientfd] = Client(clientfd);
}

void Server::remove_client(int clientfd) {
    _clients_map.erase(clientfd);
}

bool Server::has_client(int clientfd) {
    return _clients_map.find(clientfd) != _clients_map.end();
}

Client &Server::get_client(int clientfd) {
    return _clients_map[clientfd];
}

// Algorithme "longest prefix match" comme nginx :
// on cherche le bloc location dont le préfixe URL est le plus long à correspondre au path.
Location *Server::find_location(const std::string &path) {
    Location *best     = NULL;
    size_t    best_len = 0;

    for (size_t i = 0; i < _locations.size(); i++) {
        const std::string &loc_path = _locations[i].get_path();
        if (loc_path.empty())
            continue;

        // Le path doit commencer par le préfixe de la location
        if (path.substr(0, loc_path.size()) != loc_path)
            continue;

        // Et le caractère suivant doit être '/', '\0', ou le préfixe est "/"
        // → évite que "/up" matche "/uploads"
        size_t after = loc_path.size();
        if (after < path.size() && path[after] != '/' && loc_path != "/")
            continue;

        if (loc_path.size() > best_len) {
            best_len = loc_path.size();
            best     = &_locations[i];
        }
    }
    return best;
}

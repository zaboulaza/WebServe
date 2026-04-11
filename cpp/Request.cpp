/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zaboulaza <zaboulaza@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/15 00:55:26 by zaboulaza         #+#    #+#             */
/*   Updated: 2026/04/11 00:00:00 by zaboulaza        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../hpp/Request.hpp"

Request::Request(const Request &request) {
    *this = request;
}

Request &Request::operator=(const Request &request) {
    if (this != &request) {
        _header         = request._header;
        _vec_header     = request._vec_header;
        _method         = request._method;
        _path           = request._path;
        _version        = request._version;
        _content_length = request._content_length;
        _body           = request._body;
    }
    return *this;
}

std::vector<std::string> Request::split(std::string str, char delimiter) {
    std::vector<std::string> res;
    size_t pos;

    while ((pos = str.find(delimiter)) != std::string::npos) {
        std::string token = str.substr(0, pos);
        if (!token.empty())
            res.push_back(token);
        str.erase(0, pos + 1);
    }
    if (!str.empty())
        res.push_back(str);
    return res;
}

std::vector<std::string> Request::split_first(std::string str, char delimiter) {
    std::vector<std::string> res;
    size_t pos;

    if ((pos = str.find(delimiter)) != std::string::npos) {
        res.push_back(str.substr(0, pos));
        str.erase(0, pos + 1);
    }
    if (!str.empty())
        res.push_back(str);
    return res;
}

// Parse la première ligne : METHOD PATH HTTP/VERSION
// On accepte n'importe quelle méthode valide (token HTTP) — la validation
// par rapport aux méthodes autorisées est faite dans validate_header().
// Retourne false uniquement si le format est invalide (400 Bad Request).
bool Request::set_first_line(std::string str) {
    std::vector<std::string> line = split(str, ' ');

    if (line.size() != 3)
        return false;

    // Le path doit commencer par '/' et ne pas contenir de traversal (..)
    if (line[1].empty() || line[1][0] != '/' || line[1].find("..") != std::string::npos)
        return false;

    // On n'accepte que HTTP/1.0 et HTTP/1.1
    if (line[2] != "HTTP/1.1" && line[2] != "HTTP/1.0")
        return false;

    _method  = line[0];
    _path    = line[1];
    _version = line[2];
    return true;
}

std::string Request::trim(std::string str) {
    size_t start = str.find_first_not_of(" \t\n\r\f\v\"");
    if (start == std::string::npos)
        return "";
    size_t end = str.find_last_not_of(" \t\n\r\f\v\"");
    return str.substr(start, end - start + 1);
}

bool Request::pars_head(std::string str) {
    std::vector<std::string> line = split_first(str, ':');
    for (size_t i = 0; i < line.size(); i++)
        line[i] = trim(line[i]);

    if (line.size() != 2 || line[0].empty())
        return false;

    // Content-Length est insensible à la casse selon la RFC
    if ((line[0] == "Content-Length" ||
         line[0] == "Content-length" ||
         line[0] == "content-length") && !line[1].empty()) {
        _content_length = atoi(line[1].c_str());
    } else {
        _header[line[0]] = line[1];
    }
    return true;
}

int Request::parse_header(std::string str) {
    _vec_header = split(str, '\n');

    for (size_t i = 0; i < _vec_header.size(); i++) {
        // Enlève le \r final si présent
        if (!_vec_header[i].empty() && _vec_header[i][_vec_header[i].size() - 1] == '\r')
            _vec_header[i] = _vec_header[i].substr(0, _vec_header[i].size() - 1);

        if (i == 0) {
            if (!set_first_line(_vec_header[i]))
                return -1;
        } else if (!_vec_header[i].empty()) {
            if (!pars_head(_vec_header[i]))
                return -1;
        } else {
            return 1; // ligne vide = fin du header
        }
    }
    return 1;
}

// Valide la méthode et la taille du corps.
// max_body_size : limite configurée dans le bloc server (client_max_body_size).
// Retourne 1 si OK, sinon le code d'erreur HTTP.
int Request::validate_header(std::set<std::string> allowed_methods, size_t max_body_size) {
    // Méthode non autorisée pour cette ressource → 405
    if (allowed_methods.find(_method) == allowed_methods.end())
        return 405;

    // POST sans Content-Length → 411 Length Required
    if (_method == "POST" && _content_length == -1)
        return 411;

    // Corps trop grand → 413 Payload Too Large
    if (_content_length != -1 && (size_t)_content_length > max_body_size)
        return 413;

    return 1;
}

/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zaboulaza <zaboulaza@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/24 09:12:01 by zaboulaza         #+#    #+#             */
/*   Updated: 2026/04/11 00:00:00 by zaboulaza        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Request.hpp"

class Server;
class Location;

class Response {

    public:

        Response() {}
        ~Response() {}
        Response(const Response &r);
        Response &operator=(const Response &r);

        // construit la reponse HTTP complete (pour les requetes non-CGI)
        std::string build_response(Server &server, Request &request);

        // page d'erreur : personnalisee si definie dans la config, sinon defaut
        std::string build_error_response(int code, Server &server);

        // ajoute un header custom (utilise pour Set-Cookie)
        void add_header(const std::string &key, const std::string &value);

        // cherche l'interpreteur CGI pour l'extension du fichier demande
        static std::string get_cgi_interpreter(const Request &request,
                                                const Location *loc,
                                                const Server &server);

        // encapsule la sortie brute du CGI dans une reponse HTTP
        std::string finish_cgi_response(const std::string &cgi_output, Server &server);

    private:

        // headers injectes dans toutes les reponses (Set-Cookie, etc.)
        std::map<std::string, std::string> _extra_headers;

        std::string build_GET_response(const std::string &root,
                                        const std::string &index,
                                        bool auto_index,
                                        Server &server,
                                        Request &request);

        std::string build_POST_response(const std::string &upload_folder,
                                         Server &server,
                                         Request &request);

        std::string build_DELETE_response(const std::string &root,
                                           Server &server,
                                           Request &request);

        std::string build_redirect_response(const std::string &destination);

        // Génère le listing HTML d'un répertoire (auto-index).
        std::string generate_autoindex(const std::string &fs_path,
                                        const std::string &url_path);

        // Construit les en-têtes HTTP (status line + headers standards + _extra_headers).
        std::string build_headers(const std::string &status,
                                   const std::string &content_type,
                                   size_t content_length);

        // Détermine le Content-Type selon l'extension du fichier.
        std::string get_content_type(const std::string &path);
};

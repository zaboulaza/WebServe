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

// La classe Response construit la réponse HTTP sous forme de std::string.
// Elle n'appelle jamais send() directement — c'est le Client qui envoie.
// Les _extra_headers permettent d'ajouter des en-têtes custom (ex: Set-Cookie pour les bonus).
class Response {

    public:

        Response() {}
        ~Response() {}
        Response(const Response &r);
        Response &operator=(const Response &r);

        // Point d'entrée principal : retourne la réponse HTTP complète sous forme de string.
        // Note : ne gère plus le CGI directement (Client intercepte avant d'appeler ici).
        std::string build_response(Server &server, Request &request);

        // Construit une réponse d'erreur HTTP (400, 404, 405…).
        // Vérifie si le serveur a une page d'erreur personnalisée dans sa config.
        std::string build_error_response(int code, Server &server);

        // Bonus-ready : permet d'ajouter des en-têtes supplémentaires dans toutes les réponses.
        // Exemple d'utilisation future : add_header("Set-Cookie", "session_id=abc; Path=/")
        void add_header(const std::string &key, const std::string &value);

        // Cherche l'interpréteur CGI pour l'extension du fichier demandé.
        // Méthode statique publique pour que Client.cpp puisse l'appeler sans instancier Response.
        static std::string get_cgi_interpreter(const Request &request,
                                                const Location *loc,
                                                const Server &server);

        // Encapsule la sortie brute du CGI (collectée via epoll) dans une réponse HTTP complète.
        std::string finish_cgi_response(const std::string &cgi_output, Server &server);

    private:

        // En-têtes supplémentaires injectés dans toutes les réponses (prévu pour les bonus).
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

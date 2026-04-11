/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zaboulaza <zaboulaza@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/24 09:11:29 by zaboulaza         #+#    #+#             */
/*   Updated: 2026/04/11 00:00:00 by zaboulaza        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../hpp/Response.hpp"
#include "../hpp/Server.hpp"
#include "../hpp/Location.hpp"

#include <dirent.h>
#include <sys/stat.h>
#include <ctime>

// ─── Copie / assignation ─────────────────────────────────────────────────────

Response::Response(const Response &r) {
    *this = r;
}

Response &Response::operator=(const Response &r) {
    if (this != &r)
        _extra_headers = r._extra_headers;
    return *this;
}

// Ajoute un en-tête custom (utilisé pour les bonus, ex: Set-Cookie).
void Response::add_header(const std::string &key, const std::string &value) {
    _extra_headers[key] = value;
}

// ─── Helpers ─────────────────────────────────────────────────────────────────

// Détermine le Content-Type selon l'extension du fichier.
std::string Response::get_content_type(const std::string &path) {
    size_t dot = path.find_last_of('.');
    if (dot == std::string::npos)
        return "application/octet-stream";

    std::string ext = path.substr(dot);
    if (ext == ".html" || ext == ".htm") return "text/html";
    if (ext == ".css")                   return "text/css";
    if (ext == ".js")                    return "application/javascript";
    if (ext == ".json")                  return "application/json";
    if (ext == ".png")                   return "image/png";
    if (ext == ".jpg" || ext == ".jpeg") return "image/jpeg";
    if (ext == ".gif")                   return "image/gif";
    if (ext == ".ico")                   return "image/x-icon";
    if (ext == ".txt")                   return "text/plain";
    if (ext == ".pdf")                   return "application/pdf";
    return "application/octet-stream";
}

// Construit la partie en-têtes d'une réponse HTTP (sans le corps).
// Injecte aussi les _extra_headers (bonus: cookies, etc.).
std::string Response::build_headers(const std::string &status,
                                     const std::string &content_type,
                                     size_t content_length) {
    std::ostringstream oss;
    oss << "HTTP/1.1 " << status << "\r\n";
    oss << "Content-Type: "   << content_type   << "\r\n";
    oss << "Content-Length: " << content_length  << "\r\n";
    oss << "Connection: close\r\n";

    // En-têtes supplémentaires (bonus: Set-Cookie, etc.)
    for (std::map<std::string, std::string>::const_iterator it = _extra_headers.begin();
         it != _extra_headers.end(); ++it) {
        oss << it->first << ": " << it->second << "\r\n";
    }
    oss << "\r\n";
    return oss.str();
}

// ─── Réponse d'erreur ────────────────────────────────────────────────────────

std::string Response::build_error_response(int code, Server &server) {
    // Vérifier si une page d'erreur personnalisée est configurée
    std::map<int, std::string> error_pages = server.get_error_pages();
    std::map<int, std::string>::const_iterator it = error_pages.find(code);
    if (it != error_pages.end()) {
        std::string page_path = server.get_root() + it->second;
        std::ifstream ifs(page_path.c_str());
        if (ifs) {
            std::string body((std::istreambuf_iterator<char>(ifs)),
                              std::istreambuf_iterator<char>());
            std::ostringstream code_str;
            code_str << code;
            return build_headers(code_str.str() + " Error",
                                  get_content_type(page_path),
                                  body.size()) + body;
        }
    }

    // Page d'erreur par défaut
    std::string msg;
    if      (code == 400) msg = "400 Bad Request";
    else if (code == 403) msg = "403 Forbidden";
    else if (code == 404) msg = "404 Not Found";
    else if (code == 405) msg = "405 Method Not Allowed";
    else if (code == 411) msg = "411 Length Required";
    else if (code == 413) msg = "413 Payload Too Large";
    else if (code == 500) msg = "500 Internal Server Error";
    else if (code == 502) msg = "502 Bad Gateway";
    else if (code == 504) msg = "504 Gateway Timeout";
    else {
        std::ostringstream oss;
        oss << code << " Error";
        msg = oss.str();
    }

    std::string body = "<!DOCTYPE html><html><head><title>" + msg +
                       "</title></head><body><h1>" + msg +
                       "</h1></body></html>";
    return build_headers(msg, "text/html", body.size()) + body;
}

// ─── Redirect ────────────────────────────────────────────────────────────────

// Envoie un 301 Moved Permanently (redirect permanent).
// Si la destination commence par 'http' → redirect absolu, sinon relatif.
std::string Response::build_redirect_response(const std::string &destination) {
    std::string body = "<!DOCTYPE html><html><head><title>301 Moved Permanently</title>"
                       "</head><body><h1>301 Moved Permanently</h1>"
                       "<p><a href=\"" + destination + "\">" + destination +
                       "</a></p></body></html>";
    std::ostringstream oss;
    oss << "HTTP/1.1 301 Moved Permanently\r\n";
    oss << "Location: " << destination << "\r\n";
    oss << "Content-Type: text/html\r\n";
    oss << "Content-Length: " << body.size() << "\r\n";
    oss << "Connection: close\r\n";
    for (std::map<std::string, std::string>::const_iterator it = _extra_headers.begin();
         it != _extra_headers.end(); ++it) {
        oss << it->first << ": " << it->second << "\r\n";
    }
    oss << "\r\n" << body;
    return oss.str();
}

// ─── Auto-index ──────────────────────────────────────────────────────────────

// Génère une page HTML listant le contenu d'un répertoire (comme nginx autoindex).
std::string Response::generate_autoindex(const std::string &fs_path,
                                          const std::string &url_path) {
    DIR *dir = opendir(fs_path.c_str());
    if (!dir)
        return "";

    std::ostringstream html;
    html << "<!DOCTYPE html><html><head>"
         << "<title>Index of " << url_path << "</title>"
         << "<style>body{font-family:monospace;padding:20px;}"
         << "a{text-decoration:none;color:#0066cc;}"
         << "a:hover{text-decoration:underline;}</style>"
         << "</head><body>"
         << "<h1>Index of " << url_path << "</h1><hr><ul>";

    // Lien vers le parent
    if (url_path != "/")
        html << "<li><a href=\"../\">../</a></li>";

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        std::string name = entry->d_name;
        if (name == "." || name == "..")
            continue;

        std::string full = fs_path + "/" + name;
        struct stat st;
        std::string display = name;
        std::string href    = name;

        if (stat(full.c_str(), &st) == 0 && S_ISDIR(st.st_mode)) {
            display += "/";
            href    += "/";
        }
        html << "<li><a href=\"" << href << "\">" << display << "</a></li>";
    }
    closedir(dir);

    html << "</ul><hr></body></html>";
    return html.str();
}

// ─── CGI – chercher l'interpréteur ───────────────────────────────────────────

// Méthode statique publique : cherche l'interpréteur CGI pour l'extension du
// fichier demandé. Cherche d'abord dans la location, puis dans le serveur.
// Appelée par Client.cpp pour décider si une requête doit passer par CGI.
std::string Response::get_cgi_interpreter(const Request &request,
                                           const Location *loc,
                                           const Server &server) {
    // Ne garder que le chemin, sans la query string
    std::string path = request.get_path();
    size_t qpos = path.find('?');
    if (qpos != std::string::npos)
        path = path.substr(0, qpos);

    size_t dot = path.find_last_of('.');
    if (dot == std::string::npos)
        return "";

    std::string ext = path.substr(dot);

    // Priorité à la location
    if (loc) {
        std::map<std::string, std::string> cgi = loc->get_cgi();
        std::map<std::string, std::string>::const_iterator it = cgi.find(ext);
        if (it != cgi.end())
            return it->second;
    }

    // Fallback : config du serveur
    std::map<std::string, std::string> cgi = server.get_cgi();
    std::map<std::string, std::string>::const_iterator it = cgi.find(ext);
    if (it != cgi.end())
        return it->second;

    return "";
}

// ─── CGI – encapsuler la sortie dans une réponse HTTP ────────────────────────

// Reçoit la sortie brute collectée depuis le pipe CGI (via epoll dans Epoll.cpp)
// et la transforme en réponse HTTP/1.1 complète.
std::string Response::finish_cgi_response(const std::string &output, Server &server) {
    // Le CGI peut avoir généré ses propres en-têtes (Content-Type, etc.)
    // On cherche la séquence de fin d'en-têtes (\r\n\r\n ou \n\n).
    size_t header_end = output.find("\r\n\r\n");
    size_t sep_len = 4;
    if (header_end == std::string::npos) {
        header_end = output.find("\n\n");
        sep_len = 2;
    }

    if (header_end != std::string::npos) {
        // Le CGI a produit ses propres en-têtes → on les encapsule.
        std::string cgi_headers = output.substr(0, header_end);
        std::string body        = output.substr(header_end + sep_len);

        std::ostringstream oss;
        oss << "HTTP/1.1 200 OK\r\n";
        oss << cgi_headers << "\r\n";
        oss << "Content-Length: " << body.size() << "\r\n";
        oss << "Connection: close\r\n";
        oss << "\r\n";
        oss << body;
        return oss.str();
    }

    // Aucun en-tête CGI → on génère un Content-Type text/html générique.
    (void)server;
    return build_headers("200 OK", "text/html", output.size()) + output;
}

// ─── GET ─────────────────────────────────────────────────────────────────────

std::string Response::build_GET_response(const std::string &root,
                                          const std::string &index,
                                          bool auto_index,
                                          Server &server,
                                          Request &request) {
    std::string url_path = request.get_path();

    // Tronquer la query string si présente
    size_t qpos = url_path.find('?');
    if (qpos != std::string::npos)
        url_path = url_path.substr(0, qpos);

    std::string fs_path = root + url_path;

    // Vérifier si c'est un répertoire
    struct stat st;
    if (stat(fs_path.c_str(), &st) == 0 && S_ISDIR(st.st_mode)) {
        // S'assurer qu'il y a un '/' final pour que les liens relatifs fonctionnent
        if (fs_path[fs_path.size() - 1] != '/')
            fs_path += "/";

        // Chercher le fichier index
        std::string index_path = fs_path + index;
        std::ifstream ifs_index(index_path.c_str());
        if (ifs_index) {
            std::string body((std::istreambuf_iterator<char>(ifs_index)),
                              std::istreambuf_iterator<char>());
            return build_headers("200 OK", "text/html", body.size()) + body;
        }

        // Auto-index activé → listing
        if (auto_index) {
            std::string listing = generate_autoindex(fs_path, url_path);
            if (!listing.empty())
                return build_headers("200 OK", "text/html", listing.size()) + listing;
        }

        // Répertoire sans index et sans auto-index → 403
        return build_error_response(403, server);
    }

    // Cas classique : servir le fichier
    if (url_path == "/")
        fs_path = root + "/" + index;

    std::ifstream ifs(fs_path.c_str(), std::ifstream::binary);
    if (!ifs)
        return build_error_response(404, server);

    std::string body((std::istreambuf_iterator<char>(ifs)),
                      std::istreambuf_iterator<char>());
    return build_headers("200 OK", get_content_type(fs_path), body.size()) + body;
}

// ─── POST ────────────────────────────────────────────────────────────────────

std::string Response::build_POST_response(const std::string &upload_folder,
                                           Server &server,
                                           Request &request) {
    // Créer le dossier d'upload s'il n'existe pas
    mkdir(upload_folder.c_str(), 0755);

    time_t ts = time(NULL);
    std::ostringstream ss;
    ss << ts;
    std::string file_name = ss.str() + ".dat";
    std::string path      = upload_folder + "/" + file_name;

    std::ofstream file(path.c_str(), std::ios::binary);
    if (!file)
        return build_error_response(500, server);

    file << request.get_body();
    file.close();

    std::string body = "<!DOCTYPE html><html><head><title>201 Created</title></head>"
                       "<body><h1>201 Created</h1><p>Fichier : " + file_name +
                       "</p></body></html>";
    return build_headers("201 Created", "text/html", body.size()) + body;
}

// ─── DELETE ──────────────────────────────────────────────────────────────────

std::string Response::build_DELETE_response(const std::string &root,
                                              Server &server,
                                              Request &request) {
    std::string path = root + request.get_path();

    if (std::remove(path.c_str()) == 0) {
        // 204 No Content : pas de corps
        return "HTTP/1.1 204 No Content\r\nConnection: close\r\n\r\n";
    }
    return build_error_response(404, server);
}

// ─── Point d'entrée principal ────────────────────────────────────────────────

// Construit la réponse HTTP complète pour les requêtes NON-CGI.
// Le CGI est intercepté en amont par Client.cpp (qui appelle start_cgi() et
// retourne 2 pour que Epoll enregistre le pipe de sortie CGI).
// Logique : find_location → redirect ? → méthode HTTP standard
std::string Response::build_response(Server &server, Request &request) {
    // 1. Trouver le bloc location correspondant au path (longest prefix match)
    Location *loc = server.find_location(request.get_path());

    // 2. Config effective : la location surcharge le serveur si elle a une valeur
    std::string root = server.get_root();
    if (loc && !loc->get_root().empty())
        root = loc->get_root();

    std::string index = server.get_index();
    if (loc && !loc->get_index().empty())
        index = loc->get_index();

    std::string upload_folder = server.get_upload_folder();
    if (loc && !loc->get_upload_folder().empty())
        upload_folder = loc->get_upload_folder();

    bool auto_index = server.get_auto_index();
    if (loc)
        auto_index = loc->get_auto_index();

    // 3. Redirect ?
    std::string redirect = server.get_redirect();
    if (loc && !loc->get_redirect().empty())
        redirect = loc->get_redirect();

    if (!redirect.empty())
        return build_redirect_response(redirect);

    // 4. Méthode HTTP standard (CGI déjà intercepté par Client avant cet appel)
    if (request.get_method() == "GET")
        return build_GET_response(root, index, auto_index, server, request);
    if (request.get_method() == "POST")
        return build_POST_response(upload_folder, server, request);
    if (request.get_method() == "DELETE")
        return build_DELETE_response(root, server, request);

    // Méthode reçue mais non gérée (ne devrait pas arriver après validate_header)
    return build_error_response(405, server);
}

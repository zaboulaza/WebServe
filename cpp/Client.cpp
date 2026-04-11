/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zaboulaza <zaboulaza@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/26 15:16:56 by zaboulaza         #+#    #+#             */
/*   Updated: 2026/04/11 00:00:00 by zaboulaza        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../hpp/Client.hpp"
#include "../hpp/Server.hpp"
#include "../hpp/Session.hpp"

#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <cerrno>
#include <cstring>
#include <csignal>
#include <vector>
#include <sstream>

// ─── Constructeurs ────────────────────────────────────────────────────────────

Client::Client()
    : _socket_fd(-1), _state(READING_HEADER), _send_offset(0),
      _cgi_pid(-1), _cgi_pipe_out(-1), _cgi_start_time(0),
      _new_session(false) {}

Client::Client(int fd)
    : _socket_fd(fd), _state(READING_HEADER), _send_offset(0),
      _cgi_pid(-1), _cgi_pipe_out(-1), _cgi_start_time(0),
      _new_session(false) {}

Client::Client(const Client &c) {
    *this = c;
}

Client &Client::operator=(const Client &c) {
    if (this != &c) {
        _socket_fd      = c._socket_fd;
        _state          = c._state;
        _request        = c._request;
        _raw_buffer     = c._raw_buffer;
        _send_buffer    = c._send_buffer;
        _send_offset    = c._send_offset;
        _cgi_pid        = c._cgi_pid;
        _cgi_pipe_out   = c._cgi_pipe_out;
        _cgi_output     = c._cgi_output;
        _cgi_start_time = c._cgi_start_time;
        _session_id     = c._session_id;
        _new_session    = c._new_session;
    }
    return *this;
}

// ─── Machine à états ─────────────────────────────────────────────────────────

// Point d'entrée appelé par Epoll à chaque événement sur ce fd.
// Retourne : -1=terminé/erreur, 0=continuer EPOLLIN, 1=besoin EPOLLOUT, 2=CGI démarré
int Client::handle_event(Server &server, uint32_t events) {
    if (events & (EPOLLERR | EPOLLHUP)) {
        _state = DONE;
        return -1;
    }
    if ((events & EPOLLIN) && (_state == READING_HEADER || _state == READING_BODY)) {
        if (_state == READING_HEADER)
            return read_header(server);
        return read_body(server);
    }
    if ((events & EPOLLOUT) && _state == SENDING)
        return do_send();
    return 0;
}

// ─── Lecture du header ───────────────────────────────────────────────────────

// Lit des octets dans _raw_buffer jusqu'à trouver \r\n\r\n.
// Un seul recv() par événement epoll → jamais bloquant.
// Retourne : -1=fin, 0=attendre plus de données, 2=CGI démarré
int Client::read_header(Server &server) {
    char tmp[8192];
    int bytes = recv(_socket_fd, tmp, sizeof(tmp), 0);

    if (bytes < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            return 0; // pas encore de données, on attend
        _state = DONE;
        return -1;
    }
    if (bytes == 0) { // client déconnecté
        _state = DONE;
        return -1;
    }
    _raw_buffer.append(tmp, bytes);

    size_t pos = _raw_buffer.find("\r\n\r\n");

    // Header incomplet : vérifier la limite de taille avant de continuer
    if (pos == std::string::npos) {
        if (_raw_buffer.size() > 8192) {
            Response resp;
            prepare_send(resp.build_error_response(431, server));
            return do_send();
        }
        return 0; // header incomplet, on attend le prochain événement epoll
    }

    // Header complet : vérifier que la partie header ne dépasse pas 8 Ko
    if (pos > 8192) {
        Response resp;
        prepare_send(resp.build_error_response(431, server));
        return do_send();
    }

    // Header complet : on sépare header et début du corps éventuel
    std::string header = _raw_buffer.substr(0, pos + 4);
    _raw_buffer        = _raw_buffer.substr(pos + 4); // reste = début du corps

    // --- Parsing ---
    if (_request.parse_header(header) == -1) {
        Response resp;
        prepare_send(resp.build_error_response(400, server));
        return do_send();
    }

    // Validation de la méthode et de la taille du corps
    // On cherche le bloc location pour avoir les méthodes autorisées spécifiques
    Location *loc = server.find_location(_request.get_path());
    std::set<std::string> methods =
        (loc && !loc->get_allowed_methods().empty())
        ? loc->get_allowed_methods()
        : server.get_allowed_methods();

    int validate = _request.validate_header(methods, server.get_client_max_body_size());
    if (validate != 1) {
        Response resp;
        prepare_send(resp.build_error_response(validate, server));
        return do_send();
    }

    // ── Gestion des sessions (bonus) — avant tout branchement CGI/body ────────
    {
        std::string cookie = _request.get_header("Cookie");
        std::string sid    = SessionManager::extract_from_cookie(cookie);
        _session_id = SessionManager::instance().get_or_create(sid, _new_session);
        SessionManager::instance().touch(_session_id);
    }

    // Si POST avec corps, on passe en lecture du corps
    if (_request.get_method() == "POST" && _request.get_content_length() > 0) {
        _state = READING_BODY;
        return read_body(server); // traite les octets déjà reçus dans _raw_buffer
    }

    // Vérifier si c'est une requête CGI (GET ou DELETE sur un script)
    std::string interpreter = Response::get_cgi_interpreter(_request, loc, server);
    if (!interpreter.empty()) {
        // Vérifier que le script existe avant de forker
        std::string cgi_root = server.get_root();
        if (loc && !loc->get_root().empty()) cgi_root = loc->get_root();
        std::string url_cgi = _request.get_path();
        size_t qp = url_cgi.find('?');
        if (qp != std::string::npos) url_cgi = url_cgi.substr(0, qp);
        struct stat cgi_st;
        if (stat((cgi_root + url_cgi).c_str(), &cgi_st) != 0) {
            Response resp;
            prepare_send(resp.build_error_response(404, server));
            return do_send();
        }
        int pipe_fd = start_cgi(server, interpreter);
        if (pipe_fd < 0) {
            Response resp;
            prepare_send(resp.build_error_response(500, server));
            return do_send();
        }
        _state = CGI_RUNNING;
        return 2; // signaler à Epoll d'enregistrer le pipe CGI
    }

    // Sinon, on construit directement la réponse
    Response resp;
    if (_new_session)
        resp.add_header("Set-Cookie", "session_id=" + _session_id + "; Path=/; HttpOnly");
    prepare_send(resp.build_response(server, _request));
    return do_send();
}

// ─── Lecture du corps (POST) ─────────────────────────────────────────────────

// Accumule les octets jusqu'à atteindre Content-Length.
// Un seul recv() par événement epoll.
// Retourne : -1=fin, 0=attendre plus de données, 2=CGI démarré
int Client::read_body(Server &server) {
    size_t needed = (size_t)_request.get_content_length();

    // S'il manque encore des octets, on en lit un chunk
    if (_raw_buffer.size() < needed) {
        char tmp[8192];
        int bytes = recv(_socket_fd, tmp, sizeof(tmp), 0);
        if (bytes < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                return 0; // pas encore de données
            _state = DONE;
            return -1;
        }
        if (bytes == 0) { // déconnexion avant la fin du corps
            _state = DONE;
            return -1;
        }
        _raw_buffer.append(tmp, bytes);
    }

    // Corps toujours incomplet → on attend le prochain événement epoll
    if (_raw_buffer.size() < needed)
        return 0;

    // Corps complet
    _request.set_body(_raw_buffer.substr(0, needed));

    // Vérifier si c'est un POST CGI
    Location *loc = server.find_location(_request.get_path());
    std::string interpreter = Response::get_cgi_interpreter(_request, loc, server);
    if (!interpreter.empty()) {
        // Vérifier que le script existe avant de forker
        std::string cgi_root2 = server.get_root();
        if (loc && !loc->get_root().empty()) cgi_root2 = loc->get_root();
        std::string url_cgi2 = _request.get_path();
        size_t qp2 = url_cgi2.find('?');
        if (qp2 != std::string::npos) url_cgi2 = url_cgi2.substr(0, qp2);
        struct stat cgi_st2;
        if (stat((cgi_root2 + url_cgi2).c_str(), &cgi_st2) != 0) {
            Response resp;
            prepare_send(resp.build_error_response(404, server));
            return do_send();
        }
        int pipe_fd = start_cgi(server, interpreter);
        if (pipe_fd < 0) {
            Response resp;
            prepare_send(resp.build_error_response(500, server));
            return do_send();
        }
        _state = CGI_RUNNING;
        return 2; // signaler à Epoll d'enregistrer le pipe CGI
    }

    Response resp;
    if (_new_session)
        resp.add_header("Set-Cookie", "session_id=" + _session_id + "; Path=/; HttpOnly");
    prepare_send(resp.build_response(server, _request));
    return do_send();
}

// ─── CGI : démarrage du processus ────────────────────────────────────────────

// Fork + execve le script CGI.
// - Écrit le corps POST dans pipe_in, puis ferme pipe_in.
// - Stocke le fd pipe_out dans _cgi_pipe_out (NON bloquant).
// - Stocke le pid dans _cgi_pid et l'heure de démarrage dans _cgi_start_time.
// Retourne pipe_out fd (>= 0) en cas de succès, -1 en cas d'erreur.
int Client::start_cgi(Server &server, const std::string &interpreter) {
    // Résoudre root et chemin complet du script
    Location *loc = server.find_location(_request.get_path());

    std::string root = server.get_root();
    if (loc && !loc->get_root().empty())
        root = loc->get_root();

    // Séparer path et query string
    std::string url_path = _request.get_path();
    size_t qpos = url_path.find('?');
    if (qpos != std::string::npos)
        url_path = url_path.substr(0, qpos);

    std::string full_path = root + url_path;

    // Répertoire du script pour chdir()
    std::string script_dir = full_path.substr(0, full_path.find_last_of('/'));
    if (script_dir.empty())
        script_dir = ".";

    int pipe_in[2];
    int pipe_out[2];
    pipe_in[0] = pipe_in[1] = -1;
    pipe_out[0] = pipe_out[1] = -1;

    if (pipe(pipe_in) == -1) return -1;
    if (pipe(pipe_out) == -1) {
        close(pipe_in[0]); close(pipe_in[1]);
        return -1;
    }

    // pipe_in[1] en non-bloquant : évite de bloquer l'event loop si le buffer pipe est plein
    fcntl(pipe_in[1], F_SETFL, O_NONBLOCK);

    pid_t pid = fork();
    if (pid < 0) {
        close(pipe_in[0]);  close(pipe_in[1]);
        close(pipe_out[0]); close(pipe_out[1]);
        return -1;
    }

    if (pid == 0) {
        // ── Processus enfant (CGI) ──────────────────────────────────────

        // Rediriger stdin ← pipe_in, stdout → pipe_out
        dup2(pipe_in[0],  STDIN_FILENO);
        dup2(pipe_out[1], STDOUT_FILENO);
        close(pipe_in[0]);  close(pipe_in[1]);
        close(pipe_out[0]); close(pipe_out[1]);

        // Se placer dans le répertoire du script (requis par le sujet)
        if (chdir(script_dir.c_str()) == -1)
            exit(1);

        // Variables d'environnement CGI/1.1 standard
        std::vector<std::string> env_vars;
        env_vars.push_back("REQUEST_METHOD=" + _request.get_method());
        env_vars.push_back("SCRIPT_FILENAME=" + full_path);
        env_vars.push_back("SCRIPT_NAME="     + _request.get_path());
        env_vars.push_back("PATH_INFO="        + _request.get_path());
        env_vars.push_back("SERVER_PROTOCOL=HTTP/1.1");
        env_vars.push_back("SERVER_PORT="      + server.get_port());
        env_vars.push_back("GATEWAY_INTERFACE=CGI/1.1");
        env_vars.push_back("REDIRECT_STATUS=200"); // requis par PHP-CGI

        // Host
        std::string host = _request.get_header("Host");
        if (!host.empty())
            env_vars.push_back("HTTP_HOST=" + host);

        // Query string (après '?' dans l'URL)
        std::string query_string;
        size_t qs_pos = _request.get_path().find('?');
        if (qs_pos != std::string::npos)
            query_string = _request.get_path().substr(qs_pos + 1);
        env_vars.push_back("QUERY_STRING=" + query_string);

        if (_request.get_method() == "POST") {
            std::ostringstream cl;
            cl << _request.get_content_length();
            env_vars.push_back("CONTENT_LENGTH=" + cl.str());
            std::string ct = _request.get_header("Content-Type");
            if (ct.empty())
                ct = "application/x-www-form-urlencoded";
            env_vars.push_back("CONTENT_TYPE=" + ct);
        }

        // Bonus : cookie et session
        std::string cookie = _request.get_header("Cookie");
        if (!cookie.empty())
            env_vars.push_back("HTTP_COOKIE=" + cookie);
        if (!_session_id.empty()) {
            env_vars.push_back("SESSION_ID=" + _session_id);
            SessionData *sd = SessionManager::instance().get(_session_id);
            if (sd) {
                std::ostringstream vc;
                vc << sd->visit_count;
                env_vars.push_back("SESSION_COUNT=" + vc.str());
                if (!sd->username.empty())
                    env_vars.push_back("SESSION_USERNAME=" + sd->username);
            }
        }

        // Convertir en tableau char* pour execve
        std::vector<char *> env;
        for (size_t i = 0; i < env_vars.size(); i++)
            env.push_back(const_cast<char *>(env_vars[i].c_str()));
        env.push_back(NULL);

        // Nom court du script (sans répertoire) pour execve
        std::string script_name = full_path.substr(full_path.find_last_of('/') + 1);

        char *args[3];
        args[0] = const_cast<char *>(interpreter.c_str());
        args[1] = const_cast<char *>(script_name.c_str());
        args[2] = NULL;

        execve(args[0], args, &env[0]);
        exit(1); // execve a échoué
    }

    // ── Processus parent ────────────────────────────────────────────────

    close(pipe_in[0]);
    close(pipe_out[1]);

    // Envoyer le corps de la requête au CGI (POST)
    // Le pipe_in est bloquant côté parent (seul pipe_out est mis en O_NONBLOCK).
    // On écrit en boucle pour gérer les écritures partielles.
    if (_request.get_method() == "POST" && !_request.get_body().empty()) {
        std::string body_copy = _request.get_body(); // évite le use-after-free sur le temporaire
        const char *buf = body_copy.c_str();
        size_t      len = body_copy.size();
        size_t      written = 0;
        while (written < len) {
            int n = write(pipe_in[1], buf + written, len - written);
            if (n > 0) {
                written += (size_t)n;
            } else if (n == 0) {
                break; // pipe fermé
            } else {
                if (errno == EAGAIN || errno == EWOULDBLOCK)
                    break; // buffer plein, on abandonne le reste (CGI doit lire plus vite)
                break; // erreur réelle
            }
        }
    }
    close(pipe_in[1]);

    // Mettre pipe_out en non-bloquant pour la lecture via epoll
    fcntl(pipe_out[0], F_SETFL, O_NONBLOCK);

    _cgi_pid        = pid;
    _cgi_pipe_out   = pipe_out[0];
    _cgi_start_time = time(NULL);
    _cgi_output.clear();

    return pipe_out[0];
}

// ─── CGI : lecture d'un chunk depuis le pipe ─────────────────────────────────

// Appelé par Epoll quand le pipe CGI est prêt en lecture.
// Lit autant d'octets que disponibles et les ajoute à _cgi_output.
// Retourne : -1 = EOF ou erreur (CGI terminé), 0 = EAGAIN (on attend plus)
int Client::read_cgi_chunk() {
    char buf[4096];
    int n = read(_cgi_pipe_out, buf, sizeof(buf));
    if (n > 0) {
        _cgi_output.append(buf, n);
        return 0; // peut encore y avoir des données
    }
    if (n == 0)
        return -1; // EOF : CGI a fermé son stdout
    // n < 0
    if (errno == EAGAIN || errno == EWOULDBLOCK)
        return 0; // rien pour l'instant, on attend le prochain événement
    return -1; // erreur réelle
}

// ─── CGI : finalisation ──────────────────────────────────────────────────────

// Appelé par Epoll quand le pipe CGI a atteint EOF (ou timeout).
// killed = true → le CGI a été tué pour timeout, on envoie un 504.
// Sinon : attend la fin du processus (waitpid), construit la réponse, l'envoie.
// Retourne le même code que do_send() : -1=envoyé/erreur, 1=envoi partiel (EPOLLOUT).
int Client::finish_cgi(Server &server, bool killed) {
    if (killed) {
        // Tuer le processus CGI puis ramasser le zombie
        kill(_cgi_pid, SIGKILL);
        waitpid(_cgi_pid, NULL, 0);
        _cgi_pid      = -1;
        _cgi_pipe_out = -1;
        Response resp;
        prepare_send(resp.build_error_response(504, server));
        return do_send();
    }

    // Récolter le zombie normalement
    int status = 0;
    waitpid(_cgi_pid, &status, 0);
    _cgi_pid      = -1;
    _cgi_pipe_out = -1;

    // Si le script a renvoyé un code non-nul → 500
    if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
        Response resp;
        prepare_send(resp.build_error_response(500, server));
        return do_send();
    }

    // Encapsuler la sortie CGI dans une réponse HTTP
    Response resp;
    if (_new_session)
        resp.add_header("Set-Cookie", "session_id=" + _session_id + "; Path=/; HttpOnly");
    prepare_send(resp.finish_cgi_response(_cgi_output, server));
    return do_send();
}

// ─── Envoi de la réponse ─────────────────────────────────────────────────────

// Prépare _send_buffer pour l'envoi.
void Client::prepare_send(const std::string &response) {
    _send_buffer = response;
    _send_offset = 0;
    _state       = SENDING;
}

// Tente d'envoyer le maximum d'octets restants dans _send_buffer.
// Retourne : -1 = tout envoyé ou erreur (DONE), 1 = envoi partiel (EPOLLOUT requis)
int Client::do_send() {
    _state = SENDING;

    size_t remaining = _send_buffer.size() - _send_offset;
    if (remaining == 0) {
        _state = DONE;
        return -1;
    }

    int sent = send(_socket_fd,
                    _send_buffer.c_str() + _send_offset,
                    remaining, 0);
    if (sent < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            return 1; // on ne peut pas encore écrire → EPOLLOUT
        _state = DONE;
        return -1;
    }
    _send_offset += (size_t)sent;

    if (_send_offset >= _send_buffer.size()) {
        _state = DONE;
        return -1; // tout envoyé
    }
    return 1; // envoi partiel → EPOLLOUT
}

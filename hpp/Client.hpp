/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zaboulaza <zaboulaza@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/27 20:27:49 by zaboulaza         #+#    #+#             */
/*   Updated: 2026/04/11 00:00:00 by zaboulaza        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Request.hpp"
#include "Response.hpp"
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <ctime>

class Server;

// États de la machine à états du client.
// Chaque état correspond à une phase du traitement d'une requête HTTP.
enum ClientState {
    READING_HEADER,  // on accumule les octets jusqu'à \r\n\r\n
    READING_BODY,    // on lit le corps (POST avec Content-Length)
    CGI_RUNNING,     // le processus CGI tourne, on lit son pipe via epoll
    SENDING,         // on envoie la réponse (peut nécessiter plusieurs send)
    DONE             // connexion à fermer
};

class Client {

    public:

        Client();
        Client(int fd);
        ~Client() {}
        Client(const Client &c);
        Client &operator=(const Client &c);

        // Point d'entrée principal appelé par epoll.
        // events : masque d'événements epoll (EPOLLIN, EPOLLOUT, EPOLLERR…)
        // Retourne : -1=done/erreur, 0=continuer EPOLLIN, 1=besoin EPOLLOUT, 2=CGI démarré
        int handle_event(Server &server, uint32_t events);

        ClientState get_state() const { return _state; }
        int get_fd() const            { return _socket_fd; }

        // Retourne le fd du pipe de sortie CGI (valide uniquement si CGI_RUNNING).
        int get_cgi_pipe() const      { return _cgi_pipe_out; }

        // Retourne le timestamp de démarrage du CGI (pour le timeout).
        time_t get_cgi_start() const  { return _cgi_start_time; }

        // Lit un chunk depuis le pipe CGI et l'ajoute à _cgi_output.
        // Retourne : -1 = EOF ou erreur, 0 = EAGAIN (plus de données pour l'instant)
        int read_cgi_chunk();

        // Termine le traitement CGI : attend le processus, construit la réponse HTTP,
        // envoie ce qui peut l'être et retourne le même code que do_send().
        // killed : true si le CGI a été tué pour timeout (envoie un 504).
        int finish_cgi(Server &server, bool killed = false);

    private:

        int         _socket_fd;
        ClientState _state;
        Request     _request;

        // Tampon brut : octets reçus en attente de traitement
        std::string _raw_buffer;
        // Tampon d'envoi : réponse HTTP complète à envoyer
        std::string _send_buffer;
        // Offset dans _send_buffer : combien d'octets ont déjà été envoyés
        size_t      _send_offset;

        // Champs CGI
        pid_t       _cgi_pid;
        int         _cgi_pipe_out;
        std::string _cgi_output;
        time_t      _cgi_start_time;

        // Lit les octets du header, parse et valide dès que \r\n\r\n est reçu.
        // Retourne : -1=fin, 0=attendre plus de données, 2=CGI démarré
        int read_header(Server &server);

        // Lit les octets du corps jusqu'à Content-Length.
        // Retourne : -1=fin, 0=attendre plus de données, 2=CGI démarré
        int read_body(Server &server);

        // Tente d'envoyer _send_buffer depuis _send_offset.
        // Retourne : -1 = tout envoyé ou erreur, 1 = envoi partiel (besoin EPOLLOUT)
        int do_send();

        // Prépare la réponse pour l'envoi.
        void prepare_send(const std::string &response);

        // Fork + execve le script CGI. Écrit le corps POST dans pipe_in, ferme pipe_in.
        // Positionne _cgi_pipe_out, _cgi_pid, _cgi_start_time.
        // Retourne le fd pipe_out (>= 0) ou -1 en cas d'erreur.
        int start_cgi(Server &server, const std::string &interpreter);
};

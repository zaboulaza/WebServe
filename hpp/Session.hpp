/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Session.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zaboulaza <zaboulaza@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/11 00:00:00 by zaboulaza         #+#    #+#             */
/*   Updated: 2026/04/11 00:00:00 by zaboulaza        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <map>
#include <string>
#include <cstdlib>
#include <ctime>
#include <sstream>
#include <unistd.h>

// Données associées à une session HTTP.
struct SessionData {
    int         visit_count;  // nombre de requêtes reçues dans cette session
    std::string username;     // champ libre (modifiable via CGI ou formulaire)
    time_t      last_seen;    // timestamp de la dernière requête

    SessionData() : visit_count(0), last_seen(time(NULL)) {}
};

// Gestionnaire de sessions HTTP (singleton).
// Stocke en mémoire un map session_id → SessionData.
// Durée de vie : celle du processus serveur.
class SessionManager {
public:

    // Accès au singleton.
    static SessionManager &instance();

    // Retourne le session_id effectif.
    // Si session_id est vide ou inconnu → crée une nouvelle session et met is_new à true.
    std::string get_or_create(const std::string &session_id, bool &is_new);

    // Incrémente visit_count et met à jour last_seen.
    void touch(const std::string &session_id);

    // Retourne un pointeur vers les données de la session, NULL si inexistante.
    SessionData *get(const std::string &session_id);

    // Extrait la valeur du cookie "session_id" depuis la valeur brute du header Cookie.
    // Ex : "session_id=abc123; foo=bar" → "abc123"
    static std::string extract_from_cookie(const std::string &cookie_header);

private:
    SessionManager();

    std::string generate_id() const;

    std::map<std::string, SessionData> _sessions;
};

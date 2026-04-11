/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Session.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zaboulaza <zaboulaza@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/11 00:00:00 by zaboulaza         #+#    #+#             */
/*   Updated: 2026/04/11 00:00:00 by zaboulaza        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../hpp/Session.hpp"

SessionManager::SessionManager() {}

SessionManager &SessionManager::instance() {
    static SessionManager inst;
    return inst;
}

// Génère un identifiant de session de 32 caractères hexadécimaux (128 bits).
std::string SessionManager::generate_id() const {
    static bool seeded = false;
    if (!seeded) {
        srand((unsigned)time(NULL) ^ (unsigned)getpid());
        seeded = true;
    }
    const char *hex = "0123456789abcdef";
    std::string id;
    id.reserve(32);
    for (int i = 0; i < 32; i++)
        id += hex[rand() % 16];
    return id;
}

std::string SessionManager::get_or_create(const std::string &session_id, bool &is_new) {
    if (!session_id.empty()) {
        std::map<std::string, SessionData>::iterator it = _sessions.find(session_id);
        if (it != _sessions.end()) {
            is_new = false;
            return session_id;
        }
    }
    // Purge périodique : si le nombre de sessions dépasse le max, on nettoie d'abord
    if ((int)_sessions.size() >= SESSION_MAX)
        cleanup_expired();
    // Si toujours trop grand après la purge, on vide tout (évite la fuite)
    if ((int)_sessions.size() >= SESSION_MAX)
        _sessions.clear();

    // Nouvelle session
    is_new = true;
    std::string new_id = generate_id();
    _sessions[new_id] = SessionData();
    return new_id;
}

// Supprime les sessions inactives depuis plus de SESSION_TTL secondes.
void SessionManager::cleanup_expired() {
    time_t now = time(NULL);
    std::map<std::string, SessionData>::iterator it = _sessions.begin();
    while (it != _sessions.end()) {
        if (difftime(now, it->second.last_seen) > SESSION_TTL) {
            std::map<std::string, SessionData>::iterator to_erase = it;
            ++it;
            _sessions.erase(to_erase);
        } else {
            ++it;
        }
    }
}

void SessionManager::touch(const std::string &session_id) {
    std::map<std::string, SessionData>::iterator it = _sessions.find(session_id);
    if (it != _sessions.end()) {
        it->second.visit_count++;
        it->second.last_seen = time(NULL);
    }
}

SessionData *SessionManager::get(const std::string &session_id) {
    std::map<std::string, SessionData>::iterator it = _sessions.find(session_id);
    if (it != _sessions.end())
        return &it->second;
    return NULL;
}

// Extrait la valeur du cookie "session_id" depuis le header Cookie brut.
// Format attendu : "key=val; key2=val2; ..."
std::string SessionManager::extract_from_cookie(const std::string &cookie_header) {
    if (cookie_header.empty())
        return "";

    const std::string key = "session_id=";
    size_t pos = cookie_header.find(key);
    if (pos == std::string::npos)
        return "";

    pos += key.size();
    size_t end = cookie_header.find(';', pos);
    if (end == std::string::npos)
        end = cookie_header.size();

    std::string val = cookie_header.substr(pos, end - pos);

    // Trim espaces
    size_t s = val.find_first_not_of(" \t");
    if (s == std::string::npos)
        return "";
    size_t e = val.find_last_not_of(" \t");
    return val.substr(s, e - s + 1);
}

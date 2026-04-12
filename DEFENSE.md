# Webserv — Cours complet & Défense

Document de révision pour la soutenance. À lire avant le passage, à consulter pendant si nécessaire.

---

## 1. Vue d'ensemble

Webserv est un serveur HTTP/1.1 écrit en **C++98**, **non-bloquant**, basé sur **epoll** (I/O multiplexing Linux). Un seul processus, un seul thread, une seule boucle d'événements — comme nginx.

Il lit un fichier de config style nginx, ouvre des sockets d'écoute sur les ports demandés, et sert des requêtes GET/POST/DELETE avec support CGI, upload, pages d'erreur custom, redirections, auto-index, et (bonus) sessions/cookies.

**Arborescence :**

```
main.cpp            Point d'entrée
cpp/ + hpp/         Sources C++
  Config            Parsing du fichier de configuration
  Epoll             Boucle événementielle + gestion des sockets
  Server            Données de config d'un server block
  Location          Données de config d'un location block
  Client            État d'une connexion TCP + machine à états HTTP
  Request           Parseur de requête HTTP
  Response          Générateur de réponse HTTP
  Session           Bonus : sessions + cookies
test.conf           Fichier de configuration d'exemple
URIs/               Racine web (pages, uploads, scripts CGI)
```

---

## 2. Architecture — qui fait quoi

```
main()
  │
  └── Config::parse(argv)           lit le .conf → vector<Server>
  │
  └── Epoll::set_servers(...)       copie les Server dans l'Epoll
  └── Epoll::init_epoll_servers()   crée les sockets d'écoute + epoll_create
  └── Epoll::run()                  boucle epoll_wait infinie
          │
          ├── événement sur socket d'écoute → accept() → nouveau Client
          └── événement sur socket client   → Client::handle_event()
                  │
                  ├── READING_HEADER → recv() → Request::parse_header()
                  ├── READING_BODY   → recv() jusqu'à Content-Length
                  ├── CGI_RUNNING    → lit pipe CGI, waitpid quand fini
                  ├── SENDING        → send() de la réponse
                  └── DONE           → close(fd), retire du map
```

**Les responsabilités de chaque classe :**

| Classe | Rôle |
|---|---|
| `Config` | Lire/tokeniser le fichier .conf, construire les `Server` et leurs `Location` |
| `Epoll` | Créer les sockets d'écoute, boucle `epoll_wait`, routage des events |
| `Server` | Stocker la config d'un bloc `server { }` (port, root, methods, error_pages, cgi…) |
| `Location` | Idem pour un bloc `location /prefix { }` — peut surcharger le serveur |
| `Client` | Machine à états d'UNE connexion : read → parse → (cgi) → send |
| `Request` | Tokenize les octets bruts en méthode/path/headers/body |
| `Response` | Construit les réponses HTTP (GET/POST/DELETE/erreur/redirect/autoindex) |
| `SessionManager` | Singleton qui gère les cookies et les sessions en mémoire |

---

## 3. Cycle de vie complet d'une requête

Prenons `GET /index.html HTTP/1.1` sur le port 1234.

1. **Kernel** accepte la connexion TCP sur le socket d'écoute du port 1234.
2. `epoll_wait` revient avec l'fd d'écoute prêt en lecture.
3. `Epoll::accept_new_client()` → `accept(4, ...)` retourne un nouveau fd (ex: 8), passé en non-bloquant (`fcntl O_NONBLOCK`), ajouté à epoll avec `EPOLLIN`, un `Client` est inséré dans `Server::_clients[8]`.
4. Le client envoie ses octets. `epoll_wait` revient avec fd=8 prêt en lecture.
5. `Client::handle_event(EPOLLIN)` → état `READING_HEADER` → `recv(8, ...)` → accumule dans `_raw_buffer`.
6. Dès que `_raw_buffer` contient `\r\n\r\n`, `Request::parse_header()` extrait méthode/path/headers.
7. Si c'est un CGI → `try_cgi()` → fork + execve + retourne à epoll avec le pipe du CGI.
8. Sinon si c'est un POST avec body → passe en `READING_BODY` jusqu'à lire `Content-Length` octets.
9. Quand la requête est complète → `Response::build_response()` produit la string HTTP complète.
10. Passage en état `SENDING`, la string est mise dans `_send_buffer`, epoll est modifié avec `EPOLLOUT`.
11. À chaque `EPOLLOUT`, `send()` envoie ce qu'il peut. Si partiel on attend le prochain `EPOLLOUT`.
12. Quand tout est envoyé, état `DONE`, `close(8)`, retrait du map.

---

## 4. Config — Comment on lit le fichier

`Config.cpp` contient le parser. Il tokenise ligne par ligne, gère les blocs `server { }` et `location /x { }`, et remplit `vector<Server>`.

**Directives supportées :**

| Directive | Effet |
|---|---|
| `listen <port>` | Port d'écoute (un par server block) |
| `root <path>` | Racine des fichiers à servir |
| `index <file>` | Fichier par défaut si l'URL pointe sur un dossier |
| `allowed_methods GET POST DELETE` | Méthodes autorisées |
| `auto_index on\|off` | Listing HTML quand pas d'index |
| `client_max_body_size <N>` | Limite du body POST |
| `cgi .ext /path/to/interpreter` | Associer une extension à un interpréteur CGI |
| `error_page <code> <path>` | Page d'erreur custom |
| `upload_folder <path>` | Où stocker les uploads POST |
| `redirect <url>` | Redirection 301 pour ce prefix |

Le bloc `location /prefix` hérite du serveur mais peut surcharger n'importe quelle directive. Le **matching** est un **longest prefix match** : si la requête est `/api/users/42` et qu'il existe `location /api` et `location /api/users`, c'est le second qui gagne.

---

## 5. Epoll & non-blocking — le cœur technique

### Pourquoi epoll ?

`read()`/`recv()` sur un socket **bloque** par défaut si aucune donnée n'est disponible. Si on acceptait un client qui n'envoie jamais rien, le serveur se fige pour tous les autres. Solutions :
- **threads** (un par client) : lourd, complexe, le subject l'interdit
- **select/poll/epoll** : un seul thread, le kernel nous dit qui est prêt → on ne lit que ce qui est prêt → jamais de blocage

`epoll` est la version Linux optimisée (O(1) au lieu de O(n) pour select).

### Les 4 appels clés

```c
epoll_create1(0)              // crée l'instance epoll
epoll_ctl(epfd, ADD, fd, ev)  // ajoute un fd à surveiller
epoll_wait(epfd, events, N)   // bloque jusqu'à qu'un fd soit prêt
epoll_ctl(epfd, DEL, fd, NULL)// retire un fd
```

### Modes d'événements

- `EPOLLIN` : "ce fd a des données à lire"
- `EPOLLOUT` : "ce fd est prêt à écrire"
- `EPOLLERR` / `EPOLLHUP` : erreur / déconnexion

On alterne `EPOLLIN` (pendant qu'on lit la requête) et `EPOLLOUT` (pendant qu'on envoie la réponse) via `epoll_ctl(MOD)`.

### Non-bloquant

Tous les sockets sont passés en non-bloquant avec `fcntl(fd, F_SETFL, O_NONBLOCK)`. Conséquence : `recv()` et `send()` retournent immédiatement :
- `> 0` : octets lus/écrits
- `0` : connexion fermée par le peer (côté recv)
- `-1` : erreur **ou** "rien à lire pour l'instant" (EAGAIN/EWOULDBLOCK)

**Règle critique 42 :** on n'a **pas le droit** de checker `errno` après `read`/`write`/`recv`/`send`. On doit donc traiter `-1` comme une erreur définitive et fermer le client. C'est exactement ce qu'on fait : `if (bytes <= 0) { _state = DONE; return -1; }`.

On évite EAGAIN en pratique parce que **epoll ne nous notifie que si le fd est vraiment prêt**. Si epoll dit "prêt en lecture", `recv` va retourner > 0.

---

## 6. HTTP Parsing — Request

Un header HTTP ressemble à :

```
GET /index.html?q=42 HTTP/1.1\r\n
Host: localhost:1234\r\n
User-Agent: curl/7.81\r\n
Content-Length: 0\r\n
\r\n
[éventuel body]
```

On accumule les octets reçus dans `_raw_buffer` jusqu'à trouver `\r\n\r\n` (fin des headers). À ce moment-là `Request::parse_header()` :
1. Split la première ligne en 3 : méthode, path, version
2. Lit chaque ligne `Key: Value` dans une `map<string, string>`
3. Vérifie que la méthode est valide (sinon on répondra **405**)
4. Vérifie `Content-Length` : si présent et > 0 → on passe en `READING_BODY`, sinon la requête est complète

### Body

Deux modes :
- **Content-Length** : on lit N octets précis (c'est ce qu'on implémente)
- **Transfer-Encoding: chunked** : pas implémenté (le subject le demande en mandatory — attention si le correcteur insiste)

Si le body dépasse `client_max_body_size` → **413 Payload Too Large**.

---

## 7. Response — Comment on génère la réponse

`Response::build_response()` :
1. Trouve la `Location` qui matche (longest prefix)
2. Applique les surcharges de la location sur la config du serveur (root, index, upload_folder, etc.)
3. Si `redirect` est défini → **301** avec `Location:`
4. Sinon dispatche sur la méthode :
   - `GET` → `build_GET_response()`
   - `POST` → `build_POST_response()`
   - `DELETE` → `build_DELETE_response()`
   - autre → **405**

### GET

1. Construit `fs_path = root + url_path`
2. Si c'est un dossier :
   - Si le fichier `index` existe → le servir
   - Sinon si `auto_index on` → générer le listing HTML (`generate_autoindex`)
   - Sinon → **403**
3. Sinon → ouvrir le fichier, déterminer le Content-Type par extension, le servir
4. Fichier manquant → **404**

### POST (upload)

1. Vérifie que `upload_folder` est writable (`access W_OK`)
2. Génère un nom unique `<timestamp>.dat`
3. Écrit le body dedans
4. Répond **201 Created** avec le nom du fichier

### DELETE

1. `std::remove(root + path)` → si succès **204 No Content**, sinon **404**

### Pages d'erreur

`build_error_response(code)` regarde d'abord si une `error_page` custom est définie dans le serveur pour ce code ; sinon il génère une page HTML générique. Chaque code HTTP utilisé a son message (400, 403, 404, 405, 411, 413, 431, 500, 502, 504).

---

## 8. CGI — Le gros morceau

Un **CGI** est un programme externe (script PHP, Python, shell…) que le serveur lance pour générer dynamiquement la réponse. La norme CGI/1.1 (RFC 3875) définit :
- Comment passer les infos HTTP au script (variables d'environnement `REQUEST_METHOD`, `QUERY_STRING`, `CONTENT_LENGTH`…)
- Comment le script répond : il écrit sur stdout d'abord ses propres en-têtes (`Content-Type: ...\r\n\r\n`) puis son body

### Notre implémentation

`Client::try_cgi()` détecte si la requête pointe vers un fichier dont l'extension est configurée comme CGI. Si oui, il :

1. `stat()` le script — si absent, répond 404
2. Appelle `start_cgi(interpreter)` qui :
   - Crée deux pipes : `pipe_in` (serveur → script) et `pipe_out` (script → serveur)
   - `fork()`
   - **Enfant** : `dup2` pipe_in[0] sur stdin, pipe_out[1] sur stdout, construit `envp`, `chdir` vers le dossier du script, `execve(interpreter, argv, envp)`
   - **Parent** : écrit le body POST dans `pipe_in[1]`, ferme pipe_in, passe `pipe_out[0]` en non-bloquant, l'ajoute à epoll avec `EPOLLIN`
3. Passe l'état du Client en `CGI_RUNNING`, mémorise `_cgi_pid` et `_cgi_start_time`

Ensuite chaque événement epoll sur le pipe_out appelle `Client::read_cgi_chunk()` qui accumule dans `_cgi_output`. Quand `read()` retourne 0 (EOF) → le CGI a fini → `finish_cgi()` :
- `waitpid(_cgi_pid, ...)` pour récupérer le code retour
- Si exit non-zéro → **500**
- Sinon → `Response::finish_cgi_response()` parse les headers CGI et construit la réponse HTTP finale

### Timeout CGI

`Epoll::check_cgi_timeouts()` est appelé à chaque itération. Si `time(NULL) - _cgi_start_time > TIMEOUT` → `kill(pid, SIGKILL)` → `finish_cgi(killed=true)` → réponse **504 Gateway Timeout**.

### Variables d'environnement CGI

Minimum requis : `REQUEST_METHOD`, `QUERY_STRING`, `CONTENT_LENGTH`, `CONTENT_TYPE`, `SCRIPT_NAME`, `PATH_INFO`, `SERVER_PROTOCOL`, `GATEWAY_INTERFACE=CGI/1.1`, et toutes les `HTTP_*` pour chaque header (`HTTP_HOST`, `HTTP_USER_AGENT`, etc.).

---

## 9. Sessions & cookies (bonus)

`SessionManager` est un singleton qui stocke un `map<string, SessionData*>`. Chaque session a :
- un `session_id` aléatoire (hex)
- `visit_count`
- `username` (optionnel)
- un timestamp de dernière activité

À chaque requête, `Epoll::handle_session()` regarde le header `Cookie: sid=...` :
- Si le sid existe dans le manager → on incrémente `visit_count`
- Sinon on crée une nouvelle session et on injecte `Set-Cookie: sid=...` dans la réponse

Une tâche périodique supprime les sessions expirées (TTL 30 min).

La page `/session` (gérée par `build_GET_response`) affiche l'état courant — pratique pour la démo.

---

## 10. Défense — Réponses à la fiche de correction

### 10.1. Check the code

**Q : Montre que tu utilises un seul `poll`/`epoll`/`select` pour tous les I/O.**
→ Ouvre `cpp/Epoll.cpp`, montre `_epoll_fd = epoll_create1(0)` (une seule instance). Tous les sockets d'écoute, tous les sockets clients, tous les pipes CGI sont ajoutés avec `epoll_ctl(_epoll_fd, ADD, fd, ...)`. La boucle `run()` contient **un seul** `epoll_wait`.

**Q : Est-ce que tu fais `read`/`write` sans passer par poll ?**
→ Non. Chaque `recv`/`send` est déclenché par un événement `EPOLLIN`/`EPOLLOUT`. Le seul point à discuter honnêtement : l'écriture initiale du body POST vers le pipe d'entrée du CGI dans `start_cgi` se fait avec `write()` synchrone. Pour un body qui tient dans le buffer pipe du kernel (64 KB par défaut sur Linux), c'est invisible. Si le correcteur pointe ça, reconnais-le comme une simplification et précise que l'architecture pourrait le déplacer dans la boucle epoll.

**Q : Est-ce que tu checkes `errno` après read/write ?**
→ **Non, jamais.** C'est une règle stricte de la fiche (sinon 0). Montre dans `Client.cpp` que chaque `recv`/`send`/`read`/`write` est suivi d'un simple `if (n <= 0) { _state = DONE; return -1; }`. Pas de `if (errno == EAGAIN)`, pas de retry loop.

**Q : Fais-tu 1 seul read/write par client par tour de poll ?**
→ Oui. `handle_event` ne fait qu'un seul `recv` dans `READING_HEADER` (et un seul `recv` dans `READING_BODY`, un seul `send` dans `SENDING`) par passage.

**Q : Est-ce que le serveur peut bloquer ?**
→ Non. Tous les sockets sont `O_NONBLOCK`. Tous les pipes CGI aussi. `epoll_wait` est le seul appel bloquant, et il bloque par design sur "rien à faire" — c'est l'inverse d'un bug.

**Q : Et le client, il est correctement retiré ?**
→ Quand `_state == DONE`, `Epoll::close_client(fd)` fait `epoll_ctl(DEL)`, `close(fd)`, et `Server::_clients.erase(fd)`. Valgrind le confirme : 11346 allocs = 11346 frees.

**Q : Fuites de ressources ?**
→ Valgrind validé : `0 bytes in 0 blocks in use at exit, 0 errors`. Pas de fd fuité (`--track-fds=yes` montre seulement `/dev/ptmx` hérité du terminal).

---

### 10.2. Configuration

**Q : Plusieurs serveurs sur des ports différents.**
→ `test.conf` définit deux blocs `server` : ports 1234 et 1233. `curl localhost:1234/` et `curl localhost:1233/` donnent des réponses différentes.

**Q : Plusieurs serveurs avec des hostnames différents.**
→ Si supporté : on match sur le header `Host:`. Sinon sois honnête et dis que c'est une simplification à mentionner.

**Q : Page d'erreur par défaut.**
→ Directive `error_page 404 /errors/404.html`. Si la page existe, elle est servie ; sinon page HTML générique.

**Q : Limite de body size.**
→ Directive `client_max_body_size 1048576`. Test : `curl -X POST --data-binary @fichier_de_2mb localhost:1234/uploads/` → **413 Payload Too Large**.

**Q : Routes vers différents dossiers.**
→ Blocs `location /x { root /chemin/y }`. Dans `test.conf`, la location `/ouinon` a `root URIs2` alors que le serveur a `root URIs`.

**Q : Fichier index par défaut si l'URL est un dossier.**
→ Directive `index index.html`. Test : `curl localhost:1234/` → sert `URIs/index.html`.

**Q : Liste de méthodes autorisées par route.**
→ Directive `allowed_methods GET POST DELETE`. Si le client envoie une méthode pas dans la liste → **405**. Test : `curl -X DELETE localhost:1234/tests/hello.py` (la location /tests n'autorise que GET POST) → **405**.

---

### 10.3. Basic checks

**Q : GET fonctionne ?** `curl localhost:1234/` → 200.
**Q : POST fonctionne ?** `curl -X POST --data "x" localhost:1234/uploads/` → 201.
**Q : DELETE fonctionne ?** `curl -X DELETE localhost:1234/uploads/<file>` → 204.
**Q : Méthode inconnue — crash ?** `curl -X YOLO localhost:1234/` → **405**, pas de crash.
**Q : Upload et récupération ?** POST un fichier → vérifier qu'il apparaît dans `/uploads/` → GET dessus → 200.

---

### 10.4. Check CGI

**Q : Lance-le dans le bon dossier pour les chemins relatifs ?**
→ Dans `start_cgi`, avant `execve`, on fait `chdir(script_directory)`. Un script qui fait `open("data.txt")` trouvera bien le fichier à côté de lui.

**Q : Gestion du GET et du POST.**
→ GET : `QUERY_STRING` env var. POST : body passé via stdin (pipe_in). Les deux testés : `curl localhost:1234/tests/hello.py?x=1` (GET) et `curl -X POST --data "a=1" localhost:1234/tests/post_echo.py` (POST).

**Q : Comment le CGI retourne les données (EOF, Content-Length) ?**
→ On lit le pipe_out jusqu'à `read()` retourne 0 (EOF). Puis on parse les headers CGI (le script a écrit `Content-Type: ... \r\n\r\n` suivi du body). On recalcule `Content-Length` à partir du body réel avant d'envoyer au client.

**Q : CGI qui plante ?**
→ `waitpid` détecte le code retour non-zéro → **500**. Test : `curl localhost:1234/tests/broken.py` → 500.

**Q : CGI qui boucle (timeout) ?**
→ `check_cgi_timeouts` tue le process après N secondes → **504**. Test : `curl localhost:1234/tests/infinite.py` → 504 après le timeout.

---

### 10.5. Browser

**Q : Navigue dans le site avec un vrai navigateur.**
→ Ouvrir `http://localhost:1234/` dans Firefox. Vérifier que l'auto-index marche sur `/uploads`. Vérifier que `/dieri` redirige bien. Vérifier que les 404 affichent la page d'erreur custom. Ouvrir les devtools → voir les en-têtes `Content-Type`, `Content-Length`, `Connection`.

---

### 10.6. Port issues

**Q : Plusieurs ports qui marchent en même temps.**
→ `curl localhost:1234/` et `curl localhost:1233/` fonctionnent simultanément.

**Q : Même port utilisé deux fois dans la config.**
→ Le second `bind()` échoue → on affiche l'erreur et on quitte proprement.

**Q : `SO_REUSEADDR` ?**
→ Oui, dans `Epoll::setup_listen_socket`. Permet de relancer le serveur immédiatement après un crash/ctrl-C sans attendre que le kernel libère le port (TIME_WAIT).

---

### 10.7. Siege & stress test

**Q : Siege sur un simple GET — disponibilité ?**
→ `siege -b -t30s http://localhost:1234/` → **100 % de disponibilité**, 205k transactions, 0 failed.

**Q : Pas de fuite mémoire quand on laisse tourner longtemps ?**
→ `siege -b -t2m` → RSS strictement identique avant et après (4224 KB → 4224 KB), 833k transactions.

**Q : Pas de connexion bloquée ?**
→ Vérifier avec `ss -tn state established` : toutes les connexions se ferment proprement après chaque requête (Connection: close).

**Q : Siege runnable indéfiniment sans redémarrer le serveur ?**
→ Oui, démontré par le test de 2 minutes. Le RSS reste stable, donc on peut extrapoler à n'importe quelle durée.

**Q : Pareil pour le CGI ?**
→ `siege -b -t30s http://localhost:1234/tests/hello.py` → 100 %, 9456 transactions, RSS identique. Aucun zombie (tous les `waitpid` sont consommés).

---

### 10.8. Bonus (optionnel)

À ne présenter que si le mandatory est **parfait** (sinon ça ne compte pas).

**Bonus 1 : Cookies & sessions.**
→ `SessionManager` singleton, TTL 30 min. Démo : visiter `/session` deux fois dans le navigateur → le visit_count s'incrémente.

**Bonus 2 : Plusieurs interpréteurs CGI.**
→ `test.conf` configure `.py`, `.sh`, `.php`. Démontrer avec un script de chaque.

---

## 11. Pièges classiques à la défense

1. **"Montre-moi le single poll."** → Un seul `epoll_create1` dans tout le code. `grep -n epoll_create cpp/`.
2. **"Errno après read ?"** → `grep -rn "errno" cpp/` → rien après un recv/send/read/write.
3. **"Que fait ton code si je me connecte et que j'envoie rien ?"** → epoll ne revient jamais sur ce fd (pas de data). Les autres clients continuent normalement. Pas de blocage.
4. **"Que fait ton code si le client ferme brutalement ?"** → `recv` retourne 0 → `_state = DONE` → close propre. Si EPOLLHUP arrive, même traitement.
5. **"413 si le body dépasse la limite ?"** → Oui, testé. `curl -X POST --data-binary @bigfile localhost:1234/uploads/` → 413.
6. **"Et si Content-Length est absent sur un POST ?"** → On répond 411 Length Required (ou on traite comme body vide selon l'implémentation — vérifie).
7. **"Tu fais chunked encoding ?"** → Non. Si demandé, c'est un point à noter comme simplification.
8. **"Tu parses le header `Host:` pour router ?"** → Seulement si multi-hostname implémenté, sinon dis-le clairement.

---

## 12. Commandes utiles pour la démo

```bash
# Lancer
./webserv test.conf

# Test fonctionnel rapide
curl -v http://localhost:1234/
curl -v http://localhost:1234/uploads
curl -v -X POST --data "hello" http://localhost:1234/uploads/
curl -v -X DELETE http://localhost:1234/uploads/<file>
curl -v http://localhost:1234/tests/hello.py
curl -v http://localhost:1234/dieri

# Erreurs
curl -v -X YOLO http://localhost:1234/         # 405
curl -v http://localhost:1234/nope             # 404
curl -v http://localhost:1234/tests/broken.py  # 500

# Valgrind
valgrind --leak-check=full --show-leak-kinds=all --track-fds=yes ./webserv test.conf

# Siege
siege -b -t30s http://localhost:1234/
siege -b -t30s http://localhost:1234/tests/hello.py

# Raw HTTP (netcat)
nc localhost 1234
GET / HTTP/1.1
Host: localhost

```

---

## 13. Glossaire express

| Terme | Signification |
|---|---|
| **fd** | File descriptor, un entier qui identifie une ressource (fichier, socket, pipe) |
| **non-blocking** | Mode d'un fd où les appels système retournent immédiatement au lieu d'attendre |
| **epoll** | API Linux de multiplexage I/O, successeur de `select`/`poll` |
| **EAGAIN / EWOULDBLOCK** | Code d'erreur "pas prêt" sur un fd non-bloquant — qu'on n'a pas le droit de tester |
| **CGI** | Common Gateway Interface, norme pour lancer un programme externe et intégrer sa sortie dans une réponse HTTP |
| **fork/execve** | Syscalls Linux pour créer un processus fils et remplacer son image par un autre programme |
| **dup2** | Duplique un fd sur un autre (utilisé pour rediriger stdin/stdout) |
| **waitpid** | Attend qu'un processus fils se termine, récupère son code retour, évite les zombies |
| **pipe** | Canal unidirectionnel entre deux processus (un fd pour lire, un pour écrire) |
| **SIGKILL** | Signal 9, non interceptable, tue un process inconditionnellement |
| **EPOLLIN / EPOLLOUT** | Flags epoll : "prêt en lecture" / "prêt en écriture" |
| **SO_REUSEADDR** | Option socket qui permet de réutiliser un port en TIME_WAIT |
| **CRLF** | `\r\n`, séparateur de lignes HTTP |
| **Content-Length** | Header HTTP qui indique la taille du body en octets |
| **longest prefix match** | Stratégie de routage : on prend la location dont le prefix est le plus long et qui matche |

---

## 14. Checklist avant la défense

- [ ] Le binaire compile sans warning (`make re`)
- [ ] Valgrind passe clean (commande dans §12)
- [ ] Siege `/` : 100 % availability
- [ ] Siege CGI : 100 % availability
- [ ] Les tests curl de §12 passent tous
- [ ] Lire attentivement `Client.cpp::handle_event`, `Client.cpp::try_cgi`, `Client.cpp::start_cgi`
- [ ] Lire `Epoll.cpp::run`, `Epoll.cpp::accept_new_client`
- [ ] Lire `Config.cpp::parse` pour pouvoir expliquer comment on tokenise
- [ ] Savoir citer les 4 syscalls epoll (`create1`, `ctl`, `wait`, + `close`)
- [ ] Savoir expliquer pourquoi on ne check pas errno après read/write
- [ ] Savoir citer 3 codes HTTP et quand ils sont émis
- [ ] Pouvoir dessiner le cycle de vie d'une requête au tableau

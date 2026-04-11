*This project has been created as part of the 42 curriculum by zaboulaza, lenakach.*

## Description

**webserv** is an HTTP/1.1 server written in C++98, built from scratch as part of the 42 school curriculum. The goal is to understand how web servers work at the protocol level — from raw TCP socket management to HTTP request parsing and response generation.

The server is non-blocking and event-driven, using **Linux's `epoll`** for I/O multiplexing (similar to how Nginx works internally). A single process handles all connections through a state machine, without threads.

**Key features:**
- Non-blocking event loop with `epoll` — one thread handles all clients
- Full HTTP/1.1 request parsing (GET, POST, DELETE)
- Static file serving with configurable root directories
- CGI execution (Python, Bash, PHP) via `fork` + `execve`
- Directory auto-indexing (HTML directory listings)
- HTTP redirects (301)
- File upload via POST
- Multiple virtual servers on different ports
- Location-based URL routing with longest-prefix matching
- Configurable error pages
- `client_max_body_size` limit (413 on overflow)

## Instructions

### Build

```bash
make        # compile → produces ./webserv
make re     # full rebuild from scratch
make clean  # remove obj/ files only
make fclean # remove obj/ and the webserv binary
```

Requires: `c++` with C++98 support, Linux (uses `epoll`).

### Run

```bash
./webserv test.conf          # load a configuration file (recommended)
./webserv 8080 9000          # listen on ports directly (minimal config)
```

### Test manually

```bash
# netcat — type raw HTTP requests by hand
nc localhost 1234

# curl — automatic HTTP
curl -v http://localhost:1234/
curl -X POST -d "hello" http://localhost:1234/uploads/
curl -X DELETE http://localhost:1234/uploads/somefile.dat
```

### Configuration file format

Nginx-inspired syntax. Each `server {}` block creates a virtual server.

```
server {
    listen 8080
    root /var/www
    index index.html
    allowed_methods GET POST DELETE
    auto_index on
    client_max_body_size 1048576
    cgi .py /usr/bin/python3
    cgi .sh /bin/bash
    error_page 404 /errors/404.html
    upload_folder /var/www/uploads

    location /api {
        allowed_methods POST DELETE
        root /var/api
    }

    location /old {
        redirect /new
    }
}
```

See `test.conf` for a full multi-server example.

## Resources

### HTTP Protocol
- [RFC 7230 — HTTP/1.1 Message Syntax](https://www.rfc-editor.org/rfc/rfc7230)
- [RFC 7231 — HTTP/1.1 Semantics and Content](https://www.rfc-editor.org/rfc/rfc7231)
- [MDN Web Docs — HTTP overview](https://developer.mozilla.org/en-US/docs/Web/HTTP/Overview)

### Linux I/O & System Programming
- `man 7 epoll` — Linux epoll interface
- `man 2 epoll_wait`, `man 2 epoll_ctl`, `man 2 epoll_create`
- `man 2 accept`, `man 2 recv`, `man 2 send`
- `man 2 fork`, `man 2 execve`, `man 2 waitpid`
- `man 3 getaddrinfo`

### CGI
- [RFC 3875 — The Common Gateway Interface (CGI/1.1)](https://www.rfc-editor.org/rfc/rfc3875)

### Architecture reference
- [Nginx architecture overview](https://nginx.org/en/docs/dev/development_guide.html)
- [Beej's Guide to Network Programming](https://beej.us/guide/bgnet/)

### How AI was used

AI (Claude) was used throughout this project as a learning and debugging tool:

- **Understanding concepts**: asking for explanations of epoll edge-triggered vs level-triggered, TCP state machines, CGI environment variables, and HTTP/1.1 semantics before writing code.
- **Debugging**: describing error symptoms (e.g. "CGI returns source code instead of output when query string is present") to identify root causes.
- **Code review**: submitting implemented functions to check for edge cases (e.g. non-blocking recv in the state machine, longest-prefix matching for locations).
- **Architecture decisions**: discussing trade-offs for the client state machine design (READING_HEADER → READING_BODY → SENDING → DONE).

All AI-generated or AI-reviewed code was read, understood, and validated by the authors before inclusion. The project was not generated wholesale by AI — it was built incrementally with AI as a reference tool, similar to using Stack Overflow or man pages.

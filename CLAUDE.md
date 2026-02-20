# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Langue

Toujours répondre en **français**, y compris dans ce fichier et tous les commentaires/explications.

## Teaching Emphasis

This is a **learning project**. The user is building this to understand how HTTP servers work. When helping:
- **Explain concepts before (or instead of) writing code.** Describe *why* something works, not just *what* to write.
- Ask the user to implement things themselves when appropriate, then review their code.
- Point to relevant man pages, RFCs, or POSIX documentation when relevant (e.g., `man 7 epoll`, RFC 7230 for HTTP/1.1).
- When the user makes an error, explain the underlying concept that the error reveals — don't just fix it.

## Build Commands

```bash
make          # compile → produces ./webserv
make re       # full rebuild from scratch
make clean    # remove obj/ files only
make fclean   # remove obj/ and webserv binary
```

Compiler: `c++`, standard C++98, flags: `-std=c++98 -g3` (debug symbols included).

## Running the Server

```bash
./webserv test.conf          # load configuration file (recommended)
./webserv 8080 9000          # open ports directly (minimal config)
```

## Manual Testing

```bash
# netcat - lets you type raw HTTP by hand (great for learning)
nc localhost 8080

# Then type a raw HTTP request:
GET / HTTP/1.1\r\n
Host: localhost\r\n
\r\n

# curl - sends valid HTTP automatically
curl -v http://localhost:8080/
```

## Architecture Overview

The project is a non-blocking, epoll-based HTTP server using an event loop (similar to Nginx). No threads — one process handles all connections through I/O multiplexing.

```
main.cpp
  └── Epoll
        ├── parses config file → creates Server objects
        ├── creates + binds sockets for each Server
        ├── runs infinite epoll_wait() event loop
        │     ├── server socket event  → accept new client
        │     └── client socket event  → read request
        └── owns all Server objects

Server
  ├── holds configuration (port, root, index, allowed_methods, etc.)
  ├── owns a map<int fd, Client> for active connections
  └── holds a vector<Location> for URL routing rules

Client
  ├── wraps a single TCP connection (one fd)
  └── owns a Request object (the in-progress parse)

Request
  └── parses the raw bytes into: method, path, version, headers, body

Location
  └── mirrors Server config, but scoped to a URL prefix (like nginx location blocks)
```

**Data flow for one request:**
1. `epoll_wait` wakes up → client fd is ready to read
2. `Epoll::handle_client_event()` finds the `Client` in the correct `Server`'s map
3. `Client::recv_request()` calls `recv()` and accumulates bytes until `\r\n\r\n`
4. `Request::parse_header()` tokenizes the accumulated buffer
5. *(not yet implemented)* body is read if `Content-Length > 0`
6. *(not yet implemented)* a response is built and sent back

## Configuration File Format

Nginx-like syntax. Each `server {}` block becomes a `Server` object. `location /path {}` blocks inside become `Location` objects scoped to that URL prefix.

```
server {
    listen 8080
    root /var/www
    index index.html
    allowed_methods GET POST DELETE
    auto_index on
    cgi .py /usr/bin/python3
    error_page 404 /errors/404.html
    upload_folder /tmp/uploads
    redirect /old /new

    location /api {
        allowed_methods POST DELETE
        root /var/api
    }
}
```

See `test.conf` for a full multi-server example, `message.conf` for a minimal one.

## Current Implementation Status

| Layer | Status |
|---|---|
| Config parsing | Complete |
| Socket setup + epoll loop | Complete |
| HTTP header parsing | Complete |
| HTTP body parsing | **In progress** — `Client.cpp` stub exists, logic not finished |
| HTTP response generation | **Not started** |
| Location routing | **Not started** |
| CGI execution | **Not started** |
| Error pages | **Not started** |

The development notes in `cour.md` (written in French) describe the body-parsing rules that still need to be implemented.

## Key Linux/POSIX Concepts in Use

- **epoll** (`man 7 epoll`) — Linux-specific, efficient I/O event notification
- **Non-blocking sockets** — `fcntl(fd, F_SETFL, O_NONBLOCK)` — `recv()`/`send()` return immediately instead of blocking
- **`getaddrinfo()`** — resolves host/port to a usable socket address (handles IPv4 + IPv6)
- **`SO_REUSEADDR`** — allows rebinding to a port shortly after the server restarts

## C++98 Constraint

The project must compile under C++98 (`-std=c++98`). This means:
- No `auto`, no range-for, no lambdas, no `nullptr` (use `NULL`)
- No `std::to_string` (use `std::ostringstream`)
- No `<cstdint>` fixed-width types

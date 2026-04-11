*This project has been created as part of the 42 curriculum by zaboulaza, lenakach.*

# webserv

A non-blocking HTTP/1.1 server written in C++98, built from scratch.

---

## Description

**webserv** is a fully functional HTTP server implemented in C++98 as part of the 42 school curriculum. The goal is to understand how web servers work at the protocol level — from raw TCP socket management to HTTP request parsing, CGI execution, and response generation.

The server is **non-blocking and event-driven**, using Linux's `epoll` for I/O multiplexing. A single process handles all connections through a state machine, without threads, in a design similar to Nginx's core loop.

### Features

| Feature | Status |
|---|---|
| Non-blocking `epoll` event loop | ✓ |
| HTTP/1.1 GET, POST, DELETE | ✓ |
| Static file serving | ✓ |
| Directory auto-indexing | ✓ |
| File upload via POST | ✓ |
| HTTP 301 redirects | ✓ |
| CGI execution (.py, .sh, .php) | ✓ |
| CGI timeout (504 on hang) | ✓ |
| Multiple virtual servers (different ports) | ✓ |
| Location-based URL routing (longest prefix) | ✓ |
| Configurable error pages | ✓ |
| `client_max_body_size` limit (413) | ✓ |
| Cookie & session management *(bonus)* | ✓ |
| Multiple CGI types *(bonus)* | ✓ |

### Architecture

```
main.cpp
  └── Epoll                  ← event loop (epoll_wait)
        ├── Server[]          ← one per listen block in config
        │     ├── Location[]  ← URL routing rules
        │     └── Client[]    ← active TCP connections
        ├── accept            ← new connection → new Client
        ├── handle_client_event  ← EPOLLIN/EPOLLOUT on client fd
        └── handle_cgi_pipe_event  ← EPOLLIN on CGI stdout pipe
```

**Request lifecycle:**
1. `epoll_wait` wakes up → client fd ready
2. `Client::read_header()` accumulates bytes until `\r\n\r\n`
3. `Request::parse_header()` tokenizes method, path, headers
4. If POST: `Client::read_body()` accumulates body up to `Content-Length`
5. Route resolution via `Server::find_location()` (longest prefix match)
6. Static file → `Response::build_response()` → `send()`
7. CGI → `fork()` + `execve()` → pipe monitored by epoll → `finish_cgi()`

---

## Instructions

### Requirements

- Linux (uses `epoll`)
- `c++` with C++98 support (`-std=c++98`)
- Python 3 for `.py` CGI scripts
- Bash for `.sh` CGI scripts

### Build

```bash
make          # compile → produces ./webserv
make re       # full rebuild from scratch
make clean    # remove obj/ files only
make fclean   # remove obj/ and the webserv binary
```

### Run

```bash
./webserv test.conf        # recommended: full-featured config
./webserv message.conf     # minimal single-port config
```

### Configuration file format

Nginx-inspired syntax. Each `server {}` block creates a virtual server, identified by its port. `location /path {}` blocks scope routing rules to a URL prefix.

```
server
{
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

    location /api
    {
        allowed_methods POST DELETE
        root /var/api
    }

    location /old
    {
        redirect /new
    }
}
```

**Directives:**

| Directive | Description |
|---|---|
| `listen <port>` | Port to listen on |
| `root <path>` | Root directory for file serving |
| `index <file>` | Default file when requesting a directory |
| `allowed_methods` | Space-separated: `GET POST DELETE` |
| `auto_index on\|off` | Enable directory listing |
| `client_max_body_size <bytes>` | Max request body (default: 10 MB) |
| `cgi <.ext> <interpreter>` | CGI handler by file extension |
| `error_page <code> <path>` | Custom error page |
| `upload_folder <path>` | Where uploaded files are stored |
| `redirect <destination>` | 301 redirect (absolute or relative) |

### Manual testing

```bash
# Raw HTTP via netcat (equivalent to telnet)
printf "GET / HTTP/1.1\r\nHost: localhost\r\n\r\n" | nc localhost 1234

# curl
curl -v http://localhost:1234/
curl -X POST -H "Content-Type: text/plain" --data "hello" http://localhost:1234/uploads/
curl -X DELETE http://localhost:1234/uploads/somefile.dat

# Test 413 (body too large — requires client_max_body_size in config)
python3 -c "print('A'*1200000, end='')" > /tmp/big.txt
curl -X POST -H "Content-Type: text/plain" --data-binary @/tmp/big.txt http://localhost:1234/uploads/

# Test CGI
curl http://localhost:1234/tests/hello.py
curl http://localhost:1234/tests/hello.sh

# Test sessions (bonus)
curl -c /tmp/jar.txt http://localhost:1234/
curl -b /tmp/jar.txt http://localhost:1234/session
```

### Provided test scripts

All CGI scripts are in `URIs/tests/`:

| Script | Purpose |
|---|---|
| `hello.py` | Basic Python CGI (GET + POST) |
| `hello.sh` | Basic Bash CGI (multi-type demo) |
| `session_cgi.py` | Session variables via CGI environment |
| `post_echo.py` | Echoes POST body |
| `cwd_test.py` | Verifies CGI runs in correct directory |
| `infinite.py` | Triggers CGI timeout (→ 504) |
| `broken.py` | Syntax error (→ 500) |
| `empty_out.py` | No output from CGI |
| `early_exit.py` | CGI closes stdout early |

---

## HTTP status codes reference

The server produces the following status codes:

| Code | Meaning | Trigger |
|---|---|---|
| 200 | OK | Successful GET / static file |
| 201 | Created | Successful POST upload |
| 204 | No Content | Successful DELETE |
| 301 | Moved Permanently | `redirect` directive |
| 400 | Bad Request | Malformed request line |
| 403 | Forbidden | Directory without permission |
| 404 | Not Found | File or CGI script not found |
| 405 | Method Not Allowed | Method not in `allowed_methods` |
| 411 | Length Required | POST without Content-Length |
| 413 | Payload Too Large | Body exceeds `client_max_body_size` |
| 431 | Request Header Fields Too Large | Header section > 8 KB |
| 500 | Internal Server Error | CGI fork/exec failure |
| 504 | Gateway Timeout | CGI exceeds 10-second timeout |

---

## Resources

### HTTP Protocol
- [RFC 7230 — HTTP/1.1 Message Syntax and Routing](https://www.rfc-editor.org/rfc/rfc7230)
- [RFC 7231 — HTTP/1.1 Semantics and Content](https://www.rfc-editor.org/rfc/rfc7231)
- [MDN Web Docs — HTTP overview](https://developer.mozilla.org/en-US/docs/Web/HTTP/Overview)
- [MDN — HTTP response status codes](https://developer.mozilla.org/en-US/docs/Web/HTTP/Status)

### Linux I/O & System Programming
- `man 7 epoll` — Linux epoll interface
- `man 2 epoll_wait`, `man 2 epoll_ctl`, `man 2 epoll_create`
- `man 2 recv`, `man 2 send`, `man 2 accept`
- `man 2 fork`, `man 2 execve`, `man 2 waitpid`, `man 2 pipe`
- `man 3 getaddrinfo`
- [Beej's Guide to Network Programming](https://beej.us/guide/bgnet/)

### CGI
- [RFC 3875 — The Common Gateway Interface (CGI/1.1)](https://www.rfc-editor.org/rfc/rfc3875)

### Architecture reference
- [Nginx architecture overview](https://nginx.org/en/docs/dev/development_guide.html)
- [The C10K problem](http://www.kegel.com/c10k.html)

### How AI was used

AI (Claude) was used throughout this project as a **learning and debugging tool**, not as a code generator:

- **Understanding concepts**: asking for explanations of epoll level-triggered vs edge-triggered, TCP state machines, CGI/1.1 environment variables, and HTTP/1.1 semantics before writing any code.
- **Debugging**: describing error symptoms (e.g., "CGI returns source code instead of output when a query string is present") and using AI to identify the root cause.
- **Code review**: submitting implemented functions to check for edge cases — e.g., non-blocking `recv` in the state machine, longest-prefix matching for locations, pipe FD leak on fork failure.
- **Architecture decisions**: discussing trade-offs for the client state machine design (`READING_HEADER → READING_BODY → SENDING → DONE`) and for CGI timeout handling via `epoll_wait` with a 5-second watchdog.

All AI-reviewed code was read, understood, and validated by the authors before inclusion. The project was built incrementally with AI as a reference tool, similar to using `man` pages or Stack Overflow.

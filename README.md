*This project has been created as part of the 42 curriculum by mipang, xzhen.*

# webserv

## Description

An HTTP server written from scratch in C++98, with no external library.

The server reads an NGINX-like configuration file, opens one listening socket
per `server` block, and serves static websites over HTTP/1.1. Every socket —
listening sockets and client sockets alike — is non-blocking and driven by a
single `poll()` call, so one slow or silent client can never block the others.

### Architecture

```
ConfigParser  ──>  ServerConfig / LocationConfig      (parsed once at startup)
                          │
Server  ──  one poll() loop over every socket
   │
   └── Connection  (one per client: READING -> WRITING -> DONE)
              │
              ├── HttpRequest    raw text  -> method / path / headers / body
              ├── Router         request + config -> which file to serve
              └── HttpResponse   status + headers + body -> raw text
```

| Class | Responsibility |
| --- | --- |
| `ConfigParser` | Tokenises the config file and builds one `ServerConfig` per `server` block |
| `ServerConfig` / `LocationConfig` | Plain data holders for the parsed directives |
| `Server` | Owns the listening sockets and the single `poll()` loop |
| `Connection` | State machine for one client; buffers a partial request and a partial response |
| `HttpRequest` | Parses the raw request text |
| `Router` | Decides what to answer, using the config of the port the client arrived on |
| `HttpResponse` | Builds the raw response text |

### Implemented

- Single `poll()` loop for every socket, listening sockets included
- Non-blocking sockets; `recv`/`send` are only ever called after `poll()` reports readiness
- No use of `errno` after a read or write operation
- Several `server` blocks, one listening port each, served by the same process
- Partial reads and partial writes handled across several loop iterations
- `GET` and `DELETE`
- Static file serving with a MIME type per extension
- `location` blocks with prefix matching, per-location `root` and `index`
- Idle-connection timeout, so a request can never hang forever
- Graceful shutdown on `SIGINT`; `SIGPIPE` ignored
- Malformed requests answered with `400` instead of crashing the server

### Not implemented yet

- `POST` and request bodies (`Content-Length`, chunked transfer encoding)
- File upload
- CGI
- `allowed_methods`, `autoindex`, `error_page`, `client_max_body_size`, HTTP redirection
- Path traversal protection (`..` in a URL is not rejected yet)

## Instructions

### Build

```bash
make
```

Compiled with `c++ -Wall -Wextra -Werror -std=c++98`. Other rules: `make clean`,
`make fclean`, `make re`.

### Run

```bash
./webserv [configuration file]
```

Without an argument the server falls back to `configs/default.conf`.

```bash
./webserv configs/multi.conf
```

### Configuration

```nginx
server {
    listen 8080;
    root ./www/blog;
    index index.html;

    location /public {
        root ./www/public;
        index index.html;
    }
}

server {
    listen 9090;
    root ./www/shop;
    index index.html;
}
```

| Directive | Where | Meaning |
| --- | --- | --- |
| `listen` | `server` | Port this block listens on. Two blocks may not share a port. |
| `root` | `server`, `location` | Directory the URL path is resolved against |
| `index` | `server`, `location` | File served when the URL points at a directory |
| `location <prefix>` | `server` | Rules for every URL starting with `<prefix>` |

Path resolution strips the matched `location` prefix before appending the rest
to `root`. With `location /public { root ./www/public; }`, the URL
`/public/a.html` is served from `./www/public/a.html`.

### Testing

```bash
./webserv configs/multi.conf

curl -i http://localhost:8080/          # blog index
curl -i http://localhost:9090/          # shop index, same process
curl -i http://localhost:8080/public    # location block
curl -i http://localhost:8080/nope      # 404
```

To check that a silent client cannot block the server, open a connection that
sends half a request and leave it hanging, then issue normal requests on both
ports; they must still be answered.

## Resources

### References

- RFC 7230 — HTTP/1.1: Message Syntax and Routing
  <https://datatracker.ietf.org/doc/html/rfc7230>
- RFC 7231 — HTTP/1.1: Semantics and Content
  <https://datatracker.ietf.org/doc/html/rfc7231>
- RFC 3875 — The Common Gateway Interface (CGI) Version 1.1
  <https://datatracker.ietf.org/doc/html/rfc3875>
- Beej's Guide to Network Programming
  <https://beej.us/guide/bgnet/>
- NGINX documentation, used as the reference for configuration syntax and
  behaviour <https://nginx.org/en/docs/>
- MDN Web Docs — HTTP <https://developer.mozilla.org/en-US/docs/Web/HTTP>
- `man 2 poll`, `man 2 socket`, `man 2 accept`, `man 2 fcntl`

### Use of AI

<!--
  ⚠️ TODO: 这一节 PDF 强制要求，必须如实反映你们的实际情况。
  下面是根据本项目实际过程写的草稿，交作业前请自己核对、按需增删。
  评审可能会就这里写的内容提问，所以只保留你能解释的部分。
-->

AI assistance was used for the following:

- Reading the subject: translating the requirements and explaining the parts of
  the socket, `poll` and CGI specifications we were unfamiliar with.
- Small standalone example programs used to understand `fork`, `pipe`, `dup2`
  and `execve` in isolation. They were learning material and are not part of the
  server.
- Reviewing our own code against the subject: finding blocking calls, missing
  `close()`, iterator invalidation, and uses of `errno` that the subject forbids.
- Discussing the design of the event loop before writing it, and reviewing the
  result.

The configuration parser, the request and response classes and the router were
written by us. For the event loop, the class design was discussed with AI and
parts of the implementation were written with its help; every line was read,
tested and is understood by us. Behaviour was verified against NGINX and with
our own test requests.

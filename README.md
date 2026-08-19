*This project has been created as part of the 42 curriculum by mipang, xzhen.*

# webserv

## Description

An HTTP/1.1 server written in C++98, with no external library.

It reads an NGINX-like configuration file, opens one listening socket per
`listen` address, and serves static sites, file uploads and CGI scripts. Every
socket and every pipe is non-blocking and goes through a single `poll()` call,
so one slow client cannot block the others.

### How the code is organised

```
main.cpp            reads the config file, then starts the server
ConfigParser        turns the config text into ServerConfig objects
ServerConfig        one server block: port, root, error pages, locations
LocationConfig      one location block: root, allowed methods, cgi, upload...
Server              owns the listening sockets and the poll() loop
Connection          one client: reads the request, then sends the response
CgiHandler          one child process and its two pipes
HttpRequest         parses the raw request text
Router              decides what to answer, using the config
HttpResponse        builds the raw response text
```

A request goes through them in this order:

```
Server (accept) -> Connection (recv) -> HttpRequest -> Router -> HttpResponse
                                            |
                                            +-> CgiHandler when the URL is a script
```

### What works

- One `poll()` loop for every socket and every CGI pipe
- Several `server` blocks, one interface/port each, served by the same process
- `GET`, `POST`, `DELETE`
- Static files with a MIME type per extension
- `location` blocks with prefix matching
- `allowed_methods`, `autoindex`, `error_page`, `client_max_body_size`,
  `return` (301 redirection), `upload_store`, `cgi_ext`
- File upload, both as a raw body and as a browser form (multipart/form-data)
- CGI (tested with Python), with the environment variables of RFC 3875.
  Every request header is passed to the script as `HTTP_<NAME>`
- Chunked request bodies are un-chunked before anything else sees them, and
  the size limit is applied to the decoded body
- Header names are matched without case, as the RFC asks
- Partial reads and partial writes are resumed on the next poll() round
- Idle connections and runaway CGI scripts are closed after a timeout
- Requests with `..` that would leave the root are refused with 400

### Known limits

- Virtual hosts (several sites on one port, told apart by `Host`) are not
  implemented. The subject says they are out of scope.
- `location` matching returns the first block whose prefix matches, not the
  longest one.
- Only `HTTP/1.1` is accepted; `HTTP/1.0` gets a 505.
- Responses are not chunked, only requests are un-chunked.
- When a URL matches a `cgi_ext` extension, the interpreter is started even if
  the target file is missing, because the interpreter is the program that
  answers. A missing script therefore ends in 502 and not in 404.
- A large body is kept in memory while it is being handled, so a 100 MB upload
  costs about 200 MB of RSS.

## Instructions

### Build

```bash
make
```

Rules: `all`, `clean`, `fclean`, `re`. Compiled with
`c++ -Wall -Wextra -Werror -std=c++98`.

### Run

```bash
./webserv [configuration file]
```

Without an argument it falls back to `configs/default.conf`.

```bash
./webserv configs/multi.conf
```

The CGI scripts need the execute bit:

```bash
chmod +x www/cgi-bin/*.py
```

### Configuration

Everything after a `#` is ignored, up to the end of the line.

```nginx
server {
    listen 127.0.0.1:8080;  # host:port, or just 8080 for all interfaces
    root ./www/blog;
    index index.html;
    client_max_body_size 1m;
    error_page 404 ./www/errors/404.html;

    location /public {
        root ./www/public;
        allowed_methods GET;
    }

    location /uploads {
        root ./www/uploads;
        allowed_methods GET POST DELETE;
        upload_store ./www/uploads;
        autoindex on;
    }

    location /old {
        return /public;
    }

    location /cgi-bin {
        root ./www/cgi-bin;
        allowed_methods GET POST;
        cgi_ext .py /usr/bin/python3;
    }
}
```

| Directive | Where | What it does |
| --- | --- | --- |
| `listen` | server | Address of this block. Use `8080`, `127.0.0.1:8080` or `*:8080`. Two blocks cannot share the same address; `0.0.0.0:PORT` conflicts with every interface on that port. |
| `root` | server, location | Directory the URL is resolved against |
| `index` | server, location | File served when the URL points at a directory |
| `client_max_body_size` | server, location | Largest body accepted. `1m`, `512k` or plain bytes. Over it: 413. In a `location` it lowers the server value for that route only |
| `error_page` | server | `error_page 404 ./www/errors/404.html;` |
| `location <prefix>` | server | Rules for URLs starting with `<prefix>` |
| `allowed_methods` | location | Methods this route accepts. Missing means all. Refused: 405 |
| `autoindex` | location | `on` lists the directory when there is no index file |
| `upload_store` | location | Where POST bodies are written. Missing means uploads are refused |
| `return` | location | Answers 301 with a `Location` header |
| `cgi_ext` | location | `cgi_ext .py /usr/bin/python3;` |

The matched `location` prefix is removed before the rest is added to `root`.
With `location /public { root ./www/public; }`, the URL `/public/a.html` is
served from `./www/public/a.html`.

### Testing

```bash
./webserv configs/multi.conf
```

From another terminal:

```bash
curl -i http://localhost:8080/                  # static page
curl -i http://localhost:9090/                  # other site, same process
curl -i http://localhost:8080/nothing.html      # custom 404 page
curl -i http://localhost:8080/old               # 301 to /public
curl -i -X DELETE http://localhost:8080/public  # 405, this route is GET only
curl -i -F "file=@somefile" http://localhost:8080/uploads/
curl -i http://localhost:8080/uploads/          # autoindex
curl -i "http://localhost:8080/cgi-bin/hello.py?name=42"
```

A browser is the easiest way to see the upload form
(`http://localhost:8080/upload.html`) and the directory listing.

### The tester given with the subject

`configs/tester.conf` is written for it. Build the tree it expects first:

```bash
mkdir -p YoupiBanane/nop YoupiBanane/Yeah
echo "youpi bad extension content" > YoupiBanane/youpi.bad_extension
echo "youpi bla content"           > YoupiBanane/youpi.bla
echo "nop bad ext"                 > YoupiBanane/nop/youpi.bad_extension
echo "other pouic"                 > YoupiBanane/nop/other.pouic
echo "not happy"                   > YoupiBanane/Yeah/not_happy.bad_extension
cp <path to cgi_tester> YoupiBanane/ && chmod +x YoupiBanane/cgi_tester
```

`cgi_tester` has to sit inside `YoupiBanane` because the child process moves
into the directory of the script before `execve`, so `./cgi_tester` is resolved
from there. Then:

```bash
./webserv configs/tester.conf
./tester http://localhost:8080
```

The last test opens 20 connections that each upload 100 MB. It needs a few GB
of free memory.

Two checks that matter more than the others:

- Open a connection with `telnet localhost 8080`, type `GET / HTTP/1.1` and
  stop there. Other requests must still be answered immediately.
- Ask for `www/cgi-bin/slow.py`, which sleeps forever. It is killed after five
  seconds and the server keeps serving everyone else in the meantime.

## Resources

### References

- RFC 7230, HTTP/1.1 message syntax and routing
  <https://datatracker.ietf.org/doc/html/rfc7230>
- RFC 7231, HTTP/1.1 semantics and content
  <https://datatracker.ietf.org/doc/html/rfc7231>
- RFC 3875, the CGI interface, for the list of environment variables
  <https://datatracker.ietf.org/doc/html/rfc3875>
- Beej's Guide to Network Programming <https://beej.us/guide/bgnet/>
- NGINX docs, used as the reference for the configuration syntax and for
  behaviour we were unsure about <https://nginx.org/en/docs/>
- MDN, HTTP section <https://developer.mozilla.org/en-US/docs/Web/HTTP>
- `man 2 poll`, `man 2 socket`, `man 2 accept`, `man 2 execve`, `man 2 fcntl`

On CGI:

- Python docs on the cgi module, a short and clear introduction to what a CGI
  script actually is <https://docs.python.org/fr/3.5/library/cgi.html>
- Lawrence University course notes on CGI and processes. This one helped us
  the most: it shows the fork / pipe / dup2 sequence step by step
  <http://www2.lawrence.edu/fast/GREGGJ/CMSC480/process/cgi.html>

On chunked transfer encoding:

- Pinggy, Content-Length and chunked encoding explained side by side
  <https://pinggy.io/blog/understanding_content_length_header_and_chunked_encoding/>
- A video tutorial on writing a chunked decoder in C++
  <https://www.youtube.com/watch?v=vwbWD3FO89c>

### How we used AI

We used an AI assistant in these ways:

- Finding useful tutorials and documentation, instead of searching blindly.
- Explaining concepts we had never met before: sockets, `poll()`, what a normal
  config file looks like, chunked encoding, CGI, POST and file upload, path
  traversal, RFC, and why one slow client used to block the whole server.
- Suggesting which functions to write and what each one should do, before we
  started coding.
- Reviewing the functions we wrote, testing our changes, and telling us what
  was wrong and how to fix it.
- Helping us read and take over the part of the code written by the other
  member of the group.
- Generating edge case tests to find bugs, for example malformed requests,
  broken CGI scripts and oversized bodies.

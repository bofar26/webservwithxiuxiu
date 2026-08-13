#!/usr/bin/env python3
# A CGI script does not talk to the network. It only:
#   - reads the request through environment variables (and stdin for the body)
#   - prints its own headers, an empty line, then the body
# The server captures that on a pipe and turns it into an HTTP response.

import os
import sys
import datetime

method = os.environ.get("REQUEST_METHOD", "?")
query = os.environ.get("QUERY_STRING", "")
length = os.environ.get("CONTENT_LENGTH", "0")

body = ""
if method == "POST":
    try:
        body = sys.stdin.read(int(length or 0))
    except ValueError:
        body = ""

print("Content-Type: text/html")
print()                       # the empty line ends the CGI headers
print("<!DOCTYPE html><html><head><meta charset='utf-8'>")
print("<title>CGI</title></head><body>")
print("<h1>Hello from a CGI script</h1>")
print("<ul>")
print("<li>REQUEST_METHOD = %s</li>" % method)
print("<li>QUERY_STRING   = %s</li>" % query)
print("<li>CONTENT_LENGTH = %s</li>" % length)
print("<li>SERVER_PORT    = %s</li>" % os.environ.get("SERVER_PORT", "?"))
print("<li>SCRIPT_NAME    = %s</li>" % os.environ.get("SCRIPT_NAME", "?"))
print("<li>generated at   = %s</li>" % datetime.datetime.now().strftime("%H:%M:%S"))
print("</ul>")
if body:
    print("<h2>Body received</h2><pre>%s</pre>" % body)
print("</body></html>")

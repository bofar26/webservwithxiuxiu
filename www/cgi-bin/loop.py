#!/usr/bin/env python3
# Infinite loop on purpose. The server must kill it after its CGI timeout and
# answer 504, while still serving every other client in the meantime.

while True:
    pass

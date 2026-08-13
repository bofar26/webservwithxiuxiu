#!/usr/bin/env python3
# Deliberately hangs, to prove the server kills a runaway script instead of
# waiting forever, and that other clients are still served while it runs.

import time

time.sleep(60)

print("Content-Type: text/plain")
print()
print("you should never see this")

#!/usr/bin/env python3
import os, time
print("Content-Type: text/plain")
print("")
# Lance 3 sous-processus orphelins (pas de fork-bomb réelle)
for i in range(3):
    pid = os.fork()
    if pid == 0:
        time.sleep(0.1)
        os._exit(0)
print("done")

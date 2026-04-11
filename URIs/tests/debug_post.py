#!/usr/bin/env python3
import os, sys

cl = int(os.environ.get("CONTENT_LENGTH", 0))
body_bytes = sys.stdin.buffer.read(cl) if cl > 0 else b""

print("Content-Type: text/plain")
print("")
print("CL env: " + os.environ.get("CONTENT_LENGTH", "?"))
print("Body hex: " + body_bytes.hex())
print("Body repr: " + repr(body_bytes))

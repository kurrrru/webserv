#!/usr/bin/env python3
import sys

print("Location: /redirect2.py", file=sys.stderr)
print("Location: /redirect2.py")
print("")
print("Script redirect1.py executed successfully", file=sys.stderr)
sys.stdout.flush()
sys.stderr.flush()
sys.exit(0)
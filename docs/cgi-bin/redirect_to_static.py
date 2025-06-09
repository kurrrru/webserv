#!/usr/bin/env python3
import sys

print("Location: /static_file.html", file=sys.stderr)
print("Location: /static_file.html")
print("")
print("Script executed successfully", file=sys.stderr)
sys.stdout.flush()
sys.stderr.flush()
sys.exit(0)
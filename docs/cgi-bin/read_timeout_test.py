#!/usr/bin/env python3

import time
import sys
import os

print("Content-Type: text/html")
print()
print("<html><body>")
print("<h1>Timeout Test</h1>")
print("<p>This should not be displayed if timeout works correctly.</p>")
print("</body></html>")
time.sleep(60)
sys.stdout.flush()
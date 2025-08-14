#!/usr/bin/env python3
import os
import sys

src, dest = sys.argv[1], sys.argv[2]
try:
    os.remove(dest)
except OSError:
    pass
os.symlink(src, dest)

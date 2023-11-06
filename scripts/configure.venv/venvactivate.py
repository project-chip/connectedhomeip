# Activates the current venv as if the activate script had been sourced
import collections
import os
import sys

# Prepend the venv bin to PATH (without introducing duplicate entries)
path = [os.path.join(sys.prefix, 'bin')] + os.environ['PATH'].split(':')
os.environ['PATH'] = ':'.join(collections.OrderedDict.fromkeys(path).keys())

# Set VIRTUAL_ENV to the venv directory
os.environ['VIRTUAL_ENV'] = sys.prefix

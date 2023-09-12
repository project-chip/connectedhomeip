# Platform

This directory contains the logging transport implementation for various platforms.

## Adding new platforms

1. Add the platform package in this directory (`/platform`).
2. Create a class which subclasses  `capture.base.PlatformLogStreamer`.
3. Add the class to `__all__` in `/platform/__init__.py`.

# Implementation requirements

Captures should be run in separate processes and minimize blocking main.    
Start should be able to be called repeatedly without restarting streaming.  
Stop should not cause an error even if the stream is not running.

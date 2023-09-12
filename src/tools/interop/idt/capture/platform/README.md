# Platform

This directory contains the logging transport implementation for various
platforms.

## Adding new platforms

1. Add the platform package in this directory (`/platform`).
1. Create a subclass of `capture.base.PlatformLogStreamer`.
1. Add the new class to `__all__` in `/platform/__init__.py`.

# Implementation requirements

Captures should be run in separate processes and minimize blocking main.  
Start should be able to be called repeatedly without restarting streaming.  
Stop should not cause an error even if the stream is not running.

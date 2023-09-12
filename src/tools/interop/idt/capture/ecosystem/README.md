# Ecosystem

This directory contains the capture process and log analysis implementations and validation tests for each ecosystem.

## Adding new ecosystems

1. Add the ecosystem package in this directory (`/ecosystem`).
1. Create a class which subclasses `capture.base.EcosystemCapture`.
1. Add the class to `__all__` in `/ecosystem/__init__.py`

### Notes

Captures should be run in separate processes and minimize blocking main.    

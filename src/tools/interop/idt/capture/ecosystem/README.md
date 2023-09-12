# Ecosystem

This directory contains the capture process and log analysis implementations for
each ecosystem.

## Adding new ecosystems

1. Add the ecosystem package in this directory (`/ecosystem`).
1. Create a subclass of `capture.base.EcosystemCapture`.
1. Add the new class to `__all__` in `/ecosystem/__init__.py`

### Notes

Captures should be run in separate processes and minimize blocking main.

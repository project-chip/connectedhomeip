# Platform

This directory contains the logging transport implementation for various
platforms.

## Extending platforms

The platform loader functions the same as `capture/ecosystem`. See the `README`
there for additional details.

For each package in `capture/platform`, the platform loader expects a module
name matching the package name.  
This module must contain a single class which subclasses
`capture.base.PlatformLogStreamer`.

Note the following runtime expectations of platforms:

-   Start should be able to be called repeatedly without restarting streaming.
-   Stop should not cause an error even if the stream is not running.

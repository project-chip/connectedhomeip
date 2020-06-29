# Connected Home over IP

[![Build Status](https://travis-ci.com/project-chip/connectedhomeip.svg?token=hBypqqXx6Ha9n3zZgyp7&branch=master)](https://travis-ci.com/project-chip/connectedhomeip)

[![codecov](https://codecov.io/gh/project-chip/connectedhomeip/branch/master/graph/badge.svg?token=KP9791CN9W)](https://codecov.io/gh/project-chip/connectedhomeip)

# Getting started with CHIP

All documentation is inside of the [docs section](./docs/README.md)

Instructions about how to build and develop CHIP can be found
[here](./docs/README.md#building-and-developing).

# Need help?

There are numerous avenues for CHIP support:

-   Bugs and feature requests
    [submit to the Issue Tracker](https://github.com/project-chip/connectedhomeip/issues)
-   Stack Overflow
    [post questions using the chip tag](http://stackoverflow.com/questions/tagged/chip)

# Directory Structure

The CHIP repository is structured as follows:

| File / Folder                 | Contents                                                                                                      |
| ----------------------------- | ------------------------------------------------------------------------------------------------------------- |
| `.default-version`            | Default CHIP version if none is available via source code control tags, `.dist-version`, or `.local-version`. |
| `bootstrap`                   | GNU autotools bootstrap script for the CHIP build system.                                                     |
| `bootstrap-configure`         | Convenience script that will bootstrap the CHIP build system, via `bootstrap`, and invoke `configure`.        |
| `build/`                      | Build system support content and build output directories                                                     |
| `BUILDING.md`                 | More detailed information on configuring and building CHIP for different targets                              |
| `CODE_OF_CONDUCT.md`          | Code of Conduct for CHIP, and contributions to it                                                             |
| `CONTRIBUTING.md`             | Guidelines for contributing to CHIP                                                                           |
| `CONTRIBUTORS.md`             | Contributors to CHIP                                                                                          |
| `docs/`                       | Documentation                                                                                                 |
| `examples/`                   | Example firmware applications that demonstrate use of CHIP technology                                         |
| `integrations/`               | Third party integrations related to this project                                                              |
| `integrations/github/`        | GitHub integrations related to this project                                                                   |
| `integrations/github/actions` | GitHub actions related to this project                                                                        |
| `integrations/docker/`        | Docker scripts and Dockerfiles                                                                                |
| `LICENSE`                     | CHIP License file (Apache 2.0)                                                                                |
| `Makefile-Android`            | Convenience makefile for building CHIP against Android.                                                       |
| `Makefile-iOS`                | Convenience makefile for building CHIP against iOS.                                                           |
| `Makefile-Standalone`         | Convenience makefile for building CHIP as a standalone package on desktop and server systems.                 |
| `README.md`                   | This file                                                                                                     |
| `src/`                        | Implementation of CHIP                                                                                        |
| `tests/`                      | Tests for CHIP                                                                                                |
| `third_party/`                | Third-party code used by CHIP.                                                                                |
| `tools/`                      | Tools needed to work with the CHIP repo, as well as develop in the repository                                 |

# Contributing

We would love for you to contribute to CHIP and help make it even better than it
is today! See the [CONTRIBUTING.md](./CONTRIBUTING.md) file for more
information.

# License

CHIP is released under the [Apache 2.0 license](./LICENSE) Please read more
details therein

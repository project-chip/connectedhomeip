# Connected Home over IP

[![Build Status](https://travis-ci.com/project-chip/connectedhomeip.svg?token=hBypqqXx6Ha9n3zZgyp7&branch=master)](https://travis-ci.com/project-chip/connectedhomeip)
[![codecov](https://codecov.io/gh/project-chip/connectedhomeip/branch/master/graph/badge.svg?token=KP9791CN9W)](https://codecov.io/gh/project-chip/connectedhomeip)

# What is Project Connected Home over IP?

Visit [connectedhomeip.com](https://www.connectedhomeip.com/) to learn more.

# Current Status of Project Connected Home over IP

The Zigbee Alliance officially opened the Project Connected Home over IP
(Project CHIP) Working Group on January 17, 2020 and is in the process of
drafting the specification.

The Project’s design and technical processes are intended to be open and
transparent to the general public, including to Work Group non-members wherever
possible. The availability of this GitHub repository and its source code under
an [Apache v2 license](./LICENSE) is an important and demonstrable step to
achieving this commitment.

The Project endeavors to bring together the best aspects of market-tested
technologies and to redeploy them as a unified and cohesive whole-system
solution. The overall goal of this approach is to bring the benefits of the
Project to consumers and manufacturers as quickly as possible. As a result, what
you observe in this repository is an implementation-first approach to the
technical specification, vetting integrations in practice.

# Getting started with Project CHIP

All documentation is inside of the [docs section](./docs/README.md)

Instructions about how to build and develop Project CHIP can be found
[here](./docs/README.md#building-and-developing).

# Need help?

There are numerous avenues for Project CHIP support:

-   Bugs and feature requests
    [submit to the Issue Tracker](https://github.com/project-chip/connectedhomeip/issues)
-   Stack Overflow
    [post questions using the chip tag](http://stackoverflow.com/questions/tagged/chip)

# Directory Structure

The Project CHIP repository is structured as follows:

| File / Folder                 | Contents                                                                                                             |
| ----------------------------- | -------------------------------------------------------------------------------------------------------------------- |
| `.default-version`            | Default Project CHIP version if none is available via source code control tags, `.dist-version`, or `.local-version` |
| `bootstrap`                   | GNU autotools bootstrap script for the Project CHIP build system                                                     |
| `bootstrap-configure`         | Convenience script that will bootstrap the Project CHIP build system, via `bootstrap`, and invoke `configure`        |
| `build/`                      | Build system support content and build output directories                                                            |
| `BUILDING.md`                 | More detailed information on configuring and building Project CHIP for different targets                             |
| `CODE_OF_CONDUCT.md`          | Code of Conduct for Project CHIP, and contributions to it                                                            |
| `CONTRIBUTING.md`             | Guidelines for contributing to Project CHIP                                                                          |
| `CONTRIBUTORS.md`             | Contributors to Project CHIP                                                                                         |
| `docs/`                       | Documentation                                                                                                        |
| `examples/`                   | Example firmware applications that demonstrate use of Project CHIP technology                                        |
| `integrations/`               | Third party integrations related to this project                                                                     |
| `integrations/github/`        | GitHub integrations related to this project                                                                          |
| `integrations/github/actions` | GitHub actions related to this project                                                                               |
| `integrations/docker/`        | Docker scripts and Dockerfiles                                                                                       |
| `images/`                     | Images                                                                                                               |
| `LICENSE`                     | Project CHIP License file (Apache 2.0)                                                                               |
| `Makefile-Android`            | Convenience makefile for building Project CHIP against Android                                                       |
| `Makefile-iOS`                | Convenience makefile for building Project CHIP against iOS                                                           |
| `Makefile-Standalone`         | Convenience makefile for building Project CHIP as a standalone package on desktop and server systems                 |
| `README.md`                   | This file                                                                                                            |
| `src/`                        | Implementation of Project CHIP                                                                                       |
| `tests/`                      | Tests for Project CHIP                                                                                               |
| `third_party/`                | Third-party code used by Project CHIP                                                                                |
| `tools/`                      | Tools needed to work with the Project CHIP repo, as well as develop in the repository                                |

# Contributing

We would love for you to contribute to Project CHIP. Read our contribution
guidelines [here](./CONTRIBUTING.md).

# License

Project CHIP is released under the [Apache 2.0 license](./LICENSE).

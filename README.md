# Connected Home over IP

![Main](https://github.com/project-chip/connectedhomeip/workflows/Builds/badge.svg)
![Examples](https://github.com/project-chip/connectedhomeip/workflows/Examples/badge.svg)
![QEMU](https://github.com/project-chip/connectedhomeip/workflows/QEMU/badge.svg)

# What is Project Connected Home over IP?

Visit [connectedhomeip.com](https://connectedhomeip.com) to learn more.

# Current Status of Project Connected Home over IP

The Zigbee Alliance officially opened the Project Connected Home over IP
(Project CHIP) Working Group on January 17 and is in the process of drafting the
specification.

The Project’s design and technical processes are intended to be open and
transparent to the general public, including to Work Group non-members wherever
possible. The availability of this GitHub repository and its source code under
an Apache v2 license is an important and demonstrable step to achieving this
commitment.

The Project endeavors to bring together the best aspects of market-tested
technologies and to redeploy them as a unified and cohesive whole-system
solution. The overall goal of this approach is to bring the benefits of the
Project to consumers and manufacturers as quickly as possible. As a result, what
you observe in this repository is an implementation-first approach to the
technical specification, vetting integrations in practice.

# How to Contribute

We would love for you to contribute to Project Connected Home over IP. Read our
contribution guidelines
[here](https://github.com/project-chip/connectedhomeip/blob/master/CONTRIBUTING.md).

# Building and Developing in Project

Instructions about how to build Project Connected Home over IP can be found
[here](./docs/README.md#building-and-developing).

# Directory Structure

The Connected Home over IP repository is structured as follows:

| File / Folder          | Contents                                                                                                             |
| ---------------------- | -------------------------------------------------------------------------------------------------------------------- |
| `.default-version`     | Default Project CHIP version if none is available via source code control tags, `.dist-version`, or `.local-version` |
| `bootstrap`            | GNU autotools bootstrap script for the Project CHIP build system                                                     |
| `bootstrap-configure`  | Convenience script that will bootstrap the Project CHIP build system, via `bootstrap`, and invoke `configure`        |
| `build/`               | Build system support content and build output directories                                                            |
| `BUILDING.md`          | More detailed information on configuring and building Project CHIP for different targets                             |
| `CODE_OF_CONDUCT.md`   | Code of Conduct for Project CHIP, and contributions to it                                                            |
| `CONTRIBUTING.md`      | Guidelines for contributing to Project CHIP                                                                          |
| `CONTRIBUTORS.md`      | Contributors to Project CHIP                                                                                         |
| `docs/`                | Documentation                                                                                                        |
| `examples/`            | Example firmware applications that demonstrate use of Project CHIP technology                                        |
| `integrations/`        | Third party integrations related to this project                                                                     |
| `integrations/docker/` | Docker scripts and Dockerfiles                                                                                       |
| `LICENSE`              | Project CHIP [License file](./LICENSE) (Apache 2.0)                                                                  |
| `Makefile-Android`     | Convenience makefile for building Project CHIP against Android                                                       |
| `Makefile-Standalone`  | Convenience makefile for building Project CHIP as a standalone package on desktop and server systems                 |
| `README.md`            | This file                                                                                                            |
| `src/`                 | Implementation of Project CHIP                                                                                       |
| `third_party/`         | Third-party code used by Project CHIP                                                                                |
| `tools/`               | Tools needed to work with the Project CHIP repo                                                                      |

# License

Project CHIP is released under the [Apache 2.0 license](./LICENSE).

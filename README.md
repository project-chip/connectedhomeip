# Matter

[![Builds](https://github.com/project-chip/connectedhomeip/workflows/Builds/badge.svg)](https://github.com/project-chip/connectedhomeip/actions/workflows/build.yaml)

[![Examples - EFR32](https://github.com/project-chip/connectedhomeip/workflows/Build%20example%20-%20EFR32/badge.svg)](https://github.com/project-chip/connectedhomeip/actions/workflows/examples-efr32.yaml)
[![Examples - ESP32](https://github.com/project-chip/connectedhomeip/workflows/Build%20example%20-%20ESP32/badge.svg)](https://github.com/project-chip/connectedhomeip/actions/workflows/examples-esp32.yaml)
[![Examples - i.MX Linux](https://github.com/project-chip/connectedhomeip/workflows/Build%20example%20-%20i.MX%20Linux/badge.svg)](https://github.com/project-chip/connectedhomeip/actions/workflows/examples-linux-imx.yaml)
[![Examples - K32W with SE051](https://github.com/project-chip/connectedhomeip/workflows/Build%20example%20-%20K32W%20with%20SE051/badge.svg)](https://github.com/project-chip/connectedhomeip/actions/workflows/examples-k32w.yaml)
[![Examples - Linux Standalone](https://github.com/project-chip/connectedhomeip/workflows/Build%20example%20-%20Linux%20Standalone/badge.svg)](https://github.com/project-chip/connectedhomeip/actions/workflows/examples-linux-standalone.yaml)
[![Examples - nRF Connect SDK](https://github.com/project-chip/connectedhomeip/workflows/Build%20example%20-%20nRF%20Connect%20SDK/badge.svg)](https://github.com/project-chip/connectedhomeip/actions/workflows/examples-nrfconnect.yaml)
[![Examples - QPG](https://github.com/project-chip/connectedhomeip/workflows/Build%20example%20-%20QPG/badge.svg)](https://github.com/project-chip/connectedhomeip/actions/workflows/examples-qpg.yaml)
[![Examples - TI CC26X2X7](https://github.com/project-chip/connectedhomeip/workflows/Build%20example%20-%20TI%20CC26X2X7/badge.svg)](https://github.com/project-chip/connectedhomeip/actions/workflows/examples-cc13x2x7_26x2x7.yaml)
[![Build example - Infineon](https://github.com/project-chip/connectedhomeip/actions/workflows/examples-infineon.yaml/badge.svg)](https://github.com/project-chip/connectedhomeip/actions/workflows/examples-infineon.yaml)
[![Build example - BouffaloLab](https://github.com/project-chip/connectedhomeip/workflows/Build%20example%20-%20BouffaloLab/badge.svg)](https://github.com/project-chip/connectedhomeip/actions/workflows/examples-bouffalolab.yaml)

[![Android](https://github.com/project-chip/connectedhomeip/workflows/Android/badge.svg)](https://github.com/project-chip/connectedhomeip/actions/workflows/android.yaml)

[![Unit / Interation Tests](https://github.com/project-chip/connectedhomeip/workflows/Unit%20/%20Interation%20Tests/badge.svg)](https://github.com/project-chip/connectedhomeip/actions/workflows/unit_integration_test.yaml)
[![Cirque](https://github.com/project-chip/connectedhomeip/workflows/Cirque/badge.svg)](https://github.com/project-chip/connectedhomeip/actions/workflows/cirque.yaml)
[![QEMU](https://github.com/project-chip/connectedhomeip/workflows/QEMU/badge.svg)](https://github.com/project-chip/connectedhomeip/actions/workflows/qemu.yaml)

[![ZAP Templates](https://github.com/project-chip/connectedhomeip/workflows/ZAP/badge.svg)](https://github.com/project-chip/connectedhomeip/actions/workflows/zap_templates.yaml)

# What is Matter?

Matter (formerly Project Connected Home over IP, or Project CHIP) is a new
Working Group within the Connectivity Standards Alliance (CSA, formerly Zigbee
Alliance). This Working Group plans to develop and promote the adoption of a
new, royalty-free connectivity standard to increase compatibility among smart
home products, with security as a fundamental design tenet.

The goal of the Matter project is to simplify development for manufacturers and
increase compatibility for consumers. The project is built around a shared
belief that smart home devices should be secure, reliable, and seamless to use.
By building upon Internet Protocol (IP), the project aims to enable
communication across smart home devices, mobile apps, and cloud services and to
define a specific set of IP-based networking technologies for device
certification.

The CSA officially opened the Matter Working Group on January 17, 2020 and is in
the process of drafting the specification.

Visit [buildwithmatter.com](https://buildwithmatter.com) to learn more and read
the latest news and updates about the project.

# Project Overview

## Development Goals

Matter is developed with the following goals and principles in mind:

**Unifying:** Matter will produce a new specification, building with and on top
of market-tested, existing technologies.

**Interoperable:** The specification permits communication between any
Matter-certified device, subject to users’ permission.

**Secure:** The specification leverages modern security practices and protocols.

**User Control:** The end user is in control of authorization for interaction
with devices.

**Federated:** No single entity serves as a throttle or a
single-point-of-failure for root of trust.

**Robust:** The set of protocols specify a complete lifecycle of a device —
starting with the seamless out-of-box experience, through operational protocols,
to device and system management specifications required for proper function in
the presence of change.

**Low Overhead:** The protocols are practically implementable on low
compute-resource devices, such as MCUs.

**Pervasive:** The protocols are broadly deployable and accessible, thanks to
leveraging IP and being implementable on low-capability devices.

**Ecosystem-Flexible:** The protocol must be flexible enough to accommodate
deployment in ecosystems with differing policies.

**Easy to Use:** The protocol should aim to provide smooth, cohesive, integrated
provisioning and out-of-box experience.

**Open:** The Project’s design and technical processes should be open and
transparent to the general public, including to non-members wherever possible.

## Architecture Overview

![Matter Architecture Overview](docs/images/CHIP_IP_pyramid.png)

The Project, as illustrated above, defines the application layer that will be
deployed on devices and controllers as well as the supported IPv6-based networks
to help achieve our interoperability architectural goal. Matter will initially
support Wi-Fi and Thread for core, operational communications and Bluetooth Low
Energy (BLE) to simplify device commissioning and setup.

The Application Layer can be further broken down into seven main components:

![Matter Stack Architecture](docs/images/CHIP_Arch_Pyramid.png)

1. **Application:** High order business logic of a device. For example, an
   application that is focused on lighting might contain logic to handle turning
   on/off the bulb as well as its color characteristics.

2. **Data Model:** Data primitives that help describe the various
   functionalities of the devices. The Application operates on these data
   structures when there is intent to interact with the device.

3. **Interaction Model:** Represents a set of actions that can be performed on
   the devices to interact with it. For example, reading or writing attributes
   on a device would correspond to interactions with the devices. These actions
   operate on the structures defined by the data model.

4. **Action Framing:** Once an action is constructed using the Interaction
   Model, it is framed into a prescriptive packed binary format to enable being
   well represented on the “wire”.

5. **Security:** An encoded action frame is then sent down to the Security Layer
   to encrypt and sign the payload to ensure that data is secured and
   authenticated by both sender and receiver of a packet.

6. **Message Framing & Routing:** With an interaction encrypted and signed, the
   Message Layer constructs the payload format with required and optional header
   fields; which specify properties of the message as well as some routing
   information.

7. **IP Framing & Transport Management:** After the final payload has been
   constructed, it is sent to the underlying transport protocol for IP
   management of the data.

# Current Status of Matter

Matter’s design and technical processes are intended to be open and transparent
to the general public, including to Work Group non-members wherever possible.
The availability of this GitHub repository and its source code under an Apache
v2 license is an important and demonstrable step to achieving this commitment.

Matter endeavors to bring together the best aspects of market-tested
technologies and to redeploy them as a unified and cohesive whole-system
solution. The overall goal of this approach is to bring the benefits of Matter
to consumers and manufacturers as quickly as possible. As a result, what you
observe in this repository is an implementation-first approach to the technical
specification, vetting integrations in practice.

The Matter repository is growing and evolving to implement the overall
architecture. The repository currently contains the security foundations,
message framing and dispatch, an implementation of the interaction model and
data model. The code examples show simple interactions, and are supported on
multiple transports -- Wi-Fi and Thread -- starting with resource-constrained
(i.e., memory, processing) silicon platforms to help ensure Matter’s
scalability.

# How to Contribute

We welcome your contributions to Matter. Read our contribution guidelines
[here](./CONTRIBUTING.md).

# Building and Developing in Matter

Instructions about how to build Matter can be found [here](./docs/README.md).

# Directory Structure

The Matter repository is structured as follows:

| File / Folder                          | Contents                                                                           |
| -------------------------------------- | ---------------------------------------------------------------------------------- |
| `build/`                               | Build system support content and build output directories                          |
| [BUILDING.md](docs/guides/BUILDING.md) | More detailed information on configuring and building Matter for different targets |
| `CODE_OF_CONDUCT.md`                   | Code of Conduct for Matter, and contributions to it                                |
| [CONTRIBUTING.md](./CONTRIBUTING.md)   | Guidelines for contributing to Matter                                              |
| `docs/`                                | Documentation, including [guides](./docs/guides)                                   |
| `examples/`                            | Example firmware applications that demonstrate use of the Matter technology        |
| `integrations/`                        | Third party integrations related to this project                                   |
| `integrations/docker/`                 | Docker scripts and Dockerfiles                                                     |
| `LICENSE`                              | Matter [License file](./LICENSE) (Apache 2.0)                                      |
| `BUILD.gn`                             | Top level GN build file                                                            |
| `README.md`                            | This file                                                                          |
| `src/`                                 | Implementation of Matter                                                           |
| `third_party/`                         | Third-party code used by Matter                                                    |
| `scripts/`                             | Scripts needed to work with the Matter repository                                  |

# License

Matter is released under the [Apache 2.0 license](./LICENSE).

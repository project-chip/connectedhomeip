# Texas Instruments Matter over Thread Overview

The following diagram is a simplified representation of a Matter application
which is built on the TI SimpleLink™ platform.

![matter_ti_overview_simplified](docs/guides/images/matter_ti_overview_simplified.png)

## Bluetooth® Low Energy and Thread stacks

In the TI application examples, the Bluetooth Low Energy protocol is used to
provision the Thread network to enable Matter communication. Following Thread
provisioning, the OpenThread stack, available within the Matter repository, is
used for IP communication with other Matter-enabled devices.

The TI application examples leverage the Bluetooth Low Energy stack on the
CC13X2 and CC26X2 families. This Bluetooth Low Energy software is distributed in
binary form. The Bluetooth Low Energy stack leverages code that is present in
the device ROM for certain common Bluetooth Low Energy operations.

These connection protocols can be run concurrently by using the Texas
Instruments Dynamic Multi-Protocol Manager.

## LwIP stack

The Lightweight IP stack interfaces with the OpenThread stack to offer standard
IP connectivity protocols that OpenThread does not natively support. This offers
a standard socket-based interface to the Matter platform.

## MbedTLS

The MbedTLS library is used by OpenThread and Matter for a wide variety of
protocols. This ranges from basic AES and SHA to cryptographic protocols like
ECDSA and ECDH. The MbedTLS library is hardware accelerated using the TI
SimpleLink SDK drivers. This is achieved through the usage of `_ALT` defines in
the MbedTLS configuration file.

## Matter integration

Matter interacts with LwIP, OpenThread, and the TI Bluetooth Low Energy stack to
achieve the protocol and application functionality. A Bluetooth Low Energy
profile is registered with the TI Bluetooth Low Energy stack to enable
provisioning and configuration. Once the device is provisioned Matter will
configure the OpenThread interface to connect to an existing Thread network or
to start its own network. From there the Matter IP messages are sent to the LwIP
stack to be routed to the OpenThread stack for transmission.

Overall, applications generally only need to interface with the Cluster Library
from Matter. The transport of messages and configuration of the device is all
handled by the platform implementation files.

## Matter example applications

Sample Matter applications are provided for the TI SimpleLink platform. These
can be used as reference for your own application.

-   [all-clusters-app](examples/all-clusters-app/cc13x2x7_26x2x7/README.md)
-   [all-clusters-minimal-app](examples/all-clusters-minimal-app/cc13x2x7_26x2x7/README.md)
-   [lock-app](examples/lock-app/cc13x2x7_26x2x7/README.md)
-   [pump-app](examples/pump-app/cc13x2x7_26x2x7/README.md)
-   [pump-controller-app](examples/pump-controller-app/cc13x2x7_26x2x7/README.md)
-   [shell](examples/shell/cc13x2x7_26x2x7/README.md)

## Getting Started

See the [Matter on Thread Getting Started Guide][matter_e2e_guide] to get
started with Matter on SimpleLink Thread devices.

[matter_e2e_guide]:
    https://e2e.ti.com/support/wireless-connectivity/zigbee-thread-group/zigbee-and-thread/f/zigbee-thread-forum/1082428/faq-cc2652r7-matter----getting-started-guide

## Build system

The TI platform uses GN to generate ninja build scripts. Build files have
already been written to build and link the TI specific code within the
SimpleLink SDK.

## Release Notes

The release notes containing the list of features, bug fixes and known issues is
located at [Release Notes](./RELEASE_NOTES.md).

## TI Support

For technical support, please consider creating a post on TI's [E2E
forum][e2e_forum]. Additionally, we welcome any feedback.

[e2e_forum]: https://e2e.ti.com/support/wireless-connectivity/zigbee-and-thread

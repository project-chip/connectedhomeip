```{toctree}
:glob:
:maxdepth: 1
:hidden:

matter-migration-guide/matter_cc2674_migration.md
matter-syscfg/getting-started.md
matter-syscfg/sysconfig-board.md
matter-users-guide/ti_openthread_library_usage.md
matter-users-guide/ti_factory_data_user_guide.md
matter-users-guide/enabling_icd_on_ti_devices.md
```

# Texas Instruments platform overview

The Texas Instruments Matter platform is based on the TI SimpleLink™ SDK.

The following diagram is a simplified representation of a Matter application
which is built on the TI Platform.

<div style="text-align: center;">
  <img src="../images/matter_ti_overview_simplified.png" width=30%>
</div>

## Texas Instruments SimpleLink SDK

This Matter GitHub repository contains the software development components and
tools that enable engineers to develop Matter based products. This repository is
the starting point for Matter development on all SimpleLink Thread and Wi-Fi
wireless microcontrollers (MCUs).

The SimpleLink MCU portfolio offers a single development environment that
delivers flexible hardware, software, and tool options for customers developing
wired and wireless applications. With 100 percent code reuse across host MCUs,
Wi-Fi™, Bluetooth Low Energy, 2.4GHz, Sub-1GHz devices and more, choose the MCU
or connectivity standard that fits your design. A one-time investment with the
SimpleLink software development kit allows you to reuse often, opening the door
to create unlimited applications. For more information, visit
www.ti.com/simplelink.

<hr>

## Bluetooth LE and Thread stacks

In the TI example applications the Bluetooth Low Energy protocol is used to
provision the Thread protocol to enable Matter communication. Then Thread is
used for IP communication with other Matter devices.

The TI applications leverage the Bluetooth Low Energy stack on the CC13XX and
CC26XX families. This Bluetooth LE software is distributed in binary form within
the TI SimpleLink SDK. The Bluetooth LE stack leverages code that is present in
the device ROM for certain common Bluetooth LE operations.

These applications leverage the OpenThread stack available within the Matter
repository for Thread communication. Platform support source is built from the
SimpleLink SDK.

These connection protocols can be run concurrently by using the Texas
Instruments Dynamic Multi-protocol Manager.

<hr>

## LwIP stack

The Lightweight IP stack interfaces with the OpenThread stack to offer standard
IP connectivity protocols that OpenThread does not natively support. This offers
a standard socket based interface to the Matter platform.

<hr>

## MbedTLS

The MbedTLS library is used by OpenThread and Matter for a wide variety of
protocols. This ranges from basic AES and SHA to cryptographic protocols like
ECDSA and ECDH.

The MbedTLS library is hardware accelerated using the TI SimpleLink SDK drivers.
This is achieved through the usage of `_ALT` defines in the MbedTLS
configuration file.

<hr>

## Matter Stack to TI Platform Interface

Matter Stack interacts with LwIP, OpenThread, and the TI-BLE stack to achieve
the protocol and application functionality. A Bluetooth LE profile is registered
with the TI-BLE stack to enable provisioning and configuration. Once the device
is provisioned Matter will configure the OpenThread interface to connect to an
existing Thread network or to start its own network. From there the Matter IP
messages are sent to the LwIP stack to be routed to the OpenThread stack for
transmission. Matter Impl (Implementation) layer acts as an interface between
Matter stack and the TI platform components such as BLE stack, OpenThread,
FreeRTOS. It also supports components such as connectivity manager that provides
the implementation for functionality required by Matter stack. Overall,
applications generally only need to interface with the Cluster Library from
Matter. The transport of messages and configuration of the device is all handled
by the platform implementation files.

<hr>

## Matter Development Resources

Below are several resources available for Matter development:

-   [Matter Protocol Overview](https://handbook.buildwithmatter.com/howitworks/roles/)
-   [Matter Build Guide](../BUILDING.md)
-   [Matter over Thread Getting Started](https://dev.ti.com/tirex/explore/node?node=A__AciOYyNq9gli.nsvJzBtQg__com.ti.SIMPLELINK_ACADEMY_CC13XX_CC26XX_SDK__AfkT0vQ__LATEST)
-   [TI Matter over Wi-Fi Getting Started](https://e2e.ti.com/support/wireless-connectivity/wi-fi-group/wifi/f/wi-fi-forum/1122413/faq-cc3235sf-matter----getting-started-guide)
-   [TI Matter Application Development](https://dev.ti.com/tirex/explore/node?node=A__AXNOPYikmtBCHJ-L6eRivA__com.ti.SIMPLELINK_ACADEMY_CC13XX_CC26XX_SDK__AfkT0vQ__LATEST)
-   [TI Matter OTA Guide](https://dev.ti.com/tirex/explore/node?node=A__AYTiKtu5heqgH4KPFa.6RQ__com.ti.SIMPLELINK_ACADEMY_CC13XX_CC26XX_SDK__AfkT0vQ__LATEST)

<hr>

## Matter Example Applications

Sample Matter applications are provided for the TI platform. These can be used
as reference for your own application.

-   [lock-app](../../../examples/lock-app/cc13x4_26x4/README.md)
-   [pump-app](../../../examples/pump-app/cc13x4_26x4/README.md)
-   [pump-controller-app](../../../examples/pump-controller-app/cc13x4_26x4/README.md)
-   [lighting-app](../../../examples/lighting-app/cc13x4_26x4/README.md)
    <hr>

### Build system

The TI platform uses GN to generate ninja build scripts. Build files have
already been written to build and link the TI specific code within the
SimpleLink SDK.

<hr>

## CC2674 Migration

For instructions on how to migrate the CC1354P10-6 examples to either the
CC2674P10 or the CC2674R10, please refer to the guide linked below.

-   [TI CC2674 Migration Guide](./matter-migration-guide/matter_cc2674_migration.md)

<hr>

## Factory Data Programming Tool

For instructions on how to program custom factory data on TI devices, please
refer to the guide linked below.

-   [TI Factory Data User Guide](./matter-users-guide/ti_factory_data_user_guide.md)

<hr>

## Intermittently Connected Devices

For instructions on how to use the Matter ICD feature on TI devices, please
refer to the guide linked below.

-   [Enabling ICD On TI Devices](./matter-users-guide/enabling_icd_on_ti_devices.md)

<hr>

## Matter OpenThread Library Configuration

For instructions on how to configure the OpenThread build configuration for a
Matter application, please refer to the guide linked below.

-   [OpenThread Library Configuration](./matter-users-guide/ti_openthread_library_usage.md)

### TI Support

For technical support, please consider creating a post on TI's [E2E forum][e2e].
Additionally, we welcome any feedback.

[e2e]: https://e2e.ti.com/support/wireless-connectivity/zigbee-and-thread
[matter_gh]: https://github.com/project-chip/connectedhomeip

# Matter NXP Lighting Example Application

-   [Overview](#overview)
-   [Supported Platforms](#supported-platforms)
-   [Environment Setup, Building, and Testing](#build)
-   [Data Model](#data-model)
-   [Manufacturing Data](#factory-data)

<a name="overview"></a>

## Overview

This reference application implements an On/Off Light device type. It uses board
buttons or `matter-cli` for user input and LEDs for state feedback. You can use
this example as a reference for creating your own application.

The example is based on
[Project CHIP](https://github.com/project-chip/connectedhomeip) and NXP SDK, and
provides a prototype application that demonstrates device commissioning and
different cluster control.

The light bulb device communicates with clients over a low-power, 802.15.4
Thread network. It can be commissioned into an existing Matter network using a
controller such as `chip-tool`.

This example implements a `User-Intent Commissioning Flow`, meaning that the
user is required to press a button on the device in order to get it ready for
commissioning. The initial commissioning is usually performed using the
`ble-thread` pairing method.

The Thread network dataset will be transferred on the device using a secure
session over Bluetooth LE. In order to start the commissioning process, the user
must enable BLE advertising on the device manually. To pair successfully, the
commissioner must know the commissioning information corresponding to the
device: setup passcode and/or discriminator. This data is usually encoded within
a QR code or printed to the device's UART console.

<a name="supported-platforms"></a>

## Supported Platforms

The On/Off Light example is supported on the following platforms:

| NXP platform | Dedicated readme                                                    |
| ------------ | ------------------------------------------------------------------- |
| MCXW71       | [NXP MCXW71 Guide](../../../docs/platforms/nxp/nxp_mcxw71_guide.md) |
| MCXW72       | [NXP MCXW72 Guide](../../../docs/platforms/nxp/nxp_mcxw72_guide.md) |

For details on platform-specific requirements and configurations, please refer
to the respective platform's readme.

A list of popular standard lighting app targets is presented below. These
targets can be used with the `build_example.py` tool.

| Target name                                                         | Description                                                                     |
| ------------------------------------------------------------------- | ------------------------------------------------------------------------------- |
| `nxp-<device>-freertos-lighting-thread-frdm`                        | Default lighting app                                                            |
| `nxp-<device>-freertos-lighting-thread-factory-frdm`                | Default lighting app with factory data                                          |
| `nxp-<device>-freertos-lighting-thread-rotating-id-frdm`            | Lighting app with rotating device id support                                    |
| `nxp-<device>-freertos-lighting-thread-factory-dac-conversion-frdm` | Lighting app that leverages a secure enclave to secure the DAC private key      |
| `nxp-<device>-freertos-lighting-thread-sw-v2-frdm`                  | Lighting app with software version 2 (can be used to test OTA)                  |
| `nxp-<device>-freertos-lighting-thread-factory-sw-v2-frdm`          | Lighting app with factory data and software version 2 (can be used to test OTA) |

where `device` is be one of the [supported platforms](#supported-platforms).

<a name="build"></a>

## Environment Setup, Building, and Testing

All the information required to set up the environment, build the application,
and test it can be found in the common readme for NXP platforms:

-   NXP FreeRTOS Platforms : Refer to the
    [CHIP NXP Examples Guide for FreeRTOS platforms](../../../docs/platforms/nxp/nxp_examples_freertos_platforms.md)

<a name="data-model"></a>

## Data Model

The application uses an NXP specific data model file:

| Path                      | Description                             |
| ------------------------- | --------------------------------------- |
| `zap/lighting-on-off.zap` | Data model for On/Off Light device type |

<a name="factory-data"></a>

## Manufacturing Data

The support for manufacturing data is enabled at build time when you use the
appropriate build configuration files for this feature. Please refer to the
[NXP Matter examples guide for FreeRTOS](../../../docs/platforms/nxp/nxp_examples_freertos_platforms.md)
for details on the build process and available build configurations.
Manufacturing data can also be forced in the build command line by using the
`CONFIG_CHIP_FACTORY_DATA=y` build option or by using the appropriate
`build_examples.py` [targets](#supported-platforms).

For a full guide on the manufacturing flow, please refer to the
[Guide for writing manufacturing data on NXP devices](../../../docs/platforms/nxp/nxp_manufacturing_flow.md).

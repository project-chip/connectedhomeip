# Matter NXP Contact Sensor Example Application

-   [Overview](#overview)
-   [Supported Platforms](#supported-platforms)
-   [Environment Setup, Building, and Testing](#build)
-   [Data Model](#data-model)
-   [Manufacturing Data](#factory-data)
-   [Long Idle Time ICD Support](#long-idle-time-icd-support)
-   [Low Power](#low-power)

<a name="overview"></a>

## Overview

This reference application implements a Contact Sensor device type. It uses
board buttons or `matter-cli` for user input and LEDs for state feedback. You
can use this example as a reference for creating your own application.

The example is based on
[Project CHIP](https://github.com/project-chip/connectedhomeip) and NXP SDK, and
provides a prototype application that demonstrates device commissioning and
different cluster control.

This contact sensor communicates with clients over a low-power, 802.15.4 Thread
network. It can be commissioned into an existing Matter network using a
controller, such as `chip-tool`.

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

The Contact Sensor example is supported on the following platforms:

| NXP platform | Dedicated readme                                                    |
| ------------ | ------------------------------------------------------------------- |
| MCXW71       | [NXP MCXW71 Guide](../../../docs/platforms/nxp/nxp_mcxw71_guide.md) |
| MCXW72       | [NXP MCXW72 Guide](../../../docs/platforms/nxp/nxp_mcxw72_guide.md) |

For details on platform-specific requirements and configurations, please refer
to the respective platform's readme.

A list of popular standard contact sensor app build targets is presented below.
These targets can be used with the `build_example.py` tool.

| Target name                                                                    | Description                                                                                 |
| ------------------------------------------------------------------------------ | ------------------------------------------------------------------------------------------- |
| `nxp-<device>-freertos-contact-sensor-thread-mtd-frdm`                         | Default contact sensor                                                                      |
| `nxp-<device>-freertos-contact-sensor-thread-mtd-low-power-frdm`               | Default low-power contact sensor                                                            |
| `nxp-<device>-freertos-contact-sensor-thread-mtd-low-power-factory-frdm`       | Default low-power contact sensor with factory data                                          |
| `nxp-<device>-freertos-contact-sensor-thread-mtd-low-power-lit-frdm`           | Low-power contact sensor as LIT ICD                                                         |
| `nxp-<device>-freertos-contact-sensor-thread-mtd-low-power-sw-v2-frdm`         | Low-power contact sensor with software version 2 (can be used to test OTA)                  |
| `nxp-<device>-freertos-contact-sensor-thread-mtd-low-power-factory-sw-v2-frdm` | Low-power contact sensor with factory data and software version 2 (can be used to test OTA) |

where `device` is one of the [supported platforms](#supported-platforms).

<a name="build"></a>

## Environment Setup, Building, and Testing

All the information required to set up the environment, build the application,
and test it can be found in the common readme for NXP platforms:

-   NXP FreeRTOS Platforms : Refer to the
    [CHIP NXP Examples Guide for FreeRTOS platforms](../../../docs/platforms/nxp/nxp_examples_freertos_platforms.md)

<a name="data-model"></a>

## Data Model

There are two available data models that can be used by the application:

| Path                             | Description                    |
| -------------------------------- | ------------------------------ |
| `zap-lit/contact-sensor-app.zap` | Data model for LIT ICD support |
| `zap-sit/contact-sensor-app.zap` | Data model for SIT ICD support |

The data model is selected automatically by the build system based on the user
ICD flavor build option (SIT or LIT).

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

<a name="long-idle-time-icd-support"></a>

## Long Idle Time ICD Support

By default, the application is compiled as a SIT ICD (Short Idle Time
Intermittently Connected Device).

The following is a list of ICD configuration build options. These options can be
provided on the build command line to override default values. Please refer to
your [platform guide](#supported-platforms) for details on the build process.

| Build option                            | Default value | Description                                                                                                |
| --------------------------------------- | ------------- | ---------------------------------------------------------------------------------------------------------- |
| `CONFIG_CHIP_ICD_SLOW_POLL_INTERVAL`    | 2000 (ms)     | OT Idle Mode poll interval duration                                                                        |
| `CONFIG_CHIP_ICD_FAST_POLLING_INTERVAL` | 500 (ms)      | OT Active Mode poll interval duration                                                                      |
| `CONFIG_CHIP_ICD_IDLE_MODE_DURATION`    | 600 (s)       | Idle Mode Interval duration                                                                                |
| `CONFIG_CHIP_ICD_ACTIVE_MODE_DURATION`  | 10000 (ms)    | Active Mode Interval duration                                                                              |
| `CONFIG_CHIP_ICD_ACTIVE_MODE_THRESHOLD` | 5000 (ms)     | Active Mode Threshold duration                                                                             |
| `CONFIG_CHIP_ICD_CLIENTS_PER_FABRIC`    | 2             | Registration slots per fabric                                                                              |
| `CONFIG_CHIP_ICD_DSLS_SUPPORT`          | n             | Enable LIT ICD DSLS support                                                                                |
| `CONFIG_CHIP_PERSISTENT_SUBSCRIPTIONS`  | y             | Try once to re-establish subscriptions from the server side after reboot. May be disabled for LIT use case |

If LIT ICD support is required then `prj_thread_mtd_low_power_lit.conf` build
configuration file should be used when building the application. This will
automatically configure the above parameters to define a LIT device. You can
still override these values on the build command line in case you want to use
different values from the defaults.

| Build option                            | LIT ICD default value |
| --------------------------------------- | --------------------- |
| `CONFIG_CHIP_ICD_SLOW_POLL_INTERVAL`    | 15000 (ms)            |
| `CONFIG_CHIP_ICD_FAST_POLLING_INTERVAL` | 500 (ms)              |
| `CONFIG_CHIP_ICD_IDLE_MODE_DURATION`    | 3600 (s)              |
| `CONFIG_CHIP_ICD_ACTIVE_MODE_DURATION`  | 15000 (ms)            |
| `CONFIG_CHIP_ICD_ACTIVE_MODE_THRESHOLD` | 30000 (ms)            |

<a name="low-power"></a>

## Low Power

This example also offers the possibility to run in low power mode. This means
that the device will use deep sleep/power down most of the time to minimize
power consumption.

Low power support is enabled at build time when using the build configuration
files appropriate for low-power app configurations. Please refer to the
[NXP Matter examples guide for FreeRTOS](../../../docs/platforms/nxp/nxp_examples_freertos_platforms.md)
for details on the build process and available build configurations.

In order to maintain low power consumption, the UI LEDs are disabled and device
log and Matter CLI will not be available. For debugging purposes device log can
be forced using the `CONFIG_LOG=y` build option, but this might affect low power
timings. Also, please note that once the application is flashed onto the board
the debugger might disconnect when the board enters low power.

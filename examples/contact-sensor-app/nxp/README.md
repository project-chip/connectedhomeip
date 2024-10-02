# Matter NXP Contact Sensor Example Application

This reference application implements a Contact Sensor device type. It uses
board buttons or `matter-cli` for user input and LEDs for state feedback. You
can use this example as a reference for creating your own application.

The example is based on:

-   [Matter](https://github.com/project-chip/connectedhomeip)
-   [NXP github SDK](https://github.com/nxp-mcuxpresso/mcux-sdk)

-   [Matter NXP Contact Sensor Example Application](#matter-nxp-contact-sensor-example-application)
    -   [Supported devices](#supported-devices)
    -   [Introduction](#introduction)
    -   [Device UI](#device-ui)
    -   [Prerequisites for building](#prerequisites-for-building)
    -   [Building](#building)
        -   [Data model](#data-model)
        -   [Manufacturing data](#manufacturing-data)
        -   [Long Idle Time ICD Support](#long-idle-time-icd-support)
        -   [Low power](#low-power)
    -   [Flashing and debugging](#flashing-and-debugging)

## Supported devices

-   [k32w1](k32w1/README.md)
-   [mcxw71](mcxw71/README.md)

## Introduction

The application showcases a contact sensor that communicates with clients over a
low-power, 802.15.4 Thread network.

It can be commissioned into an existing Matter network using a controller such
as `chip-tool`.

This example implements a `User-Intent Commissioning Flow`, meaning the user has
to press a button in order for the device to be ready for commissioning. The
initial commissioning is done through `ble-thread` pairing method.

The Thread network dataset will be transferred on the device using a secure
session over Bluetooth LE. In order to start the commissioning process, the user
must enable BLE advertising on the device manually. To pair successfully, the
commissioner must know the commissioning information corresponding to the
device: setup passcode and discriminator. This data is usually encoded within a
QR code or printed to the UART console.

## Device UI

The example application provides a simple UI that depicts the state of the
device and offers basic user control. This UI is implemented via the
general-purpose LEDs and buttons built in the evaluation boards. Please see each
supported device readme file for details.

## Prerequisites for building

In order to build the example, it is recommended to use a Linux distribution.
Please visit the supported Operating Systems list in
[BUILDING.md](../../../docs/guides/BUILDING.md#prerequisites).

-   Make sure that below prerequisites are correctly installed (as described in
    [BUILDING.md](../../../docs/guides/BUILDING.md#prerequisites))

```
sudo apt-get install git gcc g++ pkg-config libssl-dev libdbus-1-dev libglib2.0-dev libavahi-client-dev ninja-build python3-venv python3-dev python3-pip unzip libgirepository1.0-dev libcairo2-dev libreadline-dev
```

-   Step 1: checkout NXP specific submodules only

    ```
    user@ubuntu:~/Desktop/git/connectedhomeip$ scripts/checkout_submodules.py --shallow --platform nxp --recursive
    ```

-   Step 2: activate local environment

    ```
    user@ubuntu:~/Desktop/git/connectedhomeip$ source scripts/activate.sh
    ```

    If the script says the environment is out of date, you can update it by
    running the following command:

    ```
    user@ubuntu:~/Desktop/git/connectedhomeip$ source scripts/bootstrap.sh
    ```

-   Step 3: Init NXP SDK(s)

    ```
    user@ubuntu:~/Desktop/git/connectedhomeip$ third_party/nxp/nxp_matter_support/scripts/update_nxp_sdk.py --platform common
    ```

Note: By default, `update_nxp_sdk.py` will try to initialize all NXP SDKs.
Please run the script with arg `--help` to view all available options.

## Building

There are two options for building this reference app:

-   Using `build_examples.py` framework.
-   Manually generating `ninja` files using `gn`.

For manual generation and building, please see the specific readme file for your
device.

A list of all available contact sensor targets can be viewed in the following
table:

| target name                                                | description                                                                                 |
| ---------------------------------------------------------- | ------------------------------------------------------------------------------------------- |
| nxp-device-freertos-contact-sensor-low-power               | Default low-power contact sensor                                                            |
| nxp-device-freertos-contact-sensor-low-power-factory       | Default low-power contact sensor with factory data                                          |
| nxp-device-freertos-contact-sensor-low-power-lit           | Low-power contact sensor as LIT ICD                                                         |
| nxp-device-freertos-contact-sensor-low-power-sw-v2         | Low-power contact sensor with software version 2 (can be used to test OTA)                  |
| nxp-device-freertos-contact-sensor-low-power-factory-sw-v2 | Low-power contact sensor with factory data and software version 2 (can be used to test OTA) |

where `device` can be one of the [Supported devices](#supported-devices).

### Data model

There are two available data models that can be used by the application:

| path                             | description                    |
| -------------------------------- | ------------------------------ |
| `zap-lit/contact-sensor-app.zap` | Data model for LIT ICD support |
| `zap-sit/contact-sensor-app.zap` | Data model for SIT ICD support |

The selection is done automatically by the build system based on the ICD
configuration.

The data model can be changed by simply replacing the gn `deps` statement
corresponding to data model target.

### Manufacturing data

Use `chip_with_factory_data=1` in the gn build command to enable factory data.

For a full guide on manufacturing flow, please see
[Guide for writing manufacturing data on NXP devices](../../../docs/guides/nxp/nxp_manufacturing_flow.md).

### Long Idle Time ICD Support

By default, the application is compiled as a SIT ICD (Short Idle Time
Intermittently Connected Device).

This is a list of ICD configuration gn args.

| gn arg                                                                         | default value | description                                                                                                |
| ------------------------------------------------------------------------------ | ------------- | ---------------------------------------------------------------------------------------------------------- |
| nxp_ot_idle_interval_ms                                                        | 2000 (ms)     | OT Idle Interval duration                                                                                  |
| nxp_ot_active_interval_ms                                                      | 500 (ms)      | OT Active Interval duration                                                                                |
| nxp_idle_mode_duration_s                                                       | 600 (s)       | Idle Mode Interval duration                                                                                |
| nxp_active_mode_duration_ms                                                    | 10000 (ms)    | Active Mode Interval duration                                                                              |
| nxp_active_mode_threshold_ms                                                   | 1000 (ms)     | Active Mode Threshold value                                                                                |
| nxp_icd_supported_clients_per_fabric                                           | 2             | Registration slots per fabric                                                                              |
| chip_enable_icd_lit                                                            | false         | Enable LIT ICD support                                                                                     |
| chip_enable_icd_dsls                                                           | false         | Enable LIT ICD DSLS support                                                                                |
| chip_persist_subscriptions                                                     | true          | Try once to re-establish subscriptions from the server side after reboot. May be disabled for LIT use case |
| chip_subscription_timeout_resumption                                           | true          | Same as above, but try to re-establish timeout out subscriptions                                           |
| using `Fibonacci Backoff` for retries pacing. May be disabled for LIT use case |

If LIT ICD support is needed then `chip_enable_icd_lit=true` must be specified
as gn argument and the above parameters must be modified to comply with LIT
requirements (e.g.: LIT devices must configure
`chip_ot_idle_interval_ms > 15000`). Example LIT configuration:

```
nxp_ot_idle_interval_ms = 15000           # 15s Idle Intervals
nxp_ot_active_interval_ms = 500           # 500ms Active Intervals
nxp_idle_mode_duration_s = 3600           # 60min Idle Mode Interval
nxp_active_mode_duration_ms = 0           # 0 Active Mode Interval
nxp_active_mode_threshold_ms = 30000      # 30s Active Mode Threshold
```

### Low power

The example also offers the possibility to run in low power mode. This means
that the board will go in deep sleep most of the time and the power consumption
will be very low.

In order to build with low power support, the following gn args must be used:

```
chip_with_low_power = 1
chip_openthread_ftd = false
chip_with_ot_cli = 0
chip_logging = false
```

In order to maintain a low power consumption, the UI LEDs are disabled. Console
logs can be used instead, but it might affect low power timings. Also, please
note that once the board is flashed with MCUXpresso the debugger disconnects
because the board enters low power.

## Flashing and debugging

Please see the device specific readme file.

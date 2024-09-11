# Matter NXP Lighting Example Application

This reference application implements an On/Off Light device type. It uses board
buttons or `matter-cli` for user input and LEDs for state feedback. You can use
this example as a reference for creating your own application.

The example is based on:

-   [Matter](https://github.com/project-chip/connectedhomeip)
-   [NXP github SDK](https://github.com/nxp-mcuxpresso/mcux-sdk)

-   [Matter NXP Lighting Example Application](#matter-nxp-lighting-example-application)
    -   [Supported devices](#supported-devices)
    -   [Introduction](#introduction)
    -   [Device UI](#device-ui)
    -   [Prerequisites for building](#prerequisites-for-building)
    -   [Building](#building)
        -   [Data model](#data-model)
        -   [Manufacturing data](#manufacturing-data)
    -   [Flashing and debugging](#flashing-and-debugging)

## Supported devices

-   [k32w1](k32w1/README.md)
-   [mcxw71](mcxw71/README.md)

## Introduction

The application showcases a light bulb device that communicates with clients
over a low-power, 802.15.4 Thread network.

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

| target name                                         | description                                                                     |
| --------------------------------------------------- | ------------------------------------------------------------------------------- |
| nxp-device-freertos-lighting                        | Default lighting app                                                            |
| nxp-device-freertos-lighting-factory                | Default lighting app with factory data                                          |
| nxp-device-freertos-lighting-rotating-id            | Lighting app with rotating device id support                                    |
| nxp-device-freertos-lighting-factory-dac-conversion | Lighting app that leverages a secure enclave to secure the DAC private key      |
| nxp-device-freertos-lighting-sw-v2                  | Lighting app with software version 2 (can be used to test OTA)                  |
| nxp-device-freertos-lighting-factory-sw-v2          | Lighting app with factory data and software version 2 (can be used to test OTA) |

where `device` can be one of the [Supported devices](#supported-devices).

### Data model

The application uses an NXP specific data model file:

| path                      | description                             |
| ------------------------- | --------------------------------------- |
| `zap/lighting-on-off.zap` | Data model for On/Off Light device type |

The data model can be changed by simply replacing the gn `deps` statement
corresponding to data model target.

### Manufacturing data

Use `chip_with_factory_data=1` in the gn build command to enable factory data.

For a full guide on manufacturing flow, please see
[Guide for writing manufacturing data on NXP devices](../../../docs/guides/nxp/nxp_manufacturing_flow.md).

## Flashing and debugging

Please see the device specific readme file.

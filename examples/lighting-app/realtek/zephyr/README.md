# Matter Realtek Lighting Example Application

This reference application implements an On/Off Light device type. It uses
buttons to test changing the lighting and device states and LEDs to show the
state of these changes. You can use this example as a reference for creating
your own application.

The example is based on:

-   [Matter](https://github.com/project-chip/connectedhomeip)
-   [Realtek zephyr SDK](https://github.com/rtkconnectivity/zephyr)
-   [Realtek HAL module](https://github.com/rtkconnectivity/hal_realtek)

## Supported Device

-   RTL87x2G

## Introduction

The application showcases a light bulb device that communicates with clients
over a low-power, 802.15.4 Thread network.

It can be commissioned into an existing Matter network using a controller such
as `chip-tool`.

This example implements a `Standard commissioning flow`, meaning devices not
connected to the network automatically broadcast advertising when powered on.
The initial commissioning is done through `ble-thread` pairing method.

The Thread network dataset will be transferred on the device using a secure
session over Bluetooth LE.To pair successfully, the commissioner must know the
commissioning information corresponding to the device: setup passcode and
discriminator. This data is usually encoded within a QR code or printed to the
UART console.

## Prerequisites for building

In order to build the example, it is recommended to use a Linux distribution.
Please visit the supported Operating Systems list in
[BUILDING.md](../../../../docs/guides/BUILDING.md#prerequisites).

-   Make sure that below prerequisites are correctly installed (as described in
    [BUILDING.md](../../../../docs/guides/BUILDING.md#prerequisites))

```
sudo apt-get install git gcc g++ pkg-config libssl-dev libdbus-1-dev libglib2.0-dev libavahi-client-dev ninja-build python3-venv python3-dev python3-pip unzip libgirepository1.0-dev libcairo2-dev libreadline-dev
```

-   Step 1: Pull docker image

    ```bash
    $ docker pull ghcr.io/project-chip/chip-build-realtek-zephyr:181
    ```

-   Step 2: Run docker container:

    ```bash
    $ docker run -it -v ${CHIP_DIR}:/root/chip ghcr.io/project-chip/chip-build-realtek-zephyr:181
    ```

-   Step 3: Activate build environment

    ```bash
    $ source ./scripts/activate.sh
    ```

    If the script says the environment is out of date, you can update it by
    running the following command:

    ```bash
    $ source ./scripts/bootstrap.sh
    ```

## Building

-   To build the demo application:

        ```bash
        $ ./scripts/build/build_examples.py --target realtek-rtl87x2g-lighting build
        ```

    The output `zephyr.hex` file will be available in the
    `out/realtek-rtl87x2g-lighting/zephyr` directory.

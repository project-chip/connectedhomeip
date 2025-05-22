# Matter BEE4 Light Switch Example Application

This reference application implements an light switch device type. You can use
this example as a reference for creating your own application.

The example is based on:

-   [Matter](https://github.com/project-chip/connectedhomeip)
-   [BEE4 github SDK](https://github.com/rtkconnectivity/rtl87x2g_sdk)
-   [OT-REALTEK](https://github.com/rtkconnectivity/ot-realtek)

## Supported devices

-   [RTL8777G]

## Introduction

The application showcases a light switch device that communicates with clients
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

-   Step 1: checkout Realtek specific submodules only

    ```bash
    $ scripts/checkout_submodules.py --shallow --platform realtek --recursive
    ```

-   Step 1: Init REALTEK SDK

    ```bash
    $ git submodule update --init --recursive third_party/openthread/ot-realtek
    ```

-   Step 3: activate local environment

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
    $ ./scripts/build/build_examples.py --target realtek-rtl8777g-light-switch build
    ```

    You can find the target built file called **matter-cli-mtd** under the
    `out/realtek-rtl8777g-light-switch/bin` directory.

## More information

For more information on our product line and support options, please visit
[www.realmcu.com](https://www.realmcu.com/)

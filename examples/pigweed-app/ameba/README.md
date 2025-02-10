# CHIP Ameba Pigweed Example Application

This example demonstrates the usage of Pigweed module functionalities in an
application.

The example is based on [CHIP](https://github.com/project-chip/connectedhomeip),
the [Pigweed](https://pigweed.googlesource.com/pigweed/pigweed) module, which is
a collection of libraries that provide different functionalities for embedded
systems, and the Ameba platform.

You can use this example as a training ground for making experiments, testing
Pigweed module features and checking what actions are necessary to fully
integrate Pigweed in a CHIP project.

Pigweed functionalities are being gradually integrated into CHIP. Currently, the
following features are available:

-   **Echo RPC** - Creates a Remote Procedure Call server and allows sending
    commands through the serial port to the device, which makes echo and sends
    the received commands back.

---

-   [CHIP Ameba Pigweed Example Application](#chip-ameba-pigweed-example-application)
    -   [Building the Example Application](#building-the-example-application)
    -   [Testing the Example Application](#testing-the-example-application)

---

## Building the Example Application

-   Pull docker image:

          $ docker pull ghcr.io/project-chip/chip-build-ameba:112

-   Run docker container:

          $ docker run -it -v ${CHIP_DIR}:/root/chip ghcr.io/project-chip/chip-build-ameba:112

-   Setup build environment:

          $ source ./scripts/bootstrap.sh

-   To build the demo application:

          $ ./scripts/build/build_examples.py --target ameba-amebad-pigweed build

    The output image files are stored in `out/ameba-amebad-pigweed/asdk/image`
    folder.

    The bootloader image files are stored in
    `out/ameba-amebad-pigweed/asdk/bootloader` folder.

-   After building the application, **Ameba Image Tool** is used to flash it to
    Ameba board.

1. Connect your device via USB and open Ameba Image Tool.
2. Select correct serial port and set baudrate as **115200**.
3. Browse and add the corresponding image files in the Flash Download list to
   the correct locations
4. Click **Download** button.

## Testing the Example Application

Connect a USB-TTL Adapter as shown below

          Ameba         USB-TTL
          A19           TX
          A18           RX
          GND           GND

Build the
[chip-rpc console](https://github.com/project-chip/connectedhomeip/tree/master/examples/common/pigweed/rpc_console)

Reset Ameba board, then launch the chip-rpc console, where the Echo RPC commands
can be invoked

        python -m pw_hdlc.rpc_console --device /dev/tty<USB-TTL port> -b 115200

To send an Echo RPC message, type the following command, where the actual
message is the text in quotation marks after the `msg=` phrase:

        rpcs.pw.rpc.EchoService.Echo(msg="hi")

# CHIP ESP32 Pigweed Example Application

Please
[setup ESP-IDF and CHIP Environment](../../../docs/guides/esp32/setup_idf_chip.md)
and refer
[building and commissioning](../../../docs/guides/esp32/build_app_and_commission.md)
guides to get started.

---

-   [Introduction](#introduction)
-   [Testing the Example Application](#testing-the-example-application)

---

### Introduction

The ESP32 example demonstrates the usage of Pigweed module functionalities in an
application.

The example is based on [CHIP](https://github.com/project-chip/connectedhomeip),
the [Pigweed](https://pigweed.googlesource.com/pigweed/pigweed) module, which is
a collection of libraries that provide different functionalities for embedded
systems, and the ESP32 platform.

You can use this example as a training ground for making experiments, testing
Pigweed module features and checking what actions are necessary to fully
integrate Pigweed in a CHIP project.

Pigweed functionalities are being gradually integrated into CHIP. Currently, the
following features are available:

-   **Echo RPC** - Creates a Remote Procedure Call server and allows sending
    commands through the serial port to the device, which makes echo and sends
    the received commands back.

### Testing the Example Application

Run the following command to start an interactive Python shell, where the Echo
RPC commands can be invoked:

    python -m pw_hdlc.rpc_console --device /dev/tty.SLAB_USBtoUART -b 115200 $CHIP_ROOT/third_party/pigweed/repo/pw_rpc/pw_rpc_protos/echo.proto -o /tmp/pw_rpc.out

To send an Echo RPC message, type the following command, where the actual
message is the text in quotation marks after the `msg=` phrase:

    rpcs.pw.rpc.EchoService.Echo(msg="hi")

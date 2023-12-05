# STM32 Getting Started Guide

The stm32 platform uses
[Matter](https://github.com/project-chip/connectedhomeip) sdk with FreeRTOS.

---

-   [stm32 Getting Started Guide](#stm32-getting-started-guide)
    -   [Supported Chips](#supported-chips)
    -   [Matter Example Applications](#matter-example-applications)
    -   [Building the Example Application](#building-the-example-application)
    -   [Commissioning](#commissioning)
    -   [BLE-Thread mode](#ble-thread-mode)

---

## Supported Chips

The stm32 platform is supported on:

-   [STM32WB5MM-DK](https://www.st.com/en/evaluation-tools/stm32wb5mm-dk.html)

## Matter Example Applications

Sample Matter applications are provided for the stm32 platform. They can be used
to speed up development. You can find them in the samples with `/stm32`
subfolder.

## Building the Example Application

-   [Set Up Matter Environment](./BUILDING.md)

-   Set up STLINK tools

    ```
     apt-get install stlink-tools
    ```

-   Building the Application

    If the lighting example is to be built:

    ```
    ./scripts/build/build_examples.py --target stm32-$stm32_BOARD-lighting build
    ```

-   The output image files are stored in the subdirectories under `out`, the
    subdirectory name is the same as the argument specified after the option
    `--target` when build the examples.

-   After building the application, `ST-LINK` tool is used to flash it to the
    board. Before flashing the application, you will need to flash the fuse and
    the co processor binary. The co processor binary and fuse are available
    [here](https://github.com/stm32-hotspot/stm32wb-matter-device-over-thread/tree/main/Projects/STM32WB_Copro_Wireless_Binaries/STM32WB5x)
    Dynamic Concurrent Mode BLE Thread for Matter (Supports Full BLE Stack 5.2
    certified and Minimal Thread Device ready v1.3)

    ```
     sudo st-flash  write chip-stm32-lighting-example.bin 0x08000000

    ```

## Commissioning

There is one commissioning modes supported by stm32 platform:

### BLE-Thread mode

1. Build and Flash
2. The example will run automatically after booting the stm32 board.
3. Restore factory settings using B1 button by pressing at least 10 seconds
4. Commissioning with
   [Chip-Tool](https://github.com/project-chip/connectedhomeip/tree/master/examples/chip-tool),for
   example:

    ```
    ./chip-tool pairing ble-thread <node_id> <hex:xxx> <pin_code> <discriminator>
    ```

### Bluetooth LE advertising

In this example, to commission the device onto a Matter network, it must be
discoverable over Bluetooth LE. After powering up the device, the device will
advertise automatically for 30 seconds. After this delay, you will need to reset
the device to start the commissioning procedure.

### Bluetooth LE rendezvous

In Matter, the commissioning procedure is done over Bluetooth LE between a
Matter device and the Matter controller(, where the controller has the
commissioner role.

To start the rendezvous, the controller must get the commissioning information
from the Matter device. The data payload is encoded within a QR code, printed to
the UART console.

### Thread provisioning

The provisioning operation, which is the Last part of the rendezvous procedure,
involves sending the Thread network credentials from the Matter controller to
the Matter device. As a result, the device joins the Thread network via a
OpenThread border Router (OTBR) and can communicate with other devices in the
network.

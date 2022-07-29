# CHIP ESP32 Lock Example

This example demonstrates the mapping of OnOff cluster to lock/unlock logic.

---

-   [CHIP ESP32 Lock Example](#chip-esp32-lock-example)
    -   [Building the Example Application](#building-the-example-application)
    -   [Commissioning and cluster control](#commissioning-and-cluster-control)
        -   [Setting up chip-tool](#setting-up-chip-tool)
        -   [Commissioning over BLE](#commissioning-over-ble)
        -   [Cluster control](#cluster-control)
    -   [Example Demo](#example-demo)
    -   [Using the RPC Console](#using-the-rpc-console)

---

## Building the Example Application

Building the example application requires the use of the Espressif ESP32 IoT
Development Framework and the xtensa-esp32-elf toolchain.

The VSCode devcontainer has these components pre-installed, so you can skip this
step. To install these components manually, follow these steps:

-   Clone the Espressif ESP-IDF and checkout
    [v4.4.1 release](https://github.com/espressif/esp-idf/releases/tag/v4.4.1)

          $ mkdir ${HOME}/tools
          $ cd ${HOME}/tools
          $ git clone https://github.com/espressif/esp-idf.git
          $ cd esp-idf
          $ git checkout v4.4.1
          $ git submodule update --init
          $ ./install.sh

-   Install ninja-build

          $ sudo apt-get install ninja-build

Currently building in VSCode _and_ deploying from native is not supported, so
make sure the IDF_PATH has been exported(See the manual setup steps above).

-   Setting up the environment

        $ cd ${HOME}/tools/esp-idf
        $ ./install.sh
        $ . ./export.sh
        $ cd {path-to-connectedhomeip}

    To download and install packages.

        $ source ./scripts/bootstrap.sh
        $ source ./scripts/activate.sh

    If packages are already installed then simply activate them.

        $ source ./scripts/activate.sh

-   Enable Ccache for faster IDF builds

    It is recommended to have Ccache installed for faster builds

    ```
    $ export IDF_CCACHE_ENABLE=1
    ```

-   Target Set

        $ idf.py set-target esp32(or esp32c3)

-   Configuration Options

    This application uses `ESP32-DevKitC` as a default device type. To use other
    ESP32 based device types, please refer
    [examples/all-clusters-app/esp32](https://github.com/project-chip/connectedhomeip/tree/master/examples/all-clusters-app/esp32)

-   To build the demo application.

          $ idf.py build

-   After building the application, to flash it outside of VSCode, connect your
    device via USB. Then run the following command to flash the demo application
    onto the device and then monitor its output. If necessary, replace
    `/dev/tty.SLAB_USBtoUART`(MacOS) with the correct USB device name for your
    system(like `/dev/ttyUSB0` on Linux). Note that sometimes you might have to
    press and hold the `boot` button on the device while it's trying to connect
    before flashing. For ESP32-DevKitC devices this is labeled in the
    [functional description diagram](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/hw-reference/esp32/get-started-devkitc.html#functional-description).

         $ idf.py -p /dev/tty.SLAB_USBtoUART flash monitor

    Note: Some users might have to install the
    [VCP driver](https://www.silabs.com/products/development-tools/software/usb-to-uart-bridge-vcp-drivers)
    before the device shows up on `/dev/tty`.

-   Quit the monitor by hitting `Ctrl+]`.

    Note: You can see a menu of various monitor commands by hitting
    `Ctrl+t Ctrl+h` while the monitor is running.

-   If desired, the monitor can be run again like so:

          $ idf.py -p /dev/tty.SLAB_USBtoUART monitor

## Commissioning and cluster control

Commissioning can be carried out using WiFi or BLE.

1.  Set the `Rendezvous Mode` for commissioning using menuconfig; the default
    Rendezvous mode is BLE.

         $ idf.py menuconfig

Select the Rendezvous Mode via `Demo -> Rendezvous Mode`.

2.  Now flash the device with the same command as before. (Use the right `/dev`
    device)

          $ idf.py -p /dev/tty.SLAB_USBtoUART flash monitor

3.  The device should boot up. When device connects to your network, you will
    see a log like this on the device console.

          I (5524) chip[DL]: SYSTEM_EVENT_STA_GOT_IP
          I (5524) chip[DL]: IPv4 address changed on WiFi station interface: <IP_ADDRESS>...

4.  Use
    [python based device controller](https://github.com/project-chip/connectedhomeip/tree/master/src/controller/python)
    or
    [standalone chip-tool](https://github.com/project-chip/connectedhomeip/tree/master/examples/chip-tool)
    or
    [iOS chip-tool app](https://github.com/project-chip/connectedhomeip/tree/master/src/darwin/CHIPTool)
    or
    [Android chip-tool app](https://github.com/project-chip/connectedhomeip/tree/master/src/android/CHIPTool)
    to communicate with the device.

Note: The ESP32 does not support 5GHz networks. Also, the Device will persist
your network configuration. To erase it, simply run.

    $ idf.py -p /dev/tty.SLAB_USBtoUART erase_flash

-   Once ESP32 is up and running, we need to set up a device controller to
    perform commissioning and cluster control.

### Setting up chip-tool

See [the build guide](../../../docs/guides/BUILDING.md#prerequisites) for
general background on build prerequisites.

Building the example:

```
$ cd examples/chip-tool

$ rm -rf out

$ gn gen out/debug

$ ninja -C out/debug
```

which puts the binary at `out/debug/chip-tool`

### Commission a device using chip-tool

To initiate a client commissioning request to a device, run the built executable
and choose the pairing mode.

#### Commissioning over BLE

Run the built executable and pass it the discriminator and pairing code of the
remote device, as well as the network credentials to use.

The command below uses the default values hard-coded into the debug versions of
the ESP32 all-clusters-app to commission it onto a Wi-Fi network:

    $ ./out/debug/chip-tool pairing ble-wifi 12344321 ${SSID} ${PASSWORD} 20202021 3840

Parameters:

1. Discriminator: 3840
2. Setup-pin-code: 20202021
3. Node-id: 12344321 (you can assign any node id)

### Cluster control

#### onoff

To use the Client to send Matter commands, run the built executable and pass it
the target cluster name, the target command name as well as an endpoint id.

    $ ./out/debug/chip-tool onoff on 12344321 1

The client will send a single command packet and then exit.

## Example Demo

This demo app illustrates controlling OnOff cluster (Server) attributes of an
endpoint and lock/unlock status of door using LED's. For `ESP32-DevKitC`, a GPIO
(configurable through `LOCK_STATE_LED` in `main/include/AppConfig.h`) is updated
through the on/off/toggle commands from the `python-controller`. If you wish to
see the actual effect of the commands on `ESP32-DevKitC`, you will have to
connect an external LED to GPIO.

## Using the RPC console

Enable RPCs in the build using menuconfig:

    $ idf.py menuconfig

Enable the RPC library:

    Component config → CHIP Core → General Options → Enable Pigweed PRC library

After flashing a build with RPCs enabled you can use the rpc console to send
commands to the device.

Build or install the [rpc console](../../common/pigweed/rpc_console/README.md)

Start the console

    chip-console --device /dev/ttyUSB0

From within the console you can then invoke rpcs:

    rpcs.chip.rpc.Locking.Get()
    rpcs.chip.rpc.Locking.Set(locked=True)

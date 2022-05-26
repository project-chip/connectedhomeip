# CHIP ESP32 All Clusters Example

A prototype application that demonstrates device commissioning and cluster
control.

---

-   [CHIP ESP32 All Clusters Example](#chip-esp32-all-clusters-example)
    -   [Supported Devices](#supported-devices)
    -   [Building the Example Application](#building-the-example-application)
    -   [Commissioning and cluster control](#commissioning-and-cluster-control)
        -   [Setting up chip-tool](#setting-up-chip-tool)
        -   [Commissioning over BLE](#commissioning-over-ble)
        -   [Cluster control](#cluster-control)
        -   [Flashing app using script](#flashing-app-using-script)
        -   [Note](#note)
    -   [Using the RPC console](#using-the-rpc-console)
    -   [Device Tracing](#device-tracing)

---

## Supported Devices

The CHIP demo application is intended to work on three categories of ESP32
devices: the
[ESP32-DevKitC](https://www.espressif.com/en/products/hardware/esp32-devkitc/overview),
the
[ESP32-WROVER-KIT_V4.1](https://www.espressif.com/en/products/hardware/esp-wrover-kit/overview),
the [M5Stack](http://m5stack.com), and the
[ESP32C3-DevKitM](https://docs.espressif.com/projects/esp-idf/en/latest/esp32c3/hw-reference/esp32c3/user-guide-devkitm-1.html).

Note: M5Stack Core 2 display is not supported in the tft component, while other
functionality can still work fine.

## VCP Drivers

Some users might have to install the
[VCP driver](https://www.silabs.com/products/development-tools/software/usb-to-uart-bridge-vcp-drivers)
before the device shows up on `/dev/tty`.

In addition, if the following error is encountered during M5Stack flashing, the
[CH9102 VCP driver](https://docs.m5stack.com/en/download) would also need to be
installed:

```
Failed to write to target RAM (result was 01070000)
```

## Building the Example Application

Building the example application requires the use of the Espressif ESP32 IoT
Development Framework and the xtensa-esp32-elf toolchain for ESP32 modules or
the riscv-esp32-elf toolchain for ESP32C3 modules.

The VSCode devcontainer has these components pre-installed, so you can skip this
step. To install these components manually, follow these steps:

-   Clone the Espressif ESP-IDF and checkout
    [v4.4.1 release](https://github.com/espressif/esp-idf/releases/tag/v4.4.1)

          ```
          $ mkdir ${HOME}/tools
          $ cd ${HOME}/tools
          $ git clone https://github.com/espressif/esp-idf.git
          $ cd esp-idf
          $ git checkout v4.4.1
          $ git submodule update --init
          $ ./install.sh
          $ . ./export.sh
          ```

    To update an existing esp-idf toolchain to v4.4.1:

          ```
          $ cd ~/tools/esp-idf
          $ git fetch origin
          $ git checkout v4.4.1
          $ git reset --hard origin/v4.4.1
          $ git submodule update --init
          $ git clean -fdx
          $ ./install.sh
          $ . ./export.sh
          ```

-   Install ninja-build

          ```
          $ sudo apt-get install ninja-build
          ```

Currently building in VSCode _and_ deploying from native is not supported, so
make sure the IDF_PATH has been exported(See the manual setup steps above).

-   Setting up the environment

        ```
        $ cd ${HOME}/tools/esp-idf
        $ ./install.sh
        $ . ./export.sh
        $ cd {path-to-connectedhomeip}
        ```

    To download and install packages.

        ```
        $ source ./scripts/bootstrap.sh
        $ source ./scripts/activate.sh
        ```

    If packages are already installed then simply activate them.

        ```
        $ source ./scripts/activate.sh
        ```

-   Target Set

To set IDF target, first:

        ```
        $ cd {path-to-connectedhomeip}/examples/all-clusters-minimal-app/esp32/
        ```

Then run set-target with one of the commands.

        ```
        $ idf.py set-target esp32
        $ idf.py set-target esp32c3
        ```

-   Configuration Options

To build the default configuration (`sdkconfig.defaults`) skip to building the
demo application.

To build a specific configuration (as an example `m5stack`):

          ```
          $ rm sdkconfig
          $ idf.py -D 'SDKCONFIG_DEFAULTS=sdkconfig_m5stack.defaults' build
          ```

    Note: If using a specific device configuration, it is highly recommended to
    start off with one of the defaults and customize on top of that. Certain
    configurations have different constraints that are customized within the
    device specific configuration (eg: main app stack size).

To customize the configuration, run menuconfig.

          ```
          $ idf.py menuconfig
          ```

Select ESP32 based `Device Type` through `Demo`->`Device Type`. The device types
that are currently supported include `ESP32-DevKitC` (default),
`ESP32-WROVER-KIT_V4.1`, `M5Stack` and `ESP32C3-DevKitM`.

-   To build the demo application.

          ```
          $ idf.py build
          ```

-   After building the application, to flash it outside of VSCode, connect your
    device via USB. Then run the following command to flash the demo application
    onto the device and then monitor its output. If necessary, replace
    `/dev/tty.SLAB_USBtoUART` with the correct USB device name for your system
    (like `/dev/ttyUSB0` on Linux or `/dev/tty.usbserial-01CDEEDC` on Mac). Note
    that sometimes you might have to press and hold the `boot` button on the
    device while it's trying to connect before flashing. For ESP32-DevKitC
    devices this is labeled in the
    [functional description diagram](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/hw-reference/esp32/get-started-devkitc.html#functional-description).

          ```
          $ idf.py -p /dev/tty.SLAB_USBtoUART flash monitor
          ```

-   Quit the monitor by hitting `Ctrl+]`.

    Note: You can see a menu of various monitor commands by hitting
    `Ctrl+t Ctrl+h` while the monitor is running.

-   If desired, the monitor can be run again like so:

          ```
          $ idf.py -p /dev/tty.SLAB_USBtoUART monitor
          ```

## Commissioning and cluster control

Commissioning can be carried out using WiFi or BLE.

1.  Set the `Rendezvous Mode` for commissioning using menuconfig; the default
    Rendezvous mode is BLE.

          ```
          $ idf.py menuconfig
          ```

Select the Rendezvous Mode via `Demo -> Rendezvous Mode`.

NOTE: to avoid build error
`undefined reference to 'chip::DevelopmentCerts::kDacPublicKey'`, set VID to
`0xFFF1` and PID in range `0x8000..0x8005`.

`idf.py menuconfig -> Component config -> CHIP Device Layer -> Device Identification Options`

2.  Now flash the device with the same command as before. (Use the right `/dev`
    device)

          ```
          $ idf.py -p /dev/tty.SLAB_USBtoUART flash monitor
          ```

3.  The device should boot up. When device connects to your network, you will
    see a log like this on the device console.

          ```
          I (5524) chip[DL]: SYSTEM_EVENT_STA_GOT_IP
          I (5524) chip[DL]: IPv4 address changed on WiFi station interface: <IP_ADDRESS>...
          ```

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

          ```
          $ idf.py -p /dev/tty.SLAB_USBtoUART erase_flash
          ```

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
the ESP32 all-clusters-minimal-app to commission it onto a Wi-Fi network:

    ```
    $ ./out/debug/chip-tool pairing ble-wifi 12344321 ${SSID} ${PASSWORD} 20202021 3840
    ```

Parameters:

1. Discriminator: 3840 (configurable through menuconfig)
2. Setup-pin-code: 20202021 (configurable through menuconfig)
3. Node-id: 12344321 (you can assign any node id)

### Cluster control

#### onoff

To use the Client to send Matter commands, run the built executable and pass it
the target cluster name, the target command name as well as an endpoint id.

    ```
    $ ./out/debug/chip-tool onoff on 12344321 1
    ```

The client will send a single command packet and then exit.

#### levelcontrol

```bash
Usage:
  $ ./out/debug/chip-tool levelcontrol move-to-level Level=10 TransitionTime=0 OptionMask=0 OptionOverride=0  12344321 1
```

### Flashing app using script

-   Follow these steps to use `${app_name}.flash.py`.

    -   First set IDF target, run set-target with one of the commands.

            ```
            $ idf.py set-target esp32
            $ idf.py set-target esp32c3
            ```

    -   Execute below sequence of commands

        ```
        $ export ESPPORT=/dev/tty.SLAB_USBtoUART
        $ idf.py build
        $ idf.py flashing_script
        $ python ${app_name}.flash.py
        ```

### Note

This demo app illustrates controlling OnOff cluster (Server) attributes of an
endpoint. For `ESP32-DevKitC`, `ESP32-WROVER-KIT_V4.1` and `ESP32C3-DevKitM`, a
GPIO (configurable through `STATUS_LED_GPIO_NUM` in `main/main.cpp`) is updated
through the on/off/toggle commands from the `python-controller`. For `M5Stack`,
a virtual Green LED on the display is used for the same.

If you wish to see the actual effect of the commands on `ESP32-DevKitC`,
`ESP32-WROVER-KIT_V4.1`, you will have to connect an external LED to GPIO
`STATUS_LED_GPIO_NUM`. For `ESP32C3-DevKitM`, the on-board LED will show the
actual effect of the commands.

## Using the RPC console

You can use the rpc default config to setup everything correctly for RPCs:

    ```
    $ export SDKCONFIG_DEFAULTS=$PROJECT_ROOT/examples/all-clusters-minimal-app/esp32/sdkconfig_m5stack_rpc.defaults
    $ rm sdkconfig
    $ idf.py fullclean
    ```

Alternatively, Enable RPCs in the build using menuconfig:

    - Enable the RPC library and Disable ENABLE_CHIP_SHELL

        ```
        Component config → CHIP Core → General Options → Enable Pigweed PRC library
        Component config → CHIP Core → General Options → Disable CHIP Shell
        ```

    - Ensure the UART is correctly configured for your board, for m5stack:

        ```
        PW RPC Debug channel → UART port number → 0
        PW RPC Debug channel → UART communication speed → 115200
        PW RPC Debug channel → UART RXD pin number → 3
        PW RPC Debug channel → UART TXD pin number → 1
        ```

After configuring you can build and flash normally:

    ```
    $ idf.py build
    $ idf.py flash
    ```

After flashing a build with RPCs enabled you can use the rpc console to send
commands to the device.

Build or install the [rpc console](../../common/pigweed/rpc_console/README.md)

Start the console

    ```
    chip-console --device /dev/ttyUSB0
    ```

From within the console you can then invoke rpcs:

    ```python
    rpcs.chip.rpc.WiFi.Connect(ssid=b"MySSID", secret=b"MyPASSWORD")
    rpcs.chip.rpc.WiFi.GetIP6Address()

    rpcs.chip.rpc.Lighting.Get()
    rpcs.chip.rpc.Lighting.Set(on=True, level=128, color=protos.chip.rpc.LightingColor(hue=5, saturation=5))
    ```

## Device Tracing

Device tracing is available to analyze the device performance. To turn on
tracing, build with RPC enabled. See
[Using the RPC console](#using-the-rpc-console).

Obtain tracing json file.

```
    $ ./{PIGWEED_REPO}/pw_trace_tokenized/py/pw_trace_tokenized/get_trace.py -d {PORT} -o {OUTPUT_FILE} \
    -t {ELF_FILE} {PIGWEED_REPO}/pw_trace_tokenized/pw_trace_protos/trace_rpc.proto
```

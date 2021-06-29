# CHIP ESP32 All Clusters Example

A prototype application that demonstrates device commissioning and cluster
control.

---

-   [CHIP ESP32 All Clusters Example](#chip-esp32-all-clusters-example)
    -   [Supported Devices](#supported-devices)
    -   [Building the Example Application](#building-the-example-application)
    -   [Commissioning and cluster control](#commissioning-and-cluster-control)
        -   [Setting up Python Controller](#setting-up-python-controller)
        -   [Commissioning over BLE](#commissioning-over-ble)
        -   [Cluster control](#cluster-control)
    -   [Flashing app using script](#flashing-app-using-script)
    -   [Note](#note)

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

## Building the Example Application

Building the example application requires the use of the Espressif ESP32 IoT
Development Framework and the xtensa-esp32-elf toolchain for ESP32 modules or
the riscv-esp32-elf toolchain for ESP32C3 modules.

The VSCode devcontainer has these components pre-installed, so you can skip this
step. To install these components manually, follow these steps:

-   Clone the Espressif ESP-IDF and checkout
    [v4.3 tag](https://github.com/espressif/esp-idf/releases/v4.3)

          $ mkdir ${HOME}/tools
          $ cd ${HOME}/tools
          $ git clone https://github.com/espressif/esp-idf.git
          $ cd esp-idf
          $ git checkout v4.3
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

-   Target Set

To set IDF target, run set-target with one of the commands.

          $ idf.py set-target esp32
          $ idf.py set-target esp32c3

-   Configuration Options

To choose from the different configuration options, run menuconfig.

          $ idf.py menuconfig

Select ESP32 based `Device Type` through `Demo`->`Device Type`. The device types
that are currently supported include `ESP32-DevKitC` (default),
`ESP32-WROVER-KIT_V4.1`, `M5Stack` and `ESP32C3-DevKitM`.

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

Commissioning can be carried out using WiFi, BLE or Bypass.

1.  Set the `Rendezvous Mode` for commissioning using menuconfig; the default
    Rendezvous mode is BLE.

         $ idf.py menuconfig

Select the Rendezvous Mode via `Demo -> Rendezvous Mode`. If Rendezvous Mode is
ByPass then set the credentials of the WiFi Network (i.e. SSID and Password from
menuconfig).

`idf.py menuconfig -> Component config -> CHIP Device Layer -> WiFi Station Options`

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

### Setting up Python Controller

Once ESP32 is up and running, we need to set up a device controller to perform
commissioning and cluster control.

-   Set up python controller.

           $ cd {path-to-connectedhomeip}
           $ ./scripts/build_python.sh -m platform

-   Execute the controller.

           $ source ./out/python_env/bin/activate
           $ chip-device-ctrl

### Commissioning over BLE

-   Establish the secure session over BLE. BLE is the default mode in the
    application and is configurable through menuconfig.

         - chip-device-ctrl > ble-scan
         - chip-device-ctrl > connect -ble 3840 20202021 135246

         Parameters:
         1. Discriminator: 3840 (configurable through menuconfig)
         2. Setup-pin-code: 20202021 (configurable through menuconfig)
         3. Node ID: Optional.
            If not passed in this command, then it is auto-generated by the controller and displayed in the output of connect.
            The same value should be used in the next commands.
            We have chosen a random node ID which is 135246.

-   Add credentials of the Wi-Fi network you want the ESP32 to connect to, using
    the `AddWiFiNetwork` command and then enable the ESP32 to connect to it
    using `EnableWiFiNetwork` command. In this example, we have used `TESTSSID`
    and `TESTPASSWD` as the SSID and passphrase respectively.

         - chip-device-ctrl > zcl NetworkCommissioning AddWiFiNetwork 135246 0 0 ssid=str:TESTSSID credentials=str:TESTPASSWD breadcrumb=0 timeoutMs=1000

         - chip-device-ctrl > zcl NetworkCommissioning EnableNetwork 135246 0 0 networkID=str:TESTSSID breadcrumb=0 timeoutMs=1000

-   Close the BLE connection to ESP32, as it is not required hereafter.

         - chip-device-ctrl > close-ble

-   Resolve DNS-SD name and update address of the node in the device controller.

         - chip-device-ctrl > resolve 0 135246

### Cluster control

-   After successful commissioning, use the OnOff cluster command to control the
    OnOff attribute. This allows you to toggle a parameter implemented by the
    device to be On or Off.

    `chip-device-ctrl > zcl OnOff Off 135246 1 0`

### Flashing app using script

-   Follow these steps to use `${app_name}.flash.py`.

    -   First set IDF target, run set-target with one of the commands.

            $ idf.py set-target esp32
            $ idf.py set-target esp32c3

    -   Execute below sequence of commands

```
        $ export ESPPORT=/dev/tty.SLAB_USBtoUART
        $ export ESPBAUD=${baud_value}
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
`ESP32-WROVER-KIT_V4.1` and `ESP32C3-DevKitM`, you will have to connect an
external LED to GPIO `STATUS_LED_GPIO_NUM`.

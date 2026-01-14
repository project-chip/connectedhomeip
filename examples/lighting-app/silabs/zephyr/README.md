# Matter SiWx917 Lighting Example

An example showing the use of CHIP on the Silicon Labs SiWx917 WiFi SoC running
Zephyr RTOS.

<hr>

-   [Matter SiWx917 Lighting Example](#matter-siwx917-lighting-example)
    -   [Introduction](#introduction)
    -   [Building](#building)
    -   [Flashing the Application](#flashing-the-application)
    -   [Running the Complete Example](#running-the-complete-example)
        -   [Commissioning over BLE](#commissioning-over-ble)
        -   [Testing with chip-tool](#testing-with-chip-tool)
    -   [Building Options](#building-options)
        -   [Debug build / release build](#debug-build--release-build)
        -   [Disabling logging](#disabling-logging)
        -   [KVS maximum entry count](#kvs-maximum-entry-count)
    -   [OTA Software Update](#ota-software-update)
    -   [Limitations](#limitations)

<hr>

## Introduction

The SiWx917 lighting example provides a baseline demonstration of a Light
control device, built using Matter and the Silicon Labs SiWx917 WiFi SoC running
Zephyr RTOS. It can be controlled by a Matter controller over a WiFi network.

The SiWx917 device can be commissioned over Bluetooth Low Energy where the
device and the Matter controller will exchange security information with the
Rendez-vous procedure. WiFi Network credentials are then provided to the SiWx917
device which will then join the WiFi network.

This example implements a software-only lighting device without GPIO control,
suitable for testing Matter protocol functionality and WiFi connectivity on the
SiWx917 platform.

The lighting example is intended to serve both as a means to explore the
workings of Matter as well as a template for creating real products based on the
Silicon Labs SiWx917 platform.

### Configuring the application

You can configure various aspects of the application using Zephyr's menuconfig
system. This provides an interactive interface to modify build-time
configuration options such as logging levels, networking settings, memory
allocation, and Matter-specific features.

```bash
west build -b siwx917_rb4338a examples/lighting-app/silabs/zephyr -t menuconfig
```

This will open an interactive configuration menu where you can browse and modify
settings. Changes are automatically saved to the build configuration.

## Building

-   Install the required dependencies for Matter development:

    -   [Matter Prerequisites](../../../../docs/guides/BUILDING.md)

-   Install the Silicon Labs Zephyr SDK and tools:

    -   Follow the
        [Silicon Labs Zephyr Repo](https://github.com/SiliconLabsSoftware/zephyr-silabs)

-   Supported hardware:

    -   **SiWx917 SoC boards:**
        -   BRD4338A (SiWx917 Radio Board with 2.4GHz WiFi)

-   Build the example application:

    ```bash
    cd silabs_zephyr
    west build -b siwx917_rb4338a connectedhomeip/examples/lighting-app/silabs/zephyr
    ```

-   To clean the build:

    ```bash
    west build -t clean
    ```

-   For a pristine build (removes all build artifacts):

    ```bash
    rm -rf build
    ```

    OR

    ```bash
    west build -p always  -b siwx917_rb4338a connectedhomeip/examples/lighting-app/silabs/zephyr
    ```

## Flashing the Application

-   **SiWx917 SoC device support is available in the latest Simplicity
    Commander.** The SiWx917 SOC board will support `.hex` files for flashing.

-   Flash using west:

    ```bash
    west flash
    ```

-   Or flash using Simplicity Commander:

    ```bash
    commander flash build/zephyr/zephyr.hex
    ```

-   **Bootloader and Connectivity Firmware:** All SiWx917 boards require
    connectivity firmware. See Silicon Labs documentation for more info.
    Pre-built bootloader binaries are available in the Assets section of the
    Releases page on
    [Wiseconnect](https://github.com/SiliconLabs/wiseconnect/tree/v3.5.0/connectivity_firmware/standard).

## Running the Complete Example

### Commissioning over BLE

To run a Matter over WiFi application, you must first commission the device
using a Matter controller over BLE, then provide WiFi credentials.

**Creating the Matter Network**

This procedure uses the chip-tool installed on the Matter Hub. The commissioning
procedure does the following:

-   Chip-tool scans BLE and locates the Silicon Labs device that uses the
    specified discriminator
-   Establishes operational certificates
-   Sends the WiFi SSID and Passkey
-   The Silicon Labs device will join the WiFi network and get an IP address
-   It then starts providing mDNS records on IPv4 and IPv6
-   Future communications (tests) will then happen over WiFi

### Testing with chip-tool

You can provision and control the CHIP device using the python controller, Chip
tool standalone, Android or iOS app.

[CHIPTool](https://github.com/project-chip/connectedhomeip/blob/master/examples/chip-tool/README.md)

Here is an example with the chip-tool:

#### Commission the device over BLE-WiFi:

```bash
./out/linux-x64-chip-tool/chip-tool pairing ble-wifi ${NODE_ID} ${SSID} ${PASSWORD} 20202021 3840
```

#### Control the device (turn on/off):

```bash
# Turn on the light
./out/linux-x64-chip-tool/chip-tool onoff on ${NODE_ID} 1

# Turn off the light
./out/linux-x64-chip-tool/chip-tool onoff off ${NODE_ID} 1

# Read the on/off state
./out/linux-x64-chip-tool/chip-tool onoff read on-off ${NODE_ID} 1
```

#### Level control (if supported):

```bash
# Set brightness to 50%
./out/linux-x64-chip-tool/chip-tool levelcontrol move-to-level 128 1 0 0 ${NODE_ID} 1

# Read current level
./out/linux-x64-chip-tool/chip-tool levelcontrol read current-level ${NODE_ID} 1
```

Where:

-   `${NODE_ID}` is the node ID assigned to the device
-   `20202021` is the setup PIN code
-   `3840` is the discriminator
-   `${SSID}` and `${PASSWORD}` are your WiFi network credentials

## Building Options

### Debug build / release build

By default, the application is built in debug mode. To build in release mode:

```bash
west build -b siwx917_rb4338a examples/lighting-app/silabs/zephyr -- -DFILE_SUFFIX=release
```

### Disabling logging

To reduce code size and improve performance, logging can be minimized:

```bash
west build -b siwx917_rb4338a examples/lighting-app/silabs/zephyr -- -DCONFIG_LOG_MODE_MINIMAL=y
```

### KVS maximum entry count

The Key-Value Storage (KVS) maximum entry count can be configured to optimize
memory usage. The default configuration is optimized for typical Matter
applications.

To modify the KVS configuration, edit the `prj.conf` file:

```
CONFIG_NVS_LOOKUP_CACHE_SIZE=1024
```

## OTA Software Update

The SiWx917 platform supports Over-The-Air (OTA) software updates. To enable
OTA:

1. Build with OTA support:

    ```bash
    west build -b siwx917_rb4338a examples/lighting-app/silabs/zephyr -- -DCONFIG_CHIP_OTA_REQUESTOR=y
    ```

2. The device will advertise OTA capability and can receive firmware updates
   from a Matter OTA Provider.

3. Refer to the
   [Matter OTA guide](../../../../docs/guides/ota_software_update.md) for
   detailed instructions on setting up an OTA Provider.

## Limitations

This example has the following limitations specific to the SiWx917 platform:

1. **No GPIO Support:** This implementation does not include GPIO control for
   physical LEDs or buttons. All lighting operations are software-only.

2. **WiFi Only:** The SiWx917 supports only WiFi connectivity. Thread and
   Ethernet are not supported on this platform.

3. **Single Radio:** The device operates as a WiFi station only. WiFi AP mode is
   not supported in this configuration.

4. **Memory Constraints:** The SiWx917 has limited RAM and flash memory. Complex
   applications may need memory optimization.

5. **Power Management:** Advanced power management features are limited compared
   to other Silicon Labs platforms.

For production applications requiring GPIO control, consider using the EFR32
platform with the standard Silicon Labs lighting example.

---

### Additional Resources:

-   [Silicon Labs Matter Documentation](https://github.com/SiliconLabsSoftware/matter_sdk)
-   [SiWx917 Development Kit User Guide](https://www.silabs.com/development-tools/wireless/wi-fi/siwx917-development-kit)
-   [Matter Specification](https://csa-iot.org/all-solutions/matter/)
-   [Zephyr RTOS Documentation](https://docs.zephyrproject.org/)

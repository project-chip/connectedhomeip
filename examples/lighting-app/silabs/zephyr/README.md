# Matter Silicon Labs Lighting Example (Zephyr)

An example showing the use of Matter on Silicon Labs SoCs running Zephyr RTOS.
The same application can be built for either:

-   **Wi-Fi** transport on the **SiWx917** Wi-Fi SoC (board `siwx917_rb4338a`).
-   **Thread (OpenThread)** transport on the **EFR32MG24** SoC (board
    `xg24_rb4187c`).

<hr>

-   [Matter Silicon Labs Lighting Example (Zephyr)](#matter-silicon-labs-lighting-example-zephyr)
    -   [Introduction](#introduction)
    -   [Supported Hardware](#supported-hardware)
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

The Silicon Labs lighting example provides a baseline demonstration of a Light
control device, built using Matter on Silicon Labs SoCs running Zephyr RTOS.
Depending on the selected board, it can be controlled by a Matter controller
over either a Wi-Fi network (SiWx917) or a Thread network (EFR32MG24).

In both cases, the device is commissioned over Bluetooth Low Energy: the device
and the Matter controller exchange security information through the Rendez-vous
procedure, and the controller then provisions the operational network
credentials:

-   On **SiWx917 (Wi-Fi)**, Wi-Fi SSID and passphrase are sent and the device
    joins the Wi-Fi network.
-   On **EFR32MG24 (Thread)**, the Thread operational dataset is sent and the
    device joins the Thread network.

This example implements a software-only lighting device without GPIO control,
suitable for testing Matter protocol functionality and network connectivity on
the supported Silicon Labs platforms.

The lighting example is intended to serve both as a means to explore the
workings of Matter as well as a template for creating real products based on
Silicon Labs platforms.

### Configuring the application

You can configure various aspects of the application using Zephyr's menuconfig
system. This provides an interactive interface to modify build-time
configuration options such as logging levels, networking settings, memory
allocation, and Matter-specific features.

```bash
# Wi-Fi (SiWx917)
west build -b siwx917_rb4338a examples/lighting-app/silabs/zephyr -t menuconfig

# Thread (EFR32MG24)
west build -b xg24_rb4187c examples/lighting-app/silabs/zephyr -t menuconfig
```

This will open an interactive configuration menu where you can browse and modify
settings. Changes are automatically saved to the build configuration.

## Supported Hardware

The example can be built for any of the following Silicon Labs boards:

| Board             | SoC       | Transport           |
| ----------------- | --------- | ------------------- |
| `siwx917_rb4338a` | SiWx917   | Wi-Fi (2.4 GHz)     |
| `xg24_rb4187c`    | EFR32MG24 | Thread (OpenThread) |

The board name selected with `west build -b <board>` determines both the target
SoC and the network transport used by Matter.

## Building

-   Install the required dependencies for Matter development:

    -   [Matter Prerequisites](../../../../docs/guides/BUILDING.md)

-   Install the Silicon Labs Zephyr SDK and tools:

    -   Follow the
        [Silicon Labs Zephyr Repo](https://github.com/SiliconLabsSoftware/zephyr-silabs)

-   Build the example application for your target:

    ```bash
    cd silabs_zephyr

    # Wi-Fi build (SiWx917)
    west build -b siwx917_rb4338a connectedhomeip/examples/lighting-app/silabs/zephyr

    # Thread build (EFR32MG24)
    west build -b xg24_rb4187c connectedhomeip/examples/lighting-app/silabs/zephyr
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
    west build -p always -b <board> connectedhomeip/examples/lighting-app/silabs/zephyr
    ```

    where `<board>` is `siwx917_rb4338a` or `xg24_rb4187c`.

## Flashing the Application

-   Both SiWx917 and EFR32MG24 SoC devices are supported by the latest
    Simplicity Commander. The build produces a `.hex` file that can be flashed
    to either platform.

-   Flash using west:

    ```bash
    west flash
    ```

-   Or flash using Simplicity Commander:

    ```bash
    commander flash build/zephyr/zephyr.hex
    ```

-   **Bootloader and connectivity firmware:**

    -   The **SiWx917** boards require connectivity firmware in addition to the
        application image. Pre-built bootloader and connectivity firmware
        binaries are available in the Assets section of the Releases page on
        [Wiseconnect](https://github.com/SiliconLabs/wiseconnect/tree/v3.5.0/connectivity_firmware/standard).
    -   The **EFR32MG24** boards require a Gecko bootloader. Refer to the
        Silicon Labs documentation for the appropriate bootloader image for your
        board.

## Running the Complete Example

### Commissioning over BLE

To run the Matter application, you must first commission the device using a
Matter controller over BLE. The controller will then provision the operational
network credentials:

-   On **SiWx917**, Wi-Fi SSID and passphrase.
-   On **EFR32MG24**, the Thread operational dataset.

**Creating the Matter Network**

This procedure uses the chip-tool installed on the Matter Hub. The commissioning
procedure does the following:

-   chip-tool scans BLE and locates the Silicon Labs device that uses the
    specified discriminator
-   Establishes operational certificates
-   Sends the operational network credentials (Wi-Fi credentials or Thread
    dataset, depending on the platform)
-   The Silicon Labs device joins the operational network and obtains an IPv6
    (and IPv4 for Wi-Fi) address
-   It then starts providing mDNS records on the operational network
-   Future communications happen over the operational network (Wi-Fi or Thread)

### Testing with chip-tool

You can provision and control the CHIP device using the python controller, the
chip-tool standalone, or the Android or iOS app.

[CHIPTool](https://github.com/project-chip/connectedhomeip/blob/master/examples/chip-tool/README.md)

Here are examples using chip-tool:

#### Commission the device over BLE-Wi-Fi (SiWx917):

```bash
./out/linux-x64-chip-tool/chip-tool pairing ble-wifi ${NODE_ID} ${SSID} ${PASSWORD} 20202021 3840
```

#### Commission the device over BLE-Thread (EFR32MG24):

```bash
./out/linux-x64-chip-tool/chip-tool pairing ble-thread ${NODE_ID} hex:${THREAD_DATASET} 20202021 3840
```

`${THREAD_DATASET}` is the active Thread operational dataset in TLV hexadecimal
format (for example as returned by `ot-ctl dataset active -x` on an OpenThread
border router).

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
-   `${SSID}` and `${PASSWORD}` are your Wi-Fi network credentials (SiWx917)
-   `${THREAD_DATASET}` is the Thread operational dataset (EFR32MG24)

## Building Options

The build options below apply to both supported boards. Replace `<board>` with
either `siwx917_rb4338a` or `xg24_rb4187c`.

### Debug build / release build

By default, the application is built in debug mode. To build in release mode:

```bash
west build -b <board> examples/lighting-app/silabs/zephyr -- -DFILE_SUFFIX=release
```

### Disabling logging

To reduce code size and improve performance, logging can be minimized:

```bash
west build -b <board> examples/lighting-app/silabs/zephyr -- -DCONFIG_LOG_MODE_MINIMAL=y
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

This example supports Matter Over-The-Air (OTA) software updates via the generic
Zephyr image processor (`src/platform/Zephyr/OTAImageProcessorImpl`) and
MCUboot.

Enabling `CONFIG_CHIP_OTA_REQUESTOR` implies `CONFIG_BOOTLOADER_MCUBOOT`.

The app is signed with the MCUboot ECDSA-P256 development key
(`CONFIG_MCUBOOT_SIGNATURE_KEY_FILE`, set in `config/silabs/Kconfig`); replace
it with a private key for production.

### Build

```bash
west build -b xg24_rb4187c -p always examples/lighting-app/silabs/zephyr \
    -- -DCONFIG_CHIP_OTA_REQUESTOR=y -DCONFIG_CHIP_OTA_IMAGE_BUILD=y
```

Build the update image with a higher software version so the provider offers it
as newer version:

```bash
west build -b xg24_rb4187c -p always examples/lighting-app/silabs/zephyr \
    -- -DCONFIG_CHIP_OTA_REQUESTOR=y -DCONFIG_CHIP_OTA_IMAGE_BUILD=y \
       -DCONFIG_CHIP_DEVICE_SOFTWARE_VERSION=2 \
       -DCONFIG_CHIP_DEVICE_SOFTWARE_VERSION_STRING=\"2.0\"
```

Outputs in `build/zephyr/` (produced by
`config/silabs/app/zephyr-post-build.cmake`):

-   `zephyr_full.bin` — MCUboot + signed application; flash this for the initial
    (factory) image.
-   `matter.ota` — the image to serve from a Matter OTA Provider.

### Flash the base image

Use the following commander command to flash the app and bootloader.

```bash
commander flash ./build/zephyr/zephyr_full.bin --address 0x8000000
```

### Run an update

The following commands are provided as examples using a
[Silicon Labs Matter Hub image.](https://docs.silabs.com/matter/latest/matter-thread/raspi-img)
Refer to the [Matter OTA guide](../../../../docs/guides/ota_software_update.md)
for detailed instructions on setting up an OTA Provider.

1. Flash the base (v1) image and commission the device.
2. Serve the candidate (v2) image with `chip-ota-provider-app`

```bash
./chip-ota-provider-app \
    --KVS /tmp/chip_kvs_provider \
    --filepath /tmp/lighting-v2.ota \
    --discriminator 1111 \
    --passcode 123456789 \
    --secured-device-port 5541 \
    -q updateAvailable

```

3. Commission the OTA provider and update ACLs

```bash
PROVIDER_NODE_ID=1
LIGHT_NODE_ID=<your-lighting-app-node-id>

mattertool pairing onnetwork ${PROVIDER_NODE_ID} 123456789

mattertool accesscontrol write acl \
    '[{"fabricIndex":1,"privilege":5,"authMode":2,"subjects":[112233],"targets":null},{"fabricIndex":1,"privilege":3,"authMode":2,"subjects":null,"targets":[{"cluster":41,"endpoint":null,"deviceType":null}]}]' \
    ${PROVIDER_NODE_ID} 0

```

3. Announce the provider to the device with `chip-tool`

```bash
mattertool otasoftwareupdaterequestor announce-otaprovider \
    ${PROVIDER_NODE_ID} 0 0 0 ${LIGHT_NODE_ID} 0
```

4. Device will download and apply the OTA.
5. Verify the OTA was applied

```bash
mattertool basicinformation read software-version ${LIGHT_NODE_ID} 0
```

## Limitations

This example has the following limitations:

1. **No GPIO Support:** This implementation does not include GPIO control for
   physical LEDs or buttons. All lighting operations are software-only.

2. **Single Transport per Build:** A given build targets a single transport.
   Wi-Fi is supported on the SiWx917 boards and Thread is supported on the
   EFR32MG24 boards; the two cannot be combined in a single image.

3. **Memory Constraints:** Both SoCs have limited RAM and flash memory. Complex
   applications may require memory optimization.

4. **Power Management:** Advanced power management features available in other
   Silicon Labs SDKs may not yet be exposed in this Zephyr-based example.

For production applications requiring GPIO control or features not yet available
here, consider using the standard Silicon Labs lighting example on the EFR32
platform.

---

### Additional Resources:

-   [Silicon Labs Matter Documentation](https://github.com/SiliconLabsSoftware/matter_sdk)
-   [SiWx917 Development Kit User Guide](https://www.silabs.com/development-tools/wireless/wi-fi/siwx917-development-kit)
-   [EFR32xG24 Dev Kit User Guide](https://www.silabs.com/development-tools/wireless/efr32xg24-dev-kit)
-   [Matter Specification](https://csa-iot.org/all-solutions/matter/)
-   [Zephyr RTOS Documentation](https://docs.zephyrproject.org/)

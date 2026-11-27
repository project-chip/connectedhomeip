# All Devices App for NXP

This example demonstrates the Code-Driven Matter data model on NXP FreeRTOS
platforms. The application starts with a Root Node on endpoint 0 and lets you
append device instances at runtime using the Matter shell.

Each successful `devtype set` command adds a new instance without restarting the
Matter server or replacing previously added devices. The device topology is kept
in RAM and is not restored after reboot.

Refer to the top-level [all-devices-app README](../README.md) for the
Code-Driven paradigm and shared device implementations.

## Supported Hardware

The application uses the common NXP FreeRTOS platform integration. The following
board identifiers and connectivity configurations are described in the NXP
platform guides. Follow the relevant guide for transceiver hardware, board
setup, and additional build options.

| Board        | Build board  | Connectivity    | Platform guide                                          |
| ------------ | ------------ | --------------- | ------------------------------------------------------- |
| FRDM-RW612   | `frdmrw612`  | Wi-Fi or Thread | [RW61x](../../../docs/platforms/nxp/nxp_rw61x_guide.md) |
| RD-RW612-BGA | `rdrw612bga` | Wi-Fi or Thread | [RW61x](../../../docs/platforms/nxp/nxp_rw61x_guide.md) |

## Building the Example

### Prerequisites

Follow the
[NXP FreeRTOS environment setup guide](../../../docs/platforms/nxp/nxp_examples_freertos_platforms.md#set-up-the-build-environment)
to initialize the NXP submodules, install the matching MCUX SDK, and configure
the ARM GCC toolchain. This application's CMake entry point requires CMake 3.30
or later.

Run the following commands from the `connectedhomeip` repository root in an
existing, configured MCUX SDK environment. Replace the SDK and toolchain paths
with those installed on your machine:

```bash
source scripts/activate.sh
source /path/to/mcuxsdk/mcux-env.sh
export ARMGCC_DIR=/path/to/arm-gnu-toolchain
```

### Build

Build the Wi-Fi configuration for FRDM-RW612:

```bash
west build -d build_all_devices -b frdmrw612 \
    examples/all-devices-app/nxp \
    -DCONF_FILE_NAME=prj_wifi.conf
```

For the Thread FTD configuration, use a separate build directory:

```bash
west build -d build_all_devices_thread -b frdmrw612 \
    examples/all-devices-app/nxp \
    -DCONF_FILE_NAME=prj_thread_ftd.conf
```

The application configuration in [prj.conf](prj.conf) enables the Matter shell
with `CONFIG_CHIP_LIB_SHELL=y`. Keep it enabled to add devices through the
`devtype` commands.

### Flash

The CMake build produces `app.elf` in the selected build directory; for the
Wi-Fi example above, the image is `build_all_devices/app.elf`.

Follow the flashing and debugging instructions in the corresponding platform
guide. The guides describe the J-Link device selection, interface, and image
loading procedure for each board.

## Usage

Connect to the board's Matter shell using the UART connections described in the
platform guide. The common NXP console settings are 115200 baud, 8 data bits,
one stop bit, no parity, and no flow control. Shell and log output may use
separate UARTs.

### Boot Behavior

After the Matter server is initialized, the application prints:

```text
devtype: ready; use 'devtype list' or 'devtype set <type>'
```

Each boot starts with the following topology:

```text
Endpoint 0 -> Root Node
```

No application device is automatically created. Previously added device
instances are not recreated after reboot.

### List Supported Device Types

List the device types available in the current firmware:

```text
devtype list
```

Entering `devtype` without a subcommand prints the command help.

### Add a Device

Append an instance of a supported device type:

```text
devtype set on-off-light
```

If this is the first successful addition after boot, the application logs:

```text
devtype: added 'on-off-light', endpoint range 1-1 (instances: 1/32)
```

The instance is immediately registered with the running data model. No reboot or
factory reset is required.

### Add Multiple Devices

Starting from a fresh boot, run:

```text
devtype set on-off-light
devtype set temperature-sensor
devtype set on-off-light
```

If all three commands succeed, the endpoint layout is:

```text
Endpoint 0 -> Root Node
Endpoint 1 -> on-off-light
Endpoint 2 -> temperature-sensor
Endpoint 3 -> on-off-light
```

Repeated `set` commands create separate instances, including when the type is
the same. Compound device types can allocate more than one endpoint; use the
reported endpoint range rather than assuming one endpoint per command.

## Supported Device Types

Device availability is controlled by the shared
[DeviceFactory](../all-devices-common/device-factory/DeviceFactory.h) and
[enabled_devices.cmake](../all-devices-common/device-factory/enabled_devices.cmake).
Use `devtype list` to see the registered types in the firmware you are running.

`ALL_DEVICES_ENABLED_DEVICES` is a semicolon-separated CMake list of device
keys. An empty list, which is the default, enables all types covered by the
shared CMake configuration. To select a subset, pass the list when configuring a
new build directory:

```bash
west build -d build_all_devices_subset -b frdmrw612 \
    examples/all-devices-app/nxp \
    -DCONF_FILE_NAME=prj_wifi.conf \
    '-DALL_DEVICES_ENABLED_DEVICES=on-off-light;temperature-sensor'
```

This selects the types available from the factory; it does not instantiate any
devices at boot. Devices must still be added using `devtype set`.

## Testing with chip-tool

Refer to the
[chip-tool guide](../../../docs/development_controllers/chip-tool/chip_tool_guide.md)
for controller setup and commissioning instructions.

For a Wi-Fi build with BLE commissioning enabled, set `SSID`, `PASSWORD`,
`SETUP_PIN_CODE`, and `DISCRIMINATOR` to your network credentials and the
device's setup values, then commission the device as node 1:

```bash
chip-tool pairing ble-wifi 1 "$SSID" "$PASSWORD" \
    "$SETUP_PIN_CODE" "$DISCRIMINATOR"
```

For a Thread build, follow the guide's BLE-to-Thread commissioning procedure
with an active Thread operational dataset instead.

After commissioning and adding devices, read the Root Node's endpoint list:

```bash
chip-tool descriptor read parts-list 1 0
```

For the `on-off-light` on endpoint 1 in the example above:

```bash
chip-tool onoff on 1 1
chip-tool onoff read on-off 1 1
chip-tool onoff off 1 1
```

The first numeric argument is the commissioned node ID; the second is the
endpoint ID. Use the actual IDs for your device topology.

## Notes and Limitations

-   Device topology is RAM-only. Rebooting removes the runtime instances; issue
    the `devtype set` commands again to recreate them. This does not imply that
    fabric credentials or all persistent attributes are erased.
-   The current application allows up to 32 successful runtime device instances,
    as defined by `kMaxRuntimeDevices` in
    [AppTask.h](common/main/include/AppTask.h). This is an instance limit, not
    an endpoint limit or a guarantee that every combination fits in RAM.
    Allocation or registration can fail earlier.
-   Endpoint IDs are assigned automatically, starting at 1. Failed registration
    attempts can consume IDs, so subsequent ranges need not be contiguous.
-   Disabling the Matter shell leaves only the Root Node at startup; this port
    does not instantiate a default application device without a command.

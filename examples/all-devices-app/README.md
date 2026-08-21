# Matter All-Devices Application

The `all-devices-app` is a sample application for the Matter SDK that
demonstrates the **Code-Driven paradigm**, which removes the dependency on
generated code (ZAP tool) and global instances. This is the recommended approach
for developing Matter applications, as it allows testing various device types
and clusters without requiring recompilation for each configuration.

## Code-Driven Paradigm Advantages

The Code-Driven paradigm offers several advantages:

-   **Unit Testable**: Allows clusters to be unit tested easily.
-   **Dynamic Data Model**: Allows applications to change their data model
    dynamically at runtime without requiring recompilation.
-   **Composite Devices**: Supports multi-endpoint devices and bridges.
-   **Maintainability**: Decouples cluster implementations from application
    configuration.

The application simulates various device types.

## Documentation Suite

The [`docs/`](docs/) directory contains documentation for this application:

-   **[Architecture & Design Patterns](docs/architecture.md)**: Describes the
    Code-Driven Data Model, component hierarchies (`DeviceFactory`,
    `SingleEndpointDevice`), and platform separation.
-   **[Starting Up & CLI Reference](docs/starting_up.md)**: Describes
    application initialization, endpoint composition flags (`--device`), and
    network settings.
-   **[Testing & Simulation Guide](docs/testing.md)**: Instructions for
    `chip-tool` commissioning and executing automated Python regression suites
    (`src/python_testing/`).
-   **[How to Add a New Simulated Device](docs/adding_new_device.md)**:
    Instructions for implementing Matter devices, binding code-driven clusters,
    and updating build configurations.
-   **[Custom Product Baseline Guide](docs/custom_product_baseline.md)**: Guide
    on transitioning from this simulator baseline to a custom product
    application.

## Architecture and File Structure

The `all-devices-app` separates platform-agnostic code from platform-specific
implementations:

-   **`all-devices-common/`**: Contains platform-agnostic code, including:
    -   Core cluster logic and device interfaces.
    -   Base device implementations.
    -   The **`DeviceFactory`** (in
        `all-devices-common/device-factory/DeviceFactory.h`), which enables
        runtime registration and creation of supported device types.
-   **`esp32/`, `posix/`**: Contain platform-specific implementations (with
    `posix/` containing `linux/` and `darwin/` subdirectories), entry points,
    and build configurations.
    -   For example, `posix/linux/DeviceFactoryPlatformOverride.cpp` registers
        platform-specific overrides for devices at build-time.

This separation ensures core logic remains reusable across operating systems and
hardware platforms while allowing platform-specific driver integration.

## Supported Devices

The application supports the following device types (specified via the
`--device` flag). Currently supported device types include:

-   `aggregator`
-   `air-purifier`
-   `air-quality-sensor`
-   `bridged-node`
-   `chime`
-   `commissioning-proxy`
-   `contact-sensor`
-   `cooktop`
-   `device-energy-management`
-   `dimmable-light`
-   `dimmable-plug-in-unit`
-   `dishwasher`
-   `extractor-hood`
-   `fan`
-   `fan-no-onoff`
-   `flow-sensor`
-   `generic-switch`
-   `humidity-sensor`
-   `laundry-dryer`
-   `laundry-washer`
-   `light-sensor`
-   `microwave-oven`
-   `mounted-dimmable-load-control`
-   `mounted-on-off-control`
-   `network-infrastructure-manager`
-   `occupancy-sensor`
-   `on-off-light`
-   `on-off-light-switch`
-   `on-off-plug-in-unit`
-   `oven`
-   `power-source`
-   `pressure-sensor`
-   `proximity-ranger`
-   `rain-sensor`
-   `refrigerator`
-   `robotic-vacuum-cleaner`
-   `smoke-co-alarm`
-   `soil-sensor`
-   `speaker`
-   `temperature-sensor`
-   `water-freeze-detector`
-   `water-leak-detector`
-   `water-valve`

You can run the application with `--help` to see the list of valid device types.

Example output (trimmed):

```text
Usage: ./out/linux-x64-all-devices-boringssl-no-ble/all-devices-app

PROGRAM OPTIONS

  --device <aggregator|air-purifier|air-quality-sensor|bridged-node|chime|commissioning-proxy|contact-sensor|cooktop|device-energy-management|dimmable-light|dimmable-plug-in-unit|dishwasher|extractor-hood|fan|fan-no-onoff|flow-sensor|generic-switch|humidity-sensor|laundry-dryer|laundry-washer|light-sensor|microwave-oven|mounted-dimmable-load-control|mounted-on-off-control|network-infrastructure-manager|occupancy-sensor|on-off-light|on-off-light-switch|on-off-plug-in-unit|oven|power-source|pressure-sensor|proximity-ranger|rain-sensor|refrigerator|robotic-vacuum-cleaner|smoke-co-alarm|soil-sensor|speaker|temperature-sensor|water-freeze-detector|water-leak-detector|water-valve>
       Select the device to start up. Format: 'type' or 'type:endpoint' or 'type:endpoint,parent=parentId'
       Can be specified multiple times for multi-endpoint devices.
       Example: --device chime:1 --device speaker:2,parent=1

  --wifi
       Enable wifi support for commissioning
```

## Building the Application

Ensure your environment is activated:

```bash
source scripts/activate.sh
```

Build the application using the following command:

```bash
./scripts/build/build_examples.py --target linux-x64-all-devices-boringssl-no-ble build
```

## Running the Application

To run the application, specify the device type using the `--device` flag. The
format is `type:endpoint` or `type:endpoint,parent=parentId`, where the optional
`parent` option allows establishing parent/child endpoint relationships for
logical grouping.

The application supports running multiple devices simultaneously by specifying
the flag multiple times.

```bash
# Clean up KVS storage if needed
rm -rf /tmp/chip_*

# Run a chime on endpoint 1, a speaker on endpoint 2 (child of endpoint 1), and a dimmable light on endpoint 3
./out/linux-x64-all-devices-boringssl-no-ble/all-devices-app --device chime:1 --device speaker:2,parent=1 --device dimmable-light:3
```

## Bridging Support: Bridged Modifier (`,bridged`)

You can use the `,bridged` option modifier on a device definition to
automatically wrap it in a parent `bridged-node` endpoint.

Without the `,bridged` modifier, setting up a bridged device requires manually
typing out both the intermediate `bridged-node` parent and the leaf device
child, which gets extremely verbose:

```bash
./out/linux-x64-all-devices-boringssl/all-devices-app --device aggregator:1 --device bridged-node:2,parent=1 --device chime:3,parent=2
```

Using the `,bridged` modifier automatically handles the intermediate
`bridged-node` injection:

-   **Explicit Bridged Device:**

    ```bash
    ./out/linux-x64-all-devices-boringssl/all-devices-app --device aggregator:1 --device "chime:2,parent=1,bridged"
    ```

    This command explicitly maps a chime on Endpoint 2 to be bridged under the
    aggregator on Endpoint 1. The application automatically expands this to the
    following layout:

    ```text
    Endpoint 0 (Root Node)
       └── Endpoint 1 (aggregator)
              └── Endpoint 2 (bridged-node)
                     └── Endpoint 3 (chime)
    ```

## Advanced Topology: Wildcard Expansion (`*`)

You can use the wildcard `*` to automatically instantiate all supported leaf
device types. When an endpoint is specified, it represents the starting number.

-   **Standard Wildcard:** Start all devices from endpoint 1 sequentially.

    ```bash
    ./out/linux-x64-all-devices-boringssl/all-devices-app --device "*:1"
    ```

-   **Parented Wildcard:** Start all devices from endpoint 2 sequentially and
    make them all children of parent endpoint 1 (e.g., an aggregator).

    ```bash
    ./out/linux-x64-all-devices-boringssl/all-devices-app --device aggregator:1 --device "*:2,parent=1"
    ```

-   **Compound Bridged Wildcard:** Automatically wraps every leaf device
    generated by the wildcard in a dedicated `bridged-node` parent endpoint.

    ```bash
    ./out/linux-x64-all-devices-boringssl/all-devices-app --device aggregator:1 --device "*:2,parent=1,bridged"
    ```

    This automatically generates the following compound bridged device tree:

    ```text
    Endpoint 0 (Root Node)
       └── Endpoint 1 (aggregator)
              ├── Endpoint 2 (bridged-node)
              │      └── Endpoint 3 (air-quality-sensor)
              ├── Endpoint 4 (bridged-node)
              │      └── Endpoint 5 (chime)
              ├── Endpoint 6 (bridged-node)
              │      └── Endpoint 7 (contact-sensor)
              └── ...
    ```

## Commissioning Proxy

This example can act as a Commissioning Proxy over WiFi-PAF and/or BLE. The
`commissioning-proxy` device type implements the Commissioning By Proxy device
type (`MA-commissioning-by-proxy`, 0x0092) with a Commissioning Proxy cluster
server. It is available on the Linux (posix) platform only — the device is
registered from `posix/linux/DeviceFactoryPlatformOverride.cpp`.

Supported commands: `ProxyScanRequest`, the `ProxyConnectRequest` /
`ProxyMessageRequest` / `ProxyDisconnectRequest` flow, and the BackgroundScan
feature (`ProxyBackGroundScanStartRequest` / `ProxyBackGroundScanStopRequest`),
over whichever transport(s) are compiled in. The proxy serves one commissioning
session at a time across all transports (`MaxSessions` = 1) and caches up to 10
background-scan results.

Two build switches gate the device itself:

-   `commissioning-proxy` must be in the device-factory enable list. All devices
    are enabled by default; `all_devices_enabled_devices` in
    `all-devices-common/device-factory/enabled_devices.gni` selects a subset.
-   `CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONING_PROXY`, set for this app in
    `posix/include/CHIPProjectAppConfig.h`, enables the Linux platform support
    the transports call: the `BLEManagerImpl` proxy scan and peripheral→central
    role switch, and the `ConnectivityManagerImpl` WiFi-PAF proxy entry points.

The compiled-in transport(s) follow this build configuration:

-   **WiFi-PAF** is included when `chip_device_config_enable_wifipaf` is true
    (the default for Linux builds with WiFi enabled). When WiFi-PAF is compiled
    in, the cluster advertises the `WiFiNetworkInterface` feature and the
    `WiFiBand` attribute; the supported bands are derived from the `freq_list`
    passed via `--wifipaf`. Note that WiFi-PAF scanning does not work on a stock
    host: it needs a `wpa_supplicant` built with `CONFIG_NAN_USD` and the
    `discovery_only` patch. See
    [wpa_supplicant with the Matter NAN patch](all-devices-common/device/types/commissioning-proxy/README.md#2-wpa_supplicant-with-the-matter-nan-patch-proxy-device).
-   **BLE** is included when `chip_config_network_layer_ble` is true.

The `-no-ble` build variants disable the BLE transport. With no transport
compiled in the device type still starts, but every proxy command fails.

To build with both BLE and WiFi-PAF on Linux x86-64 run:

```bash
./scripts/run_in_build_env.sh "./scripts/build/build_examples.py --target linux-x64-all-devices-boringssl build"
```

To cross-compile for a Raspberry Pi (ARM64), build inside the cross-compile
container, which supplies the aarch64 sysroot. The ARM and ARM64 boards accept
`-clang` or `-nodeps` target variants:

```bash
# From the root of your checkout, on the host:
docker run -it --user "$(id -u):$(id -g)" -v "$PWD":"$PWD" -w "$PWD" \
    ghcr.io/project-chip/chip-build-crosscompile:200 /bin/bash

# Then, inside the container:
./scripts/run_in_build_env.sh "./scripts/build/build_examples.py --target linux-arm64-all-devices-boringssl-clang build"
```

To start the app as a proxy over WiFi-PAF on channel 6 (2437 MHz) on endpoint 5:

```bash
./out/linux-x64-all-devices-boringssl/all-devices-app --device commissioning-proxy:5 --wifi --wifipaf freq_list=2437
```

The cluster's attributes and commands can then be exercised on endpoint 5.

## Testing with chip-tool

You can use `chip-tool` as a controller to interact with the `all-devices-app`.
For detailed instructions on how to build and use `chip-tool` for commissioning
and sending commands, please refer to the in-project
[chip-tool documentation](../chip-tool/README.md).

### Example Interaction: Chime Device

If you ran the application with `--device chime`, you can send commands to the
Chime cluster.

#### Play a Sound

Trigger the chime sound playback (Node ID `1`, Endpoint `1`).

Playing Chime 0 (Ding Dong):

```bash
chip-tool chime play-chime-sound 1 1 --ChimeID 0
```

Playing Chime 1 (Ring Ring):

```bash
chip-tool chime play-chime-sound 1 1 --ChimeID 1
```

#### Read Attribute

```bash
chip-tool chime read selected-chime 1 1
```

#### Write Attribute

Change the selected chime to `1`:

```bash
chip-tool chime write selected-chime 1 1 1
```

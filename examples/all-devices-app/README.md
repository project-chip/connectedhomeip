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

-   `air-purifier`
-   `chime`
-   `contact-sensor`
-   `cooktop`
-   `device-energy-management`
-   `dimmable-light`
-   `dimmable-plug-in-unit`
-   `extractor-hood`
-   `fan`
-   `fan-no-onoff`
-   `flow-sensor`
-   `generic-switch`
-   `humidity-sensor`
-   `light-sensor`
-   `mounted-dimmable-load-control`
-   `mounted-on-off-control`
-   `occupancy-sensor`
-   `on-off-light`
-   `on-off-light-switch`
-   `on-off-plug-in-unit`
-   `oven`
-   `power-source`
-   `pressure-sensor`
-   `rain-sensor`
-   `refrigerator`
-   `soil-sensor`
-   `speaker`
-   `water-freeze-detector`
-   `water-leak-detector`
-   `water-valve`

You can run the application with `--help` to see the list of valid device types.

Example output (trimmed):

```text
Usage: ./out/linux-x64-all-devices-boringssl-no-ble/all-devices-app

PROGRAM OPTIONS

  --device <air-purifier|chime|contact-sensor|cooktop|device-energy-management|dimmable-light|dimmable-plug-in-unit|extractor-hood|fan|fan-no-onoff|flow-sensor|generic-switch|humidity-sensor|light-sensor|mounted-dimmable-load-control|mounted-on-off-control|occupancy-sensor|on-off-light|on-off-light-switch|on-off-plug-in-unit|oven|power-source|pressure-sensor|rain-sensor|refrigerator|soil-sensor|speaker|water-freeze-detector|water-leak-detector|water-valve>
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

# Matter All-Devices Application

The `all-devices-app` is a sample application for the Matter SDK that
demonstrates the **Code-Driven paradigm**, which removes the dependency on
generated code (ZAP tool) and global instances. This is the recommended approach
for developing Matter applications, as it allows testing various device types
and clusters without requiring recompilation for each configuration.

## What’s The “All Devices App”?

The Code-Driven paradigm offers several advantages:

-   **Unit Testable**: Allows clusters to be unit tested easily.
-   **Dynamic Data Model**: Allows applications to change their data model
    dynamically at runtime without requiring recompilation.
-   **Complex Products**: Makes it easier to develop complex products such as
    bridges.
-   **Software Evolution**: Makes it easier to iterate and evolve product
    software.

The application simulates various device types and is highly extensible.

## Architecture and File Structure

The `all-devices-app` is organized to be **platform-agnostic** at its core, with
platform-specific specializations kept separate:

-   **`all-devices-common/`**: Contains the platform-agnostic core of the
    application. This includes:
    -   Core cluster logic and device interfaces.
    -   Base device implementations.
    -   The **`DeviceFactory`** (in
        `all-devices-common/devices/device-factory/DeviceFactory.h`), which
        enables runtime registration and creation of supported device types.
-   **`esp32/`, `posix/`**: Contain platform-specific implementations (with
    `posix/` containing `linux/` and `darwin/` subdirectories), entry points,
    and build configurations.
    -   For example, `posix/linux/DeviceFactoryPlatformOverride.cpp` registers
        platform-specific overrides for devices at build-time.

This separation ensures that the core logic remains clean and reusable across
different operating systems and hardware platforms, while still allowing for
deep platform integration when needed.

## Supported Devices

The application supports the following device types (specified via the
`--device` flag). Note that this list represents what is supported currently,
but the application is constantly evolving and new device types are added often:

-   `contact-sensor`
-   `water-leak-detector`
-   `occupancy-sensor`
-   `chime`
-   `dimmable-light`
-   `on-off-light`
-   `speaker`
-   `soil-sensor`

You can run the application with `--help` to see the list of valid device types.

Example output (trimmed):

```text
Usage: ./out/linux-x64-all-devices-boringssl-no-ble/all-devices-app

PROGRAM OPTIONS

  --device <chime|contact-sensor|dimmable-light|occupancy-sensor|on-off-light|soil-sensor|speaker|water-leak-detector>
       Select the device to start up. Format: 'type' or 'type:endpoint'
       Can be specified multiple times for multi-endpoint devices.
       Example: --device chime:1 --device speaker:2

  --endpoint <endpoint-number>
       Define the endpoint for the preceding device (default 1)

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
application supports running multiple devices simultaneously by specifying the
flag multiple times.

```bash
# Clean up KVS storage if needed
rm -rf /tmp/chip_*

# Run a chime on endpoint 1 and a speaker on endpoint 2
./out/linux-x64-all-devices-boringssl-no-ble/all-devices-app --device chime:1 --device speaker:2
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

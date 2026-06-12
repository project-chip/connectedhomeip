# Application Initialization & Command Line Options

This guide describes configuring and running the `all-devices-app` executable.

The application builds its data model at runtime based on command-line arguments.

---

## Prerequisite: Building from Source

Before executing any commands, activate your build environment and compile the application from scratch:

```bash
source scripts/activate.sh
./scripts/build/build_examples.py --target linux-x64-all-devices-clang build
```

The compiled executable will be located at `./out/linux-x64-all-devices-clang/all-devices-app`.

---

## Startup Architecture

When the application boots, it executes the following sequence:
1. **Option Parsing**: Reads CLI flags via `AppOptions::GetOptions()` to determine the required storage layer, network ports, and runtime endpoint composition.
2. **Platform App Main Loop**: Initializes the Matter POSIX/Platform server stack (`AppMain()`).
3. **Endpoint Instantiation**: For each `--device`, the runtime instantiates the device using `DeviceFactory` and registers it with the Interaction Model.

---

## Command Line Options

### Endpoint Composition Options

- **`--device <type>[:<endpoint>][,parent=<parentId>]`**
  *(Required, can be repeated multiple times)*
  Instantiates a Matter device on the server.
  - `<type>`: The registered runtime key (e.g., `contact-sensor`, `dimmable-light`, `occupancy-sensor`).
  - `:<endpoint>` *(Optional)*: Assigns a specific endpoint index. If omitted, the runtime assigns an auto-incrementing ID.
  - `,parent=<parentId>` *(Optional)*: Establishes a parent/child endpoint relationship for composite endpoints or bridged devices.

  **Examples:**
  ```bash
  # Launch a single self-contained chime on auto-assigned Endpoint 1
  ./out/linux-x64-all-devices-clang/all-devices-app --device chime

  # Launch an explicit Chime on Endpoint 1 and attach a Speaker on Endpoint 2 as its child
  ./out/linux-x64-all-devices-clang/all-devices-app --device chime:1 --device speaker:2,parent=1
  ```

### Commissioning & Network Options

- **`--wifi`**
  *(Conditional: Only active if built with `CHIP_DEVICE_CONFIG_ENABLE_WIFI`)*
  Enables Wi-Fi network commissioning.

- **`--ble-controller <number>`**
  *(Conditional: Only active if built with `CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE`)*
  Selects the Bluetooth adapter controller index (e.g., `0` for `hci0`) for Matter BLE commissioning advertisements.

- **`--port <number>`**
  Binds the UDP Server to a specified port (Default is `5540`). Used for running multiple simulator instances concurrently on the same host interface.

- **`--interface-id <number>`**
  Binds Matter multicast advertisements and operational MDNS traffic to a specific Network Interface index.

- **`--groupcast`**
  Enables multi-node groupcast communication.

### Identification & Descriptor Options

- **`--discriminator <number>`**
  Overrides the 12-bit Setup Discriminator advertised during uncommissioned discovery (Default is `3840`).

- **`--vendor-id <number>`** / **`--product-id <number>`**
  Overrides the Basic Information Vendor ID (VID) and Product ID (PID) descriptors announced during commissioning.

### Persistent Storage Options

- **`--KVS <path>`**
  Sets the file path for Key-Value Store (KVS) persistent storage (Default is `/tmp/chip_all_devices_kvs`).

  *Note*: Remove this file when changing the `--device` configuration between runs to avoid loading stale data:
  ```bash
  rm -rf /tmp/chip_all_devices_kvs
  ./out/linux-x64-all-devices-clang/all-devices-app --device contact-sensor:1
  ```

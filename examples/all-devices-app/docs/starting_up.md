# Application Initialization & Command Line Options

This guide describes configuring and running the `all-devices-app` executable.

The application builds its data model at runtime based on command-line
arguments.

---

## Prerequisite: Building from Source

Before executing any commands, activate your build environment and compile the
application from scratch:

```bash
source scripts/activate.sh
./scripts/build/build_examples.py --target linux-x64-all-devices-clang build
```

The compiled executable will be located at
`./out/linux-x64-all-devices-clang/all-devices-app`.

---

## Startup Architecture

When the application boots, it executes the following sequence:

1. **Option Parsing**: Reads CLI flags via `AppOptions::GetOptions()` to
   determine the required storage layer, network ports, and runtime endpoint
   composition.
2. **Platform App Main Loop**: Initializes the Matter POSIX/Platform server
   stack (`AppMain()`).
3. **Endpoint Instantiation**: For each `--device`, the runtime instantiates the
   device using `DeviceFactory` and registers it with the Interaction Model.

---

## Core Runtime Composition

Unlike static Matter targets, `all-devices-app` allows you to dynamically
assemble your exact endpoint structure at boot using the `--device` option.

### `--device <type>[:<endpoint>][,parent=<parentId>]`

Instantiates a specific simulated device feature on the Matter server. This flag
can be repeated multiple times to build composite or multi-endpoint topologies.

-   `<type>`: The registered runtime key (e.g., `contact-sensor`,
    `dimmable-light`, `occupancy-sensor`).
-   `:<endpoint>` _(Optional)_: Assigns an explicit endpoint index. If omitted,
    the runtime automatically assigns an incrementing ID starting from 1.
-   `,parent=<parentId>` _(Optional)_: Attaches this endpoint as a child of
    another endpoint, establishing a logical tree composition (such as attaching
    a Speaker to a parent Chime or building an Aggregator bridge).

> [!TIP] This application inherits the Matter SDK's complete baseline option
> parser. For the complete, live list of available network commissioning
> (`--wifi`, `--ble-controller`), operational binding (`--port`,
> `--interface-id`), descriptor (`--discriminator`, `--vendor-id`), and
> persistent storage (`--KVS`) arguments, execute the binary with `--help`:
>
> ```bash
> ./out/linux-x64-all-devices-clang/all-devices-app --help
> ```

---

## Practical Operating Recipes

### Recipe 1: Booting a Clean Composite Tree Device

When launching new multi-endpoint devices, always wipe your previous persistent
storage to ensure the server doesn't reload stale data model definitions:

```bash
# Safely wipe the default storage file
rm -rf /tmp/chip_all_devices_kvs

# Boot an explicit Chime on Endpoint 1 and attach a Speaker on Endpoint 2 as its child
./out/linux-x64-all-devices-clang/all-devices-app --device chime:1 --device speaker:2,parent=1
```

### Recipe 2: Running Multiple Simulators Concurrently

To launch multiple independent simulator processes side-by-side on the same host
interface without socket or commissioning collisions, provide distinct UDP
ports, storage paths, and Setup Discriminators:

```bash
# Boot Instance Alpha (Default settings)
./out/linux-x64-all-devices-clang/all-devices-app \
    --device occupancy-sensor:1 \
    --KVS /tmp/chip_kvs_alpha \
    --port 5540 \
    --discriminator 3840

# Boot Instance Beta (Isolated settings)
./out/linux-x64-all-devices-clang/all-devices-app \
    --device contact-sensor:1 \
    --KVS /tmp/chip_kvs_beta \
    --port 5541 \
    --discriminator 3841
```

---
name: device-data-model-dump
description: Dump and inspect the runtime Matter Data Model (endpoints, clusters, attributes, commands, events, and device types) from a Matter device or example application using TC_IDM_12_1 (TC_DeviceBasicComposition.py) or chip-tool Descriptor cluster queries. Use when inspecting the live data model topology of a Matter device or generating MatterTlvJson device dumps for conformance testing.
---

# Matter Device Data Model Dump & Runtime Discovery

## Overview

You can inspect or export the ground-truth runtime Data Model (endpoints, clusters, attributes, commands, events, and device types) from any running Matter device or example application using two standard approaches:

1. **Full Data Model Dump (`TC_IDM_12_1` in `TC_DeviceBasicComposition.py`)**: Performs a wildcard read across all endpoints, clusters, and attributes, producing both a machine-readable `MatterTlvJson` (`.json`) file and a human-readable decoded summary (`.txt`).
2. **Interactive Runtime Discovery (`chip-tool` via the `Descriptor` Cluster `0x001D`)**: Queries active endpoints (`PartsList`), server/client clusters (`ServerList`, `ClientList`), device types (`DeviceTypeList`), and cluster global attributes directly from the command line.

---

## Method 1: Full Data Model Dump via `TC_IDM_12_1` (`TC_DeviceBasicComposition.py`)

The `test_TC_IDM_12_1` test in `src/python_testing/TC_DeviceBasicComposition.py` (implemented via `BasicCompositionTests.dump_wildcard` in `src/python_testing/matter_testing_infrastructure/matter/testing/basic_composition.py`) performs a full wildcard attribute and event read of the device and writes **two** files:

- **`<name>.json` (`MatterTlvJson` format)**: Raw TLV structure of all endpoints, clusters, and attributes. Used for CSA certification submission and offline conformance testing (`TC_DeviceConformance.py`).
- **`<name>.txt` (Human-readable format)**: Decoded Python `pprint` dump listing every endpoint, cluster class name, attribute class name, and decoded value.

By default, if no custom path is provided, the files are written to the current working directory using the `VendorID`, `ProductID`, and `SoftwareVersion` from Endpoint 0 `BasicInformation`:

```text
device_dump_0x<VID>_0x<PID>_<SW_VER>.json
device_dump_0x<VID>_0x<PID>_<SW_VER>.txt
```

For example, with default test vendor/product IDs (`0xFFF1` / `0x8001` / version `1`), it creates `device_dump_0xFFF1_0x8001_1.json` and `device_dump_0xFFF1_0x8001_1.txt`.

### Customizing the Output File Path

Pass `--string-arg dump_device_composition_path:<path>` to specify a custom destination path. The script automatically replaces or appends the `.json` and `.txt` suffixes:

```text
--string-arg dump_device_composition_path:/tmp/my_device_dump
```

This generates `/tmp/my_device_dump.json` and `/tmp/my_device_dump.txt`.

### 1. Running Locally Against an Example App (`run_python_test.py`)

Ensure the Python testing environment (`out/python_env`) and target application (e.g., `all-devices-app`) are built:

```bash
# Build python_env if not already present
./scripts/build_python.sh -i out/python_env

# Build all-devices-app if not already built
source scripts/activate.sh && ./scripts/build/build_examples.py --target linux-x64-all-devices-boringssl build
```

Run `test_TC_IDM_12_1` using `run_python_test.py` (which launches the app, connects over PASE using default discriminator `3840` / manual code `34970112332`, and writes the dump files):

```bash
rm -f /tmp/chip_* && ./scripts/run_in_python_env.sh out/python_env \
  './scripts/tests/run_python_test.py \
    --app ./out/linux-x64-all-devices-boringssl/all-devices-app \
    --app-args "--device on-off-light" \
    --script src/python_testing/TC_DeviceBasicComposition.py \
    --script-args "--storage-path /tmp/chip_admin_storage.json --manual-code 34970112332 --tests test_TC_IDM_12_1 --string-arg dump_device_composition_path:/tmp/my_device_dump"'
```

### 2. Running Standalone Against a Running Device

If the Matter device or application is already running, invoke `TC_DeviceBasicComposition.py` directly inside the Python environment. By omitting `--commissioning-method`, the script connects directly over a PASE session without needing to commission the device:

```bash
./scripts/run_in_python_env.sh out/python_env \
  'python3 src/python_testing/TC_DeviceBasicComposition.py \
    --manual-code 34970112332 \
    --storage-path /tmp/chip_admin_storage.json \
    --tests test_TC_IDM_12_1 \
    --string-arg dump_device_composition_path:/tmp/my_device_dump'
```

### 3. Saving the Dump When Running via Matter Test Harness (`th-cli` on Raspberry Pi)

When running `TC_IDM_12_1` via `th-cli` on the Matter Test Harness (Raspberry Pi), the test executes inside an **ephemeral Docker container** spun up by the Test Harness backend. Any files written to the container's default working directory are discarded when the container exits.

The Test Harness backend bind-mounts its `python_testing` directory into the container at `/root/python_testing`. To persist the `.json` and `.txt` files on the host, pass `dump_device_composition_path` pointing to `/root/python_testing/my_device_dump`:

```text
--string-arg dump_device_composition_path:/root/python_testing/my_device_dump
```

The generated `my_device_dump.json` and `my_device_dump.txt` files will be saved on the Raspberry Pi host under:

```text
~/certification-tool/backend/test_collections/matter/sdk_tests/sdk_checkout/python_testing/
```

### 4. Running Offline Conformance Tests Against a Dump File

Once you have a `.json` dump file (`MatterTlvJson` format), you can run full cluster and device-type conformance checks offline without needing the physical device or running app:

```bash
./scripts/run_in_python_env.sh out/python_env \
  'python3 src/python_testing/TC_DeviceConformance.py \
    --string-arg test_from_file:/tmp/my_device_dump.json'
```

### 5. Generating an Interactive HTML Viewer from a Dump File

Because raw `.json` and `.txt` dumps contain numeric cluster, attribute, command, and device-type IDs that can be hard to read, this skill includes `scripts/dump_to_html.py` to convert any `MatterTlvJson` `.json` dump into a self-contained, interactive `.html` file.

The HTML viewer automatically:
- Reconstructs the parent-child **Endpoint Composition Tree** from `Descriptor::PartsList` (handling both Full-Family and Tree patterns, including bridged devices and multi-endpoint sub-devices).
- Resolves Matter XML specification names and `ClusterObjects` types for all device types, `BridgedDeviceBasicInformation::NodeLabel` labels, `Descriptor::TagList` semantic tags, clusters, active `FeatureMap` flags, accepted/generated commands, and decoded attribute values.
- Provides a **Topology Tree Overview**, collapsible tree sidebar, breadcrumb navigation, and instant search/filtering:

```bash
./scripts/run_in_python_env.sh out/python_env \
  'python3 .agents/skills/device-data-model-dump/scripts/dump_to_html.py \
    /tmp/my_device_dump.json \
    -o /tmp/my_device_dump.html'
```

---

## Method 2: Interactive Runtime Discovery via `chip-tool` (`Descriptor` Cluster `0x001D`)

Every Matter endpoint implements the **Descriptor Cluster (`0x001D`)**, which exposes the live topology of the node. You can query it directly with `chip-tool` to discover endpoints, clusters, and device types.

### 1. Commission the Device

Start the target device (e.g., `./out/linux-x64-all-devices-boringssl/all-devices-app --device on-off-light`) and commission it with `chip-tool` using a chosen `<node-id>` (e.g., `1`):

```bash
./out/linux-x64-chip-tool/chip-tool pairing onnetwork 1 20202021
```

### 2. Discover All Active Endpoints (`PartsList`)

Read the `PartsList` attribute (`0x0003`) on the Root Node (**Endpoint `0`**) to list all active non-root endpoints on the device:

```bash
./out/linux-x64-chip-tool/chip-tool descriptor read parts-list 1 0
```

Example output:
```text
Endpoint: 0 Cluster: 0x0000_001D Attribute 0x0000_0003 DataVersion: ...
  PartsList: 1 entries
    [1]: 1
```

### 3. Discover Clusters on an Endpoint (`ServerList` and `ClientList`)

Read `ServerList` (`0x0001`) and `ClientList` (`0x0002`) on a specific endpoint (e.g., Endpoint `1`), or pass the wildcard endpoint **`0xFFFF`** to dump the cluster list across **all** endpoints at once:

```bash
# Server clusters on Endpoint 1
./out/linux-x64-chip-tool/chip-tool descriptor read server-list 1 1

# Server clusters across ALL endpoints (wildcard endpoint 0xFFFF)
./out/linux-x64-chip-tool/chip-tool descriptor read server-list 1 0xFFFF

# Client clusters across ALL endpoints
./out/linux-x64-chip-tool/chip-tool descriptor read client-list 1 0xFFFF
```

Example output:
```text
Endpoint: 1 Cluster: 0x0000_001D Attribute 0x0000_0001 DataVersion: ...
  ServerList: 5 entries
    [1]: 4 (Groups)
    [2]: 6 (OnOff)
    [3]: 98 (ScenesManagement)
    [4]: 3 (Identify)
    [5]: 29 (Descriptor)
```

### 4. Discover Device Types per Endpoint (`DeviceTypeList`)

Read `DeviceTypeList` (`0x0000`) on a specific endpoint or across all endpoints using `0xFFFF`:

```bash
# Device types on Endpoint 1
./out/linux-x64-chip-tool/chip-tool descriptor read device-type-list 1 1

# Device types across ALL endpoints
./out/linux-x64-chip-tool/chip-tool descriptor read device-type-list 1 0xFFFF
```

Example output:
```text
Endpoint: 1 Cluster: 0x0000_001D Attribute 0x0000_0000 DataVersion: ...
  DeviceTypeList: 1 entries
    [1]: {
      DeviceType: 256 (On/Off Light)
      Revision: 4
     }
```

### 5. Inspect Attributes, Commands, and Features on a Specific Cluster

To inspect the supported attributes, accepted/generated commands, feature map, and revision of any cluster on an endpoint (e.g., `onoff` on Endpoint `1`):

```bash
# List all supported attribute IDs on the cluster
./out/linux-x64-chip-tool/chip-tool onoff read attribute-list 1 1

# List accepted (client-to-server) command IDs
./out/linux-x64-chip-tool/chip-tool onoff read accepted-command-list 1 1

# List generated (server-to-client) command IDs
./out/linux-x64-chip-tool/chip-tool onoff read generated-command-list 1 1

# Read the FeatureMap bitmask and ClusterRevision
./out/linux-x64-chip-tool/chip-tool onoff read feature-map 1 1
./out/linux-x64-chip-tool/chip-tool onoff read cluster-revision 1 1
```

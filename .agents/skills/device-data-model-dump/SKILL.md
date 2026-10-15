---
name: device-data-model-dump
description:
    Dump and inspect the runtime Matter Data Model (endpoints, clusters,
    attributes, commands, and device types) from a Matter device or example
    application using TC_IDM_12_1 (TC_DeviceBasicComposition.py) or chip-tool
    Descriptor cluster queries. Use when inspecting the live data model topology
    of a Matter device or generating MatterTlvJson device dumps for conformance
    testing.
---

# Matter Device Data Model Dump & Runtime Discovery

## Overview

You can inspect or export the ground-truth runtime Data Model (endpoints,
clusters, attributes, commands, and device types) from any running Matter device
or example application using two standard approaches:

1. **Full Data Model Dump (`TC_IDM_12_1` in `TC_DeviceBasicComposition.py`)**:
   Performs a wildcard read across all endpoints, clusters, and attributes,
   producing a machine-readable `MatterTlvJson` (`.json`) file, a human-readable
   decoded summary (`.txt`), and a self-contained interactive HTML viewer
   (`.html`).
2. **Interactive Runtime Discovery (`chip-tool` via the `Descriptor` Cluster
   `0x001D`)**: Queries active endpoints (`PartsList`), server/client clusters
   (`ServerList`, `ClientList`), device types (`DeviceTypeList`), and cluster
   global attributes directly from the command line.

---

## Method 1: Full Data Model Dump via `TC_IDM_12_1` (`TC_DeviceBasicComposition.py`)

The `test_TC_IDM_12_1` test in `src/python_testing/TC_DeviceBasicComposition.py`
(implemented via `BasicCompositionTests.dump_wildcard` in
`src/python_testing/matter_testing_infrastructure/matter/testing/basic_composition.py`)
performs a full wildcard attribute read of the device and writes **three**
files:

-   **`<name>.json` (`MatterTlvJson` format)**: Raw TLV structure of all
    endpoints, clusters, and attributes. Used for CSA certification submission
    and offline conformance testing (`TC_DeviceConformance.py`).
-   **`<name>.txt` (Human-readable format)**: Decoded Python `pprint` dump
    listing every endpoint, cluster class name, attribute class name, and
    decoded value.
-   **`<name>.html` (Interactive HTML viewer)**: Self-contained clickable HTML
    viewer displaying the endpoint composition tree, server/client clusters,
    feature bitmasks, accepted/generated commands, and decoded attributes.

By default, if no custom path is provided, the files are written to the current
working directory using the `VendorID`, `ProductID`, and `SoftwareVersion` from
Endpoint 0 `BasicInformation`:

```text
device_dump_0x<VID>_0x<PID>_<SW_VER>.json
device_dump_0x<VID>_0x<PID>_<SW_VER>.txt
device_dump_0x<VID>_0x<PID>_<SW_VER>.html
```

For example, with default test vendor/product IDs (`0xFFF1` / `0x8001` / version
`1`), it creates `device_dump_0xFFF1_0x8001_1.json`,
`device_dump_0xFFF1_0x8001_1.txt`, and `device_dump_0xFFF1_0x8001_1.html`.

### Customizing the Output File Path

Pass `--string-arg dump_device_composition_path:<path>` to specify a custom
destination path. The script automatically replaces or appends the `.json`,
`.txt`, and `.html` suffixes:

```text
--string-arg dump_device_composition_path:/tmp/my_device_dump
```

This generates `/tmp/my_device_dump.json`, `/tmp/my_device_dump.txt`, and
`/tmp/my_device_dump.html`.

### 1. Running Locally Against an Example App (`run_python_test.py`)

Ensure the Python testing environment (`out/venv`) and target application (e.g.,
`all-devices-app`) are built:

```bash
# Build venv if not already present
./scripts/build_python.sh -i out/venv --enable_ipv4 true

# Build all-devices-app if not already built
source scripts/activate.sh && ./scripts/build/build_examples.py --target linux-x64-all-devices-clang build
```

Run `test_TC_IDM_12_1` using `run_python_test.py` (which launches the app,
connects over PASE using default discriminator `3840` / manual code
`34970112332`, and writes the dump files):

```bash
./scripts/run_in_python_env.sh out/venv \
  './scripts/tests/run_python_test.py \
    --factory-reset \
    --app ./out/linux-x64-all-devices-clang/all-devices-app \
    --app-args "--device on-off-light --KVS /tmp/chip_kvs_dump" \
    --script src/python_testing/TC_DeviceBasicComposition.py \
    --script-args "--storage-path /tmp/chip_admin_storage.json --manual-code 34970112332 --tests test_TC_IDM_12_1 --string-arg dump_device_composition_path:/tmp/my_device_dump"'
```

### 2. Running Standalone Against a Running Device

If the Matter device or application is already running, invoke
`TC_DeviceBasicComposition.py` directly inside the Python environment. By
omitting `--commissioning-method`, the script connects directly over a PASE
session without needing to commission the device:

```bash
./scripts/run_in_python_env.sh out/venv \
  'python3 src/python_testing/TC_DeviceBasicComposition.py \
    --manual-code 34970112332 \
    --storage-path /tmp/chip_admin_storage.json \
    --tests test_TC_IDM_12_1 \
    --string-arg dump_device_composition_path:/tmp/my_device_dump'
```

### 3. Saving the Dump When Running via Matter Test Harness (`th-cli` on Raspberry Pi)

When running `TC_IDM_12_1` via `th-cli` on the Matter Test Harness (Raspberry
Pi), the test executes inside an **ephemeral Docker container** spun up by the
Test Harness backend. Any files written to the container's default working
directory are discarded when the container exits.

The Test Harness backend bind-mounts its `python_testing` directory into the
container at `/root/python_testing`. To persist the `.json`, `.txt`, and `.html`
files on the host, pass `dump_device_composition_path` pointing to
`/root/python_testing/my_device_dump`:

```text
--string-arg dump_device_composition_path:/root/python_testing/my_device_dump
```

The generated `my_device_dump.json`, `my_device_dump.txt`, and
`my_device_dump.html` files will be saved on the Raspberry Pi host under:

```text
~/certification-tool/backend/test_collections/matter/sdk_tests/sdk_checkout/python_testing/
```

### 4. Running Offline Conformance Tests Against a Dump File

Once you have a `.json` dump file (`MatterTlvJson` format), you can run full
cluster and device-type conformance checks offline without needing the physical
device or running app:

```bash
./scripts/run_in_python_env.sh out/venv \
  'python3 src/python_testing/TC_DeviceConformance.py \
    --string-arg test_from_file:/tmp/my_device_dump.json'
```

### 5. Generating an Interactive HTML Viewer from an Existing `.json` Dump File

In addition to being generated automatically by `dump_wildcard()`, you can
convert any existing `.json` dump file into a single, self-contained, clickable
`.html` file using `json_dump_to_html.py` with standard `python3` (no
third-party dependencies or virtual environment required):

```bash
python3 src/python_testing/matter_testing_infrastructure/matter/testing/json_dump_to_html.py \
  /tmp/my_device_dump.json \
  -o /tmp/my_device_dump.html
```

---

## Method 2: Interactive Runtime Discovery via `chip-tool` (`Descriptor` Cluster `0x001D`)

Every Matter endpoint implements the **Descriptor Cluster (`0x001D`)**, which
exposes the live topology of the node. You can query it directly with
`chip-tool` to discover endpoints, clusters, and device types.

### 1. Commission the Device

Start the target device (e.g.,
`./out/linux-x64-all-devices-clang/all-devices-app --device on-off-light`) and
commission it with `chip-tool` using a chosen `<node-id>` (e.g., `1`):

```bash
./out/linux-x64-chip-tool-clang/chip-tool pairing onnetwork 1 20202021
```

### 2. Discover All Active Endpoints (`PartsList`)

Read the `PartsList` attribute (`0x0003`) on the Root Node (**Endpoint `0`**) to
list all active non-root endpoints on the device:

```bash
./out/linux-x64-chip-tool-clang/chip-tool descriptor read parts-list 1 0
```

Example output:

```text
Endpoint: 0 Cluster: 0x0000_001D Attribute 0x0000_0003 DataVersion: ...
  PartsList: 1 entries
    [1]: 1
```

### 3. Discover Clusters on an Endpoint (`ServerList` and `ClientList`)

Read `ServerList` (`0x0001`) and `ClientList` (`0x0002`) on a specific endpoint
(e.g., Endpoint `1`), or pass the wildcard endpoint **`0xFFFF`** to dump the
cluster list across **all** endpoints at once:

```bash
# Server clusters on Endpoint 1
./out/linux-x64-chip-tool-clang/chip-tool descriptor read server-list 1 1

# Server clusters across ALL endpoints (wildcard endpoint 0xFFFF)
./out/linux-x64-chip-tool-clang/chip-tool descriptor read server-list 1 0xFFFF

# Client clusters across ALL endpoints
./out/linux-x64-chip-tool-clang/chip-tool descriptor read client-list 1 0xFFFF
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

Read `DeviceTypeList` (`0x0000`) on a specific endpoint or across all endpoints
using `0xFFFF`:

```bash
# Device types on Endpoint 1
./out/linux-x64-chip-tool-clang/chip-tool descriptor read device-type-list 1 1

# Device types across ALL endpoints
./out/linux-x64-chip-tool-clang/chip-tool descriptor read device-type-list 1 0xFFFF
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

To inspect the supported attributes, accepted/generated commands, feature map,
and revision of any cluster on an endpoint (e.g., `onoff` on Endpoint `1`):

```bash
# List all supported attribute IDs on the cluster
./out/linux-x64-chip-tool-clang/chip-tool onoff read attribute-list 1 1

# List accepted (client-to-server) command IDs
./out/linux-x64-chip-tool-clang/chip-tool onoff read accepted-command-list 1 1

# List generated (server-to-client) command IDs
./out/linux-x64-chip-tool-clang/chip-tool onoff read generated-command-list 1 1

# Read the FeatureMap bitmask and ClusterRevision
./out/linux-x64-chip-tool-clang/chip-tool onoff read feature-map 1 1
./out/linux-x64-chip-tool-clang/chip-tool onoff read cluster-revision 1 1
```

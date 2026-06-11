---
name: all-devices-app-new-device
description: >-
    Guidelines for adding a new simulated device type to the all-devices-app
    example application. Includes C++ device skeleton definition, device factory
    integration, target expansion mapping, and build test snapshot update.
---

# Adding a New Device to all-devices-app

The `all-devices-app` is a modular application allowing runtime endpoint
composition of different device types. Adding a new simulated device type (e.g.,
`aggregator`, `chime`, or `occupancy-sensor`) requires a coordinated set of
changes across the device implementations, build configuration scripts, the
pluggable factory, and expansion targets.

## Step 0: Plan & Research (Matter Specification)

Before writing any code, you must determine the required device composition,
Device Type ID, and required clusters for the new device type as defined by the
Matter Specification.

1. Refer to the `matter-specification-access` skill to find and read the latest
   version of the Matter Device Library Specification (or Device Type specs).
2. Look up the device type name (e.g. `Aggregator` or `Chime`) in the
   specification to identify:
    - The official **Device Type ID** (e.g., `0x000E` for Aggregator).
    - Mandatory and optional clusters that must be implemented (e.g.,
      `Identify`, `Descriptor`, etc.).
    - Endpoint constraints and relationship rules (e.g., Aggregator's child
      composition).

## Design Principles: Genericity & Dependency Injection

To keep the `all-devices-app` modular, extensible, and clean:

1. **Keep the `devices` directory generic**:
    - Avoid hardcoding platform-specific assumptions, platform-specific APIs, or
      specific application state management inside the core device classes under
      `devices/`.
    - Any hardware interaction or platform services (e.g. playing audio,
      toggling GPIO pins, interacting with network interfaces) must be defined
      using abstract delegate interfaces. The device implementation should only
      interact with these abstract interfaces.
2. **Use Dependency Injection**:
    - Instead of instantiating, managing, or copying global/system dependencies
      (like `TimerDelegate`, custom platform delegates, or hardware drivers)
      inside the device implementation, inject references or pointers to them
      via the constructor (e.g., passing `TimerDelegate &` or a custom delegate
      pointer).
    - This allows platform-specific specializations (e.g. POSIX Audio sound
      player vs Silabs buzzer) to be instantiated and managed externally in the
      main application logic, while keeping the core device class purely generic
      and portable.

---

## Step 1: Implement the Device C++ Class

Every device type is implemented as a standalone target under
`examples/all-devices-app/all-devices-common/devices/`.

### 1. Directory Setup

Create a new subdirectory:

```bash
mkdir -p examples/all-devices-app/all-devices-common/devices/<device-name>
```

### 2. Device Class Definition

Create `<DeviceName>Device.h` inheriting from `SingleEndpointDevice` (or
`EndpointDevice` if managing sub-endpoints).

```cpp
#pragma once

#include <app/clusters/identify-server/IdentifyCluster.h>
#include <data-model-providers/codedriven/CodeDrivenDataModelProvider.h>
#include <devices/interface/SingleEndpointDevice.h>
#include <lib/support/TimerDelegate.h>

namespace chip {
namespace app {

class <DeviceName>Device : public SingleEndpointDevice
{
public:
    <DeviceName>Device(TimerDelegate & timerDelegate);
    ~<DeviceName>Device() override = default;

    CHIP_ERROR Register(chip::EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                        EndpointId parentId = kInvalidEndpointId) override;
    void Unregister(CodeDrivenDataModelProvider & provider) override;

protected:
    TimerDelegate & mTimerDelegate;
    LazyRegisteredServerCluster<Clusters::IdentifyCluster> mIdentifyCluster;
};

} // namespace app
} // namespace chip
```

### 3. Device Class Implementation

Create `<DeviceName>Device.cpp` implementing the registration logic. Ensure that
if any sub-step in registration fails, you gracefully roll back and unregister
to avoid orphaned or partial configurations.

```cpp
#include <devices/Types.h>
#include <devices/<device-name>/<DeviceName>Device.h>
#include <lib/support/CodeUtils.h>

namespace chip {
namespace app {

<DeviceName>Device::<DeviceName>Device(TimerDelegate & timerDelegate) :
    SingleEndpointDevice(Span<const DataModel::DeviceTypeEntry>(&Device::Type::k<DeviceTypeLabel>, 1)),
    mTimerDelegate(timerDelegate)
{}

CHIP_ERROR <DeviceName>Device::Register(chip::EndpointId endpoint, CodeDrivenDataModelProvider & provider, EndpointId parentId)
{
    ReturnErrorOnFailure(SingleEndpointRegistration(endpoint, provider, parentId));

    mIdentifyCluster.Create(Clusters::IdentifyCluster::Config(endpoint, mTimerDelegate));
    ReturnErrorOnFailure(provider.AddCluster(mIdentifyCluster.Registration()));

    return provider.AddEndpoint(mEndpointRegistration);
}

void <DeviceName>Device::Unregister(CodeDrivenDataModelProvider & provider)
{
    SingleEndpointUnregistration(provider);

    if (mIdentifyCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mIdentifyCluster.Cluster()));
        mIdentifyCluster.Destroy();
    }
}

} // namespace app
} // namespace chip
```

### 4. Create the Local `BUILD.gn`

Create
`examples/all-devices-app/all-devices-common/devices/<device-name>/BUILD.gn`:

```gn
# Copyright (c) 2026 Project CHIP Authors
# ... (Standard Apache License Header)

import("//build_overrides/chip.gni")

source_set("<device-name>") {
  sources = [
    "<DeviceName>Device.cpp",
    "<DeviceName>Device.h",
  ]

  public_deps = [
    "${chip_root}/examples/all-devices-app/all-devices-common/devices/interface:single-endpoint-device",
    "${chip_root}/src/data-model-providers/codedriven",
    "${chip_root}/src/lib/core",
    # Add dependency on any specific cluster servers needed (e.g., identify-server)
    "${chip_root}/src/app/clusters/identify-server",
  ]
}
```

---

## Step 2: Build System & Config Integration

To make the new device compile and be selectable, update the common
configuration files:

> [!IMPORTANT] Matter builds enforce strict alphabetical sorting in many list
> configurations. Ensure any insertions respect alphabetical ordering or
> `# keep-sorted` block checks to prevent Restyled CI errors.

### 1. `enabled_devices_config.h.in`

Add the CMake configuration macro:

```c
#cmakedefine01 ALL_DEVICES_ENABLE_<DEVICE_UPPERCASE>
```

### 2. `enabled_devices.gni`

Add the device entry to `_available_devices` (keep sorted alphabetically):

```gn
_available_devices = [
  ...
  [
    "<device-name>",
    "<DEVICE_UPPERCASE>",
  ],
  ...
]
```

### 3. `enabled_devices.cmake`

Add both the implementation file and targets key:

```cmake
set(ALL_DEVICES_DEVICE_SOURCES
    # keep-sorted: start
    ...
    "${ALL_DEVICES_COMMON_DIR}/devices/<device-name>/<DeviceName>Device.cpp"
    ...
)

foreach(_key
        # keep-sorted: start
        ...
        <device-name>
        ...
)
```

### 4. Device Factory Dependency (`device-factory/BUILD.gn`)

Add your new device target dependency to the public dependencies of
`device-factory` (keep sorted alphabetically):

```gn
  public_deps = [
    ":enabled_devices_buildconfig",
    ...
    "${chip_root}/examples/all-devices-app/all-devices-common/devices/<device-name>",
    ...
  ]
```

### 5. Platform Executable Dependencies (`BUILD.gn` files)

Add the target dependency to the relevant platform builds where
`all-devices-app` is built (e.g., `posix/BUILD.gn`, `silabs/BUILD.gn`,
`esp32/BUILD.gn`, `telink/BUILD.gn`):

```gn
# In the platform executable target's deps block
    "${chip_root}/examples/all-devices-app/all-devices-common/devices/<device-name>",
```

---

## Step 3: Register in the Device Factory

Update the pluggable factory
`examples/all-devices-app/all-devices-common/device-factory/DeviceFactory.h` to
allow runtime initialization.

1. Include the new header (keep sorted):
    ```cpp
    #include <devices/<device-name>/<DeviceName>Device.h>
    ```
2. In `InitCreator()`, register the creation lambda:
    ```cpp
    if constexpr (ALL_DEVICES_ENABLE_<DEVICE_UPPERCASE>)
    {
        RegisterCreator("<device-name>", [this]() {
            VerifyOrDie(mContext.has_value());
            return std::make_unique<<DeviceName>Device>(mContext->timerDelegate);
        });
    }
    ```

---

## Step 4: Expand Build Targets & Update Snapshots

To allow building with the device as an expanded variant, update the python
build targets.

### 1. `targets.py`

Open `scripts/build/build/targets.py` and add your device string to
`_ALL_DEVICES_APP_DEVICES` (keep sorted):

```python
_ALL_DEVICES_APP_DEVICES = [
    # keep-sorted: start
    ...
    '<device-name>',
    ...
]
```

### 2. Update Unit Test Snapshot

Because the list of generated build targets is validated against a snapshot
file, updating `targets.py` will cause `scripts/build/test.py` to fail.

To update the snapshot:

1. Run the build scripts tests inside the build environment:
    ```bash
    scripts/run_in_build_env.sh "python3 scripts/build/test.py"
    ```
    This run will fail but generate a `.actual` file containing the updated
    targets list.
2. Overwrite the expected snapshot file with the generated actual file:
    ```bash
    cp all_targets_linux_x64.txt.actual scripts/build/testdata/all_targets_linux_x64.txt
    rm all_targets_linux_x64.txt.actual
    ```
3. Re-run tests to confirm they now pass successfully (`OK`):
    ```bash
    scripts/run_in_build_env.sh "python3 scripts/build/test.py"
    ```

---

## Step 5: Verification & Testing

Verify your new device type integration:

1. **Compile the all-devices-app**:
    ```bash
    scripts/run_in_build_env.sh "./scripts/build/build_examples.py --target linux-x64-all-devices-boringssl build"
    ```
2. **Execute the composition checks**: Activate the python virtual environment:

    ```bash
    source out/venv/bin/activate
    ```

    Run the basic composition python test, instructing the app to spawn your new
    device type on a child endpoint:

    ```bash
    ./scripts/tests/run_python_test.py \
      --factory-reset \
      --app ./out/linux-x64-all-devices-boringssl/all-devices-app \
      --app-args "--device <device-name>:2 --discriminator 1234 --KVS kvs1" \
      --script src/python_testing/TC_DeviceBasicComposition.py \
      --script-args "--storage-path admin_storage.json --commissioning-method on-network --discriminator 1234 --passcode 20202021"
    ```

    Verify that all test cases execute and pass successfully.

3. **Interactive Testing with `chip-tool`**: Refer to the `chip-tool-testing`
   skill to perform manual interactive verification:

    - Build `chip-tool` and start the `all-devices-app` configured with the new
      device type.
    - Commission the all-devices-app and send read/write attribute requests or
      commands to confirm functional cluster operations.

4. **Identify and Run Integration/Certification Tests**: Identify existing
   Python integration tests in `src/python_testing/` or YAML test cases in
   `src/app/tests/suites/` that match the clusters or device behavior you
   implemented.
    - Search `src/python_testing/` for tests matching the device functionality
      or specific clusters (e.g. `TC_CHIME_*.py` or
      `TC_DeviceBasicComposition.py`).
    - Run these integration tests using the virtual environment to ensure the
      new device type passes certification checks.

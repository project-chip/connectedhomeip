# How to Add a New Simulated Device

This guide explains how to implement a simulated Matter device in the
`all-devices-app`.

We will walk through creating a device named `MySensor` (`--device my-sensor`),
implementing its data model, registering it in the modular factory, expanding
build targets, and executing certification tests.

---

## Step 0: Plan & Research (Matter Specification)

Before writing any code, consult the latest Matter Specification (specifically
the Matter Device Library Specification) to determine:

-   **Device Type ID**: The official hex identifier for your device (e.g.,
    `0x0302` for Temperature Sensor, `0x000E` for Aggregator).
-   **Mandatory & Optional Clusters**: The exact server clusters that must be
    instantiated to comply with the device type definition (e.g., `Identify`,
    `Descriptor`, `Temperature Measurement`).
-   **Endpoint Constraints & Composition Rules**: Rules governing tree
    composition, such as whether the device type acts as a root endpoint or
    requires specific child sub-endpoints.

---

## Design Principles: Generic Structure & Dependency Injection

To ensure the `all-devices-app` remains highly portable, modular, and testable,
adhere strictly to these architectural patterns:

1. **Keep the `devices/` Directory Highly Generic**:

    - Avoid injecting platform-specific code, RTOS dependencies, or direct
      application state management inside core device classes.
    - Any hardware interactions or platform services (e.g., playing audio,
      toggling GPIO pins, interacting with network interfaces) must be
      abstracted behind pure virtual delegate interfaces.

2. **Strict Dependency Injection**:
    - Do not instantiate, manage, or copy global system dependencies (such as
      `TimerDelegate`, custom platform delegates, or hardware drivers) inside
      the device implementation.
    - Inject all required dependencies via the device constructor. This allows
      platform-specific specializations to be instantiated and managed
      externally in the application's main setup, keeping core device classes
      100% generic.

---

## Step 1: Create the Device Implementation

Create a standalone directory in `all-devices-common/devices/` for your
implementation, e.g., `all-devices-common/devices/my-sensor/`.

### The Header (`MySensorDevice.h`)

Derive your class from `SingleEndpointDevice` (or `EndpointDevice` if managing
sub-endpoints). Encapsulate your Code-Driven server clusters using
`LazyRegisteredServerCluster` and inject any required delegates via the
constructor.

```cpp
#pragma once

#include <app/clusters/identify-server/IdentifyCluster.h>
#include <app/clusters/my-sensor-server/MySensorServerCluster.h> // Example code-driven cluster
#include <data-model-providers/codedriven/CodeDrivenDataModelProvider.h>
#include <devices/interface/SingleEndpointDevice.h>
#include <lib/support/TimerDelegate.h>

namespace chip::app {

class MySensorDevice : public SingleEndpointDevice
{
public:
    MySensorDevice(TimerDelegate & timerDelegate);
    ~MySensorDevice() override = default;

    // DeviceInterface pure virtual lifecycle hooks
    CHIP_ERROR Register(chip::EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                        EndpointId parentId = kInvalidEndpointId) override;
    void Unregister(CodeDrivenDataModelProvider & provider) override;

    Clusters::MySensorCluster & MySensorCluster();

protected:
    TimerDelegate & mTimerDelegate;
    LazyRegisteredServerCluster<Clusters::IdentifyCluster> mIdentifyCluster;
    LazyRegisteredServerCluster<Clusters::MySensorCluster> mMySensorCluster;
};

} // namespace chip::app
```

### The Source (`MySensorDevice.cpp`)

Implement `Register()` to instantiate dynamic clusters and register them with
the provider. Ensure robust error handling: if any registration step fails,
gracefully roll back to prevent orphaned configurations. Implement
`Unregister()` to destroy them cleanly.

```cpp
#include "MySensorDevice.h"
#include <devices/Types.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip::app::Clusters;

namespace chip::app {

MySensorDevice::MySensorDevice(TimerDelegate & timerDelegate) :
    SingleEndpointDevice(Span<const DataModel::DeviceTypeEntry>(&Device::Type::kMySensor, 1)),
    mTimerDelegate(timerDelegate)
{}

CHIP_ERROR MySensorDevice::Register(chip::EndpointId endpoint, CodeDrivenDataModelProvider & provider, EndpointId parentId)
{
    // 1. Complete base single-endpoint registration
    ReturnErrorOnFailure(SingleEndpointRegistration(endpoint, provider, parentId));

    // 2. Instantiate and register common Identify cluster
    mIdentifyCluster.Create(IdentifyCluster::Config(endpoint, mTimerDelegate));
    ReturnErrorOnFailure(provider.AddCluster(mIdentifyCluster.Registration()));

    // 3. Instantiate and register our strongly-typed code-driven cluster
    mMySensorCluster.Create(endpoint);
    ReturnErrorOnFailure(provider.AddCluster(mMySensorCluster.Registration()));

    // 4. Register the endpoint with the Data Model Provider
    return provider.AddEndpoint(mEndpointRegistration);
}

void MySensorDevice::Unregister(CodeDrivenDataModelProvider & provider)
{
    SingleEndpointUnregistration(provider);
    if (mMySensorCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mMySensorCluster.Cluster()));
        mMySensorCluster.Destroy();
    }
    if (mIdentifyCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mIdentifyCluster.Cluster()));
        mIdentifyCluster.Destroy();
    }
}

Clusters::MySensorCluster & MySensorDevice::MySensorCluster()
{
    VerifyOrDie(mMySensorCluster.IsConstructed());
    return mMySensorCluster.Cluster();
}

} // namespace chip::app
```

### The GN Build (`BUILD.gn`)

Create `all-devices-common/devices/my-sensor/BUILD.gn` to define your standalone
source set:

```text
# Copyright (c) 2026 Project CHIP Authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import("//build_overrides/chip.gni")

source_set("my-sensor") {
  sources = [
    "MySensorDevice.cpp",
    "MySensorDevice.h",
  ]

  public_deps = [
    "${chip_root}/examples/all-devices-app/all-devices-common/devices/interface:single-endpoint-device",
    "${chip_root}/src/app/clusters/identify-server",
    # Add public_deps for your specific cluster servers here
    "${chip_root}/src/data-model-providers/codedriven",
    "${chip_root}/src/lib/core",
  ]
}
```

---

## Step 2: Register the Capability in `DeviceFactory`

To enable runtime initialization via the `--device my-sensor` CLI flag, register
your device creation callback in
`all-devices-common/device-factory/DeviceFactory.h`.

1. **Include your Header** (keep the include list sorted alphabetically):

    ```cpp
    #include <devices/my-sensor/MySensorDevice.h>
    ```

2. **Register the Creator** inside `InitCreator()`:
    ```cpp
    if constexpr (ALL_DEVICES_ENABLE_MY_SENSOR)
    {
        RegisterCreator("my-sensor", [this]() {
            VerifyOrDie(mContext.has_value());
            return std::make_unique<MySensorDevice>(mContext->timerDelegate);
        });
    }
    ```
    _Note: Extract any needed runtime dependencies (timers, storage, group data
    providers) from `mContext` and inject them directly into your constructor._

---

## Step 3: Register Files in Build Configurations

Register your new device files and macros across the core build configurations.

> [!IMPORTANT] Matter builds enforce strict alphabetical sorting in many list
> configurations. Ensure any insertions respect alphabetical ordering or
> `# keep-sorted` block checks to prevent automated CI failures.

### 1. Macro Template (`all-devices-common/device-factory/enabled_devices_config.h.in`)

Add the compile-time CMake hook:

```c
#cmakedefine01 ALL_DEVICES_ENABLE_MY_SENSOR
```

### 2. GN Configuration (`all-devices-common/device-factory/enabled_devices.gni`)

Add the device entry to `_available_devices` (keep sorted):

```text
_available_devices = [
  ...
  [
    "my-sensor",
    "MY_SENSOR",
  ],
  ...
]
```

### 3. CMake Configuration (`all-devices-common/device-factory/enabled_devices.cmake`)

Add your `.cpp` source file to `ALL_DEVICES_DEVICE_SOURCES` (keep sorted):

```cmake
set(ALL_DEVICES_DEVICE_SOURCES
    # keep-sorted: start
    ...
    "${ALL_DEVICES_COMMON_DIR}/devices/my-sensor/MySensorDevice.cpp"
    ...
    # keep-sorted: end
)
```

Also add your key to the activation loop in the same file (keep sorted):

```cmake
foreach(_key
        # keep-sorted: start
        ...
        my-sensor
        ...
        # keep-sorted: end
    )
```

### 4. Device Factory Dependency (`all-devices-common/device-factory/BUILD.gn`)

<<<<<<< conflict 2 of 3 +++++++ lntxykqw 2a6a8351 "Fix misspell CI check
failures in all-devices-app documentation" (rebase destination)

Add your new device target to the `public_deps` of `device-factory` (keep
sorted):

````gn
```text
  public_deps = [
    ...
    "${chip_root}/examples/all-devices-app/all-devices-common/devices/my-sensor",
    ...
  ]
````

### 5. Platform Executable Dependencies (`BUILD.gn` files)

<<<<<<< conflict 3 of 3 +++++++ lntxykqw 2a6a8351 "Fix misspell CI check
failures in all-devices-app documentation" (rebase destination)

Add the target dependency to the relevant platform executable targets where
`all-devices-app` is built (e.g., `examples/all-devices-app/posix/BUILD.gn`, and
embedded platform builds such as Silicon Labs or ESP32):

````gn
```text
    "${chip_root}/examples/all-devices-app/all-devices-common/devices/my-sensor",
````

---

## Step 4: Expand Build Targets & Update Snapshots

To support building automated variants of the application with your device
enabled by default, expand the Python build targets.

### 1. `targets.py`

Open `scripts/build/build/targets.py` and append your device string to
`_ALL_DEVICES_APP_DEVICES` (keep sorted):

```python
_ALL_DEVICES_APP_DEVICES = [
    # keep-sorted: start
    ...
    'my-sensor',
    ...
]
```

### 2. Update Unit Test Snapshots

Because the list of generated build targets is verified against a golden
snapshot file, modifying `targets.py` will cause `scripts/build/test.py` to
fail. Follow this exact execution recipe to update the snapshot:

1. Run the build script tests within the activated build environment (this run
   will fail but generate an actual output file):
    ```bash
    scripts/run_in_build_env.sh "python3 scripts/build/test.py"
    ```
2. Overwrite the golden snapshot file with the newly generated actual file:
    ```bash
    cp all_targets_linux_x64.txt.actual scripts/build/testdata/all_targets_linux_x64.txt
    rm all_targets_linux_x64.txt.actual
    ```
3. Re-run the tests to confirm they now pass successfully (`OK`):
    ```bash
    scripts/run_in_build_env.sh "python3 scripts/build/test.py"
    ```

---

## Step 5: End-to-End Verification & Certification Testing

Fully validate your new device integration using end-to-end testing tools.

### 1. Compile the Application

Build the executable using the standard Python build scripts:

```bash
scripts/run_in_build_env.sh "./scripts/build/build_examples.py --target linux-x64-all-devices-clang build"
```

### 2. Execute Basic Composition Verification

Activate the automated test virtual environment and run the core device basic
composition certification test. This instructs the binary to dynamically spawn
your new device on a child endpoint:

```bash
source out/venv/bin/activate

./scripts/tests/run_python_test.py \
  --factory-reset \
  --app ./out/linux-x64-all-devices-clang/all-devices-app \
  --app-args "--device my-sensor:2 --discriminator 1234 --KVS kvs1" \
  --script src/python_testing/TC_DeviceBasicComposition.py \
  --script-args "--storage-path admin_storage.json --commissioning-method on-network --discriminator 1234 --passcode 20202021"
```

Confirm that all test cases execute and pass successfully.

### 3. Interactive Testing with `chip-tool`

For manual interactive validation, execute `chip-tool` (refer to the
`chip-tool-testing` skill for detailed commissioning setups):

1. Start your application configured with your new device:
    ```bash
    ./out/linux-x64-all-devices-clang/all-devices-app --device my-sensor
    ```
2. Commission the app and execute interaction model read/write commands or
   custom cluster commands against your new device endpoint to verify functional
   data model operations.

### 4. Run Dedicated Cluster Certification Suites

-   Identify existing Python integration tests in `src/python_testing/` or YAML
    test cases in `src/app/tests/suites/` that match the specific clusters
    implemented by your device.
-   Execute those dedicated test scripts via `run_python_test.py` to guarantee
    full compliance with the Matter Specification.

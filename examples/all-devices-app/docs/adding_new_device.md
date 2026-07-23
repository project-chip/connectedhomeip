# How to Add a New Simulated Device

This guide explains how to implement a simulated Matter device in the
`all-devices-app`.

We will walk through creating a device named `MySensor` (`--device my-sensor`),
implementing its data model, registering it in the device factory, expanding
build targets, and executing certification tests.

---

## Step 0: Research Specification Requirements

1. **Matter Device Library Specification (Primary)**: Reference the latest CSA
   Matter spec for authoritative next-version requirements.
2. **Local XML Reference (Secondary)**: Cross-reference
   `data_model/{version}/device_types/{DeviceType}.xml` for quick lookup. Note:
   XMLs are static snapshots of a certifiable spec version; they may need
   augmentation for next-version features.
3. **Endpoint Support Constraint**: Endpoints must contain at least one
   functional, domain-specific server cluster. 0-cluster endpoints are
   disallowed.
4. **Identify Key Requirements**:
    - **Device Type ID**: Hex identifier (e.g., `0x0302` for Temperature
      Sensor).
    - **Mandatory & Optional Clusters**: Server clusters required to comply.
    - **Endpoint Constraints / Composition Rules**: Tree composition rules.
5. **AI Agent Assistant**: If using an AI coding agent, it can use the
   [spec access skill](../../../.agents/skills/matter-specification-access/SKILL.md)
   to retrieve relevant sections of the specification.

---

## Step 0.5: Test Plan & Test Discovery

1. **Locate Test Plans**: Reference the private `chip-test-plans` repository
   (accessible to CSA members) to understand cluster verification steps.
2. **Locate Integration Tests**: Search `src/python_testing/` for
   `TC_{CLUSTER}_*.py` or `src/app/tests/suites/` for YAML tests corresponding
   to the implemented clusters.
3. **Plan CI Runs**: Plan to enable a subset of cluster tests in CI. This is
   done by adding run configurations to the `=== BEGIN CI TEST ARGUMENTS ===`
   block of the python script or to the YAML test suites, passing
   `--device {your-device}`.

---

## Architectural Best Practices

When implementing your device, keep these key architectural patterns in mind to
make your code compile-time safe, highly reusable across different platforms,
and easy to test:

1. **Abstract Hardware Interactions**:

    - Build your core device class so it doesn't depend on specific RTOS or
      platform-specific libraries.
    - Abstract hardware-specific actions (like toggling LED pins or playing
      sound) behind pure virtual delegate interfaces. This allows contributors
      to reuse your exact device behavior on their specific target boards.

2. **Prefer References (`Delegate &`) for Mandatory Delegates**:

    - If a delegate is mandatory for a cluster's core functionality (e.g.,
      `OnOffDelegate` for a light, or `IdentifyDelegate` for a device), always
      require it as a **C++ reference (`&`)** in the constructor.
    - This enforces at compile-time that a valid, functional handler must be
      provided, completely eliminating the risk of runtime null-pointer crashes
      or devices that silently ignore commands.
    - Use optional pointers (`Delegate *` defaulting to `nullptr`) only for
      clusters that are optional based on feature flags or configuration.

3. **Expose Public Cluster Getters**:

    - Always provide public C++ getter methods (e.g., `OnOffCluster()`,
      `IdentifyCluster()`) to expose the underlying cluster instances.
    - This allows local application code (such as physical button drivers,
      platform leaf classes, shell commands, or Out-of-Band accessors) to
      programmatically read, write, or modify the device state.

4. **Symmetrical Logging Mocks (`impl/` subfolder)**:

    - For every base device class, implement a corresponding
      "logging-by-default" mock subclass in a dedicated `impl/` subfolder (e.g.,
      `LoggingMySensorDevice`).
    - Use the **self-delegate pattern** where the mock subclass inherits from
      both the base device class and the delegate interfaces, implements the
      callbacks to log to the console, and passes `*this` to the base
      constructor.
    - This makes the mock completely self-contained and ready to be instantiated
      by `DeviceFactory` or unit tests without requiring external wiring.

5. **Spec-Pure Directory Layout & Extracted Capabilities**:
    - Keep the root of the `devices/` directory pure: it should contain _only_
      real, spec-defined Matter Device Types (like `dimmable-light`, `fan`,
      `air-purifier`).
    - **Avoid non-spec inheritance** (such as making a plug-in unit inherit from
      a light) just to reuse code.
    - If multiple device types share a common capability (like dimming or air
      circulation), extract it into an abstract capability base class under
      `device/capabilities/<capability-name>/` (e.g.,
      `device/capabilities/dimmable-load/`). Concrete leaf devices then inherit
      publicly from this capability base.

---

### The Header (`MySensorDevice.h`)

Derive your class from `SingleEndpointDevice` (or `EndpointDevice` if managing
sub-endpoints). Require all mandatory delegates as references in the constructor
and declare public getters to expose the underlying clusters:

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
    MySensorDevice(TimerDelegate & timerDelegate, Clusters::IdentifyDelegate & identifyDelegate);
    ~MySensorDevice() override = default;

    // DeviceInterface pure virtual lifecycle hooks
    CHIP_ERROR Register(chip::EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                        EndpointComposition composition = {}) override;
    void Unregister(CodeDrivenDataModelProvider & provider) override;

    // Public cluster getters for programmatic control
    Clusters::IdentifyCluster & IdentifyCluster() { return mIdentifyCluster.Cluster(); }
    Clusters::MySensorCluster & MySensorCluster() { return mMySensorCluster.Cluster(); }

private:
    TimerDelegate & mTimerDelegate;
    Clusters::IdentifyDelegate & mIdentifyDelegate;

    LazyRegisteredServerCluster<Clusters::IdentifyCluster> mIdentifyCluster;
    LazyRegisteredServerCluster<Clusters::MySensorCluster> mMySensorCluster;
};

} // namespace chip::app
```

### The Source (`MySensorDevice.cpp`)

In `Register()`, wire up your mandatory delegates using the `.WithDelegate()`
helper when creating the cluster instances:

```cpp
#include "MySensorDevice.h"
#include <devices/Types.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip::app::Clusters;

namespace chip::app {

MySensorDevice::MySensorDevice(TimerDelegate & timerDelegate, Clusters::IdentifyDelegate & identifyDelegate) :
    SingleEndpointDevice(Span<const DataModel::DeviceTypeEntry>(&Device::Type::kMySensor, 1)),
    mTimerDelegate(timerDelegate), mIdentifyDelegate(identifyDelegate)
{}

CHIP_ERROR MySensorDevice::Register(chip::EndpointId endpoint, CodeDrivenDataModelProvider & provider, EndpointComposition composition)
{
    VerifyOrReturnError(mEndpointId == kInvalidEndpointId, CHIP_ERROR_INCORRECT_STATE);
    DeviceRegistrationTransaction transaction(*this, provider);

    ReturnErrorOnFailure(RegisterDescriptor(endpoint, provider, composition));

    // Wire up the mandatory identify delegate
    mIdentifyCluster.Create(IdentifyCluster::Config(endpoint, mTimerDelegate).WithDelegate(&mIdentifyDelegate));
    ReturnErrorOnFailure(provider.AddCluster(mIdentifyCluster.Registration()));

    mMySensorCluster.Create(endpoint);
    ReturnErrorOnFailure(provider.AddCluster(mMySensorCluster.Registration()));

    ReturnErrorOnFailure(provider.AddEndpoint(mEndpointRegistration));
    transaction.Commit();
    return CHIP_NO_ERROR;
}

void MySensorDevice::Unregister(CodeDrivenDataModelProvider & provider)
{
    UnregisterDescriptor(provider);
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

} // namespace chip::app
```

### The Symmetrical Logging Mock (`impl/LoggingMySensorDevice.h` & `.cpp`)

To provide a self-contained simulator variant ready for `DeviceFactory`,
implement the self-delegate logging mock under the `impl/` subfolder:

#### Header (`impl/LoggingMySensorDevice.h`)

```cpp
#pragma once

#include <devices/my-sensor/MySensorDevice.h>
#include <lib/support/logging/CHIPLogging.h>

namespace chip::app {

class LoggingMySensorDevice : public MySensorDevice, public Clusters::IdentifyDelegate
{
public:
    explicit LoggingMySensorDevice(TimerDelegate & timerDelegate) :
        MySensorDevice(timerDelegate, *this)
    {}
    ~LoggingMySensorDevice() override = default;

    // IdentifyDelegate implementation
    void OnIdentifyStart(Clusters::IdentifyCluster & cluster) override
    {
        ChipLogProgress(DeviceLayer, "MySensor: OnIdentifyStart");
    }
    void OnIdentifyStop(Clusters::IdentifyCluster & cluster) override
    {
        ChipLogProgress(DeviceLayer, "MySensor: OnIdentifyStop");
    }
    void OnTriggerEffect(Clusters::IdentifyCluster & cluster) override
    {
        ChipLogProgress(DeviceLayer, "MySensor: OnTriggerEffect");
    }
    bool IsTriggerEffectEnabled() const override { return true; }
};

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

## Step 2: Register the Device Type in `DeviceFactory`

To enable runtime initialization via the `--device my-sensor` CLI flag, register
your self-contained **logging mock** in
`all-devices-common/device-factory/DeviceFactory.h`.

1. **Include your Logging Mock Header** (keep the include list sorted
   alphabetically):

    ```cpp
    #include <devices/my-sensor/impl/LoggingMySensorDevice.h>
    ```

2. **Register the Creator** inside the `DeviceFactory` constructor:
    ```cpp
    if constexpr (ALL_DEVICES_ENABLE_MY_SENSOR)
    {
        RegisterCreator("my-sensor", [this]() {
            VerifyOrDie(mContext.has_value());
            return std::make_unique<LoggingMySensorDevice>(mContext->timerDelegate);
        });
    }
    ```
    _Note: We instantiate the Logging mock version in the factory so it runs
    completely out-of-the-box with self-contained console logging, keeping the
    platform main clean._

---

## Step 3: Register Files in Build Configurations

Next, register your implementation files across our core build scripts.

> [!NOTE] To keep project lists clean, our build systems enforce alphabetical
> sorting. Please insert your new device entries in alphabetical order to ensure
> automated CI formatting checks pass successfully.

### 1. Macro Template (`all-devices-common/device-factory/enabled_devices_config.h.in`)

Add the compile-time CMake hook:

```text
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

Add your new device target to the `public_deps` of `device-factory` (keep
sorted):

```text
  public_deps = [
    ...
    "${chip_root}/examples/all-devices-app/all-devices-common/devices/my-sensor",
    ...
  ]
```

### 5. Platform Executable Dependencies (`BUILD.gn` files)

Add the target dependency to the relevant platform executable targets where
`all-devices-app` is built (e.g., `examples/all-devices-app/posix/BUILD.gn`, and
embedded platform builds such as Silicon Labs or ESP32):

```text
    "${chip_root}/examples/all-devices-app/all-devices-common/devices/my-sensor",
```

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

### 2. Update Build Test Golden Snapshots

Our automated PR workflows verify that build variants match a golden snapshot.
Updating `targets.py` will temporarily cause `scripts/build/test.py` to fail
until you regenerate this golden file:

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
source scripts/activate.sh
./scripts/build/build_examples.py --target linux-x64-all-devices-clang build
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

1. Compile `chip-tool` from source:

    ```bash
    source scripts/activate.sh
    ./scripts/build/build_examples.py --target linux-x64-chip-tool-clang build
    ```

2. Start your application configured with your new device:

    ```bash
    ./out/linux-x64-all-devices-clang/all-devices-app --device my-sensor
    ```

3. Commission the app and execute interaction model read/write commands or
   custom cluster commands against your new device endpoint to verify functional
   data model operations.

### 4. Run Dedicated Cluster Certification Suites

-   Identify existing Python integration tests in `src/python_testing/` or YAML
    test cases in `src/app/tests/suites/` that match the specific clusters
    implemented by your device.
-   Execute those dedicated test scripts via `run_python_test.py` to guarantee
    full compliance with the Matter Specification.

---

## Step 6: Update Implementation Status Documentation

After implementing and registering your device, manually update the tracking
documentation:

1. **`supported_device_types.md`**: Move your device type entry from the
   **Unimplemented** table to the **Implemented** table, and increment the total
   count.
2. **`supported_clusters.md`**: If your device introduced support for a new
   cluster, update its **Used in All-Devices** status to `Yes` and list your
   device type in the **Notes/Devices** column.
3. **`README.md`**: Update the **Supported Devices** list and the `--device` CLI
   help options example block under the **Running the Application** section.

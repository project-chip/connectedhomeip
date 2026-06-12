# How to Add a New Simulated Device

This guide shows how to implement a simulated Matter device in the
`all-devices-app`.

We will create an example device named `MySensor` (`--device my-sensor`),
implement it, and register it in the build configurations.

---

## Step 1: Create the Device Implementation

Create a new directory in `all-devices-common/devices/` for your implementation,
e.g., `all-devices-common/devices/my-sensor/`.

### The Header (`MySensorDevice.h`)

Derive your class from `SingleEndpointDevice` and encapsulate your Code-Driven
server clusters using `LazyRegisteredServerCluster`.

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

Implement `Register()` to create dynamic clusters and register them with the
provider, and `Unregister()` to destroy them.

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

---

## Step 2: Register the Capability in `DeviceFactory`

To enable the `--device my-sensor` CLI flag, register its creator callback in
`all-devices-common/device-factory/DeviceFactory.h`.

1. **Include your Header**:

    ```cpp
    #include <devices/my-sensor/MySensorDevice.h>
    ```

2. **Add the Initialization Block** inside the `DeviceFactory()` constructor:
    ```cpp
    if constexpr (ALL_DEVICES_ENABLE_MY_SENSOR)
    {
        RegisterCreator("my-sensor", [this]() {
            VerifyOrDie(mContext.has_value());
            return std::make_unique<MySensorDevice>(mContext->timerDelegate);
        });
    }
    ```
    _Note: If your device requires timers, storage, or group data providers,
    extract them from `mContext` and inject them into your constructor._

---

## Step 3: Register Files in the Build Configurations

Register source files and macros in the build configurations.

### 1. GN Build (`all-devices-common/devices/my-sensor/BUILD.gn`)

Create a `BUILD.gn` file for your device source set and ensure it is referenced
upstream by `all-devices-common/device-factory/BUILD.gn`.

### 2. CMake Configuration (`all-devices-common/device-factory/enabled_devices.cmake`)

Add your `.cpp` source file to `ALL_DEVICES_DEVICE_SOURCES`:

```cmake
set(ALL_DEVICES_DEVICE_SOURCES
    # keep-sorted: start
    ...
    "${ALL_DEVICES_COMMON_DIR}/devices/my-sensor/MySensorDevice.cpp"
    ...
    # keep-sorted: end
)
```

Also add your device configuration name to the activation loop inside the same
file:

```cmake
foreach(_key
        # keep-sorted: start
        ...
        my-sensor
        ...
        # keep-sorted: end
    )
```

### 3. GN Configuration (`all-devices-common/device-factory/enabled_devices.gni`)

Keep the macro definition structure exactly synchronized with the CMake loop by
updating `_available_devices`:

```gni
_available_devices = [
  ...
  [
    "my-sensor",
    "MY_SENSOR",
  ],
  ...
]
```

### 4. Macro Template (`all-devices-common/device-factory/enabled_devices_config.h.in`)

Add the compile-time `#cmakedefine01` hook so `enabled_devices.h` exposes the
boolean:

```c
#cmakedefine01 ALL_DEVICES_ENABLE_MY_SENSOR
```

---

## Step 4: Verification & Testing

Recompile the application to verify the new device registers:

```bash
source scripts/activate.sh
./scripts/build/build_examples.py --target linux-x64-all-devices-clang build
```

Run the executable with the new device flag:

```bash
./out/linux-x64-all-devices-clang/all-devices-app --device my-sensor
```

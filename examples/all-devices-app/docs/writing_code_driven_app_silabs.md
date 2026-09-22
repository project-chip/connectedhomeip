# Writing a Code-Driven Application: Silicon Labs (GN)

This guide accompanies
[Writing a Code-Driven Application](writing_code_driven_app.md) and describes
the specific changes required in
[`examples/all-devices-app/silabs/`](../silabs/) to convert the multi-device
simulator into a single-device application.

---

## 1. GN Build Deltas ([`silabs/BUILD.gn`](../silabs/BUILD.gn))

Use [`silabs/BUILD.gn`](../silabs/BUILD.gn) as the starting template and apply
the following changes:

1. **Remove Simulator Device Lists & Factory**:
    - Delete the `import(".../device-factory/enabled_devices.gni")` statement.
    - Remove
      `"${chip_root}/examples/all-devices-app/all-devices-common/device-factory"`
      from `deps`.
    - Remove `DeviceShellCommands.cpp`, `DeviceShellCommands.h`, and `AppKeys.h`
      from `sources` (used only for the `devtype` KVS shell switch).
2. **Keep Only Target Device & Root Node Dependencies**:
    - Keep the existing `root-node:wifi`, `root-node:thread`, and
      `root-node:ota` conditional blocks in
      [`silabs/BUILD.gn`](../silabs/BUILD.gn) unchanged.
    - From the list of `all-devices-common/device/types/*` dependencies, keep
      **only** the base device target for your product (for example,
      `all-devices-common/device/types/dimmable-light`) and add your product
      device source files:

```gn
  sources += [
    "include/MyProductDimmableLight.h",
    "src/MyProductDimmableLight.cpp",
  ]

  deps = [
    ":sdk",
    "${chip_root}/src/platform/logging:default",
    "${chip_root}/src/data-model-providers/codedriven",

    # Single base device type for Endpoint 1 (instead of device-factory + all device types)
    "${chip_root}/examples/all-devices-app/all-devices-common/device/types/dimmable-light",

    "${chip_root}/zzz_generated/app-common/devices",
  ]
```

---

## 2. Entrypoint Deltas ([`silabs/src/AppTask.cpp`](../silabs/src/AppTask.cpp))

In [`silabs/src/AppTask.cpp`](../silabs/src/AppTask.cpp),
`AppTask::InitCodeDrivenDataModel()` already performs three steps:

1. Initializes `DefaultAttributePersistenceProvider`,
   `DefaultSafeAttributePersistenceProvider`, and `CodeDrivenDataModelProvider`.
2. Populates `RootNode::Context` and constructs `sRootNode` using
   `RootNodeWith<ThreadFeature, OtaFeature>` (or `WifiFeature` depending on
   build flags), then registers it on `kRootEndpointId` (0).
3. Initializes `NoHooksDeviceFactory` and instantiates devices from
   `ALL_DEVICES_DEFAULT_DEVICES` or the NVM3/KVS key `kDeviceTypeKey`.

### What to Change

-   **Keep steps 1 and 2 unchanged** (up through `sRootNode->Register(...)`).
-   **Remove `#include <app_config/enabled_devices.h>` and
    `#include <device-factory/DeviceFactory.h>`**.
-   **Replace step 3** (`NoHooksDeviceFactory::GetInstance().Init(...)` through
    the end of `InitCodeDrivenDataModel()`) with direct instantiation and
    registration of your product device on `kDeviceEndpointId`
    (`EndpointId(1)`):

```cpp
    // Keep existing sRootNode construction and registration above this line
    ReturnErrorOnFailure(sRootNode->Register(rootAllocator, *sDataModelProvider));

    // Replace NoHooksDeviceFactory and KVS lookup with static device registration:
    sProductDevice = std::make_unique<chip::app::MyProductDimmableLight>(chip::app::MyProductDimmableLight::Context{
        .groupDataProvider = *groupDataProvider,
        .fabricTable       = chip::Server::GetInstance().GetFabricTable(),
        .timerDelegate     = sTimerDelegate,
        .identifyDelegate  = sIdentifyDelegate,
    });
    VerifyOrReturnError(sProductDevice != nullptr, CHIP_ERROR_NO_MEMORY);
    return sProductDevice->Register(kDeviceEndpointId, *sDataModelProvider);
```

# Writing a Code-Driven Application: Silicon Labs (GN)

Build and entrypoint deltas for converting
[`examples/all-devices-app/silabs/`](../silabs/) into a single-device product
application. See [Writing a Code-Driven Application](writing_code_driven_app.md)
for architecture and device class implementation.

---

## 1. GN Build Deltas ([`silabs/BUILD.gn`](../silabs/BUILD.gn))

-   **Delete**:
    -   `import(".../device-factory/enabled_devices.gni")`
    -   `"${chip_root}/examples/all-devices-app/all-devices-common/device-factory"`
        and unused `all-devices-common/device/types/*` entries from `deps`
    -   `include/AppKeys.h`, `include/DeviceShellCommands.h`, and
        `src/DeviceShellCommands.cpp` from `sources`
-   **Keep**:
    -   `root-node:wifi`, `root-node:thread`, and `root-node:ota` conditional
        blocks
    -   Device info provider targets (`all-devices-example-device-info-provider`
        and `all-devices-example-device-instance-info-provider`, or replace with
        production factory providers)
-   **Add**:
    -   Product device source files and the single base device target (e.g.,
        `device/types/speaker`):

```text
  sources += [
    "include/MyProductSpeaker.h",
    "src/MyProductSpeaker.cpp",
  ]

  deps = [
    ":sdk",
    "${chip_root}/src/platform/logging:default",
    "${chip_root}/src/data-model-providers/codedriven",
    "${chip_root}/examples/all-devices-app/all-devices-common/device/types/speaker",
    "${chip_root}/examples/all-devices-app/all-devices-common/providers:all-devices-example-device-info-provider",
    "${chip_root}/examples/all-devices-app/all-devices-common/providers:all-devices-example-device-instance-info-provider",
    "${chip_root}/zzz_generated/app-common/devices",
  ]
```

---

## 2. Entrypoint Deltas ([`silabs/src/AppTask.cpp`](../silabs/src/AppTask.cpp))

-   **Keep**:
    -   Persistence initialization, `sDataModelProvider` creation, and
        `sRootNode` (`RootNodeWith<...>`) construction and registration on
        `kRootEndpointId` (`0`) in `AppTask::InitCodeDrivenDataModel()`.
-   **Delete**:
    -   `#include "AppKeys.h"`, `#include <DeviceShellCommands.h>`,
        `#include <app_config/enabled_devices.h>`, and
        `#include <device-factory/DeviceFactory.h>`
    -   `chip::Shell::DeviceCommands::GetInstance().Register()` in
        `AppTask::AppInit()`
    -   `kMaxConstructedDevices` / `sConstructedDevices` (replace with
        `std::unique_ptr<chip::app::MyProductSpeaker> sProductDevice;`)
-   **Replace**:
    -   Replace the `NoHooksDeviceFactory::GetInstance().Init(...)` and KVS
        lookup block after `sRootNode->Register(...)` with direct registration
        on `kDeviceEndpointId` (`EndpointId(1)`):

```cpp
    ReturnErrorOnFailure(sRootNode->Register(rootAllocator, *sDataModelProvider));

    sProductDevice = std::make_unique<chip::app::MyProductSpeaker>(sTimerDelegate);
    VerifyOrReturnError(sProductDevice != nullptr, CHIP_ERROR_NO_MEMORY);
    return sProductDevice->Register(kDeviceEndpointId, *sDataModelProvider);
```

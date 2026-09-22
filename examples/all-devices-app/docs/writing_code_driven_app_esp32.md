# Writing a Code-Driven Application: ESP32 (CMake)

Build and entrypoint deltas for converting
[`examples/all-devices-app/esp32/`](../esp32/) into a single-device product
application. See [Writing a Code-Driven Application](writing_code_driven_app.md)
for architecture and device class implementation.

---

## 1. CMake Build Deltas ([`esp32/main/CMakeLists.txt`](../esp32/main/CMakeLists.txt))

-   **Delete from `esp32/main/CMakeLists.txt`**:
    -   `include("${ALL_DEVICES_COMMON_DIR}/device-factory/enabled_devices.cmake")`
    -   `${ALL_DEVICES_EXTRA_INCLUDE_DIRS}`, `${ALL_DEVICES_DEVICE_SOURCES}`,
        and `${ALL_DEVICES_CLUSTER_SOURCES}`
-   **Delete simulator files from `esp32/main/`** (since
    `idf_component_register` compiles all `.cpp` files in
    `SRC_DIRS "${CMAKE_CURRENT_LIST_DIR}"`):
    -   `DeviceFactoryPlatformOverride.cpp`, `DeviceFactoryPlatformOverride.h`,
        `DeviceShellCommands.cpp`, `DeviceShellCommands.h`,
        `DeviceTypeSelection.h`, and `AppDeviceFactory.h`
-   **Append required base and device sources to `APP_TOPLEVEL_EXTRA_SRCS`**
    (note that `config/esp32/components/chip` already compiles the SDK cluster
    servers into `libCHIP.a`):

```cmake
list(APPEND APP_TOPLEVEL_EXTRA_SRCS
    "${ALL_DEVICES_COMMON_DIR}/device/api/Interface.cpp"
    "${ALL_DEVICES_COMMON_DIR}/device/api/SingleEndpoint.cpp"
    "${ALL_DEVICES_COMMON_DIR}/device/types/root-node/RootNode.cpp"
    "${ALL_DEVICES_COMMON_DIR}/device/types/speaker/Speaker.cpp"
)
```

---

## 2. Entrypoint Deltas ([`esp32/main/main.cpp`](../esp32/main/main.cpp))

-   **Keep**:
    -   Persistence initialization, `dataModelProvider` setup, and `gRootNode`
        (`WifiRootNode`) registration on `kRootEndpointId` (`0`) in
        `PopulateCodeDrivenDataModelProvider()`.
-   **Delete**:
    -   `#include "AppDeviceFactory.h"`, `#include "DeviceTypeSelection.h"`,
        `#include "DeviceFactoryPlatformOverride.h"`, and
        `#include <DeviceShellCommands.h>`
    -   `AppDeviceFactory::GetInstance()` and
        `RegisterDeviceFactoryOverrides(...)` in `InitServer()`, and
        `chip::Shell::DeviceCommands::GetInstance().Register()` in `app_main()`
    -   `gConstructedDevices` (replace with
        `std::unique_ptr<MyProductSpeaker> gProductDevice;`)
-   **Replace**:
    -   Replace the `AppDeviceFactory::GetInstance()` block after
        `gRootNode->Register(...)` in `PopulateCodeDrivenDataModelProvider()`
        with direct registration on `EndpointId(1)`:

```cpp
    if (gRootNode->Register(rootAllocator, dataModelProvider) != CHIP_NO_ERROR)
    {
        return nullptr;
    }

    gProductDevice = std::make_unique<MyProductSpeaker>(gTimerDelegate);
    if (gProductDevice->Register(EndpointId(1), dataModelProvider) != CHIP_NO_ERROR)
    {
        return nullptr;
    }

    return &dataModelProvider;
```

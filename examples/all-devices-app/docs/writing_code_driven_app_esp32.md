# Writing a Code-Driven Application: ESP32 (CMake)

This guide accompanies
[Writing a Code-Driven Application](writing_code_driven_app.md) and describes
the specific changes required in [`examples/all-devices-app/esp32/`](../esp32/)
to convert the multi-device simulator into a single-device application.

---

## 1. CMake Build Deltas ([`esp32/main/CMakeLists.txt`](../esp32/main/CMakeLists.txt))

In [`esp32/main/CMakeLists.txt`](../esp32/main/CMakeLists.txt), the simulator
includes
[`enabled_devices.cmake`](../all-devices-common/device-factory/enabled_devices.cmake),
which populates `${ALL_DEVICES_DEVICE_SOURCES}` and
`${ALL_DEVICES_CLUSTER_SOURCES}` for every enabled device type and its
`impl/Logging*` classes.

To build a single-product application from
[`esp32/main/CMakeLists.txt`](../esp32/main/CMakeLists.txt):

1. **Remove Simulator CMake Inclusion & Sources**:
    - Delete
      `include("${ALL_DEVICES_COMMON_DIR}/device-factory/enabled_devices.cmake")`.
    - Remove `${ALL_DEVICES_EXTRA_INCLUDE_DIRS}`,
      `${ALL_DEVICES_DEVICE_SOURCES}`, and `${ALL_DEVICES_CLUSTER_SOURCES}`.
    - Remove simulator-only files from `esp32/main/` (`DeviceTypeSelection.cpp`,
      `DeviceShellCommands.cpp`, `AppDeviceFactory.h`, and
      `DeviceFactoryPlatformOverride.h`).
2. **Add Explicit Root Node, Base Device, and Cluster Sources**:
    - Keep `APP_TOPLEVEL_EXTRA_SRCS` (which already selects `ThreadFeature.cpp`
      or `WifiFeature.cpp` based on `CONFIG_ENABLE_MATTER_OVER_THREAD`) and
      append `RootNode.cpp` plus your single base device `.cpp` (without
      `impl/Logging*`):

```cmake
list(APPEND APP_TOPLEVEL_EXTRA_SRCS
    "${ALL_DEVICES_COMMON_DIR}/device/types/root-node/RootNode.cpp"
    "${ALL_DEVICES_COMMON_DIR}/device/capabilities/dimmable-load/DimmableLoad.cpp"
    "${ALL_DEVICES_COMMON_DIR}/device/types/dimmable-light/DimmableLight.cpp"
)
```

-   For cluster server `.cpp` files, compile only the clusters used by
    `RootNode` and your base device (see the per-device cluster mapping in
    [`enabled_devices.cmake`](../all-devices-common/device-factory/enabled_devices.cmake)
    for the exact cluster directories required by your chosen device type, and
    filter out `*CodegenIntegration.cpp`).

---

## 2. Entrypoint Deltas ([`esp32/main/main.cpp`](../esp32/main/main.cpp))

In [`esp32/main/main.cpp`](../esp32/main/main.cpp),
`PopulateCodeDrivenDataModelProvider()` already initializes
`gAttributePersistenceProvider`, `gSafeAttributePersistenceProvider`,
`CodeDrivenDataModelProvider`, and `gRootNode` (`WifiRootNode`), and registers
`gRootNode` on `kRootEndpointId` (0).

Note that
[`esp32/main/ESP32DimmableLight.h`](../esp32/main/ESP32DimmableLight.h) and
[`esp32/main/ESP32DimmableLight.cpp`](../esp32/main/ESP32DimmableLight.cpp)
provide a compiled hardware-backed device class in this directory.

### What to Change

In `PopulateCodeDrivenDataModelProvider()`
([`esp32/main/main.cpp`](../esp32/main/main.cpp)):

1. **Keep everything up through
   `gRootNode->Register(rootAllocator, dataModelProvider)` unchanged**.
2. **Remove `#include "AppDeviceFactory.h"`, `#include "DeviceTypeSelection.h"`,
   and `#include "DeviceFactoryPlatformOverride.h"`**.
3. **Replace the `AppDeviceFactory::GetInstance()` block** (from
   `auto & deviceFactory = AppDeviceFactory::GetInstance();` to the end of
   `PopulateCodeDrivenDataModelProvider()`) with direct instantiation and
   registration of your product device on `EndpointId(1)`:

```cpp
    // Keep existing gRootNode construction and registration above this line
    if (gRootNode->Register(rootAllocator, dataModelProvider) != CHIP_NO_ERROR)
    {
        return nullptr;
    }

    // Replace AppDeviceFactory and NVS dev-type selection with static registration:
    gProductDevice = std::make_unique<ESP32DimmableLight>(ESP32DimmableLight::Context{
        .groupDataProvider = gGroupDataProvider,
        .fabricTable       = Server::GetInstance().GetFabricTable(),
        .timerDelegate     = gTimerDelegate,
        .identifyDelegate  = gIdentifyDelegate,
    });
    if (gProductDevice->Register(EndpointId(1), dataModelProvider) != CHIP_NO_ERROR)
    {
        return nullptr;
    }

    return &dataModelProvider;
```

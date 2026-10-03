# Writing a Code-Driven Application: ESP32 (CMake)

Build and entrypoint deltas for converting
[`examples/all-devices-app/esp32/`](../esp32/) into a single-device product
application. See [Writing a Code-Driven Application](writing_code_driven_app.md)
for architecture and device class implementation.

---

## 1. CMake Build Deltas ([`esp32/main/CMakeLists.txt`](../esp32/main/CMakeLists.txt))

-   **Remove Simulator & Example Scaffolding**:
    -   Delete
        `include("${ALL_DEVICES_COMMON_DIR}/device-factory/enabled_devices.cmake")`
        and remove `${ALL_DEVICES_EXTRA_INCLUDE_DIRS}`,
        `${ALL_DEVICES_DEVICE_SOURCES}`, and `${ALL_DEVICES_CLUSTER_SOURCES}`.
    -   Remove simulator device-factory, shell device-switching, and example
        peripheral directories/files (such as `display/` and `devices/chime/`)
        from `esp32/main/` and `CMakeLists.txt`.
-   **Append Base & Product Device Sources**:
    -   Keep `APP_TOPLEVEL_EXTRA_SRCS` (which selects `ThreadFeature.cpp` or
        `WifiFeature.cpp` based on `CONFIG_ENABLE_MATTER_OVER_THREAD`) and
        append the `device/api/*` base sources, `RootNode.cpp`, and the single
        base device `.cpp` (note that `config/esp32/components/chip` already
        compiles the SDK cluster servers into `libCHIP.a`):

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
    -   Platform initialization, persistence setup, `dataModelProvider`
        creation, and `gRootNode` (`WifiRootNode`) registration on
        `kRootEndpointId` (`0`) in `PopulateCodeDrivenDataModelProvider()`.
-   **Remove**:
    -   All `AppDeviceFactory` registration, NVS `dev-type` device-selection
        state, shell device-switching commands, and sample display UI hooks in
        `main.cpp`.
-   **Replace**:
    -   Immediately after `gRootNode->Register(...)` in
        `PopulateCodeDrivenDataModelProvider()`, instantiate and register the
        product device on `EndpointId(1)`:

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

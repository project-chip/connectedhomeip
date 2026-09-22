# Writing a Code-Driven Application: Silicon Labs (GN)

Build and entrypoint deltas for converting
[`examples/all-devices-app/silabs/`](../silabs/) into a single-device product
application. See [Writing a Code-Driven Application](writing_code_driven_app.md)
for architecture and device class implementation.

---

## 1. GN Build Deltas ([`silabs/BUILD.gn`](../silabs/BUILD.gn))

-   **Remove Simulator Scaffolding**:
    -   Remove the `enabled_devices.gni` import,
        `all-devices-common/device-factory`, unused
        `all-devices-common/device/types/*` targets, and the shell/KVS
        device-selection files (`DeviceShellCommands.*`, `AppKeys.h`).
-   **Keep Platform & Root Node Dependencies**:
    -   Keep the `root-node:wifi`, `root-node:thread`, and `root-node:ota`
        conditional blocks, along with the device info provider targets.
-   **Link the Product Device**:
    -   Add your product device sources and depend on the single base device
        target (e.g., `device/types/speaker`):

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
    -   Platform initialization, persistence setup, `sDataModelProvider`
        creation, and `sRootNode` (`RootNodeWith<...>`) construction and
        registration on `kRootEndpointId` (`0`) in
        `AppTask::InitCodeDrivenDataModel()`.
-   **Remove**:
    -   All `DeviceFactory` (`NoHooksDeviceFactory`), build-time device list
        (`enabled_devices.h`), KVS device-type selection, and shell
        device-switching code.
-   **Replace**:
    -   Immediately after `sRootNode->Register(...)` in
        `AppTask::InitCodeDrivenDataModel()`, instantiate and register the
        product device on `kDeviceEndpointId` (`EndpointId(1)`):

```cpp
    ReturnErrorOnFailure(sRootNode->Register(rootAllocator, *sDataModelProvider));

    sProductDevice = std::make_unique<chip::app::MyProductSpeaker>(sTimerDelegate);
    VerifyOrReturnError(sProductDevice != nullptr, CHIP_ERROR_NO_MEMORY);
    return sProductDevice->Register(kDeviceEndpointId, *sDataModelProvider);
```

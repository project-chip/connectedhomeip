# Writing a Code-Driven Application: POSIX (GN)

Build and entrypoint deltas for converting
[`examples/all-devices-app/posix/`](../posix/) into a single-device product
application. See [Writing a Code-Driven Application](writing_code_driven_app.md)
for architecture and device class implementation.

---

## 1. GN Build Deltas ([`posix/BUILD.gn`](../posix/BUILD.gn), [`posix/linux/BUILD.gn`](../posix/linux/BUILD.gn), [`posix/darwin/BUILD.gn`](../posix/darwin/BUILD.gn))

-   **Delete from `posix/BUILD.gn`**:
    -   `OOBAccessorHook.h`, `named_pipe/Hook.h`, and
        `include/DeviceFactoryPlatformOverride.h` from `sources`
    -   `all-devices-common/device-factory`, `oob-accessors:oob-accessors`,
        `posix/named_pipe`, unused `device/types/*` targets, and all `:posix`
        sub-targets (`device/types/<device>:posix`, which compile
        `impl/Logging*` classes) from `deps`
-   **Delete from [`posix/linux/BUILD.gn`](../posix/linux/BUILD.gn) and
    [`posix/darwin/BUILD.gn`](../posix/darwin/BUILD.gn)**:
    -   `"../include/DeviceFactoryPlatformOverride.h"` from `sources`
    -   `"${chip_root}/examples/all-devices-app/all-devices-common/device-factory"`
        from `deps`
-   **Keep in `posix/BUILD.gn`**:
    -   `posix/app_options:app-options` (used by `Initialize()` /
        `RunApplication()` for `--discriminator`, `--passcode`, `--kvs`,
        `--wifi`, and `--dac-provider`)
    -   `device/types/root-node`, `device/types/root-node:posix`,
        `device/types/root-node:wifi`, and the single base device target (e.g.,
        `device/types/speaker`):

```gn
  sources = [
    "MyProductSpeaker.cpp",
    "MyProductSpeaker.h",
    "include/CHIPProjectAppConfig.h",
    "main.cpp",
  ]

  deps = [
    "${chip_root}/examples/all-devices-app/all-devices-common/device/types/root-node",
    "${chip_root}/examples/all-devices-app/all-devices-common/device/types/root-node:posix",
    "${chip_root}/examples/all-devices-app/all-devices-common/device/types/root-node:wifi",
    "${chip_root}/examples/all-devices-app/all-devices-common/device/types/speaker",
    "${chip_root}/examples/all-devices-app/posix/app_options:app-options",
    # ... keep existing platform/linux, providers, and codedriven persistence deps ...
  ]
```

---

## 2. Entrypoint Deltas ([`posix/main.cpp`](../posix/main.cpp))

-   **Keep**:
    -   `mAttributePersistence`, `mDataModelProvider`, and `mRootNode`
        (`AppRootNode`) construction in `CodeDrivenDataModelDevices`.
-   **Delete**:
    -   `#include <PosixAudioManager.h>` and `gAudioManager`
    -   `PosixDeviceFactory`, `RegisterDeviceFactoryOverrides(...)`,
        `SetupNamedPipe(...)`, and `AppOptions::GetDeviceTypeEntries()`
        (`--device`) handling in `CodeDrivenDataModelDevices::Startup()` and
        `RunApplication()`.
-   **Replace**:
    -   Replace
        `std::vector<std::unique_ptr<DeviceInterface>> mConstructedDevices` in
        `CodeDrivenDataModelDevices` with a member instance
        `MyProductSpeaker mProductDevice` (initialized with
        `mProductDevice(mContext.timerDelegate)` in the constructor initializer
        list) and register it in `Startup()` alongside `mRootNode.RootDevice()`:

```cpp
    CHIP_ERROR Startup()
    {
        ReturnErrorOnFailure(mAttributePersistence.Init(&mContext.storageDelegate));
        ReturnErrorOnFailure(mRootNode.RootDevice().Register(kRootEndpointId, mDataModelProvider));
        ReturnErrorOnFailure(mProductDevice.Register(EndpointId(1), mDataModelProvider));
        return CHIP_NO_ERROR;
    }

    void Shutdown()
    {
        mProductDevice.Unregister(mDataModelProvider);
        mRootNode.RootDevice().Unregister(mDataModelProvider);
    }
```

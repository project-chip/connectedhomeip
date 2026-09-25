# Writing a Code-Driven Application: POSIX (GN)

Build and entrypoint deltas for converting
[`examples/all-devices-app/posix/`](../posix/) into a single-device product
application. See [Writing a Code-Driven Application](writing_code_driven_app.md)
for architecture and device class implementation.

---

## 1. GN Build Deltas ([`posix/BUILD.gn`](../posix/BUILD.gn), [`posix/linux/BUILD.gn`](../posix/linux/BUILD.gn), [`posix/darwin/BUILD.gn`](../posix/darwin/BUILD.gn))

-   **Remove Simulator & Example Scaffolding**:
    -   Remove `all-devices-common/device-factory` and
        `DeviceFactoryPlatformOverride.h` from `posix/BUILD.gn`,
        [`posix/linux/BUILD.gn`](../posix/linux/BUILD.gn), and
        [`posix/darwin/BUILD.gn`](../posix/darwin/BUILD.gn).
    -   Remove `oob-accessors`, `posix/named_pipe`, sample peripheral sources
        (`PosixAudioManager.cpp`, `PosixChime.cpp`, `PosixSpeaker.cpp`), unused
        `device/types/*` targets, and the `device/types/<device>:posix` logging
        sub-targets (excluding `device/types/root-node:posix`).
-   **Keep Platform & Root Node Dependencies**:
    -   Keep `posix/app_options:app-options`, `device/types/root-node`
        (including `:posix` and `:wifi`), and the single base device target
        (e.g., `device/types/speaker`):

```text
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
    -   Platform initialization, `mAttributePersistence`, `mDataModelProvider`,
        and `mRootNode` (`AppRootNode`) construction in
        `CodeDrivenDataModelDevices`.
-   **Remove**:
    -   All `PosixDeviceFactory` registration, CLI `--device` topology loops,
        named pipe setup, and sample audio manager hooks (including the
        audio-only `ApplicationShutdown()` helper).
-   **Replace**:
    -   Own a member instance `MyProductSpeaker mProductDevice` in
        `CodeDrivenDataModelDevices` and register/unregister it in `Startup()`
        and `Shutdown()` alongside `mRootNode.RootDevice()`:

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

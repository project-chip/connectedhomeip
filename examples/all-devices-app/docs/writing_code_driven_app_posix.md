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
        [`posix/darwin/BUILD.gn`](../posix/darwin/BUILD.gn) (if your target
        device relies on platform setup in
        [`DeviceFactoryPlatformOverride.h`](../posix/include/DeviceFactoryPlatformOverride.h),
        such as `PosixAudioManager` or `CommissioningProxy` transport adapters,
        move that initialization into your entrypoint before removing the
        header).
    -   Remove `all-devices-common/device-factory` and `:device-type-parser`
        from [`posix/app_options/BUILD.gn`](../posix/app_options/BUILD.gn),
        remove `<app_options/DeviceTypeParser.h>` and `GetDeviceTypeEntries()`
        from [`AppOptions.h`](../posix/app_options/AppOptions.h), and remove
        `--device` / `NoHooksDeviceFactory` handling from
        [`AppOptions.cpp`](../posix/app_options/AppOptions.cpp) (include
        `<lib/support/BytesToHex.h>` directly in `AppOptions.cpp` if
        `HexToBytes` is retained).
    -   Remove `oob-accessors`, `posix/named_pipe`, sample peripheral sources
        (`PosixAudioManager.cpp`, `PosixChime.cpp`, `PosixSpeaker.cpp`), unused
        `device/types/*` targets, and all `:posix` sub-targets (which compile
        `NamedPipeTranslators.cpp` and depend on `posix/named_pipe`; mock
        delegates live in `:logging` or `:simulated` sub-targets). Also remove
        unused `commissioning-proxy`, `miniaudio`, and `jsoncpp` dependencies
        from `posix/BUILD.gn`, `posix/linux/BUILD.gn`, and
        `posix/darwin/BUILD.gn`.
-   **Keep Platform & Root Node Dependencies**:
    -   Keep `posix/app_options:app-options`, `device/types/root-node` (and
        `:wifi`), and the single base device target (e.g.,
        `device/types/speaker`). If copying `posix/` into a standalone
        directory, preserve the `build_overrides` and `third_party` symlinks and
        update `//examples/all-devices-app/posix/...` paths in `args.gni` and
        `BUILD.gn`:

```text
  sources = [
    "MyProductSpeaker.cpp",
    "MyProductSpeaker.h",
    "include/CHIPProjectAppConfig.h",
    "main.cpp",
  ]

  deps = [
    "${chip_root}/examples/all-devices-app/all-devices-common/device/types/root-node",
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
        and `mRootNode` ([`AppRootNode`](../posix/include/AppRootNode.h))
        construction in `CodeDrivenDataModelDevices` (note that `AppRootNode`
        only wraps plain and Wi-Fi root nodes by default; enabling OTA on POSIX
        requires linking `device/types/root-node:ota` and composing
        `OtaFeature`).
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

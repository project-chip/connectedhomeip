# Writing a Code-Driven Application: POSIX (GN)

This guide accompanies
[Writing a Code-Driven Application](writing_code_driven_app.md) and describes
the specific changes required in [`examples/all-devices-app/posix/`](../posix/)
to convert the multi-device simulator into a single-device application.

---

## 1. GN Build Deltas ([`posix/BUILD.gn`](../posix/BUILD.gn))

Use [`posix/BUILD.gn`](../posix/BUILD.gn) as the starting template and apply the
following changes:

1. **Remove Simulator Infrastructure & Logging Targets**:
    - Remove `OOBAccessorHook.h`, `named_pipe/Hook.h`, and
      `DeviceFactoryPlatformOverride.h` from `sources`.
    - Remove `all-devices-common/device-factory`, `oob-accessors`,
      `posix/app_options:app-options`, `posix/named_pipe`, and all `:posix`
      sub-targets (`device/types/<device>:posix`, which compile the
      `impl/Logging*` classes) from `deps`.
2. **Keep Only Target Device & Root Node Dependencies**:
    - Keep `all-devices-common/device/types/root-node` (and `:wifi` if Wi-Fi
      commissioning is used) and the single base device target for your product
      (e.g., `all-devices-common/device/types/dimmable-light`):

```gn
  sources = [
    "MyProductDimmableLight.cpp",
    "MyProductDimmableLight.h",
    "include/CHIPProjectAppConfig.h",
    "main.cpp",
  ]

  deps = [
    "${chip_root}/examples/all-devices-app/all-devices-common/device/capabilities/identify:LoggingIdentifyDelegate",
    "${chip_root}/examples/all-devices-app/all-devices-common/device/types/dimmable-light",
    "${chip_root}/examples/all-devices-app/all-devices-common/device/types/root-node",
    "${chip_root}/examples/all-devices-app/all-devices-common/device/types/root-node:wifi",
    # ... keep existing platform/linux and codedriven persistence deps ...
  ]
```

---

## 2. Entrypoint Deltas ([`posix/main.cpp`](../posix/main.cpp))

In [`posix/main.cpp`](../posix/main.cpp), `CodeDrivenDataModelDevices` already
owns `mAttributePersistence`, `mDataModelProvider`, and `mRootNode`, and
registers `mRootNode` on `kRootEndpointId` (0) inside `Init()`. It then uses
`PosixDeviceFactory` (`DeviceFactory<OOBAccessorHook, NamedPipe::Hook>`) to
instantiate a `std::vector<std::unique_ptr<DeviceInterface>> mDevices` parsed
from CLI `--device` flags.

### What to Change

In `CodeDrivenDataModelDevices` ([`posix/main.cpp`](../posix/main.cpp)):

1. **Keep `mAttributePersistence`, `mDataModelProvider`, and `mRootNode`
   initialization unchanged**.
2. **Replace `std::vector<std::unique_ptr<DeviceInterface>> mDevices` and
   `PosixDeviceFactory`** with a direct member instance of your product device
   class (`MyProductDimmableLight mProductDevice`):

```cpp
    CHIP_ERROR Init()
    {
        ReturnErrorOnFailure(mAttributePersistence.Init(&mContext.storageDelegate));
        ReturnErrorOnFailure(mRootNode.Register(kRootEndpointId, mDataModelProvider));

        // Replace PosixDeviceFactory and CLI --device loop with fixed registration:
        ReturnErrorOnFailure(mProductDevice.Register(EndpointId(1), mDataModelProvider));
        return CHIP_NO_ERROR;
    }

    void Shutdown()
    {
        mProductDevice.Unregister(mDataModelProvider);
        mRootNode.Unregister(mDataModelProvider);
    }
```

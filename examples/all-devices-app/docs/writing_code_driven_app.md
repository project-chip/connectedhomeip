# Writing a Code-Driven Application

Convert `all-devices-app` from a runtime multi-device simulator into a
fixed-function Matter product by retaining platform setup, `RootNode`, and spec
base classes (`all-devices-common/device/types/`) while stripping
`DeviceFactory` and `impl/Logging*` mocks.

---

## 1. Simulator vs. Product Architecture

| Layer / Component                        | `all-devices-app` Simulator                                                                                                                                                                                                                                          | Product Application Baseline                                                                                                           |
| :--------------------------------------- | :------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | :------------------------------------------------------------------------------------------------------------------------------------- |
| **Platform Entrypoint & Hardware Setup** | `main.cpp` / `AppTask.cpp` initializes platform stack, storage, and `CodeDrivenDataModelProvider`.                                                                                                                                                                   | **Reuse**. Keep platform initialization, event loop, and `CodeDrivenDataModelProvider` setup.                                          |
| **Endpoint 0 (Root Node)**               | [`RootNode`](../all-devices-common/device/types/root-node/RootNode.h) / [`RootNodeWith<Features...>`](../all-devices-common/device/types/root-node/RootNodeWith.h).                                                                                                  | **Reuse**. Compose `RootNodeWith<...>` with `WifiFeature` or `ThreadFeature` and optional `OtaFeature`.                                |
| **Base Device Types**                    | [`all-devices-common/device/types/<device-name>/`](../all-devices-common/device/types/) (e.g., [`Speaker`](../all-devices-common/device/types/speaker/Speaker.h), [`TemperatureSensor`](../all-devices-common/device/types/temperature-sensor/TemperatureSensor.h)). | **Reuse**. Inherit from these base classes, which wire the Descriptor cluster and mandatory server clusters.                           |
| **Hardware / Cluster Delegates**         | `impl/Logging*` and simulated classes (e.g., [`LoggingSpeaker`](../all-devices-common/device/types/speaker/impl/LoggingSpeaker.h)).                                                                                                                                  | **Replace**. Subclass the base device type and implement cluster `Delegate` interfaces to drive hardware (starting from `TODO` stubs). |
| **Device Factory & Registries**          | [`DeviceFactory`](../all-devices-common/device-factory/DeviceFactory.h) mapping string names to creators.                                                                                                                                                            | **Remove**. Instantiate `RootNode` and product device objects directly.                                                                |
| **Build-Time Multi-Device Lists**        | [`enabled_devices.cmake`](../all-devices-common/device-factory/enabled_devices.cmake) / [`enabled_devices.gni`](../all-devices-common/device-factory/enabled_devices.gni).                                                                                           | **Remove**. Compile and link only `RootNode`, `device/api/*`, and the single base device type.                                         |
| **Runtime Topology Selection**           | CLI `--device` flags, NVS/NVM3 `dev-type` keys, wildcard (`*`) loops, and OOB/named-pipe hooks.                                                                                                                                                                      | **Remove**. Register fixed `EndpointId` values (`kRootEndpointId` and `EndpointId(1)`) at startup.                                     |

---

## 2. Endpoint Topology Checklist

Map Matter Base Device Type and Device Library requirements to C++ types before
coding:

| Endpoint                            | Role               | Code-Driven Class / Hook                                                                   | Configuration                                                                                                                                                                                                                                                                                             |
| :---------------------------------- | :----------------- | :----------------------------------------------------------------------------------------- | :-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `EndpointId(0)` (`kRootEndpointId`) | Root Node          | [`RootNodeWith<Features...>`](../all-devices-common/device/types/root-node/RootNodeWith.h) | Pass [`WifiFeature`](../all-devices-common/device/types/root-node/features/WifiFeature.h) or [`ThreadFeature`](../all-devices-common/device/types/root-node/features/ThreadFeature.h), plus optional [`OtaFeature`](../all-devices-common/device/types/root-node/features/OtaFeature.h).                  |
| `EndpointId(1)`                     | Application Device | [`all-devices-common/device/types/<device>/`](../all-devices-common/device/types/)         | Mandatory clusters wired by base class. Pass feature maps and optional attributes via constructor config. Attach optional spec clusters via `RegisterAdditionalClusters()` / `UnregisterAdditionalClusters()` (if provided by the base class) or in `Register()` / `Unregister()` before `AddEndpoint()`. |

---

## 3. Product Device Subclass

Do not instantiate `impl/Logging*` classes in product builds. Subclass the
device base class (`device/types/<device>/<Device>.h`) and implement its cluster
`Delegate` interfaces to drive hardware peripherals (see
[`device/types/README.md`](../all-devices-common/device/types/README.md)):

-   **Required Delegates & Inheritance Order**: Inspect
    `device/types/<device>/impl/Logging*.h` (e.g.,
    [`LoggingSpeaker.h`](../all-devices-common/device/types/speaker/impl/LoggingSpeaker.h)).
    Inherit `private` `Delegate` interfaces **before** `public <BaseDevice>` so
    delegate base classes initialize before `*this` is passed to `<BaseDevice>`.
-   **Hardware Peripheral Reference**: See
    [`PosixSpeaker.h`](../posix/include/PosixSpeaker.h) or
    [`ESP32DimmableLight.h`](../esp32/main/ESP32DimmableLight.h) for compiled
    subclasses driving platform hardware.

```cpp
class MyProductSpeaker : private Clusters::LevelControlDelegate,
                         private Clusters::OnOffDelegate,
                         public Speaker
{
public:
    explicit MyProductSpeaker(TimerDelegate & timerDelegate) :
        Speaker(*this, *this, timerDelegate)
    {}

protected:
    // Optional (when declared virtual by the base device class): attach extra spec-optional clusters
    // CHIP_ERROR RegisterAdditionalClusters(EndpointId endpoint, CodeDrivenDataModelProvider & provider) override;
    // void UnregisterAdditionalClusters(CodeDrivenDataModelProvider & provider) override;

private:
    // Override virtual methods from LevelControlDelegate and OnOffDelegate with hardware TODOs
};
```

---

## 4. Endpoint Registration

Platform entrypoints (`main.cpp` / `AppTask.cpp`) already construct
`CodeDrivenDataModelProvider` and register `RootNode` on `kRootEndpointId`
(`0`):

-   **Wi-Fi**: `WifiRootNode` (`RootNodeWith<WifiFeature>`,
    [`WifiRootNode.h`](../all-devices-common/device/types/root-node/WifiRootNode.h))
-   **Thread**: `ThreadRootNode` (`RootNodeWith<ThreadFeature>`,
    [`ThreadRootNode.h`](../all-devices-common/device/types/root-node/ThreadRootNode.h))
-   **With OTA**: `RootNodeWith<WifiFeature, OtaFeature>` or
    `RootNodeWith<ThreadFeature, OtaFeature>`

Keep `RootNode` registration on `kRootEndpointId` (`0`) and replace the
`DeviceFactory` block with direct registration of the product device on
`EndpointId(1)`:

```cpp
ReturnErrorOnFailure(mRootNode->Register(kRootEndpointId, *mDataModelProvider));
ReturnErrorOnFailure(mProductDevice->Register(EndpointId(1), *mDataModelProvider));
```

---

## 5. Production Providers

Replace example providers in
[`all-devices-common/providers/`](../all-devices-common/providers/) with
hardware-backed implementations before starting the Matter server:

1. **`DeviceAttestationCredentialsProvider`**: Bind
   [`SetDeviceAttestationCredentialsProvider`](../../../src/credentials/DeviceAttestationCredsProvider.h)
   to the platform factory data partition or secure element instead of
   `AllDevicesExampleDACProvider`.
2. **`DeviceInstanceInfoProvider` & `DeviceInfoProvider`**: Register the
   platform factory data provider via `SetDeviceInstanceInfoProvider` and
   `SetDeviceInfoProvider`.

---

## 6. Build Systems & Platform Guides

-   **GN Build System**:
    -   **[Silicon Labs (`writing_code_driven_app_silabs.md`)](writing_code_driven_app_silabs.md)**:
        Deltas for [`silabs/BUILD.gn`](../silabs/BUILD.gn) and
        [`silabs/src/AppTask.cpp`](../silabs/src/AppTask.cpp).
    -   **[POSIX (`writing_code_driven_app_posix.md`)](writing_code_driven_app_posix.md)**:
        Deltas for [`posix/BUILD.gn`](../posix/BUILD.gn),
        [`posix/linux/BUILD.gn`](../posix/linux/BUILD.gn), and
        [`posix/main.cpp`](../posix/main.cpp).
-   **CMake Build System**:
    -   **[ESP32 (`writing_code_driven_app_esp32.md`)](writing_code_driven_app_esp32.md)**:
        Deltas for [`esp32/main/CMakeLists.txt`](../esp32/main/CMakeLists.txt)
        and [`esp32/main/main.cpp`](../esp32/main/main.cpp).

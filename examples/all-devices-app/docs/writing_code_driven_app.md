# Writing a Code-Driven Application

The `all-devices-app` application serves as a multi-device simulator for the
Code-Driven Data Model. Because it is designed to instantiate and test any
Matter device type dynamically at runtime, it includes simulator infrastructure
(such as a string-keyed `DeviceFactory`, simulated/logging device
implementations, and runtime topology selection) that should not be included in
a standalone product application.

This guide describes how to use the `all-devices-app` architecture, build
layout, and compiled base device classes as a baseline for a fixed-function
Matter product.

---

## 1. Simulator vs. Product Architecture

When creating a standalone application from `all-devices-app`, reuse the
platform initialization, `RootNode`, and abstract base device types while
removing the multi-device simulator scaffolding:

| Layer / Component                        | `all-devices-app` Simulator                                                                                                                                                           | Product Application Baseline                                                                                                              |
| :--------------------------------------- | :------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ | :---------------------------------------------------------------------------------------------------------------------------------------- |
| **Platform Entrypoint & Hardware Setup** | `main.cpp` / `AppTask.cpp` initializes platform stack, storage, and `CodeDrivenDataModelProvider`.                                                                                    | **Reuse**. Keep the platform initialization, event loop, and `CodeDrivenDataModelProvider` setup.                                         |
| **Endpoint 0 (Root Node)**               | [`RootNode`](../all-devices-common/device/types/root-node/RootNode.h) / [`RootNodeWith<Features...>`](../all-devices-common/device/types/root-node/RootNodeWith.h).                   | **Reuse**. Compose `RootNodeWith<...>` with the product's network transport (`WifiFeature` or `ThreadFeature`) and optional `OtaFeature`. |
| **Base Device Types**                    | [`all-devices-common/device/types/<device-name>/`](../all-devices-common/device/types/) (e.g., [`DimmableLight`](../all-devices-common/device/types/dimmable-light/DimmableLight.h)). | **Reuse**. Inherit from these spec-compliant base classes, which encapsulate the Descriptor cluster and mandatory server clusters.        |
| **Hardware / Cluster Delegates**         | `impl/Logging*` and simulated classes (e.g., [`LoggingDimmableLight`](../all-devices-common/device/types/dimmable-light/impl/LoggingDimmableLight.h)).                                | **Replace**. Implement a product-specific subclass where cluster `Delegate` methods drive hardware (starting from `TODO` stubs).          |
| **Device Factory & Registries**          | [`DeviceFactory`](../all-devices-common/device-factory/DeviceFactory.h) mapping string names to creators.                                                                             | **Remove**. Instantiate the `RootNode` and product device class directly as static or member objects.                                     |
| **Build-Time Multi-Device Lists**        | [`enabled_devices.cmake`](../all-devices-common/device-factory/enabled_devices.cmake) / [`enabled_devices.gni`](../all-devices-common/device-factory/enabled_devices.gni).            | **Remove**. Compile and link only the `RootNode` sources, the single base device type, and the product's cluster servers.                 |
| **Runtime Topology Selection**           | CLI `--device` flags, NVS/NVM3 `dev-type` keys, wildcard (`*`) loops, and OOB/named-pipe hooks.                                                                                       | **Remove**. Register fixed `EndpointId` values (`kRootEndpointId` and `EndpointId(1)`) unconditionally at startup.                        |

---

## 2. Matter Specification Checklist (Endpoint Topology)

Before writing application code, consult the Matter Base Device Type and Device
Library specifications to determine the required endpoints, clusters, and
feature maps:

1. **Endpoint 0 (`kRootEndpointId` = 0 — Root Node)**:
    - Every Matter node requires a Root Node on `EndpointId(0)` hosting core
      administrative and diagnostic clusters (encapsulated by
      [`RootNode`](../all-devices-common/device/types/root-node/RootNode.h)).
    - **Network Transport**: Determine whether the hardware commissions and
      operates over Wi-Fi
      ([`WifiFeature`](../all-devices-common/device/types/root-node/features/WifiFeature.h))
      or Thread
      ([`ThreadFeature`](../all-devices-common/device/types/root-node/features/ThreadFeature.h)).
    - **Software Updates**: Determine whether the product acts as an
      Over-The-Air Software Update Requestor
      ([`OtaFeature`](../all-devices-common/device/types/root-node/features/OtaFeature.h)).
2. **Functional Endpoint(s) (e.g., `EndpointId(1)` — Application Device Type)**:
    - Look up the target Device Type ID in the Matter Device Library
      specification.
    - **Mandatory Clusters**: Already registered and wired by the corresponding
      base class in
      [`all-devices-common/device/types/<device>/`](../all-devices-common/device/types/).
    - **Feature Maps & Optional Attributes**: Check which cluster features and
      optional attributes your hardware supports, and pass them via the base
      device's `Config` struct.
    - **Optional Clusters**: Identify any additional server or client clusters
      required by your product that are optional in the Device Library spec;
      attach these via `SingleEndpoint::RegisterAdditionalClusters` and
      `UnregisterAdditionalClusters` (defined in
      [`SingleEndpoint.h`](../all-devices-common/device/api/SingleEndpoint.h)).

---

## 3. Implementing the Product Device Class (`device/types/` + `TODO` Delegates)

The device classes in
[`all-devices-common/device/types/`](../all-devices-common/device/types/)
separate specification conformance from hardware behavior:

1. **Base Device Class (`device/types/<device>/<Device>.h`)**: Inherits from
   [`SingleEndpoint`](../all-devices-common/device/api/SingleEndpoint.h),
   declares the Matter Device Type ID, instantiates mandatory server clusters,
   and accepts a `Delegates` struct in its constructor.
2. **Simulator Subclass (`device/types/<device>/impl/Logging<Device>.h`)**:
   Inherits from the required cluster `Delegate` interfaces and passes `*this`
   to the base device constructor.

For a real application, **do not instantiate `Logging<Device>`**. Instead, copy
the structure of `impl/Logging<Device>.h/.cpp` to create your own product class
where each overridden delegate method starts as a `// TODO` stub for your
hardware driver.

### Compiled Reference Examples in the Repository

-   **Delegate List Reference**: Inspect the corresponding `impl/Logging*.h` for
    your device type (for example,
    [`LoggingDimmableLight.h`](../all-devices-common/device/types/dimmable-light/impl/LoggingDimmableLight.h)
    and
    [`LoggingDimmableDelegate.h`](../all-devices-common/device/capabilities/dimmable-load/impl/LoggingDimmableDelegate.h))
    to see the exact `Delegate` interfaces and virtual methods required by that
    device type.
-   **Hardware-Backed Implementation**: See
    [`ESP32DimmableLight.h`](../esp32/main/ESP32DimmableLight.h) and
    [`ESP32DimmableLight.cpp`](../esp32/main/ESP32DimmableLight.cpp) for a
    compiled example that subclasses `DimmableLight` and implements the cluster
    delegates against hardware peripherals.

### Structural Pattern

List the cluster `Delegate` bases as `private` **before** `public <BaseDevice>`
so their vtables are initialized before `*this` is passed to `<BaseDevice>`:

```cpp
class MyProductDimmableLight : private Clusters::OnOffDelegate,
                               private Clusters::LevelControlDelegate,
                               private Clusters::OnOffEffectDelegate,
                               private Clusters::IdentifyDelegate,
                               public DimmableLight
{
public:
    explicit MyProductDimmableLight(const Context & context, const Config & config = {}) :
        DimmableLight(context, Delegates{ .onOff = *this, .levelControl = *this, .effect = *this, .identify = *this }, config)
    {}

protected:
    // Optional: attach extra spec-optional clusters before the endpoint is added to the provider
    CHIP_ERROR RegisterAdditionalClusters(EndpointId endpoint, CodeDrivenDataModelProvider & provider) override;
    void UnregisterAdditionalClusters(CodeDrivenDataModelProvider & provider) override;

private:
    // Implement virtual methods from OnOffDelegate, LevelControlDelegate, etc. with hardware TODOs
};
```

---

## 4. Configuring Endpoint 0 (`RootNodeWith<...>`) and Registering Endpoints

Endpoint 0 is implemented by
[`RootNode`](../all-devices-common/device/types/root-node/RootNode.h) and
composed with compile-time feature policies via
[`RootNodeWith<Features...>`](../all-devices-common/device/types/root-node/RootNodeWith.h):

-   **Wi-Fi only**: `RootNodeWith<WifiFeature>` (aliased as `WifiRootNode` in
    [`WifiRootNode.h`](../all-devices-common/device/types/root-node/WifiRootNode.h))
-   **Thread only**: `RootNodeWith<ThreadFeature>` (aliased as `ThreadRootNode`
    in
    [`ThreadRootNode.h`](../all-devices-common/device/types/root-node/ThreadRootNode.h))
-   **With OTA Requestor**: `RootNodeWith<WifiFeature, OtaFeature>` or
    `RootNodeWith<ThreadFeature, OtaFeature>` (using
    [`OtaFeature.h`](../all-devices-common/device/types/root-node/features/OtaFeature.h))

In every platform entrypoint (`main.cpp` or `AppTask.cpp`), the simulator
already constructs `CodeDrivenDataModelProvider` and registers `RootNode` on
`kRootEndpointId` (`EndpointId(0)`). Keep that `RootNode` initialization
unchanged, and replace the `DeviceFactory` block that follows it with direct
registration of your product device class on `EndpointId(1)`:

```cpp
// Keep existing CodeDrivenDataModelProvider and RootNode registration on kRootEndpointId (0):
ReturnErrorOnFailure(mRootNode->Register(kRootEndpointId, *mDataModelProvider));

// Replace DeviceFactory creation/lookup with direct registration of the product device:
ReturnErrorOnFailure(mProductDevice->Register(EndpointId(1), *mDataModelProvider));
```

---

## 5. Production Credentials & Device Information Providers

Before starting the Matter server in a product application, replace the example
providers in [`all-devices-common/providers/`](../all-devices-common/providers/)
with hardware-backed implementations:

1. **`DeviceAttestationCredentialsProvider`**: Bind
   [`SetDeviceAttestationCredentialsProvider`](../../../src/credentials/DeviceAttestationCredsProvider.h)
   to your platform's factory data partition or secure element rather than
   `AllDevicesExampleDACProvider`.
2. **`DeviceInstanceInfoProvider` & `DeviceInfoProvider`**: Register your
   platform's factory data provider via `SetDeviceInstanceInfoProvider` and
   `SetDeviceInfoProvider`.

---

## 6. Build Systems & Platform-Specific Guides

Adapting the build configuration and application entrypoint (`main.cpp` or
`AppTask.cpp`) depends on the build system used by your target platform:

-   **GN Build System**:
    -   **[Silicon Labs Guide (`writing_code_driven_app_silabs.md`)](writing_code_driven_app_silabs.md)**:
        Deltas for [`silabs/BUILD.gn`](../silabs/BUILD.gn) and
        [`silabs/src/AppTask.cpp`](../silabs/src/AppTask.cpp).
    -   **[POSIX Guide (`writing_code_driven_app_posix.md`)](writing_code_driven_app_posix.md)**:
        Deltas for [`posix/BUILD.gn`](../posix/BUILD.gn) and
        [`posix/main.cpp`](../posix/main.cpp).
-   **CMake Build System**:
    -   **[ESP32 Guide (`writing_code_driven_app_esp32.md`)](writing_code_driven_app_esp32.md)**:
        Deltas for [`esp32/main/CMakeLists.txt`](../esp32/main/CMakeLists.txt)
        and [`esp32/main/main.cpp`](../esp32/main/main.cpp).

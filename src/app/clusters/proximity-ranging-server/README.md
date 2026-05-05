# Proximity Ranging Cluster

The Proximity Ranging cluster enables the configuration of proximity ranging
sessions and reporting of proximity ranging data between Matter devices using
various technologies (BLE Beacon RSSI, Bluetooth Channel Sounding, Wi-Fi USD,
UWB).

## Overview

This directory contains a code-driven C++ implementation of the Matter Proximity
Ranging cluster server (cluster ID `0x0433`). The implementation follows the
`DefaultServerCluster` pattern with `CodegenIntegration` (matching the
BooleanStateConfiguration cluster pattern) and delegates all technology-specific
operations to a `ProximityRangingDriver` set via `SetDriver()`.

The cluster handles:

-   Matter protocol interactions (attribute reads, command dispatch)
-   Session ID allocation and tracking
-   Event emission for ranging results and session status changes
-   Attribute change notifications from the driver

The application is responsible for:

-   Implementing the `ProximityRangingDriver` interface
-   Owning ranging technology adapters and the controller
-   Persisting device identity attributes (e.g., BLE Device ID)

## Architecture

```
┌─────────────────────────────────────────────────────────────┐
│ ProximityRangingCluster (SDK, per-endpoint)                 │
│   - ReadAttribute, InvokeCommand, Events                    │
│   - SetDriver(ProximityRangingDriver *)                     │
└──────────────────────────┬──────────────────────────────────┘
                           │ ProximityRangingDriver interface
┌──────────────────────────▼──────────────────────────────────┐
│ DefaultProximityRangingDriver (example, per-endpoint)       │
│   - Routes to RangingTechnologyController                   │
│   - Forwards callbacks to cluster                           │
└──────────────────────────┬──────────────────────────────────┘
                           │ RangingTechnologyController::Listener
┌──────────────────────────▼──────────────────────────────────┐
│ RangingTechnologyController (example, shared)               │
│   - Routes sessions to adapters by technology               │
│   - Multi-listener support for multiple endpoints           │
└──────────┬────────────────────────────────┬─────────────────┘
           │                                │
┌──────────▼───────────┐     ┌──────────────▼─────────────────┐
│ BleRssiRangingAdapter│     │ (Future: WiFi/UWB adapters)    │
│   - Encode/Decode    │     │                                │
│   - Platform subclass│     │                                │
└──────────────────────┘     └────────────────────────────────┘
```

## Features

The cluster supports the following optional features:

-   **Wi-Fi USD Proximity Detection (WFUSDPD)**: Ranging based on Wi-Fi USD.
-   **Bluetooth Channel Sounding (BLTCS)**: Ranging based on Bluetooth Channel
    Sounding.
-   **BLE Beacon RSSI (BLERBC)**: Ranging based on BLE Beacon RSSI. Enables the
    `BLEDeviceId` attribute.

At least one feature must be enabled.

## CodegenIntegration

The cluster uses `CodegenIntegration.cpp` to bridge between ZAP-generated
endpoint configuration and the code-driven cluster. The framework calls
`MatterProximityRangingClusterInitCallback` which registers the cluster via
`LazyRegisteredServerCluster`. The application then calls
`FindClusterOnEndpoint()` to get the cluster instance and set the driver:

```cpp
#include <app/clusters/proximity-ranging-server/CodegenIntegration.h>

auto * cluster = ProximityRanging::FindClusterOnEndpoint(endpointId);
if (cluster != nullptr)
{
    cluster->SetDriver(&myDriver);
}
```

## Usage (CodeDriven pattern)

The all-devices-app demonstrates the recommended integration pattern where
platform-specific code owns the ranging infrastructure.

### 1. Platform Override File

Each platform (POSIX, ESP32) has a `DeviceFactoryPlatformOverride` that owns the
controller, driver, and adapter as statics:

```cpp
// DeviceFactoryPlatformOverride.cpp
namespace {
using namespace chip::app::Clusters::ProximityRanging;

DarwinBleRssiRangingAdapter sBleAdapter;
RangingTechnologyController sRangingController;
DefaultProximityRangingDriver sRangingDriver{ sRangingController, BitMask<Feature>(Feature::kBleBeaconRssi) };
} // namespace

void RegisterDeviceFactoryOverrides(TimerDelegate & timerDelegate, PersistentStorageDelegate * storageDelegate)
{
    sBleAdapter.Init(storageDelegate);
    sRangingController.RegisterAdapter(sBleAdapter);

    DeviceFactory::GetInstance().RegisterCreator("proximity-ranger", [&timerDelegate]() {
        return std::make_unique<ProximityRangerDevice>(ProximityRangerDevice::Context{
            .timerDelegate = timerDelegate,
            .driver        = sRangingDriver,
        });
    });
}
```

### 2. Implement a Platform Adapter

Platform adapters inherit from `BleRssiRangingAdapter` (which provides
encode/decode and technology identification) and implement session management:

```cpp
class DarwinBleRssiRangingAdapter : public PosixBleRssiRangingAdapter
{
public:
    ResultCodeEnum StartSession(uint8_t sessionId, const StartRangingRequest::DecodableType & request) override
    {
        AddSession(sessionId);
        // Start platform BLE advertising/scanning
        return ResultCodeEnum::kAccepted;
    }

    CHIP_ERROR StopSession(uint8_t sessionId) override
    {
        VerifyOrReturnError(RemoveSession(sessionId), CHIP_ERROR_NOT_FOUND);
        // Stop platform BLE if no sessions remain
        return CHIP_NO_ERROR;
    }
};
```

### 3. Lifecycle

The `DefaultProximityRangingDriver` connects to the controller via
`AddListener`/`RemoveListener`:

-   `Init()`: Stores the cluster callback, adds itself as a controller listener
-   `Shutdown()`: Removes itself as a listener, nulls the callback

The controller and adapters are static objects with app lifetime. The
controller's destruction calls `StopAllSessions()` on all adapters during
program exit.

## Driver Interface

| Method                     | Description                                           |
| -------------------------- | ----------------------------------------------------- |
| `Init(callback)`           | Store callback, register as listener                  |
| `Shutdown()`               | Remove as listener, null callback                     |
| `HandleStartRanging()`     | Start a ranging session (synchronous)                 |
| `HandleStopRanging()`      | Stop a ranging session (synchronous)                  |
| `GetRangingCapabilities()` | Encode the list of supported capabilities             |
| `GetFeatureMap()`          | Return the feature bitmap for this driver             |
| `GetActiveSessionIds()`    | Return active session IDs (for session ID generation) |
| `GetBleDeviceId()`         | Return BLE Device ID (optional, BLERBC feature)       |
| `GetWiFiDevIK()`           | Return Wi-Fi Device Identity Key (optional, WFUSDPD)  |
| `GetBLTDevIK()`            | Return BLT Device Identity Key (optional, BLTCS)      |
| `GetBLTCSSecurityLevel()`  | Return BLTCS security level (optional, BLTCS)         |
| `GetBLTCSModeCapability()` | Return BLTCS mode capability (optional, BLTCS)        |

## Attribute Change Notifications

Adapters can notify the cluster when attributes change (e.g., BLE Device ID
rotation). The notification chain is:

```
Adapter: mCallback->OnAttributeChanged(BLEDeviceID::Id)
    → Controller: broadcasts to all listeners
        → Driver: mCallback->OnAttributeChanged(attributeId)
            → Cluster: NotifyAttributeChanged(attributeId) → subscription reports
```

## Command Flow

`StartRangingRequest` and `StopRangingRequest` are handled synchronously:

```
Client                    Cluster                     Driver/Controller/Adapter
  │                         │                           │
  ├─ StartRangingRequest ──►│                           │
  │                         ├─ GenerateSessionId() ────►│ (GetActiveSessionIds)
  │                         ├─ HandleStartRanging() ───►│
  │                         │◄── ResultCodeEnum ────────┤
  │◄─ StartRangingResponse ─┤                           │
```

## Events

-   **RangingResult**: Emitted when measurement data is available. Delivered via
    `Callback::OnMeasurementData()`.
-   **RangingSessionStatus**: Emitted when a session ends asynchronously.
    Delivered via `Callback::OnSessionStopped()`.

## BLE Beacon Payload

The `BleRssiRangingAdapter` provides static encode/decode methods for the
proximity ranging BLE advertisement payload
(`ChipBLEProximityRangingIdentificationInfo`, 21 bytes):

```cpp
BleRssiRangingAdapter::EncodeBeaconPayload(bleDeviceId, msgCounter, txPower, sessionKey, outPayload);
BleRssiRangingAdapter::DecodeBeaconPayload(payload, candidateId, sessionKey);
```

The BLE Device ID is obfuscated in the beacon using HMAC (not yet implemented;
currently encoded in plain form).

## Persistent Storage

The `BleRssiRangingAdapter::Init(PersistentStorageDelegate *)` loads or
generates the BLE Device ID and persists it under key `"g/pr/bledevid"`. This
ensures the device maintains a stable identity across reboots.

## Multi-Endpoint Support

Multiple endpoints can share a single `RangingTechnologyController` and adapter
set. Each endpoint has its own or can share the `DefaultProximityRangingDriver`
which registers as a listener on the shared controller. All listeners receive
session events and attribute change notifications.

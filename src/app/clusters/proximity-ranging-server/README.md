# Proximity Ranging Cluster

The Proximity Ranging cluster enables the configuration of proximity ranging
sessions and reporting of proximity ranging data between Matter devices using
various technologies (BLE Beacon RSSI, Bluetooth Channel Sounding, Wi-Fi USD,
UWB).

## Overview

This directory contains a code-driven C++ implementation of the Matter Proximity
Ranging cluster server (cluster ID `0x0433`). The implementation follows the
`DefaultServerCluster` pattern with a `Config` + Builder API for feature
configuration and `CodegenIntegration` for ZAP-based deployments. All
technology-specific runtime operations are delegated to a
`ProximityRangingDriver` set via `SetDriver()`.

The cluster handles:

-   Matter protocol interactions (attribute reads, command dispatch)
-   Feature map ownership (immutable, set at construction via Config)
-   Session ID allocation and tracking
-   Spec preflight validation of `StartRangingRequest` (technology in feature
    map, matching `DeviceRoleConfig` present, role enum within the matching
    family, trigger and reporting-condition constraints) before any request
    reaches the driver
-   Event emission for ranging results and session status changes
-   Attribute change notifications from the driver

The application is responsible for:

-   Configuring which features are supported (via `Config` builders)
-   Implementing the `ProximityRangingDriver` interface for runtime behavior
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
│ Application-supplied driver                                 │
│   - DefaultProximityRangingDriver in all-devices-app routes │
│     to RangingTechnologyController + per-tech adapters      │
└─────────────────────────────────────────────────────────────┘
```

## Features

The cluster supports the following optional features, configured at construction
time via `Config::WithFeatures()`:

-   **Wi-Fi USD Proximity Detection (WFUSDPD)**:
    `Feature::kWiFiUsdProximityDetection` — enables ranging based on Wi-Fi USD
    and the `WiFiDevIK` attribute.
-   **Bluetooth Channel Sounding (BLTCS)**: `Feature::kBluetoothChannelSounding`
    — enables ranging based on Bluetooth Channel Sounding and the `BLTDevIK`,
    `BLTCSSecurityLevel`, and `BLTCSModeCapability` attributes.
-   **BLE Beacon RSSI (BLERBC)**: `Feature::kBleBeaconRssi` — enables ranging
    based on BLE Beacon RSSI and the `BLEDeviceId` attribute.
-   **UWB Ranging (UWB)**: `Feature::kUwbRanging` — enables ranging based on
    UWB.

`WithFeatures()` sets the feature bits and marks the mandatory associated
attributes as present. At least one feature must be enabled.

## CodegenIntegration

The cluster uses `CodegenIntegration.cpp` to bridge between ZAP-generated
endpoint configuration and the code-driven cluster. The framework calls
`MatterProximityRangingClusterInitCallback` which registers the cluster via
`LazyRegisteredServerCluster`. The bridge reads the feature map from ember
attribute storage and passes it directly to `Config::WithFeatures()`:

```cpp
// Inside CodegenIntegration.cpp — CreateRegistration()
ProximityRangingCluster::Config config(endpointId);
config.WithFeatures(BitMask<Feature>(featureMap));

gServers[idx].Create(config);
```

The application then calls `FindClusterOnEndpoint()` to get the cluster instance
and set the driver:

```cpp
#include <app/clusters/proximity-ranging-server/CodegenIntegration.h>

auto * cluster = ProximityRanging::FindClusterOnEndpoint(endpointId);
if (cluster != nullptr)
{
    cluster->SetDriver(&myDriver);
}
```

## Driver Interface

| Method                     | Description                                           |
| -------------------------- | ----------------------------------------------------- |
| `Init(callback)`           | Store callback, register as listener                  |
| `Shutdown()`               | Remove as listener, null callback                     |
| `HandleStartRanging()`     | Start a ranging session (synchronous)                 |
| `HandleStopRanging()`      | Stop a ranging session (synchronous)                  |
| `GetRangingCapabilities()` | Encode the list of supported capabilities             |
| `GetActiveSessionIds()`    | Return active session IDs (for session ID generation) |
| `GetBleRbcConfig()`        | Return `BleRbcConfig` (optional, BLERBC feature)      |
| `GetWiFiUsdConfig()`       | Return `WiFiUsdConfig` (optional, WFUSDPD feature)    |
| `GetBltcsConfig()`         | Return `BltcsConfig` (optional, BLTCS feature)        |

See `ProximityRangingDriver.h` for callback method documentation.

## Preflight Validation

`HandleStartRangingRequest` runs spec-derived validation before invoking the
driver. A request is rejected with `kRejectedInfeasibleRanging` if any of the
following hold:

-   The requested technology's feature bit is not set in `FeatureMap`.
-   The matching `DeviceRoleConfig` Optional is missing, OR a non-matching one
    is present (inconsistent role configuration).
-   The role enum is not in the family for the requested technology (e.g.,
    `kBLTInitiatorRole` for `kBLEBeaconRSSIRanging`).
-   `ReportingCondition.MinDistanceCondition == 0` or
    `MaxDistanceCondition == 0`.

Trigger-shape violations are rejected with `kRejectedInfeasibleRangingTriggers`:

-   `Trigger.EndTime <= Trigger.StartTime`.
-   `Trigger.RangingInstanceInterval == 0` (when present).

Drivers can rely on these checks and only need to handle technology-specific
infeasibility (e.g., requested frequency band unsupported by hardware).

## Command Flow

`StartRangingRequest` and `StopRangingRequest` are handled synchronously:

```
Client                    Cluster                          Driver
  │                         │                                │
  ├─ StartRangingRequest ──►│                                │
  │                         ├─ ValidateStartRangingRequest() │
  │                         ├─ GenerateSessionId() ─────────►│ (GetActiveSessionIds)
  │                         ├─ HandleStartRanging() ────────►│
  │                         │◄── ResultCodeEnum ─────────────┤
  │◄─ StartRangingResponse ─┤                                │
```

## Events

-   **RangingResult**: Emitted when measurement data is available. Delivered via
    `Callback::OnMeasurementData()`.
-   **RangingSessionStatus**: Emitted when a session ends asynchronously.
    Delivered via `Callback::OnSessionStopped()`.

## Attribute Change Notifications

Drivers notify the cluster when an attribute they own changes (e.g., BLE Device
ID rotation) by calling `Callback::OnAttributeChanged(attributeId)`. The cluster
forwards this as an attribute-changed report so subscribers observe the new
value.

## Multi-Endpoint Support

Multiple endpoints each construct their own `ProximityRangingCluster` instance
with its own feature map. Drivers may be shared (one driver registering with
multiple cluster instances) or per-endpoint depending on the application.

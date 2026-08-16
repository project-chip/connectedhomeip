# Proximity Ranging Cluster

The Proximity Ranging cluster enables the configuration of proximity ranging
sessions and reporting of proximity ranging data between Matter devices using
various technologies (BLE Beacon RSSI, Bluetooth Channel Sounding, Wi-Fi USD,
UWB).

## Overview

This directory contains a code-driven C++ implementation of the Matter Proximity
Ranging cluster server (cluster ID `0x0433`). The implementation follows the
`DefaultServerCluster` pattern with a `Config` + Builder API for feature
configuration and `CodegenIntegration` for ZAP-based deployments.
Technology-specific runtime operations are delegated to a
`ProximityRangingDriver` (concrete, supplied by the SDK) which fans out to one
`RangingAdapter` per supported technology.

The cluster handles:

-   Matter protocol interactions (attribute reads, command dispatch)
-   Feature map ownership (immutable, set at construction via Config)
-   Spec early validation of `StartRangingRequest` (technology in feature map,
    matching `DeviceRoleConfig` present, role enum within the matching family,
    trigger and reporting-condition constraints) before any request reaches the
    driver
-   Event emission for ranging results and session status changes
-   Attribute change notifications from the driver

The `ProximityRangingDriver` (SDK) handles:

-   Session ID allocation and bookkeeping (session → adapter mapping)
-   Routing `StartRangingRequest` / `StopRangingRequest` to the adapter that
    owns the requested technology / session
-   Aggregating `RangingCapabilities` and feature bits across registered
    adapters
-   Forwarding async adapter callbacks (measurement data, session-stopped,
    attribute-changed) to the cluster

The application is responsible for:

-   Implementing one `RangingAdapter` per supported ranging technology
-   Supplying those adapters as a `Span<RangingAdapter * const>` to the driver's
    constructor — the adapter set is fixed for the driver's lifetime
-   Persisting any per-adapter device identity values (e.g. BLE Device ID)

## Architecture

```
┌─────────────────────────────────────────────────────────────┐
│ ProximityRangingCluster (SDK, per-endpoint)                 │
│   - ReadAttribute, InvokeCommand, Events                    │
│   - Config { ProximityRangingDriver & driver }              │
└──────────────────────────┬──────────────────────────────────┘
                           │ ProximityRangingDriver::Callback
┌──────────────────────────▼──────────────────────────────────┐
│ ProximityRangingDriver (SDK, concrete)                      │
│   - Session ID allocation + session→adapter table           │
│   - Routes Start/Stop to the adapter owning the technology  │
│   - Aggregates capabilities / supported features            │
└──────────────────────────┬──────────────────────────────────┘
                           │ RangingAdapter interface
┌──────────────────────────▼──────────────────────────────────┐
│ Application-supplied RangingAdapters (one per technology)   │
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
attributes as present. At least one feature must be enabled. The application
declares the feature mask explicitly at the construction site so the cluster's
spec contract is visible at the call site rather than inferred at runtime from
the registered adapter set; keeping the two in sync is a configuration
responsibility, not something the driver derives.

## Codegen Integration

`ProximityRangingCluster` requires a `ProximityRangingDriver &` at construction.
That reference is owned by the application, so the auto-generated
`MatterProximityRangingClusterInitCallback` cannot construct the cluster — by
the time it fires the application has not yet had a chance to provide a driver.

The codegen integration is therefore a no-op stub. Codegen consumers instead use
`ProximityRangingServer`, which owns a `ProximityRangingCluster` bound to an
endpoint and registers it with the codegen data model provider's registry from
the application's post-`Server::Init()` hook.

```cpp
#include <app/clusters/proximity-ranging-server/CodegenIntegration.h>
#include <app/clusters/proximity-ranging-server/ProximityRangingDriver.h>

namespace {
constexpr EndpointId kRangingEndpoint = 2;

MyBleAdapter   sBleAdapter;
MyWiFiAdapter  sWiFiAdapter;
MyBltcsAdapter sBltcsAdapter;

ProximityRanging::RangingAdapter * sAdapters[] = { &sBleAdapter, &sWiFiAdapter, &sBltcsAdapter };
ProximityRanging::ProximityRangingDriver sDriver{ Span<ProximityRanging::RangingAdapter * const>(sAdapters) };
ProximityRanging::ProximityRangingServer sServer(kRangingEndpoint, sDriver);
} // namespace

void ApplicationInit()
{
    constexpr BitMask<ProximityRanging::Feature> kFeatures{
        ProximityRanging::Feature::kBleBeaconRssi,
        ProximityRanging::Feature::kWiFiUsdProximityDetection,
        ProximityRanging::Feature::kBluetoothChannelSounding,
    };
    if (CHIP_ERROR err = sServer.Init(kFeatures); err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Proximity Ranging init failed: %" CHIP_ERROR_FORMAT, err.Format());
    }
}

void ApplicationShutdown()
{
    sServer.Deinit();
}
```

Code-driven applications construct `ProximityRangingCluster` directly via its
`Config(driver)` constructor — they do not need the wrapper because they already
build the cluster registration tree explicitly.

## RangingAdapter Interface

Each supported technology is implemented as a `RangingAdapter`. The application
constructs one adapter per technology and supplies them to the driver as a
`Span` at construction.

| Method                  | Description                                              |
| ----------------------- | -------------------------------------------------------- |
| `GetTechnology()`       | Returns the `RangingTechEnum` this adapter implements    |
| `GetCapabilities()`     | Returns the `RangingCapabilitiesStruct` for this adapter |
| `StartSession()`        | Start a ranging session (synchronous)                    |
| `StopSession()`         | Stop a ranging session (synchronous)                     |
| `StopAllSessions()`     | Stop every active session (called by driver on shutdown) |
| `GetActiveSessionIds()` | Append this adapter's active session IDs to a Span       |
| `GetDeviceId()`         | Optional: per-device 64-bit ID (BLERBC adapters)         |
| `GetWiFiUsdConfig()`    | Optional: `WiFiUsdConfig` (Wi-Fi USD adapters only)      |
| `GetBltcsConfig()`      | Optional: `BltcsConfig` (BLTCS adapters only)            |

Adapters report async results to the driver through `RangingAdapter::Callback`:

| Callback                    | When emitted                                       |
| --------------------------- | -------------------------------------------------- |
| `OnMeasurementData()`       | Per-session measurement available                  |
| `OnRangingSessionStopped()` | Session ended (timeout, request, hardware failure) |
| `OnAttributeChanged()`      | Adapter-owned attribute value changed              |

The driver implements this Callback and forwards each event to the cluster after
translating session IDs into the cluster's session-event sink.

## Adapter Set

The adapter set is fixed at driver construction — it reflects the device's
physical radio configuration, which does not change at runtime.

```cpp
// Backing array must outlive the driver.
RangingAdapter * sAdapters[] = { &sBleAdapter, &sWiFiAdapter, &sBltcsAdapter };
ProximityRangingDriver sDriver{ Span<RangingAdapter * const>(sAdapters) };
```

Each pointer must be non-null and each adapter's `GetTechnology()` must be
unique within the set; violations terminate via `VerifyOrDie` because static
composition errors are configuration bugs. Up to
`CHIP_CONFIG_PROXIMITY_RANGING_MAX_SESSIONS` concurrent sessions are supported
across all adapters.

## Early Validation

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

Adapters can rely on these checks and only need to handle technology-specific
limitations (e.g., requested frequency band unsupported by hardware).

## Command Flow

`StartRangingRequest` and `StopRangingRequest` are handled synchronously:

```
Client                    Cluster                   Driver                Adapter
  │                         │                         │                     │
  ├─ StartRangingRequest ──►│                         │                     │
  │                         ├─ Validate request ─────►│                     │
  │                         ├─ HandleStartRanging ───►│                     │
  │                         │                         ├─ FindAdapter(tech)  │
  │                         │                         ├─ allocate session   │
  │                         │                         ├─ StartSession ─────►│
  │                         │                         │◄── ResultCodeEnum ──┤
  │                         │◄── ResultCodeEnum ──────┤                     │
  │◄─ StartRangingResponse ─┤                         │                     │
```

## Events

-   **RangingResult**: Emitted when measurement data is available. Delivered
    through the adapter's `Callback::OnMeasurementData()`, forwarded by the
    driver as `ProximityRangingDriver::Callback::OnMeasurementData()`.
-   **RangingSessionStatus**: Emitted when a session ends asynchronously.
    Delivered through `Callback::OnRangingSessionStopped()` on the adapter side
    and `Callback::OnSessionStopped()` on the cluster side.

## Attribute Change Notifications

Adapters notify the driver when an attribute they own changes (e.g., BLE Device
ID rotation) by calling `RangingAdapter::Callback::OnAttributeChanged()`. The
driver forwards this through
`ProximityRangingDriver::Callback::OnAttributeChanged()` to the cluster, which
in turn marks the attribute dirty so subscribers observe the new value.

## Multi-Endpoint Support

Each endpoint constructs its own `ProximityRangingCluster` with its own feature
map. `ProximityRangingDriver` currently retains a single cluster callback at a
time, so a driver instance is bound to a single cluster instance for the
duration of `Init()` / `Shutdown()`. Applications that expose proximity ranging
on multiple endpoints should construct one driver (with its own adapter set) per
endpoint.

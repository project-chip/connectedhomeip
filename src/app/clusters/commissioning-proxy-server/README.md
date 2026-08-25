# Commissioning Proxy Cluster

The Commissioning Proxy cluster (cluster ID 0x0455) provides a proxy service
that allows a Commissioner to use commissioning transports not supported
locally, or to extend its commissioning range. Commissioners can use the proxy
to discover and establish a connection to commissionable devices that are
reachable by the proxy. The proxy connection acts as a tunnel through which the
Commissioner can run a PASE session with the commissionable device and
ultimately commission it.

## Overview

This directory contains an implementation of the Matter Commissioning Proxy
cluster server (`CommissioningProxyCluster.{h,cpp}`) using the code-driven
`DefaultServerCluster` pattern.

The cluster owns **all transport-agnostic behavior**: command validation,
feature-based attribute/command gating, transport/WiFiBand constraint checks,
the writable-attribute storage and change-reporting, and — via three subsystems
it composes — proxy-session bookkeeping, `ProxyMessage` routing, the
background-scan result cache, and multi-transport `ProxyScanRequest`
aggregation:

| Subsystem                          | Responsibility                                                                                                            |
| ---------------------------------- | ------------------------------------------------------------------------------------------------------------------------- |
| `CommissioningProxySessionManager` | Session-id allocation, id→{transport,fabric} table, per-fabric isolation, `ProxyMessage` routing + response-timeout timer |
| `CommissioningProxyScanCache`      | `CachedResults` / `NumCachedResults` (one entry per device, per-entry TTL, `MaxCachedResults` cap)                        |
| `CommissioningProxyScanAggregator` | Combines a multi-transport `ProxyScanRequest` into one `ProxyScanResponse`                                                |

A fourth transport-agnostic component, `CommissioningProxyBgScanRegistry`, ships
with the cluster but is **instantiated per transport and owned by the driver**
rather than composed by the cluster:
`ProxyBackGroundScanStartRequest`/`ProxyBackGroundScanStopRequest` fan out to
every matching driver, so the per-fabric records they arbitrate are necessarily
transport-local. It holds the per-fabric scan requests and their lifetime
timers, the spec transport/band overlap arithmetic on Stop, and the
paused/deferred state used while the radio is held by a connect or foreground
scan — leaving the driver only the hardware start/stop. See
[Background scanning](#background-scanning).

The application supplies **only the platform-specific transport work**. A BLE
driver, `CommissioningProxyBleTransport`, ships with the cluster and needs only
a `CommissioningProxyBleAdapter` from the application; for any other physical
transport the application implements the `CommissioningProxyTransport` driver
interface itself and registers it with the cluster. There is no do-everything
application delegate: static device capabilities are passed to the constructor
via `Config`, writable-attribute state is owned by the cluster, and the driver
is scoped to transport actions only. See
[Architecture](#architecture-why-a-driver-not-a-delegate) below.

### How it works

A Commissioner that lacks (or is out of range of) the transport a commissionable
device advertises on uses the proxy as a tunnel. The flow is:

1. **Discover** — the Commissioner sends `ProxyScanRequest` (or subscribes to
   the background-scan cache); the proxy scans its local transports and returns
   the commissionable devices it can see as `ScanResultStruct` entries.
2. **Connect** — the Commissioner sends `ProxyConnectRequest` naming a single
   transport plus the target device (address/discriminator/VID/PID). The proxy
   opens a transport connection to the device and replies with a unique
   `SessionID` in the `ProxyConnectResponse`.
3. **Tunnel** — the Commissioner drives the PASE handshake (and the rest of
   commissioning) by sending each Matter packet in a `ProxyMessageRequest` keyed
   by `SessionID`; the proxy forwards it over the transport and returns the
   device's reply in the `ProxyMessageResponse`. The proxy is a dumb relay — the
   PASE session is end-to-end between the Commissioner and the device.
4. **Disconnect** — the Commissioner sends `ProxyDisconnectRequest` to cancel an
   in-flight connect; the proxy tears the transport connection down.

The cluster server itself is **transport-agnostic**: it validates the requested
transport against the set it advertises, then dispatches the work to the
registered `CommissioningProxyTransport` driver whose `GetTransportType()`
matches the request's transport bit. A **BLE** (BTP) driver ships today; the
**Wi-Fi PAF** (PAFTP) driver follows in a later PR. See the transport
integration sections below.

## Features

The cluster supports the following optional features:

-   **WiFiNetworkInterface (WI)**: Gates the Wi-Fi-band surface — the `WiFiBand`
    attribute and the `WiFiBand(s)` command fields. It does **not** gate the
    Wi-Fi PAF transport itself (see Transports below).
-   **BackgroundScan (BGS)**: Allows background scanning of commissioning
    transports, with results cached in the `CachedResults` / `NumCachedResults`
    attributes.

## Transports

The transports the proxy advertises in the `Transport` attribute (and accepts in
`ProxyConnectRequest` / `ProxyScanRequest`) are exactly the ones for which a
driver has been registered (`RegisterTransport`), independent of the `WI`
feature. An application typically registers a driver per compiled-in transport:

| Transport (`CapabilitiesBitmap`) | Driver registered when              | Wire protocol        |
| -------------------------------- | ----------------------------------- | -------------------- |
| `kBle` (BTP)                     | `CONFIG_NETWORK_LAYER_BLE`          | BTP over BLE         |
| `kWiFiPAF` (PAFTP)               | `CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF` | PAFTP over Wi-Fi NAN |

`ProxyConnectRequest` selects **exactly one** transport per session. A request
naming a transport with no registered driver is rejected with
`INVALID_TRANSPORT_TYPE`. (The spec's `NTL` bit is not currently supported.)

## Architecture: why a driver, not a delegate

Per the SDK cluster guidance, a cluster that triggers platform-specific actions
uses a **Delegate (or Driver) interface**, while a cluster's own attribute state
is pushed through setters rather than fetched from a delegate. This cluster
follows that split precisely, so instead of one do-everything delegate there are
four homes for what such a delegate would otherwise hold:

| Concern                                                           | Lives in                                            | Why                                                                                                              |
| ----------------------------------------------------------------- | --------------------------------------------------- | ---------------------------------------------------------------------------------------------------------------- |
| Transport actions (Connect / Scan / SendMessage / Disconnect / …) | `CommissioningProxyTransport` driver                | The only genuinely platform-specific surface (BlueZ GATT, wpa_supplicant NAN).                                   |
| Writable attributes (`ScanMaxTime`, `CacheTimeout`) + cache view  | The cluster (members + setters) and its subsystems  | The cluster owns change-reporting, so a driver can never forget `NotifyAttributeChanged`.                        |
| Static capabilities (`MaxSessions`, `MaxCachedResults`, bands)    | `Config` (constructor argument)                     | Fixed device facts, not actions.                                                                                 |
| Background-scan fabric records, band arithmetic, pause/resume     | `CommissioningProxyBgScanRegistry` (one per driver) | Transport-agnostic logic, but held per transport because background Start/Stop fan out to every matching driver. |

**How multiple platforms plug in.** A platform provides one
`CommissioningProxyTransport` implementation per physical transport and
registers it with `RegisterTransport()`. The cluster dispatches each command to
the driver whose `GetTransportType()` matches the request's transport bit, and
drivers report async results back through the cluster's shared subsystems
(`Sessions()`, `ScanCache()`, `ScanAggregator()`). Background scanning is
inherited the same way: the driver holds a `CommissioningProxyBgScanRegistry`
and implements its three-method `HardwareControl` hook. A new platform therefore
writes only its GATT/NAN driver and inherits all session/scan/message
bookkeeping unchanged.

`RegisterTransport()` is used (rather than a single constructor-injected
delegate) because there can be several drivers — one per transport — and they
are typically constructed after the cluster, whose endpoint is only known at
registration time.

## Usage

For new applications using the `CodeDrivenDataModelProvider`, instantiate and
register the cluster directly. This provides the most flexibility and control.

### 1. Implement a transport driver

BLE needs no driver of your own — register the cluster's own
`CommissioningProxyBleTransport` (see
[BLE Transport Integration](#ble-transport-integration)). For a transport the
SDK does not ship, create a class that inherits from
`chip::app::Clusters::CommissioningProxy::CommissioningProxyTransport` and
implement its virtual methods — one class per physical transport; Wi-Fi PAF is
the example below. A driver only handles the transport-specific work; the
cluster performs all spec validation first and owns the session/scan/message
bookkeeping. Methods run on the Matter task; the driver reports async results
back through its host cluster (given in `SetHost`) via `Sessions()`,
`ScanCache()`, and `ScanAggregator()`.

```cpp
#include <app/clusters/commissioning-proxy-server/CommissioningProxyTransport.h>

class MyPafTransport : public chip::app::Clusters::CommissioningProxy::CommissioningProxyTransport
{
public:
    CapabilitiesBitmap GetTransportType() const override { return CapabilitiesBitmap::kWiFiPAF; }
    void SetHost(CommissioningProxyCluster * cluster) override { mHost = cluster; }

    // Open a transport connection. On success, allocate a session via
    // mHost->Sessions().AllocSessionId()/RegisterSession() and call
    // commandObj->AddResponse() with a ProxyConnectResponse carrying the sessionID.
    Protocols::InteractionModel::Status Connect(chip::app::CommandHandler * commandObj,
                                                const DataModel::InvokeRequest & request, uint16_t discriminator,
                                                System::Clock::Seconds16 timeout) override;

    // Forward a Matter packet; deliver the reply via
    // mHost->Sessions().DispatchMessageResponse().
    CHIP_ERROR SendMessage(uint16_t sessionId, chip::System::PacketBufferHandle && buf) override;

    // Report foreground-scan results to mHost->ScanAggregator().Contribute();
    // background-scan results to mHost->ScanCache().Report().
    Protocols::InteractionModel::Status Scan(System::Clock::Seconds16 scanMaxTime) override;

    // Background scan: forward to the driver's CommissioningProxyBgScanRegistry,
    // which owns the per-fabric records, lifetime timers and band arithmetic
    // (see Background scanning below).
    Protocols::InteractionModel::Status BgScanStart(System::Clock::Seconds16 timeout, BitMask<WiFiBandBitmap> wiFiBands,
                                                    FabricIndex fabricIndex, NodeId nodeId) override;
    Protocols::InteractionModel::Status BgScanStop(BitMask<CapabilitiesBitmap> transport, BitMask<WiFiBandBitmap> wiFiBands,
                                                   FabricIndex fabricIndex, NodeId nodeId) override;

    // ... CancelPendingConnect / Disconnect / OnAllSessionsClosed /
    //     IsConnectPending / Shutdown

private:
    CommissioningProxyCluster * mHost = nullptr;
};
```

### 2. Instantiate the cluster and register the driver(s)

Construct the `CommissioningProxyCluster` for the endpoint (the endpoint id is a
constructor argument; `Config` carries the fixed feature set and device
capabilities), then register a driver per available transport.

```cpp
#include <app/clusters/commissioning-proxy-server/CommissioningProxyCluster.h>
#include <app/server-cluster/ServerClusterInterfaceRegistry.h>

using namespace chip::app::Clusters::CommissioningProxy;

MyPafTransport gPafTransport;

chip::BitMask<Feature> gFeatures(Feature::kBackgroundScan, Feature::kWiFiNetworkInterface);

// Supplies the response-timeout and scan-watchdog timers.
chip::app::DefaultTimerDelegate gTimerDelegate;

// Config: featureFlags plus the supported Wi-Fi bands. MaxSessions and
// MaxCachedResults are Fixed-quality attributes and come from
// CHIP_CONFIG_COMMISSIONING_PROXY_MAX_SESSIONS / _MAX_CACHED_RESULTS.
chip::app::RegisteredServerCluster<CommissioningProxyCluster> gCPCluster(
    CommissioningProxyEndpoint, CommissioningProxyCluster::Config(gFeatures), gTimerDelegate);

void SetUpProxy()
{
    gCPCluster.Cluster().RegisterTransport(gPafTransport); // before registration/Startup
}
```

### 3. Register the Cluster

In your application's initialization sequence, register the cluster instance
with the `CodegenDataModelProvider`:

```cpp
#include <data-model-providers/codegen/CodegenDataModelProvider.h>

void ApplicationInit()
{
    VerifyOrDie(chip::app::CodegenDataModelProvider::Instance().Registry().Register(
        gCPCluster.Registration()) == CHIP_NO_ERROR);
}
```

For a complete working example of the device wiring, see
`examples/all-devices-app/all-devices-common/device/types/commissioning-proxy/`.
It registers the cluster's `CommissioningProxyBleTransport` and supplies the
platform adapter from `examples/all-devices-app/posix/linux/`. The Wi-Fi PAF
driver joins it in a later PR.

## Transport driver methods

A `CommissioningProxyTransport` driver implements the following. Each is invoked
only after the cluster has validated the request; the driver does the
transport-specific work and reports results back through the host cluster's
subsystems.

| Method                   | Description                                                                                                                                                                            |
| ------------------------ | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `GetTransportType()`     | The single transport bit this driver services (`kBle` / `kWiFiPAF`)                                                                                                                    |
| `SetHost()`              | Bind the host cluster (set to null at cluster teardown)                                                                                                                                |
| `Connect()`              | Open a transport session; allocate + register it via `Sessions()`                                                                                                                      |
| `SendMessage()`          | Forward a packet; reply routed back via `Sessions()`                                                                                                                                   |
| `Scan()`                 | Foreground scan; results reported to `ScanAggregator()`                                                                                                                                |
| `BgScanStart()`          | Start a background scan for the given `(fabricIndex, nodeId)`; normally forwarded to the driver's `CommissioningProxyBgScanRegistry::Start()`. Results reported to `ScanCache()` (BGS) |
| `BgScanStop()`           | Stop a background scan for the given `(fabricIndex, nodeId)`; normally forwarded to `CommissioningProxyBgScanRegistry::Stop()` (BGS)                                                   |
| `CancelPendingConnect()` | Cancel an in-flight connect (null-SessionID disconnect)                                                                                                                                |
| `Disconnect()`           | Tear down an active proxy session                                                                                                                                                      |
| `OnAllSessionsClosed()`  | Notified when the last session across all transports closes                                                                                                                            |
| `IsConnectPending()`     | Whether a connect is in flight (counted against `MaxSessions`)                                                                                                                         |
| `Shutdown()`             | Cancel driver timers/state before cluster destruction                                                                                                                                  |

Note there is **no** `ProxyScanRequest`/`ProxyMessageRequest`/etc. delegate
hook: those commands' spec logic, session tracking, message routing, and scan
aggregation live in the cluster and its subsystems; the driver only exposes the
transport primitives above.

## Background scanning

`ProxyBackGroundScanStartRequest`/`ProxyBackGroundScanStopRequest` are
per-fabric and MAY name several transports, so the cluster validates the request
(BGS/WI feature gating, reserved transport and band bits, supported bands) and
then fans it out to every registered driver whose transport bit is set, passing
the requesting fabric index and node id taken from the subject descriptor. Start
reports the first non-success status a driver returned; Stop returns `NOT_FOUND`
only when no driver had a matching record.

Everything a driver then has to do that is _not_ radio work is provided by
`CommissioningProxyBgScanRegistry` — one instance per driver, constructed with a
reference to the driver's `HardwareControl` implementation:

| Registry call      | Responsibility                                                                                                                                      |
| ------------------ | --------------------------------------------------------------------------------------------------------------------------------------------------- |
| `Start()`          | Add or refresh the `(fabricIndex, nodeId)` record and its transport/band mask, arm its lifetime timer, start the hardware if it is the first record |
| `Stop()`           | Remove the requested transports/bands from the record; stop the hardware once no records remain                                                     |
| `Pause()`          | Suspend the hardware scan while the radio is needed for a connect or foreground scan; records stay registered; a second call does nothing           |
| `ResumeIfNeeded()` | Restart a paused scan once the radio is free; no-op if not paused or if no records remain                                                           |
| `Shutdown()`       | Cancel every lifetime timer and stop the hardware scan if the registry owns it                                                                      |

`Start()` with `timeoutSecs == 0` means no lifetime timer, i.e. scan until an
explicit Stop. `Stop()` applies the spec's transport/band arithmetic: a zero
transport mask means "stop the listed bands only"; a record left with no
transports or no bands is removed; `SUCCESS` is returned even when nothing
overlapped, and `NOT_FOUND` only when the fabric has no record at all.

The driver supplies the only transport-specific parts via `HardwareControl`:

| Hook                   | Contract                                                                                                         |
| ---------------------- | ---------------------------------------------------------------------------------------------------------------- |
| `StartHardwareScan()`  | Start or resume the hardware scan, wiring the driver's own discovery callback (return codes below)               |
| `StopHardwareScan()`   | Stop the hardware scan; called only while the registry owns the radio, never while paused                        |
| `ClearCachedResults()` | Drop cached results for the bands that stopped (`host->ScanCache().ClearTransport(<transport>, bands)`); 0 = all |

`StartHardwareScan()` returns `CHIP_NO_ERROR` when the scan is running,
`CHIP_ERROR_BUSY` when the radio is currently held — the registry keeps the
records, stays paused and retries on the next `ResumeIfNeeded()` — and any other
error is a hard failure that rejects the triggering `Start()`.

A driver therefore wires background scanning up as a small `HardwareControl`
implementation plus one registry instance, and forwards the two driver methods
to it:

```cpp
#include <app/clusters/commissioning-proxy-server/CommissioningProxyBgScanRegistry.h>
#include <platform/DefaultTimerDelegate.h>

// The registry drives these hooks from its own lifetime timer and from resume, i.e.
// outside the driver's call stack, so the hardware object holds its own host pointer.
// MyPafTransport::SetHost() hands it over: mHost = cluster; sHardware.SetHost(cluster);
class MyPafBgScanHardware : public CommissioningProxyBgScanRegistry::HardwareControl
{
public:
    void SetHost(CommissioningProxyCluster * host) { mHost = host; }

    // Returns CHIP_ERROR_BUSY when the single scanner is held by a connect or a
    // foreground scan; the registry then defers and retries on resume.
    CHIP_ERROR StartHardwareScan() override { return StartMyPlatformScan(OnBgScanDiscovery); }
    void StopHardwareScan() override { StopMyPlatformScan(); }

    // bands names the bands that stopped; 0 means the transport stopped entirely.
    void ClearCachedResults(BitMask<WiFiBandBitmap> bands) override
    {
        if (mHost != nullptr)
        {
            mHost->ScanCache().ClearTransport(CapabilitiesBitmap::kWiFiPAF, bands);
        }
    }

private:
    CommissioningProxyCluster * mHost = nullptr;
};

// Declared before the registry so it outlives it: the registry's destructor may
// call back into these hooks.
MyPafBgScanHardware sHardware;
chip::app::DefaultTimerDelegate sBgScanTimerDelegate;
CommissioningProxyBgScanRegistry sBgScan(sHardware, sBgScanTimerDelegate);

Status MyPafTransport::BgScanStart(System::Clock::Seconds16 timeout, BitMask<WiFiBandBitmap> wiFiBands, FabricIndex fabricIndex,
                                   NodeId nodeId)
{
    return sBgScan.Start(fabricIndex, nodeId, GetTransportType(), wiFiBands, timeout);
}

Status MyPafTransport::BgScanStop(BitMask<CapabilitiesBitmap> transport, BitMask<WiFiBandBitmap> wiFiBands,
                                 FabricIndex fabricIndex, NodeId nodeId)
{
    return sBgScan.Stop(fabricIndex, nodeId, transport, wiFiBands);
}
```

`CHIP_ERROR_BUSY` is the load-bearing case: on BLE the scanner is owned by a
connect or foreground scan, and on Wi-Fi PAF a `ProxyConnect` owns the single
NAN subscribe slot. A driver reports the conflict from `StartHardwareScan()`,
calls `Pause()` when it takes the radio and `ResumeIfNeeded()` when it releases
it, and the registry keeps the Commissioner's background scan registered across
the gap. `ResumeIfNeeded()` must be wrapped in `PlatformMgr().ScheduleWork()` if
the "radio freed" path could otherwise re-enter the driver.

## Async Command Handling

`ProxyConnectRequest` and `ProxyMessageRequest` are asynchronous — they return
`std::nullopt` from `InvokeCommand` to prevent an immediate response, and call
`CommandHandler::AddResponse()` or `CommandHandler::AddStatus()` from a
transport callback once the operation completes.

To keep the exchange alive across the async operation, store a
`CommandHandler::Handle` and extend the exchange response timeout:

```cpp
// Store the handle before returning nullopt
CommandHandler::Handle handle(commandObj);
if (auto * ec = commandObj->GetExchangeContext())
{
    ec->SetResponseTimeout(chip::System::Clock::Seconds16(responseTimeout + 10));
}
// … return std::nullopt from InvokeCommand …

// Later, in your transport callback:
auto * handler = handle.Get();
if (handler != nullptr)
{
    handler->AddResponse(commandPath, response);
}
```

## BLE Transport Integration

`CommissioningProxyBleTransport` ships with the cluster as the separate
`ble-transport` target, so an application that proxies over another transport —
or a platform with no BLE stack — can depend on `commissioning-proxy-server`
without pulling in `src/ble`. It drives a BTP connection through
`chip::Ble::BleLayer`:

-   `Connect()` — on the first BLE connect the proxy flips its own BLE role from
    peripheral to central, then calls
    `BleLayer::NewBleConnectionByDiscriminator()` to open an L2CAP/BTP
    connection to the commissionee.
-   `SendMessage()` — calls `BLEEndPoint::Send()` to push the tunneled
    commissioning packet over BTP.
-   `Disconnect()` — calls `BLEEndPoint::Close()` to drop the connection.

Incoming BTP messages are routed back to the cluster via a `ProxyBleDelegate`
(`chip::Ble::BleLayerDelegate`) that wraps the original `BleLayer` transport,
matches the connection against the active session slots, and calls
`host->Sessions().DispatchMessageResponse()`.

### The platform adapter

Two BLE operations have no portable form, so they are not in the transport:
switching the local BLE role from peripheral to central, and driving a scan for
commissionable devices. The application supplies both by implementing
`CommissioningProxyBleAdapter` and passing it to the transport's constructor:

```cpp
class MyBleProxyAdapter : public CommissioningProxyBleAdapter
{
    CHIP_ERROR EnableCentralRole() override;
    CHIP_ERROR StartScan(DiscoveryCallback cb, void * context) override;
    void StopScan() override;
};

MyBleProxyAdapter gBleAdapter;
CommissioningProxyBleTransport gBleTransport(gBleAdapter, gTimerDelegate);
```

`examples/all-devices-app/posix/linux/CommissioningProxyBleAdapter.cpp` is a
worked implementation over `BLEManagerImpl`, wired up in that app's
`posix/linux/DeviceFactoryPlatformOverride.cpp`.

## Wi-Fi PAF Transport Integration (planned)

No Wi-Fi PAF driver ships yet; this section describes the shape the one landing
in a later PR takes, and is the reference for anyone writing a PAF driver
against this cluster in the meantime.

When the build enables Wi-Fi PAF (`CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF`) the
driver (`CommissioningProxyPafTransport`) interacts with
`chip::WiFiPAF::WiFiPAFLayer` to open, send over, receive from, and close PAF
(NAN) sessions:

-   `Connect()` — calls `WiFiPAFLayer::WiFiPAFSubscribe()` to open a PAF session
    identified by the commissionee discriminator and peer address.
-   `SendMessage()` — calls `WiFiPAFLayer::SendMessage()` to send the tunneled
    commissioning packet over PAFTP.
-   `Disconnect()` — calls `WiFiPAFLayer::RmPafSession()` to release the PAF
    session.

Incoming PAF messages are routed back to the cluster via a
`WiFiPAFLayerDelegate` subclass that intercepts `WiFiPAFMessageReceived`,
matches the peer against the active session map, and calls
`host->Sessions().DispatchMessageResponse()`.

## Cluster State

The cluster tracks proxy state internally:

-   `kState_CPDisconnected` — no active proxy session
-   `kState_CPConnected` — a transport session (BLE or Wi-Fi PAF) is open and
    ready to forward messages

State transitions:

```text
ProxyConnectRequest ──► transport connect success ──► kState_CPConnected
kState_CPConnected  ──► ProxyDisconnectRequest    ──► kState_CPDisconnected
```

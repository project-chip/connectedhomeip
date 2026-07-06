# Commissioning Proxy Cluster

The Commissioning Proxy cluster (cluster ID 0x0455, Matter spec) provides a
proxy service that allows a Commissioner to use commissioning transports not
supported locally, or to extend its commissioning range. Commissioners can use
the proxy to discover and establish a connection to commissionable devices that
are reachable by the proxy. The proxy connection acts as a tunnel through which
the Commissioner can run a PASE session with the commissionable device and
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

The application supplies **only the platform-specific transport work** by
implementing the `CommissioningProxyTransport` driver interface (one per
physical transport, e.g. BLE or Wi-Fi PAF) and registering it with the cluster.
There is no do-everything application delegate: static device capabilities are
passed to the constructor via `Config`, writable-attribute state is owned by the
cluster, and the driver is scoped to transport actions only. See
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
   in-flight connect); the proxy tears the transport connection down.

The cluster server itself is **transport-agnostic**: it validates the requested
transport against the set it advertises, then dispatches the work to the
registered `CommissioningProxyTransport` driver whose `GetTransportType()`
matches the request's transport bit. Today drivers exist for **BLE** (BTP) and
**Wi-Fi PAF** (PAFTP) — see the transport integration sections below.

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
three homes for what such a delegate would otherwise hold:

| Concern                                                           | Lives in                                           | Why                                                                                       |
| ----------------------------------------------------------------- | -------------------------------------------------- | ----------------------------------------------------------------------------------------- |
| Transport actions (Connect / Scan / SendMessage / Disconnect / …) | `CommissioningProxyTransport` driver               | The only genuinely platform-specific surface (BlueZ GATT, wpa_supplicant NAN).            |
| Writable attributes (`ScanMaxTime`, `CacheTimeout`) + cache view  | The cluster (members + setters) and its subsystems | The cluster owns change-reporting, so a driver can never forget `NotifyAttributeChanged`. |
| Static capabilities (`MaxSessions`, `MaxCachedResults`, bands)    | `Config` (constructor argument)                    | Fixed device facts, not actions.                                                          |

**How multiple platforms plug in.** A platform provides one
`CommissioningProxyTransport` implementation per physical transport and
registers it with `RegisterTransport()`. The cluster dispatches each command to
the driver whose `GetTransportType()` matches the request's transport bit, and
drivers report async results back through the cluster's shared subsystems
(`Sessions()`, `ScanCache()`, `ScanAggregator()`). A new platform therefore
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

Create a class that inherits from
`chip::app::Clusters::CommissioningProxy::CommissioningProxyTransport` and
implement its virtual methods — one class per physical transport. A driver only
handles the transport-specific work; the cluster performs all spec validation
first and owns the session/scan/message bookkeeping. Methods run on the Matter
task; the driver reports async results back through its host cluster (given in
`SetHost`) via `Sessions()`, `ScanCache()`, and `ScanAggregator()`.

```cpp
#include <app/clusters/commissioning-proxy-server/CommissioningProxyTransport.h>

class MyBleTransport : public chip::app::Clusters::CommissioningProxy::CommissioningProxyTransport
{
public:
    CapabilitiesBitmap GetTransportType() const override { return CapabilitiesBitmap::kBle; }
    void SetHost(CommissioningProxyCluster * cluster) override { mHost = cluster; }

    // Open a transport connection. On success, allocate a session via
    // mHost->Sessions().AllocSessionId()/RegisterSession() and call
    // commandObj->AddResponse() with a ProxyConnectResponse carrying the sessionID.
    Protocols::InteractionModel::Status Connect(chip::app::CommandHandler * commandObj,
                                                const DataModel::InvokeRequest & request, uint16_t discriminator,
                                                uint16_t timeout) override;

    // Forward a Matter packet; deliver the reply via
    // mHost->Sessions().DispatchMessageResponse().
    CHIP_ERROR SendMessage(uint16_t sessionId, chip::System::PacketBufferHandle && buf) override;

    // Report foreground-scan results to mHost->ScanAggregator().Contribute();
    // background-scan results to mHost->ScanCache().Report().
    Protocols::InteractionModel::Status Scan(uint8_t scanMaxTime) override;

    // ... CancelPendingConnect / Disconnect / BgScanStart / BgScanStop /
    //     OnAllSessionsClosed / IsConnectPending / Shutdown

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

MyBleTransport gBleTransport;

chip::BitMask<Feature> gFeatures(Feature::kBackgroundScan, Feature::kWiFiNetworkInterface);

// Config: featureFlags, MaxSessions, MaxCachedResults, supported Wi-Fi bands.
chip::app::RegisteredServerCluster<CommissioningProxyCluster> gCPCluster(
    CommissioningProxyEndpoint, CommissioningProxyCluster::Config(gFeatures, /*aMaxSessions=*/1, /*aMaxCachedResults=*/10));

void SetUpProxy()
{
    gCPCluster.Cluster().RegisterTransport(gBleTransport); // before registration/Startup
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

A complete working example (device wiring plus the BLE and Wi-Fi PAF drivers) is
under
`examples/all-devices-app/all-devices-common/devices/commissioning-proxy/`.

## Codegen integration

For applications that register clusters through the `CodegenDataModelProvider`,
`CodegenIntegration.{h,cpp}` provide an `Instance` wrapper that constructs a
`RegisteredServerCluster<CommissioningProxyCluster>` and adds/removes it from
the provider registry on `Init()`/`Shutdown()`. Transports are registered on it
before `Init()`:

```cpp
#include <app/clusters/commissioning-proxy-server/CodegenIntegration.h>

using namespace chip::app::Clusters::CommissioningProxy;

Instance gCPInstance(endpointId, CommissioningProxyCluster::Config(Feature::kWiFiNetworkInterface));

void ApplicationInit()
{
    gCPInstance.RegisterTransport(gBleTransport);
    gCPInstance.Init();
}
```

## Transport driver methods

A `CommissioningProxyTransport` driver implements the following. Each is invoked
only after the cluster has validated the request; the driver does the
transport-specific work and reports results back through the host cluster's
subsystems.

| Method                   | Description                                                         |
| ------------------------ | ------------------------------------------------------------------- |
| `GetTransportType()`     | The single transport bit this driver services (`kBle` / `kWiFiPAF`) |
| `SetHost()`              | Bind the host cluster (set to null at cluster teardown)             |
| `Connect()`              | Open a transport session; allocate + register it via `Sessions()`   |
| `SendMessage()`          | Forward a packet; reply routed back via `Sessions()`                |
| `Scan()`                 | Foreground scan; results reported to `ScanAggregator()`             |
| `BgScanStart()`          | Start a background scan; results reported to `ScanCache()` (BGS)    |
| `BgScanStop()`           | Stop a background scan (BGS)                                        |
| `CancelPendingConnect()` | Cancel an in-flight connect (null-SessionID disconnect)             |
| `Disconnect()`           | Tear down an active proxy session                                   |
| `OnAllSessionsClosed()`  | Notified when the last session across all transports closes         |
| `IsConnectPending()`     | Whether a connect is in flight (counted against `MaxSessions`)      |
| `Shutdown()`             | Cancel driver timers/state before cluster destruction               |

Note there is **no** `ProxyScanRequest`/`ProxyMessageRequest`/etc. delegate
hook: those commands' spec logic, session tracking, message routing, and scan
aggregation live in the cluster and its subsystems; the driver only exposes the
transport primitives above.

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

When the build enables BLE (`CONFIG_NETWORK_LAYER_BLE`) the driver
(`CommissioningProxyBleTransport`) drives a BTP connection through
`chip::Ble::BleLayer`. The Linux example
(`examples/all-devices-app/all-devices-common/devices/commissioning-proxy/CommissioningProxyBleTransport.cpp`)
shows the full integration:

-   `Connect()` — on the first BLE connect the proxy flips its own BLE role from
    peripheral to central via `BLEManagerImpl::SwitchToCentralMode()` (a one-way
    switch; `IsCentralMode()` reports the state and central-mode advertising is
    then refused), then calls `BleLayer::NewBleConnectionByDiscriminator()` to
    open an L2CAP/BTP connection to the commissionee.
-   `SendMessage()` — calls `BLEEndPoint::Send()` to push the tunneled
    commissioning packet over BTP.
-   `Disconnect()` — calls `BLEEndPoint::Close()` to drop the connection.

Incoming BTP messages are routed back to the cluster via a `BleProxyDelegate`
(`chip::Ble::BleLayerDelegate`) that wraps the original `BleLayer` transport,
matches the connection against the active session map, and calls
`host->Sessions().DispatchMessageResponse()`.

## Wi-Fi PAF Transport Integration

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

```
ProxyConnectRequest ──► transport connect success ──► kState_CPConnected
kState_CPConnected  ──► ProxyDisconnectRequest    ──► kState_CPDisconnected
```

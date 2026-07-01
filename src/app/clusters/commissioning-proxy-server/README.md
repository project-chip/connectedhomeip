# Commissioning Proxy Cluster

The Commissioning Proxy cluster (cluster ID 0x0455, Matter spec §10.5) provides
a proxy service that allows a Commissioner to use commissioning transports not
supported locally, or to extend its commissioning range. Commissioners can use
the proxy to discover and establish a connection to commissionable devices that
are reachable by the proxy. The proxy connection acts as a tunnel through which
the Commissioner can run a PASE session with the commissionable device and
ultimately commission it.

## Overview

This directory contains an implementation of the Matter Commissioning Proxy
cluster server. The implementation (`CommissioningProxyCluster.h` and
`CommissioningProxyCluster.cpp`) is designed for flexibility and requires a
delegate to handle the application-specific logic.

The cluster handles command validation, feature-based attribute and command
filtering, and opt-out state checking. The application is responsible for
implementing the `CommissioningProxy::Delegate` interface to perform the actual
transport management (e.g. BLE or Wi-Fi PAF session setup and teardown).

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
4. **Disconnect** — the Commissioner sends `ProxyDisconnectRequest` (or a null
   `SessionID` to cancel an in-flight connect); the proxy tears the transport
   connection down.

The cluster server itself is **transport-agnostic**: it validates the requested
transport against the set it advertises, then hands the work to the delegate,
which owns all transport-specific logic. Today the delegate can drive **BLE**
(BTP) and **Wi-Fi PAF** (PAFTP) connections — see the transport integration
sections below.

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
`ProxyConnectRequest` / `ProxyScanRequest`) are derived purely from the build's
compile flags, independent of the `WI` feature:

| Transport (`CapabilitiesBitmap`) | Advertised when                     | Wire protocol        |
| -------------------------------- | ----------------------------------- | -------------------- |
| `kBle` (BTP)                     | `CONFIG_NETWORK_LAYER_BLE`          | BTP over BLE         |
| `kWiFiPAF` (PAFTP)               | `CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF` | PAFTP over Wi-Fi NAN |

`ProxyConnectRequest` selects **exactly one** transport per session. A request
naming a transport this build does not advertise is rejected with
`INVALID_TRANSPORT_TYPE`. (The spec's `NTL` bit is not currently supported.)

## Usage

For new applications using the `CodeDrivenDataModelProvider`, instantiate and
register the cluster directly. This provides the most flexibility and control.

### 1. Implement the Delegate

Create a class that inherits from
`chip::app::Clusters::CommissioningProxy::Delegate` and implement its virtual
methods. The delegate handles all application-specific transport logic and is
called from the Matter task; the implementation is responsible for any required
synchronization.

```cpp
#include <app/clusters/commissioning-proxy-server/CommissioningProxyDelegate.h>

class MyCPDelegate : public chip::app::Clusters::CommissioningProxy::Delegate
{
public:
    // Scan for commissionable devices using the requested transport and bands.
    Protocols::InteractionModel::Status
    ProxyScanRequest(chip::app::Clusters::CommissioningProxy::CapabilitiesBitmap transport,
                     chip::app::Clusters::CommissioningProxy::WiFiBandBitmap wiFiBands,
                     chip::app::CommandHandler * commandObj,
                     const DataModel::InvokeRequest & request) override;

    // Open a transport connection to the commissionee identified by the request.
    // On success the delegate calls commandObj->AddResponse() with a
    // ProxyConnectResponse carrying the session ID.
    Protocols::InteractionModel::Status
    ProxyConnectRequest(DataModel::Nullable<chip::ByteSpan> address,
                        chip::BitMask<chip::app::Clusters::CommissioningProxy::CapabilitiesBitmap> transport,
                        uint16_t discriminator, chip::VendorId vendorid, uint16_t productid, uint16_t timeout,
                        chip::BitMask<chip::app::Clusters::CommissioningProxy::WiFiBandBitmap> wiFiBand,
                        chip::app::CommandHandler * commandObj,
                        const DataModel::InvokeRequest & request) override;

    // Forward a commissioning message to the commissionee and return the response.
    Protocols::InteractionModel::Status
    ProxyMessageRequest(uint16_t sessionId, chip::Optional<chip::ByteSpan> message, uint8_t responseTimeout,
                        chip::app::CommandHandler * commandObj,
                        const DataModel::InvokeRequest & request) override;

    // Tear down a proxy session.
    Protocols::InteractionModel::Status
    ProxyDisconnectRequest(uint16_t sessionId, chip::FabricIndex fabricIndex) override;

    // Attribute accessors for delegate-owned state (device capabilities and the
    // scan-result cache). The writable ScanMaxTime / CacheTimeout attributes are
    // stored and change-reported by the cluster, not the delegate; read the live
    // value through GetServer()->GetScanMaxTime() / GetCacheTimeout() if needed.
    uint8_t GetMaxSessions() override;
    uint8_t GetActiveSessionCount() override;

    // ... implement other required attribute getters
};
```

### 2. Instantiate the Delegate and Cluster

Instantiate your delegate and the `CommissioningProxyCluster` for each required
endpoint. Use `RegisteredServerCluster` to simplify registration.

```cpp
#include <app/clusters/commissioning-proxy-server/CommissioningProxyCluster.h>
#include <app/server-cluster/ServerClusterInterfaceRegistry.h>

// In a .cpp file
chip::app::Clusters::CommissioningProxy::MyCPDelegate gMyCPDelegate;

chip::BitMask<chip::app::Clusters::CommissioningProxy::Feature> gFeatures(
    chip::app::Clusters::CommissioningProxy::Feature::kBackgroundScan,
    chip::app::Clusters::CommissioningProxy::Feature::kWiFiNetworkInterface
);

// The endpoint id is passed to the cluster constructor (it is only known at
// registration time); the Config carries the fixed feature set and delegate.
chip::app::RegisteredServerCluster<chip::app::Clusters::CommissioningProxy::CommissioningProxyCluster> gCPCluster(
    CommissioningProxyEndpoint,
    chip::app::Clusters::CommissioningProxy::CommissioningProxyCluster::Config(gFeatures, gMyCPDelegate)
);
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

A complete working example can be found in
`examples/commissioning-proxy-app/linux/main.cpp`.

## Legacy Usage

For backwards compatibility with applications that rely on ZAP-generated
patterns, a compatibility layer is provided in `CodegenIntegration.h` and
`CodegenIntegration.cpp`.

```cpp
#include <app/clusters/commissioning-proxy-server/CodegenIntegration.h>

chip::app::Clusters::CommissioningProxy::Instance gCPInstance(
    endpointId, myDelegate,
    chip::app::Clusters::CommissioningProxy::Feature::kWiFiNetworkInterface
);

void ApplicationInit()
{
    gCPInstance.Init();
}
```

## Delegate Methods

The delegate must implement the following methods:

| Method                              | Description                                         |
| ----------------------------------- | --------------------------------------------------- |
| `ProxyScanRequest()`                | Scan for commissionable devices                     |
| `ProxyConnectRequest()`             | Open a transport session to the commissionee        |
| `ProxyMessageRequest()`             | Forward a commissioning message to the commissionee |
| `ProxyDisconnectRequest()`          | Tear down an active proxy session                   |
| `ProxyBackGroundScanStartRequest()` | Start a background scan (BGS feature)               |
| `ProxyBackGroundScanStopRequest()`  | Stop background scanning (BGS feature)              |

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

When the build enables BLE (`CONFIG_NETWORK_LAYER_BLE`) the delegate drives a
BTP connection through `chip::Ble::BleLayer`. The Linux example
(`examples/commissioning-proxy-app/linux/CommissioningProxyBleTransport.cpp`)
shows the full integration:

-   `ProxyConnectRequest` — on the first BLE connect the proxy flips its own BLE
    role from peripheral to central via `BLEManagerImpl::SwitchToCentralMode()`
    (a one-way switch; `IsCentralMode()` reports the state and central-mode
    advertising is then refused), then calls
    `BleLayer::NewBleConnectionByDiscriminator()` to open an L2CAP/BTP
    connection to the commissionee.
-   `ProxyMessageRequest` — calls `BLEEndPoint::Send()` to push the tunneled
    commissioning packet over BTP.
-   `ProxyDisconnectRequest` — calls `BLEEndPoint::Close()` to drop the
    connection.

Incoming BTP messages are routed back to the cluster via a `BleProxyDelegate`
(`chip::Ble::BleLayerDelegate`) that wraps the original `BleLayer` transport and
matches the connection against the active session map.

## Wi-Fi PAF Transport Integration

When the build enables Wi-Fi PAF (`CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF`) the
delegate interacts with `chip::WiFiPAF::WiFiPAFLayer` to open, send over,
receive from, and close PAF (NAN) sessions:

-   `ProxyConnectRequest` — calls `WiFiPAFLayer::WiFiPAFSubscribe()` to open a
    PAF session identified by the commissionee discriminator and peer address.
-   `ProxyMessageRequest` — calls `WiFiPAFLayer::SendMessage()` to send the
    tunneled commissioning packet over PAFTP.
-   `ProxyDisconnectRequest` — calls `WiFiPAFLayer::RmPafSession()` to release
    the PAF session.

Incoming PAF messages are routed back to the cluster via a
`WiFiPAFLayerDelegate` subclass that intercepts `WiFiPAFMessageReceived` and
matches the peer against the active session map.

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

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
transport management (e.g. Wi-Fi PAF session setup and teardown).

## Features

The cluster supports the following optional features:

-   **WiFiNetworkInterface (WI)**: Allows Wi-Fi PAF commissioning
-   **BackgroundScan (BGS)**: Allows background scanning of commissioning
    transports

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

    // Open a PAF session to the commissionee identified by the request.
    Protocols::InteractionModel::Status
    ProxyConnectRequest(const Commands::ProxyConnectRequest::DecodableType & request,
                        chip::app::CommandHandler * commandObj,
                        const chip::app::ConcreteCommandPath & commandPath) override;

    // Forward a commissioning message to the commissionee and return the response.
    Protocols::InteractionModel::Status
    ProxyMessageRequest(const Commands::ProxyMessageRequest::DecodableType & request,
                        chip::app::CommandHandler * commandObj,
                        const chip::app::ConcreteCommandPath & commandPath) override;

    // Tear down a proxy session.
    Protocols::InteractionModel::Status
    ProxyDisconnectRequest(const Commands::ProxyDisconnectRequest::DecodableType & request,
                           chip::app::CommandHandler * commandObj,
                           const chip::app::ConcreteCommandPath & commandPath) override;

    // Attribute accessors
    uint8_t GetScanMaxTime() override;
    void    SetScanMaxTime(uint8_t seconds) override;

    // ... implement other required attribute getters/setters
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

chip::app::RegisteredServerCluster<chip::app::Clusters::CommissioningProxy::CommissioningProxyCluster> gCPCluster(
    chip::app::Clusters::CommissioningProxy::CommissioningProxyCluster::Config(
        CommissioningProxyEndpoint, gFeatures, gMyCPDelegate)
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

## Wi-Fi PAF Transport Integration

When the `kWiFiNetworkInterface` feature is enabled the delegate interacts with
`chip::WiFiPAF::WiFiPAFLayer` to open, send over, receive from, and close PAF
(NAN) sessions:

-   `ProxyConnectRequest` — calls `WiFiPAFLayer::WiFiPAFSubscribe()` to open a
    PAF session identified by the commissionee's discriminator and peer address.
-   `ProxyMessageRequest` — calls `WiFiPAFLayer::SendMessage()` to send the
    tunnelled commissioning packet over PAFTP.
-   `ProxyDisconnectRequest` — calls `WiFiPAFLayer::RmPafSession()` to release
    the PAF session.

Incoming PAF messages are routed back to the cluster via a
`WiFiPAFLayerDelegate` subclass that intercepts `WiFiPAFMessageReceived` and
matches the peer against the active session map.

## Cluster State

The cluster tracks proxy state internally:

-   `kState_CPDisconnected` — no active proxy session
-   `kState_CPConnected` — a PAF session is open and ready to forward messages

State transitions:

```
ProxyConnectRequest ──► OnPafConnectSuccess ──► kState_CPConnected
kState_CPConnected  ──► ProxyDisconnectRequest ──► kState_CPDisconnected
```

# Commissioning Proxy Cluster

This Commisioning Proxy cluster provides a proxy service allowing a Commissioner
to utilize commissioning transports not supported locally, or to extend the
commissioning range of the Commissioner. Commissioners can use the proxy to
discover and establish a connection to commissionable devices that are reachable
by the proxy. The proxy connection acts as a tunnel through which the
Commissioner can establish a PASE session with the commissionable device, and
ultimatelycommission it.

## Overview

This directory contains an implementation of the Matter Commissioning Proxy
cluster server. This implementation (`CommissioningProxyCluster.h` and
`CommissioningProxyCluster.cpp`) is designed for flexibility and requires a
delegate to handle the appliance-specific logic.

The cluster implementation handles command validation, feature-based attribute
and command filtering, and opt-out state checking. The application is
responsible for implementing the `CommissioingProxy::Delegate` interface to
handle the actual selected commissioning management.

## Features

The cluster supports the following optional features:

-   **WiFiNetworkInterface (WI)**: Allows Wi-Fi PAF Commissioning
-   **BackgroundScan (BGS)**: Allows background scan of commissioning transports

## Usage

For new applications using the `CodeDrivenDataModelProvider`, we strongly
recommend instantiating and registering the cluster directly. This approach
provides the most flexibility and control.

### 1. Implement the Delegate

Create a class that inherits from
`chip::app::Clusters::CommissioningProxy::Delegate` and implement its
virtual methods. The delegate handles all appliance-specific logic. This
Delegate will be accessed by the chip task to interact with the application. It
is the Delegate's implementation's responsibility to put proper synchronization
mechanisms during those interaction.

```cpp
#include <app/clusters/commissioing-proxy-server/CommissioningProxyDelegate.h>

class Delegate
{
public:
    virtual ~Delegate() = default;

    void SetEndpointId(EndpointId aEndpoint) { mEndpointId = aEndpoint; }
    EndpointId GetEndpointId() const { return mEndpointId; }

    /**
     * @brief Handles starting a Scan Request of required Transport and Bands
     *
     * @param transport  Which transports to use
     * @param wiFiBands  The frequency of the transports (if applicable)
     * @param commandObj The command handler object from the command
     * @param request    Invokde path
     * @return Success if successful
     *
     * The delegate handles all the transport specific options.
     */
    virtual Protocols::InteractionModel::Status
    ProxyScanRequest(chip::app::Clusters::CommissioningProxy::CapabilitiesBitmap transport,
                     chip::app::Clusters::CommissioningProxy::WiFiBandBitmap wiFiBands,
                     chip::app::CommandHandler * commandObj,
                     const DataModel::InvokeRequest & request) = 0;

    // ... implement other required methods

    // ------------------------------------------------------------------
    // Get attribute methods
    virtual uint8_t GetScanMaxTime()         = 0;

    // ------------------------------------------------------------------
    // Set attribute methods
    virtual void SetScanMaxTime(uint8_t seconds) = 0;

    // ... implement other getters and setters

protected:
    EndpointId mEndpointId = 0;
};
```

### 2. Instantiate Delegate and Cluster

Instantiate your delegate and the `CommissioningProxyCluster` for each
required endpoint. Using `RegisteredServerCluster` simplifies registration.

```cpp
#include <app/clusters/device-energy-management-server/CommissioningProxyCluster.h>
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
with either the `CodegenDataModelProvider` (legacy) or the
`CodeDrivenDataModelProvider`.

```cpp
#include <data-model-providers/codegen/CodegenDataModelProvider.h>

void ApplicationInit()
{
    // Register the Commissioning Proxy Code Driven mechanism
    VerifyOrDie(chip::app::CodegenDataModelProvider::Instance().Registry().Register(gCPCluster.Registration()) == CHIP_NO_ERROR);
    // ...
}
```

## Legacy Usage

For backwards compatibility with applications that rely on older ZAP-generated
patterns, a compatibility layer is provided in `CodegenIntegration.h` and
`CodegenIntegration.cpp`.

In this model, you configure the  cluster in your `.zap`
file and use the `Instance` class:

```cpp
#include <app/clusters/device-energy-management-server/CodegenIntegration.h>

// Create and initialize the instance
chip::app::Clusters::CommissioningProxy::Instance gCPInstance(
    endpointId, myDelegate, chip::app::Clusters::CommissioningProxy::Feature::kPowerAdjustment
);

void ApplicationInit()
{
    gCPInstance.Init();
}
```

## Delegate Methods

The delegate must implement the following methods:

| Method                             | Description                            |
| ---------------------------------- | -------------------------------------- |
| `ProxyConnectRequest()`            | Connect Commissioner to CP             |
| `ProxyDisconnectRequest()  `       | Disconnect Commissioner from CP        |
| `ProxyScanRequest()`               | Scan for commissionable devices        |
| `ProxyMessageRequest()`            | Send Commissioning messages to CP      |
| `ProxyBackGroundScanStartRequest()`| Start a background scan                |
| `ProxyBackGroundScanStopRequest()` | Stop background scanning               |

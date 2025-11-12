# Occupancy Sensing Cluster

The Occupancy Sensing cluster is used to measure and report the occupancy state in an area. For example, it can be used with a Passive Infrared (PIR) sensor or an ultrasonic sensor to determine whether a room is occupied or unoccupied.

## Overview

This directory contains a modern, code-driven C++ implementation of the Matter Occupancy Sensing cluster server. This implementation (`OccupancySensingCluster.h` and `OccupancySensingCluster.cpp`) is designed for flexibility, avoiding the tight coupling present in older ZAP/Ember-based implementations.

A key feature of this implementation is that it is completely decoupled from the underlying sensor hardware. The previous `occupancy-hal.h` has been removed, and the application is now responsible for initializing its own sensor hardware and notifying the cluster of occupancy changes by calling the `SetOccupancy()` method.

The cluster implementation now internally handles all timer logic related to the `HoldTime`, `PIROccupiedToUnoccupiedDelay`, `UltrasonicOccupiedToUnoccupiedDelay`, and `PhysicalContactOccupiedToUnoccupiedDelay` attributes, simplifying the application's responsibility.

It uses an optional delegate pattern (`chip::app::Clusters::OccupancySensingDelegate`) to notify the application about changes to the occupancy state or the hold time configuration.

## Modern Code-Driven Usage (Recommended)

For new applications using the `CodeDrivenDataModelProvider`, we strongly recommend instantiating and registering the cluster directly. This approach provides the most flexibility and control.

### 1. Implement the Delegate (Optional)

If your application needs to be notified of changes, create a class that inherits from `chip::app::Clusters::OccupancySensingDelegate` and implement its virtual methods.

```cpp
#include "app/clusters/occupancy-sensor-server/OccupancySensingCluster.h"

class MyOccupancyDelegate : public chip::app::Clusters::OccupancySensingDelegate
{
public:
    void OnOccupancyChanged(bool occupied) override
    {
        // Your logic to react to an occupancy change
    }

    void OnHoldTimeChanged(uint16_t holdTime) override
    {
        // Your logic to react to a hold time change
    }
};
```

### 2. Instantiate Delegates and Cluster

Instantiate your optional delegate, a timer delegate (if using the Hold Time feature), and the `OccupancySensingCluster` itself for each required endpoint. Using `RegisteredServerCluster` simplifies registration.

The `HoldTime` feature and its related attributes are optional. To enable them, call the `.WithHoldTime()` method on the `Config` object. If it is not called, these attributes will be disabled.

```cpp
#include "app/DefaultTimerDelegate.h"
#include "app/clusters/occupancy-sensor-server/OccupancySensingCluster.h"
#include "app/server-cluster/ServerClusterInterfaceRegistry.h"

// In a .cpp file
MyOccupancyDelegate gMyOccupancyDelegate;
DefaultTimerDelegate gTimerDelegate;

constexpr chip::app::Clusters::OccupancySensing::Structs::HoldTimeLimitsStruct::Type kHoldTimeLimits = {
    .holdTimeMin = 1, .holdTimeMax = 300, .holdTimeDefault = 60
};

chip::app::RegisteredServerCluster<chip::app::Clusters::OccupancySensingCluster> gOccupancyCluster(
    chip::app::Clusters::OccupancySensingCluster::Config(kYourEndpointId)
        .WithFeatures(chip::app::Clusters::OccupancySensing::Feature::kPassiveInfrared)
        .WithHoldTime(60, kHoldTimeLimits, gTimerDelegate) // Enable HoldTime feature
        .WithDelegate(gMyOccupancyDelegate)                // Attach optional delegate
);
```

### 3. Register the Cluster

In your application's initialization sequence, register the cluster instance with either the `CodegenDataModelProvider` (legacy) or the `CodeDrivenDataModelProvider`.

```cpp
#include "data-model-providers/codegen/CodegenDataModelProvider.h"

void ApplicationInit()
{
    // ... other initializations
    CHIP_ERROR err = chip::app::CodegenDataModelProvider::Instance().Registry().Register(gOccupancyCluster.Registration());
    // ...
}
```

### 4. Notify the Cluster of Occupancy Changes

Your application is responsible for monitoring the physical sensor. When the sensor state changes, call the `SetOccupancy()` method on the cluster instance.

```cpp
void MySensorHardwareCallback(bool isOccupied)
{
    // Get the cluster instance (e.g., via the RegisteredServerCluster object)
    chip::app::Clusters::OccupancySensingCluster * cluster = gOccupancyCluster.Get();
    if (cluster)
    {
        cluster->SetOccupancy(isOccupied);
    }
}
```

## Legacy Ember-Style Usage

For backwards compatibility with applications that rely on older ZAP-generated patterns (like the `all-clusters-app`), a compatibility layer is provided in `CodegenIntegration.h` and `CodegenIntegration.cpp`.

In this model, you configure the Occupancy Sensing cluster in your `.zap` file. The ZAP tool generates `MatterOccupancySensingClusterInitCallback`, which is implemented by our `CodegenIntegration` layer to automatically instantiate and configure the cluster based on your ZAP configuration.

To use the cluster in this mode, your application can get a pointer to the cluster instance and call its methods directly using `OccupancySensing::FindClusterOnEndpoint(endpointId)`.

```cpp
// In your application logic file
#include <app/clusters/occupancy-sensor-server/CodegenIntegration.h>

void MySensorHardwareCallback(bool isOccupied)
{
    chip::app::Clusters::OccupancySensingCluster * cluster = chip::app::Clusters::OccupancySensing::FindClusterOnEndpoint(endpointId);
    if (cluster != nullptr)
    {
        cluster->SetOccupancy(isOccupied);
    }
    else
    {
        ChipLogError(NotSpecified, "OccupancySensingCluster not found on endpoint %u", static_cast<unsigned>(endpointId));
    }
}
```

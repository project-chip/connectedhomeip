# Device Energy Management Cluster

The Device Energy Management (DEM) cluster allows energy management systems to
communicate with Energy Smart Appliances (ESAs) to optimize energy usage. It
enables power adjustments, forecast scheduling, and constraint-based power
management for devices like EVSEs, water heaters, and other smart appliances.

## Overview

This directory contains an implementation of the Matter Device Energy Management
cluster server. This implementation (`DeviceEnergyManagementCluster.h` and
`DeviceEnergyManagementCluster.cpp`) is designed for flexibility and requires a
delegate to handle the appliance-specific logic.

The cluster implementation handles command validation, feature-based attribute
and command filtering, and opt-out state checking. The application is
responsible for implementing the `DeviceEnergyManagement::Delegate` interface to
handle the actual power adjustments and forecast management as well as event
generation and timing compliance with the specification.

## Features

The cluster supports the following optional features:

-   **PowerAdjustment (PA)**: Allows adjustment of power consumption/generation
-   **PowerForecastReporting (PFR)**: Reports power forecasts
-   **StateForecastReporting (SFR)**: Reports state forecasts
-   **StartTimeAdjustment (STA)**: Allows adjustment of forecast start time
-   **Pausable (PAU)**: Allows pausing appliance operation
-   **ForecastAdjustment (FA)**: Allows modification of forecasts
-   **ConstraintBasedAdjustment (CON)**: Allows constraint-based forecast
    requests

## Usage

For new applications using the `CodeDrivenDataModelProvider`, we strongly
recommend instantiating and registering the cluster directly. This approach
provides the most flexibility and control.

### 1. Implement the Delegate

Create a class that inherits from
`chip::app::Clusters::DeviceEnergyManagement::Delegate` and implement its
virtual methods. The delegate handles all appliance-specific logic. This
Delegate will be accessed by the chip task to interact with the application. It
is the Delegate's implementation's responsibility to put proper synchronization
mechanisms during those interaction.

```cpp
#include <app/clusters/device-energy-management-server/DeviceEnergyManagementDelegate.h>

class MyDEMDelegate : public chip::app::Clusters::DeviceEnergyManagement::Delegate
{
public:
    Protocols::InteractionModel::Status PowerAdjustRequest(int64_t power, uint32_t duration,
                                                           AdjustmentCauseEnum cause) override
    {
        // lock the app task

        // Implement power adjustment logic

        // unlock the app task
        return Protocols::InteractionModel::Status::Success;
    }

    Protocols::InteractionModel::Status CancelPowerAdjustRequest() override
    {
        // lock the app task

        // Cancel ongoing power adjustment

        // unlock the app task
        return Protocols::InteractionModel::Status::Success;
    }

    // ... implement other required methods

    ESATypeEnum GetESAType() override { return ESATypeEnum::kEvse; }
    bool GetESACanGenerate() override { return false; }
    ESAStateEnum GetESAState() override { return mESAState; }
    // ... implement other getters

private:
    ESAStateEnum mESAState = ESAStateEnum::kOnline;
};
```

### 2. Instantiate Delegate and Cluster

Instantiate your delegate and the `DeviceEnergyManagementCluster` for each
required endpoint. Using `RegisteredServerCluster` simplifies registration.

```cpp
#include <app/clusters/device-energy-management-server/DeviceEnergyManagementCluster.h>
#include <app/server-cluster/ServerClusterInterfaceRegistry.h>

// In a .cpp file
MyDEMDelegate gMyDEMDelegate;

chip::BitMask<chip::app::Clusters::DeviceEnergyManagement::Feature> gFeatures(
    chip::app::Clusters::DeviceEnergyManagement::Feature::kPowerAdjustment,
    chip::app::Clusters::DeviceEnergyManagement::Feature::kPowerForecastReporting
);

chip::app::RegisteredServerCluster<chip::app::Clusters::DeviceEnergyManagementCluster> gDEMCluster(
    chip::app::Clusters::DeviceEnergyManagementCluster::Config(kYourEndpointId, gFeatures, &gMyDEMDelegate)
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
    // ... other initializations
    CHIP_ERROR err = chip::app::CodegenDataModelProvider::Instance().Registry().Register(gDEMCluster.Registration());
    // ...
}
```

## Legacy Usage

For backwards compatibility with applications that rely on older ZAP-generated
patterns, a compatibility layer is provided in `CodegenIntegration.h` and
`CodegenIntegration.cpp`.

In this model, you configure the Device Energy Management cluster in your `.zap`
file and use the `Instance` class:

```cpp
#include <app/clusters/device-energy-management-server/CodegenIntegration.h>

// Create and initialize the instance
chip::app::Clusters::DeviceEnergyManagement::Instance gDEMInstance(
    endpointId, myDelegate, chip::app::Clusters::DeviceEnergyManagement::Feature::kPowerAdjustment
);

void ApplicationInit()
{
    gDEMInstance.Init();
}
```

## Delegate Methods

The delegate must implement the following methods:

| Method                             | Description                            |
| ---------------------------------- | -------------------------------------- |
| `PowerAdjustRequest()`             | Handle power level adjustment requests |
| `CancelPowerAdjustRequest()`       | Cancel an active power adjustment      |
| `StartTimeAdjustRequest()`         | Adjust forecast start time             |
| `PauseRequest()`                   | Pause appliance operation              |
| `ResumeRequest()`                  | Resume paused operation                |
| `ModifyForecastRequest()`          | Modify forecast slots                  |
| `RequestConstraintBasedForecast()` | Generate forecast within constraints   |
| `CancelRequest()`                  | Cancel adjustment requests             |
| `GetESAType()`                     | Return the ESA type                    |
| `GetESACanGenerate()`              | Return whether ESA can generate power  |
| `GetESAState()`                    | Return current ESA state               |
| `GetAbsMinPower()`                 | Return absolute minimum power          |
| `GetAbsMaxPower()`                 | Return absolute maximum power          |
| `GetOptOutState()`                 | Return current opt-out state           |
| `GetPowerAdjustmentCapability()`   | Return power adjustment capability     |
| `GetForecast()`                    | Return current forecast                |
| `SetESAState()`                    | Update ESA state                       |

## Events

The cluster can generate the following events (application is responsible for
generation via the delegate):

-   `PowerAdjustStart`: When a power adjustment begins
-   `PowerAdjustEnd`: When a power adjustment ends
-   `Paused`: When the appliance is paused
-   `Resumed`: When the appliance resumes operation

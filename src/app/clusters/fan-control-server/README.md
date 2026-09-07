# Fan Control Cluster

The Fan Control cluster provides an interface for controlling fan devices. It
supports configuring fan modes, speeds, airflow directions, wind modes, and
rocking behavior.

## Overview

This directory contains a code-driven C++ implementation of the Matter Fan
Control cluster server (`FanControlCluster.h`). This implementation is designed
for flexibility, avoiding the tight coupling present in older ZAP/Ember-based
implementations.

It uses a delegate pattern (`chip::app::Clusters::FanControl::Delegate`) to
notify the application about cluster-related events and state changes, such as
when the fan mode, speed, or other settings are modified.

## Usage

To integrate the `FanControlCluster` into your application, follow these steps.

### 1. Implement the Delegate

Create a class that inherits from `chip::app::Clusters::FanControl::Delegate`
and implement its virtual methods to handle logic specific to your fan hardware.

```cpp
#include <app/clusters/fan-control-server/fan-control-delegate.h>

class MyFanControlDelegate : public chip::app::Clusters::FanControl::Delegate
{
public:
    MyFanControlDelegate(chip::EndpointId endpoint) : Delegate(endpoint) {}

    chip::Protocols::InteractionModel::Status HandleStep(
        chip::app::Clusters::FanControl::StepDirectionEnum aDirection,
        bool aWrap,
        bool aLowestOff) override
    {
        // Handle step command logic here
        return chip::Protocols::InteractionModel::Status::Success;
    }

    void OnFanDriveStateChanged(const chip::app::Clusters::FanControl::FanDriveState & newState) override
    {
        // React to fan state changes (mode, speed, etc.)
    }
};
```

### 2. Instantiate Delegate and Cluster

Instantiate your delegate and the `FanControlCluster` itself for each endpoint.
The `FanControlCluster::Config` class uses a fluent interface to specify the
features supported by your specific fan (e.g., maximum speed, wind modes,
rocking support).

```cpp
#include "app/clusters/fan-control-server/FanControlCluster.h"

// In a .cpp file
MyFanControlDelegate gMyFanDelegate(kYourEndpointId);

chip::app::RegisteredServerCluster<chip::app::Clusters::FanControlCluster> gFanControlCluster(
    chip::app::Clusters::FanControlCluster::Config(kYourEndpointId, gMyFanDelegate)
        .WithFanModeSequence(chip::app::Clusters::FanControl::FanModeSequenceEnum::kOffLowHigh)
        .WithSpeedMax(10)
        .WithStep()
);
```

### 3. Register the Cluster

In your application's initialization sequence, register the cluster instance
with the `CodegenDataModelProvider`.

```cpp
#include "data-model-providers/codegen/CodegenDataModelProvider.h"

void ApplicationInit()
{
    // ... other initializations
    CHIP_ERROR err = chip::app::CodegenDataModelProvider::Instance().Registry().Register(gFanControlCluster.Registration());
    VerifyOrDie(err == CHIP_NO_ERROR);
    // ...
}
```

## Backwards Compatibility and Code Size Considerations

For backwards compatibility with ZAP-driven static clusters, a bridge lives in
`CodegenIntegration.h` / `CodegenIntegration.cpp`. It wires attribute storage
and the generated data model to `FanControlCluster` and exposes
`SetDefaultDelegate` / `GetDelegate` (for example, the **Step** command still
goes through the delegate registered per endpoint).

`fan-control-server.h` remains only as a thin include; new code should include
`FanControlCluster.h` (or the headers shown in Usage above).

### Migrating from the Legacy API

#### Recommended usage

Instantiate `FanControlCluster`, register with `CodegenDataModelProvider`, and
attach your `Delegate` in `Config` — as in the Usage section. Remove
`SetDefaultDelegate` once nothing relies on the integration layer.

#### Legacy usage (discouraged)

Previously the cluster was provisioned from ZAP/Ember; the app supplied a
delegate for command handling (and related behavior) via the integration API:

```cpp
#include <app/clusters/fan-control-server/CodegenIntegration.h>

chip::app::Clusters::FanControl::SetDefaultDelegate(endpoint, &myDelegate);
```

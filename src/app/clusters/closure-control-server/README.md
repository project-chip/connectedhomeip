# Closure Control Cluster

The Closure Control cluster provides an interface for controlling closure
devices. It allows clients to control positioning, motion latching, and monitor
the operational state of closure devices.

## Overview

This directory contains a code-driven C++ implementation of the Matter Closure
Control cluster server. This implementation (`ClosureControlCluster.h`) is
designed for flexibility, avoiding the tight coupling present in older
codegen-based implementations.

It uses a delegate pattern
(`chip::app::Clusters::ClosureControl::ClosureControlClusterDelegate`) to
interact with the application's closure control logic and state management.

## Usage

To integrate the `ClosureControlCluster` into your application, follow these
steps:

### 1. Implement the Delegate

Create a class that inherits from
`chip::app::Clusters::ClosureControl::ClosureControlClusterDelegate` and
implement its virtual methods to handle commands and provide closure state
information.

```cpp
#include "app/clusters/closure-control-server/ClosureControlClusterDelegate.h"

class MyClosureControlDelegate : public chip::app::Clusters::ClosureControl::ClosureControlClusterDelegate
{
};
```

### 2. Build a Config with the cluster's features

Configure the cluster's feature map, optional attributes and initial state using
the builder-style `ClosureControlCluster::Config`. The resulting configuration
must be valid before cluster creation (see "Conformance Validation" below).

```cpp
#include "app/clusters/closure-control-server/ClosureControlCluster.h"

using namespace chip::app::Clusters::ClosureControl;

auto config = ClosureControlCluster::Config(/* endpoint */ 1, gMyDelegate, gTimerDelegate)
                  .WithPositioning()
                  .WithCalibration()
                  // Add other features as needed via WithSpeed(), WithVentilation(),
                  // WithPedestrian(), WithProtection(), WithManuallyOperable(),
                  // WithInstantaneous()...
                  .WithMotionLatching(chip::BitFlags<LatchControlModesBitmap>()
                                          .Set(LatchControlModesBitmap::kRemoteLatching)
                                          .Set(LatchControlModesBitmap::kRemoteUnlatching))
                  .WithCountdownTime()
                  .WithInitialMainState(MainStateEnum::kStopped);
```

### 3. Instantiate Delegates and Cluster

Instantiate your delegate and the `ClosureControlCluster` itself for each
endpoint that requires it. Using `RegisteredServerCluster` simplifies
registration.

```cpp
#include "app/server-cluster/ServerClusterInterfaceRegistry.h"
#include "lib/support/DefaultTimerDelegate.h"

// In a .cpp file
MyClosureControlDelegate gMyDelegate;
chip::support::DefaultTimerDelegate gTimerDelegate;

chip::app::RegisteredServerCluster<chip::app::Clusters::ClosureControl::ClosureControlCluster>
    gClosureControlCluster(config);
```

### 4. Register the Cluster

In your application's initialization sequence, register the cluster instance
with the `CodegenDataModelProvider`. This hooks the cluster into the Matter data
model and message processing framework.

```cpp
#include "data-model-providers/codegen/CodegenDataModelProvider.h"

void ApplicationInit()
{
    // ... other initializations

    // Register cluster BEFORE server starts
    CHIP_ERROR err = chip::app::CodegenDataModelProvider::Instance().Registry().Register(
        gClosureControlCluster.Registration());
    VerifyOrDie(err == CHIP_NO_ERROR);

    // ... server startup happens later
}
```

All cluster setters/getters are then invoked directly on
`gClosureControlCluster.Cluster()`.

## Initialization Sequence

### Code-Driven Cluster Usage (Recommended)

For new applications using the code-driven cluster pattern:

1. **Before Server Startup:**

    - Build a `ClosureControlCluster::Config` with the desired features and
      initial state via `With*` builder methods
    - Instantiate delegate and cluster
    - Register cluster with `CodegenDataModelProvider`

2. **After Startup:**
    - All getter/setter methods are safe to use directly on the cluster instance

### Interface (Legacy / backward-compatible) Usage

For backwards compatibility with applications using the legacy ZAP-generated
patterns:

```cpp
#include "app/clusters/closure-control-server/CodegenIntegration.h"

using namespace chip::app::Clusters::ClosureControl;

MyClosureControlDelegate gMyDelegate;
Interface gClosureControlInterface(/* endpoint */ 1, gMyDelegate);

CHIP_ERROR ApplicationInit()
{
    ClusterConformance conformance;
    conformance.FeatureMap().Set(Feature::kPositioning).Set(Feature::kCalibration);

    ClusterInitParameters initParams;
    initParams.mMainState = MainStateEnum::kStopped;
    initParams.mLatchControlModes.Set(LatchControlModesBitmap::kRemoteLatching)
        .Set(LatchControlModesBitmap::kRemoteUnlatching);

    return gClosureControlInterface.Init(conformance, initParams);
}
```

After `Init()` succeeds, access the cluster via
`gClosureControlInterface.Cluster()`.

## Conformance Validation

The cluster performs strict conformance validation during construction. **Any
validation failure is fatal** and will terminate the application with "Invalid
Conformance" message

### Migrating from the Legacy API

We recommend migrating to the new, direct instantiation method to improve
performance and reduce your application's footprint.

#### Recommended Usage

The new approach is to instantiate the cluster directly and register it with the
`CodegenDataModelProvider`, as detailed in the "Usage" section above.

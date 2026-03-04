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

### 2. Configure Cluster Conformance

Configure the cluster's feature map and optional attributes based on your device
capabilities. The conformance must be valid before cluster creation (see
"Conformance Validation" below).

```cpp
#include "app/clusters/closure-control-server/ClosureControlCluster.h"

chip::app::Clusters::ClosureControl::ClusterConformance conformance;
conformance.FeatureMap().Set(chip::app::Clusters::ClosureControl::Feature::kPositioning);
conformance.FeatureMap().Set(chip::app::Clusters::ClosureControl::Feature::kCalibration);
// Add other features as needed

// Optional: Configure initial state
chip::app::Clusters::ClosureControl::ClusterInitParameters initParams;
initParams.mMainState = chip::app::Clusters::ClosureControl::MainStateEnum::kStopped;
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

chip::app::Clusters::ClosureControl::ClosureControlCluster::Context clusterContext{
    .delegate = gMyDelegate,
    .timerDelegate = gTimerDelegate,
    .conformance = conformance,
    .initParams = initParams
};

chip::app::RegisteredServerCluster<chip::app::Clusters::ClosureControl::ClosureControlCluster> gClosureControlCluster(
    chip::EndpointId{ 1 }, clusterContext);
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

## Initialization Sequence

### Code-Driven Cluster Usage (Recommended)

For new applications using the code-driven cluster pattern:

1. **Before Server Startup:**

    - Configure `ClusterConformance` with valid feature map
    - Configure `ClusterInitParameters` with initial state
    - Instantiate delegate and cluster
    - Register cluster with `CodegenDataModelProvider`

2. **After Startup:**
    - All getter/setter methods are safe to use via Cluster Instance

### Legacy API Usage

For backwards compatibility with applications using the legacy ZAP-generated
patterns:

1. **Before Server Startup:**

    ```cpp
    // Set delegate (must be called before server starts)
    MatterClosureControlSetDelegate(endpointId, delegate);

    // Set conformance (must be called before server starts)
    MatterClosureControlSetConformance(endpointId, conformance);

    // Set init parameters (must be called before server starts)
    MatterClosureControlSetInitParams(endpointId, initParams);
    ```

2. **After Startup:**
    ```cpp
    // Access cluster instance
    ClosureControlCluster * cluster = GetInstance(endpointId);
    // Use cluster methods...
    ```

**Critical:** For legacy usage, ensure `MatterClosureControlSetDelegate()`,
`MatterClosureControlSetConformance()`, and
`MatterClosureControlSetInitParams()` are called **before** `ServerInit()`. The
`GetInstance()` should be called after the Server Created. The cluster instance
**must not** be used before `ServerInit()` is called. Accessing the cluster
before initialization (e.g., wrong app init order) may result in reading
uninitialized state. Ensure your application initialization order is correct.

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

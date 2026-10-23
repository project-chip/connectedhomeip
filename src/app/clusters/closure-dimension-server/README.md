# Closure Dimension Cluster

This cluster provides an interface for controlling a single degree of freedom
(also referred to as a "dimension" or an "axis" below) of a composed closure.

## Overview

This directory contains a code-driven C++ implementation of the Matter Closure
Dimension cluster server. This implementation (`ClosureDimensionCluster.h`) is
designed for flexibility, avoiding the tight coupling present in older
codegen-based implementations.

It uses a delegate pattern
(`chip::app::Clusters::ClosureDimension::ClosureDimensionClusterDelegate`) to
interact with the application's closure dimension logic and state management.

## Usage

To integrate the `ClosureDimensionCluster` into your application, follow these
steps:

### 1. Implement the Delegate

Create a class that inherits from
`chip::app::Clusters::ClosureDimension::ClosureDimensionClusterDelegate` and
implement its virtual methods to handle commands and provide dimension state
information.

```cpp
#include "app/clusters/closure-dimension-server/ClosureDimensionClusterDelegate.h"

class MyClosureDimensionDelegate : public chip::app::Clusters::ClosureDimension::ClosureDimensionClusterDelegate
{
};
```

### 2. Configure Cluster Conformance

Configure the cluster's feature map and optional attributes based on your device
capabilities. The conformance must be valid before cluster creation (see
"Conformance Validation" below).

```cpp
#include "app/clusters/closure-dimension-server/ClosureDimensionCluster.h"

chip::app::Clusters::ClosureDimension::ClusterConformance conformance;
conformance.FeatureMap().Set(chip::app::Clusters::ClosureDimension::Feature::kPositioning);
conformance.FeatureMap().Set(chip::app::Clusters::ClosureDimension::Feature::kMotionLatching);
// Add other features as needed

// Optional: Configure init parameters (translation / rotation / modulation when those features are enabled)
chip::app::Clusters::ClosureDimension::ClusterInitParameters initParams;
initParams.translationDirection = chip::app::Clusters::ClosureDimension::TranslationDirectionEnum::kDownward;
// initParams.rotationAxis / initParams.modulationType when required
```

### 3. Instantiate Delegates and Cluster

Instantiate your delegate and the `ClosureDimensionCluster` itself for each
endpoint that requires it. Using `RegisteredServerCluster` simplifies
registration.

```cpp
#include "app/server-cluster/ServerClusterInterfaceRegistry.h"

// In a .cpp file
MyClosureDimensionDelegate gMyDelegate;

chip::app::Clusters::ClosureDimension::ClosureDimensionCluster::Context clusterContext{
    .delegate = gMyDelegate,
    .conformance = conformance,
    .initParams = initParams
};

chip::app::RegisteredServerCluster<chip::app::Clusters::ClosureDimension::ClosureDimensionCluster> gClosureDimensionCluster(
    chip::EndpointId{ 2 }, clusterContext);
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
        gClosureDimensionCluster.Registration());
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

Legacy API Usage

The legacy classes `ClusterLogic` and `MatterContext` have been removed and are
no longer required in the newer implementation.

Other APIs are retained for backward compatibility and remain accessible through
the legacy Interface class through `CodegenIntegration.h`

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

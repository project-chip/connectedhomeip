# Identify Cluster

The Identify cluster is used to help an administrator identify a particular
Node. For example, it can be used to cause an LED on a device to blink, a
speaker to beep, or a display to show a QR code.

## Overview

This directory contains a code-driven C++ implementation of the Matter Identify
cluster server. This implementation (`IdentifyCluster.h`) is designed for
flexibility avoiding the the tight coupling present in older ZAP/Ember based
implementations.

It uses a delegate pattern (`chip::app::Clusters::IdentifyDelegate`) to notify
the application about cluster-related events, such as when identification
starts, stops, or an effect is triggered.

## Usage

To integrate the `IdentifyCluster` into your application, follow these steps:

### 1. Implement the Delegate

Create a class that inherits from `chip::app::Clusters::IdentifyDelegate` and
implement its virtual methods to handle identification events.

```cpp
#include "app/clusters/identify-server/IdentifyCluster.h"

class MyIdentifyDelegate : public chip::app::Clusters::IdentifyDelegate
{
public:
    void OnIdentifyStart(chip::app::Clusters::IdentifyCluster & cluster) override
    {
        // Your logic to start identification (e.g., start blinking an LED)
    }

    void OnIdentifyStop(chip::app::Clusters::IdentifyCluster & cluster) override
    {
        // Your logic to stop identification (e.g., stop blinking an LED)
    }

    void OnTriggerEffect(chip::app::Clusters::IdentifyCluster & cluster) override
    {
        // Your logic to trigger a specific effect
    }

    bool IsTriggerEffectEnabled() const override { return true; }
};
```

### 2. Instantiate Delegates and Cluster

Instantiate your delegate, a timer delegate, and the `IdentifyCluster` itself
for each endpoint that requires it. Using `RegisteredServerCluster` simplifies
registration.

```cpp
#include "app/TimerDelegateDefault.h"
#include "app/server-cluster/ServerClusterInterfaceRegistry.h"

// In a .cpp file
MyIdentifyDelegate gMyIdentifyDelegate;
DefaultTimerDelegate gTimerDelegate;

chip::app::RegisteredServerCluster<chip::app::Clusters::IdentifyCluster> gIdentifyCluster(
    chip::app::Clusters::IdentifyCluster::Config(kYourEndpointId, gTimerDelegate)
        .WithIdentifyType(chip::app::Clusters::Identify::IdentifyTypeEnum::kVisibleIndicator)
        .WithDelegate(&gMyIdentifyDelegate));
```

### 3. Register the Cluster

In your application's initialization sequence, register the cluster instance
with the `CodegenDataModelProvider`. This hooks the cluster into the Matter data
model and message processing framework.

```cpp
#include "data-model-providers/codegen/CodegenDataModelProvider.h"

void ApplicationInit()
{
    // ... other initializations
    CHIP_ERROR err = chip::app::CodegenDataModelProvider::Instance().Registry().Register(gIdentifyCluster.Registration());
    VerifyOrDie(err == CHIP_NO_ERROR);
    // ...
}
```

## Backwards Compatibility and Code Size Considerations

For backwards compatibility with applications that rely on older ZAP-generated
patterns, a legacy API is provided in `CodegenIntegration.h` and
`CodegenIntegration.cpp`. This compatibility layer allows the application to
function without being immediately updated to the new code-driven approach.

However, **this legacy approach is discouraged**. It introduces significant code
size overhead (~400 bytes) because it needs to add extra code to convert the new
API to the old API.

### Migrating from the Legacy API

We strongly recommend migrating to the new, direct instantiation method to
improve performance and reduce your application's footprint.

#### Recommended Usage

The new approach is to instantiate the cluster directly and register it with the
`CodegenDataModelProvider`, as detailed in the "Usage" section above. This gives
you more control and results in a smaller, more efficient binary.

```cpp
// In a header or source file:
#include "app/clusters/identify-server/IdentifyCluster.h"
#include "app/server-cluster/ServerClusterInterfaceRegistry.h"
#include "app/TimerDelegateDefault.h"

class MyIdentifyDelegate : public chip::app::Clusters::IdentifyDelegate { /* ... */ };

// In a .cpp file:
MyIdentifyDelegate gMyIdentifyDelegate;
DefaultTimerDelegate gTimerDelegate;

chip::app::RegisteredServerCluster<chip::app::Clusters::IdentifyCluster> gIdentifyCluster(
    chip::app::Clusters::IdentifyCluster::Config(1, gTimerDelegate)
        .WithIdentifyType(chip::app::Clusters::Identify::IdentifyTypeEnum::kVisibleIndicator)
        .WithDelegate(&gMyIdentifyDelegate));

// In your application's init function:
#include "data-model-providers/codegen/CodegenDataModelProvider.h"

void ApplicationInit()
{
    // ...
    VerifyOrDie(
        chip::app::CodegenDataModelProvider::Instance().Registry().Register(gIdentifyCluster.Registration()) == CHIP_NO_ERROR
    );
    // ...
}
```

#### Legacy Usage (Discouraged)

Previously, you might have relied on static `Identify` structs or ZAP-generated
callbacks:

```cpp
// This old pattern is found in `app/clusters/identify-server/identify-server.h`
// and is now considered legacy.

#include <app/clusters/identify-server/identify-server.h>

void OnIdentifyStart(::Identify *) { /* ... */ }
void OnIdentifyStop(::Identify *) { /* ... */ }
void OnTriggerEffect(::Identify * identify) { /* ... */ }

static Identify gIdentify1 = {
    chip::EndpointId{ 1 }, OnIdentifyStart, OnIdentifyStop,
    Clusters::Identify::IdentifyTypeEnum::kVisibleIndicator,
    OnTriggerEffect,
};

// No explicit registration was needed, as it was handled by the legacy system.
```

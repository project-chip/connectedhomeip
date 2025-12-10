# Chime Cluster

The Chime cluster provides an interface for controlling a chime device, such as
a doorbell or other audible notification device. It allows clients to discover
available sounds and trigger them.

## Overview

This directory contains a code-driven C++ implementation of the Matter Chime
cluster server. This implementation (`ChimeCluster.h`) is designed for
flexibility, avoiding the tight coupling present in older ZAP/Ember based
implementations.

It uses a delegate pattern (`chip::app::Clusters::ChimeDelegate`) to interact
with the application's sound resources and playback logic.

## Usage

To integrate the `ChimeCluster` into your application, follow these steps:

### 1. Implement the Delegate

Create a class that inherits from `chip::app::Clusters::ChimeDelegate` and
implement its virtual methods to provide the list of available sounds and handle
playback commands.

```cpp
#include "app/clusters/chime-server/ChimeCluster.h"

class MyChimeDelegate : public chip::app::Clusters::ChimeDelegate
{
public:
    CHIP_ERROR GetChimeSoundByIndex(uint8_t chimeIndex, uint8_t & chimeID, chip::MutableCharSpan & name) override
    {
        // Logic to return chime sound details by index
        if (chimeIndex == 0) {
             chimeID = 1;
             return chip::CopyCharSpanToMutableCharSpan(chip::CharSpan("Ding Dong", 9), name);
        }
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }

    CHIP_ERROR GetChimeIDByIndex(uint8_t chimeIndex, uint8_t & chimeID) override
    {
        // Logic to return chime ID by index
        if (chimeIndex == 0) {
             chimeID = 1;
             return CHIP_NO_ERROR;
        }
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }

    chip::Protocols::InteractionModel::Status PlayChimeSound() override
    {
        // Logic to play the currently selected sound
        return chip::Protocols::InteractionModel::Status::Success;
    }
};
```

### 2. Instantiate Delegates and Cluster

Instantiate your delegate and the `ChimeCluster` itself for each endpoint that
requires it. Using `RegisteredServerCluster` simplifies registration.

```cpp
#include "app/server-cluster/ServerClusterInterfaceRegistry.h"

// In a .cpp file
MyChimeDelegate gMyChimeDelegate;

chip::app::RegisteredServerCluster<chip::app::Clusters::ChimeCluster> gChimeCluster(
    chip::EndpointId{ 1 }, gMyChimeDelegate);
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
    CHIP_ERROR err = chip::app::CodegenDataModelProvider::Instance().Registry().Register(gChimeCluster.Registration());
    VerifyOrDie(err == CHIP_NO_ERROR);
    // ...
}
```

## Backwards Compatibility

For backwards compatibility with applications that rely on older ZAP-generated
patterns, a legacy API is provided in `CodegenIntegration.h` and
`CodegenIntegration.cpp`. This compatibility layer uses a `ChimeServer` wrapper
class.

The `ChimeDelegate` also maintains a protected member `mChimeServer` (accessible
via `GetChimeCluster()`) to support legacy delegates that accessed the server
instance directly.

### Migrating from the Legacy API

We recommend migrating to the new, direct instantiation method to improve
performance and reduce your application's footprint.

#### Recommended Usage

The new approach is to instantiate the cluster directly and register it with the
`CodegenDataModelProvider`, as detailed in the "Usage" section above.

#### Legacy Usage (Discouraged)

Previously, you might have used `ChimeServer` directly:

```cpp
// This old pattern is found in `app/clusters/chime-server/chime-server.h`
// and is now considered legacy wrapper.

#include <app/clusters/chime-server/chime-server.h>

MyChimeDelegate gMyChimeDelegate;
chip::app::Clusters::ChimeServer gChimeServer(chip::EndpointId{ 1 }, gMyChimeDelegate);

void ApplicationInit() {
    gChimeServer.Init(); // Registers internally
}
```

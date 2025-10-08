# Identify Cluster

The Identify cluster is used to help an administrator identify a particular Node. For example, it can be used to cause an LED on a device to blink, a speaker to beep, or a display to show a QR code.

## Overview

This directory contains a code-driven C++ implementation of the Matter Identify cluster server. This implementation (`IdentifyCluster.h`) is designed for flexibility avoiding the the tight coupling present in older ZAP/Ember based implementations.

It uses a delegate pattern (`chip::app::Clusters::IdentifyDelegate`) to notify the application about cluster-related events, such as when identification starts, stops, or an effect is triggered.

## Usage

To integrate the `IdentifyCluster` into your application, follow these steps:

### 1. Implement the Delegate

Create a class that inherits from `chip::app::Clusters::IdentifyDelegate` and implement its virtual methods to handle identification events.

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

Instantiate your delegate, a timer delegate, and the `IdentifyCluster` itself for each endpoint that requires it. Using `RegisteredServerCluster` simplifies registration.

```cpp
#include "app/TimerDelegates.h"
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

In your application's initialization sequence, register the cluster instance with the `CodegenDataModelProvider`. This hooks the cluster into the Matter data model and message processing framework.

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

For backwards compatibility with applications that rely on older ZAP-generated patterns, a legacy API is provided in `CodegenIntegration.h` and `CodegenIntegration.cpp`. This compatibility layer allows the application to function without being immediately updated to the new code-driven approach.

However, **this legacy approach is discouraged**. It introduces significant code size overhead (~400 bytes) because it needs to add extra code to convert the new API to the old API.

### Migrating from the Legacy API

We strongly recommend migrating to the new, direct instantiation method to improve performance and reduce your application's footprint.

#### Legacy Way (Discouraged)

Previously, you might have relied on static `Identify` structs or ZAP-generated callbacks:

```cpp
// This old pattern is found in `app/clusters/identify-server/identify-server.h`
// and is now considered legacy.

#include <app/clusters/identify-server/identify-server.h>

void OnIdentifyStart(::Identify *) { /* ... */ }
void OnIdentifyStop(::Identify *) { /* ... */ }
void OnTriggerEffect(::Identify * identify) { /* ... */ }

static Identify gIdentify0 = {
    chip::EndpointId{ 0 }, OnIdentifyStart, OnIdentifyStop,
    Clusters::Identify::IdentifyTypeEnum::kVisibleIndicator,
    OnTriggerEffect,
};

// No explicit registration was needed, as it was handled by the legacy system.
```

#### New Way (Recommended)

The new approach is to instantiate the cluster directly and register it with the `CodegenDataModelProvider`, as detailed in the "Usage" section above. This gives you more control and results in a smaller, more efficient binary.

```cpp
// In a header or source file:
#include "app/clusters/identify-server/IdentifyCluster.h"
#include "app/server-cluster/ServerClusterInterfaceRegistry.h"
#include "app/TimerDelegates.h"

class MyIdentifyDelegate : public chip::app::Clusters::IdentifyDelegate { /* ... */ };

// In a .cpp file:
MyIdentifyDelegate gMyIdentifyDelegate;
DefaultTimerDelegate gTimerDelegate;

chip::app::RegisteredServerCluster<chip::app::Clusters::IdentifyCluster> gIdentifyCluster(
    chip::app::Clusters::IdentifyCluster::Config(0, gTimerDelegate)
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

## Unit Tests

This cluster implementation contains a comprehensive set of unit tests. To build and run the unit tests you can run the following commands in Linux:

```
$ gn gen out

$ ninja -C out src/app/clusters/identify-server/tests:TestIdentifyCluster && ./out/tests/TestIdentifyCluster


ninja: Entering directory `out'
ninja: no work to do.
[==========] Running all tests.
[ RUN      ] TestIdentifyCluster.TestCreate
[       OK ] TestIdentifyCluster.TestCreate
[ RUN      ] TestIdentifyCluster.AttributeListTest
[       OK ] TestIdentifyCluster.AttributeListTest
[ RUN      ] TestIdentifyCluster.AcceptedCommandsMandatoryOnlyTest
[       OK ] TestIdentifyCluster.AcceptedCommandsMandatoryOnlyTest
[ RUN      ] TestIdentifyCluster.AcceptedCommandsWithOptionalTriggerEffectTest
[       OK ] TestIdentifyCluster.AcceptedCommandsWithOptionalTriggerEffectTest
[ RUN      ] TestIdentifyCluster.ReadAttributesTest
[       OK ] TestIdentifyCluster.ReadAttributesTest
[ RUN      ] TestIdentifyCluster.WriteUnchangedIdentifyTimeDoesNotNotify
[       OK ] TestIdentifyCluster.WriteUnchangedIdentifyTimeDoesNotNotify
[ RUN      ] TestIdentifyCluster.WriteReadOnlyAttributesReturnUnsupportedWriteTest
[       OK ] TestIdentifyCluster.WriteReadOnlyAttributesReturnUnsupportedWriteTest
[ RUN      ] TestIdentifyCluster.IdentifyTimeCountdownTest
[       OK ] TestIdentifyCluster.IdentifyTimeCountdownTest
[ RUN      ] TestIdentifyCluster.OnIdentifyStartStopCallbackTest
[       OK ] TestIdentifyCluster.OnIdentifyStartStopCallbackTest
[ RUN      ] TestIdentifyCluster.OnStartNotCalledMultipleTimes
[       OK ] TestIdentifyCluster.OnStartNotCalledMultipleTimes
[ RUN      ] TestIdentifyCluster.OnStopNotCalledIfNotIdentifying
[       OK ] TestIdentifyCluster.OnStopNotCalledIfNotIdentifying
[ RUN      ] TestIdentifyCluster.InvokeIdentifyCommandTest
[       OK ] TestIdentifyCluster.InvokeIdentifyCommandTest
[ RUN      ] TestIdentifyCluster.InvokeTriggerEffectCommandTest
[1759950844.360] [608415:608415] [ZCL] RX identify:trigger effect identifier 0x0 variant 0x0
[       OK ] TestIdentifyCluster.InvokeTriggerEffectCommandTest
[ RUN      ] TestIdentifyCluster.InvokeTriggerEffectCommandAllEffectsTest
[1759950844.360] [608415:608415] [ZCL] RX identify:trigger effect identifier 0x1 variant 0x0
[1759950844.360] [608415:608415] [ZCL] RX identify:trigger effect identifier 0x2 variant 0x0
[1759950844.360] [608415:608415] [ZCL] RX identify:trigger effect identifier 0xB variant 0x0
[       OK ] TestIdentifyCluster.InvokeTriggerEffectCommandAllEffectsTest
[ RUN      ] TestIdentifyCluster.InvokeTriggerEffectCommandInvalidVariantTest
[1759950844.360] [608415:608415] [ZCL] RX identify:trigger effect identifier 0x0 variant 0x1
[       OK ] TestIdentifyCluster.InvokeTriggerEffectCommandInvalidVariantTest
[ RUN      ] TestIdentifyCluster.TriggerEffectWhileIdentifyingTest
[1759950844.360] [608415:608415] [ZCL] RX identify:trigger effect identifier 0x0 variant 0x0
[       OK ] TestIdentifyCluster.TriggerEffectWhileIdentifyingTest
[ RUN      ] TestIdentifyCluster.TriggerEffectFinishEffectTest
[1759950844.361] [608415:608415] [ZCL] RX identify:trigger effect identifier 0xFE variant 0x0
[       OK ] TestIdentifyCluster.TriggerEffectFinishEffectTest
[ RUN      ] TestIdentifyCluster.TriggerEffectStopEffectTest
[1759950844.361] [608415:608415] [ZCL] RX identify:trigger effect identifier 0xFF variant 0x0
[       OK ] TestIdentifyCluster.TriggerEffectStopEffectTest
[ RUN      ] TestIdentifyCluster.IdentifyTimeAttributeReportingTest
[       OK ] TestIdentifyCluster.IdentifyTimeAttributeReportingTest
[ RUN      ] TestIdentifyCluster.TestGetters
[       OK ] TestIdentifyCluster.TestGetters
[==========] Done running all tests.
[  PASSED  ] 20 test(s).
```

### Backwards Compatibility Unit Tests

```
$ ninja -C out src/app/clusters/identify-server/tests:TestIdentifyClusterBackwardsCompatibility && ./out/tests/TestIdentifyClusterBackwardsCompatibility


ninja: Entering directory `out'
[16/16] ld tests/TestIdentifyClusterBackwardsCompatibility
[==========] Running all tests.
[ RUN      ] TestIdentifyClusterBackwardsCompatibility.TestLegacyInstantiattion
[       OK ] TestIdentifyClusterBackwardsCompatibility.TestLegacyInstantiattion
[ RUN      ] TestIdentifyClusterBackwardsCompatibility.TestLegacyCallbacks
[1759950902.579] [608781:608781] [ZCL] RX identify:trigger effect identifier 0x0 variant 0x0
[       OK ] TestIdentifyClusterBackwardsCompatibility.TestLegacyCallbacks
[ RUN      ] TestIdentifyClusterBackwardsCompatibility.TestCurrentEffectIdentifierUpdate
[1759950902.579] [608781:608781] [ZCL] RX identify:trigger effect identifier 0x0 variant 0x0
[1759950902.579] [608781:608781] [ZCL] RX identify:trigger effect identifier 0xFE variant 0x0
[       OK ] TestIdentifyClusterBackwardsCompatibility.TestCurrentEffectIdentifierUpdate
[ RUN      ] TestIdentifyClusterBackwardsCompatibility.TestIdentifyTypeInitialization
[       OK ] TestIdentifyClusterBackwardsCompatibility.TestIdentifyTypeInitialization
[ RUN      ] TestIdentifyClusterBackwardsCompatibility.TestMActive
[       OK ] TestIdentifyClusterBackwardsCompatibility.TestMActive
[==========] Done running all tests.
[  PASSED  ] 5 test(s).
```
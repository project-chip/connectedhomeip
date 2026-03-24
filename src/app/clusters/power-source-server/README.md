# Power Source Cluster

The Power Source cluster is used to monitor and report the state of a power
source. A power source can be either a battery or a wired power source.

## Overview

This directory contains an implementation of Matter Power Source cluster. This
implementation handles the persistence of the `Order` attribute, and notifies
the data model when an attribute changes in the way that the spec mandates.

## Usage

For new applications using `CodeDrivenDataModelProvider`, we strongly recommend
creating and registering the cluster directly. This can be done easily using
`RegisteredServerCluster` and `LazyRegisteredServerCluster` in
`src/app/server-cluster/SingleEndpointServerClusterRegistry.h`.

### Example

```cpp
#include <app/clusters/PowerSourceCluster.h>
#include <app/server-cluster/SingleEndpointServerClusterRegistry.h>
// for `DeviceLayer::SystemLayer()`
#include <include/platform/CHIPDeviceLayer.h>
#include <data-model-provider/CodeDrivenDataModelProvider.h>

using namespace app::Clusters;
using namespace app::Clusters;

// configuration for a wired power source
PowerSourceCluster::WiredConfiguration config("Some description"_span, PowerSourceCluster::WiredCurrentTypeEnum::kDc);

// specify which optional attribute you want to be supported
PowerSourceCluster::OptionalAttributeSet optAttributes{};
optAttributes.Set<PowerSource::Attributes::WiredAssessedInputVoltage::Id>();
optAttributes.Set<PowerSource::Attributes::WiredAssessedInputFrequency::Id>();

EndpointId endpointId = 4; // an example value

// create the cluster
RegisteredServerCluster<PowerSourceCluster> powerSourceInstance(endpointId, optAttributes, DeviceLayer::SystemLayer(), config);

// register the cluster, this will call `Startup` on the cluster
CodeDrivenDataModelProvider::Instance().Registry().Register(powerSourceInstance.Registration());

// use the cluster
powerSourceInstance.Cluster().SetWiredAssessedInputFrequency(MakeOptional(20000));
```

Or with the `LazyRegisteredServerCluster` which can defer the creation and
overall provides dynamic lifetime management. An example using it can be seen in
`examples/evse-app/evse-common/include/EVSEManufacturerImpl.h` and corresponding
`EVSEManufacturerImpl.cpp`.

### Note

The attributes that can be set in the `WiredConfiguration` or `BatteryConfiguration` do not have their
corresponding `.Set*` methods, because they are marked as `Fixed` and can be set only during construction.

## Legacy Usage (Not Recommended)

For backwards compatibility with applications that rely on older ZAP-generated
patterns (like the `all-clusters-app`), a compatibility layer is provided in
`CodegenIntegration.h` and `CodegenIntegration.cpp`.

In this model, you configure the Power Source cluster in your `.zap` file. The
ZAP tool generates `MatterPowerSourceClusterInitCallback`, which is implemented
by our `CodegenIntegration` layer to automatically instantiate and configure the
cluster based on your ZAP configuration.

To use the cluster in this mode, your application can get a pointer to the
cluster instance and call its methods directly using
`PowerSource::FindClusterOnEndpoint(endpointId)`.

Note that this method is for backwards compatibility only and is not recommended
for new applications.

```cpp
void BatPercentRemainingChangedCallback(uint8_t newValue)
{
    chip::app::Clusters::PowerSourceCluster * cluster = chip::app::Clusters::PowerSource::FindClusterOnEndpoint(endpointId);
    if (cluster != nullptr)
    {
        LogErrorOnFailure(cluster->SetBatPercentRemaining(MakeOptional(newValue)));
    }
    else
    {
        ChipLogError(NotSpecified, "PowerSourceCluster not found on endpoint %u", static_cast<unsigned>(endpointId));
    }
}
```

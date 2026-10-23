# Air Quality Cluster

This cluster follows a code-driven approach using `AirQualityCluster`
(`DefaultServerCluster`).

## Code-Driven Usage

```cpp
#include <app/clusters/air-quality-server/AirQualityCluster.h>
#include <app/server-cluster/ServerClusterInterfaceRegistry.h>
#include <data-model-providers/codegen/CodegenDataModelProvider.h>

using namespace chip::app::Clusters;

RegisteredServerCluster<AirQualityCluster> gCluster(endpointId,
    BitFlags<AirQuality::Feature>(AirQuality::Feature::kFair, AirQuality::Feature::kModerate));

// Register during init
CodegenDataModelProvider::Instance().Registry().Register(gCluster.Registration());

// Set and get air quality
gCluster.Cluster().SetAirQuality(AirQuality::AirQualityEnum::kGood);
AirQuality::AirQualityEnum current = gCluster.Cluster().GetAirQuality();
```

## Legacy (ZAP) Usage

The backward-compatible `AirQuality::Instance` wrapper handles registration
internally:

```cpp
#include <app/clusters/air-quality-server/air-quality-server.h>

using namespace chip::app::Clusters::AirQuality;

auto * instance = new Instance(endpointId, BitMask<Feature>(Feature::kFair, Feature::kModerate));
instance->Init();

instance->UpdateAirQuality(AirQualityEnum::kGood);
AirQualityEnum current = instance->GetAirQuality();

// The destructor unregisters the cluster, so delete when no longer needed
delete instance;
```

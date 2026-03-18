# Air Quality Cluster

This cluster uses a code-driven implementation based on `DefaultServerCluster`.

## Overview

The `AirQualityCluster` manages a single `AirQuality` attribute (enum) with
feature-gated validation. Features (`Fair`, `Moderate`, `VeryPoor`,
`ExtremelyPoor`) control which enum values are allowed.

Applications create `AirQuality::Instance` objects and call `Init()` to register
them with the data model provider, preserving the same API as the legacy
`AttributeAccessInterface` implementation.

## Usage

### Creating and initializing

```cpp
#include <app/clusters/air-quality-server/air-quality-server.h>

using namespace chip::app::Clusters;
using namespace chip::app::Clusters::AirQuality;

// Create with endpoint and features
auto * instance = new Instance(endpointId, BitMask<Feature>(Feature::kFair, Feature::kModerate));
instance->Init();
```

### Setting the air quality value

```cpp
auto status = instance->UpdateAirQuality(AirQualityEnum::kGood);
if (status != Protocols::InteractionModel::Status::Success)
{
    // Value rejected (feature not enabled or invalid value)
}
```

### Reading the current value

```cpp
AirQualityEnum current = instance->GetAirQuality();
```

## Internal Architecture

-   `AirQualityCluster` (in `AirQualityCluster.h`) is the code-driven cluster
    inheriting from `DefaultServerCluster`. It handles `ReadAttribute()` and
    attribute enumeration.
-   `AirQuality::Instance` (in `CodegenIntegration.h`) wraps `AirQualityCluster`
    with a `RegisteredServerCluster` and provides the application-facing API.
    `Init()` registers with `CodegenDataModelProvider`, and the destructor
    unregisters.

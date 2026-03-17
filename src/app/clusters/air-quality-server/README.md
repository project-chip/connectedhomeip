# Air Quality Cluster

This cluster uses a code-driven implementation based on `DefaultServerCluster`.

## Overview

The `AirQualityCluster` manages a single `AirQuality` attribute (enum) with
feature-gated validation. Features (`Fair`, `Moderate`, `VeryPoor`,
`ExtremelyPoor`) control which enum values are allowed.

The framework creates cluster instances automatically via `CodegenIntegration`.
Applications access instances using `FindClusterOnEndpoint()`.

## Usage

### Accessing the cluster instance

```cpp
#include <app/clusters/air-quality-server/CodegenIntegration.h>

using namespace chip::app::Clusters;

AirQualityCluster * cluster = AirQuality::FindClusterOnEndpoint(endpointId);
```

### Setting the air quality value

```cpp
auto status = cluster->SetAirQuality(AirQuality::AirQualityEnum::kGood);
if (status != Protocols::InteractionModel::Status::Success)
{
    // Value rejected (feature not enabled or invalid value)
}
```

### Reading the current value

```cpp
AirQuality::AirQualityEnum current = cluster->GetAirQuality();
```

## Migration from Legacy API

Previously, applications manually created instances:

```cpp
// Old pattern (no longer needed)
AirQuality::Instance * instance = new AirQuality::Instance(endpoint, featureBits);
instance->Init();
instance->UpdateAirQuality(AirQuality::AirQualityEnum::kGood);
```

Now the framework manages instances. The Accessors for `AirQuality` and
`FeatureMap` attributes are no longer available. Use the cluster API instead:

```cpp
// New pattern
auto * cluster = AirQuality::FindClusterOnEndpoint(endpoint);
cluster->SetAirQuality(AirQuality::AirQualityEnum::kGood);
```

`UpdateAirQuality()` is retained as a backward-compatibility alias for
`SetAirQuality()`. The type alias `AirQuality::Instance` maps to
`AirQualityCluster` for backward compatibility.

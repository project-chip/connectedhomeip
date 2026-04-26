# Concentration Measurement Cluster

A single `ConcentrationMeasurementCluster` class handles all 10 aliased concentration measurement clusters. They share an identical attribute structure; the only difference is the cluster ID passed at construction:

| Cluster | ID |
|---|---|
| CarbonDioxideConcentrationMeasurement | `CarbonDioxideConcentrationMeasurement::Id` |
| CarbonMonoxideConcentrationMeasurement | `CarbonMonoxideConcentrationMeasurement::Id` |
| NitrogenDioxideConcentrationMeasurement | `NitrogenDioxideConcentrationMeasurement::Id` |
| OzoneConcentrationMeasurement | `OzoneConcentrationMeasurement::Id` |
| Pm1ConcentrationMeasurement | `Pm1ConcentrationMeasurement::Id` |
| Pm10ConcentrationMeasurement | `Pm10ConcentrationMeasurement::Id` |
| Pm25ConcentrationMeasurement | `Pm25ConcentrationMeasurement::Id` |
| RadonConcentrationMeasurement | `RadonConcentrationMeasurement::Id` |
| TotalVolatileOrganicCompoundsConcentrationMeasurement | `TotalVolatileOrganicCompoundsConcentrationMeasurement::Id` |
| FormaldehydeConcentrationMeasurement | `FormaldehydeConcentrationMeasurement::Id` |

The cluster stores no attribute data itself. Every read is forwarded to a `Delegate` the application provides at construction time.

## Overview

The cluster handles protocol logic — feature-gating, input validation, subscriber notifications. The `Delegate` owns the actual sensor values. This replaces the old template-based `ConcentrationMeasurementServer<...>` with a single concrete class configured at runtime via `BitFlags<Feature>`.

For most applications `DefaultDelegate` is sufficient — construct one per concentration type, pass it to the cluster, and push readings with `Set*()`. If RAM is tight, subclass `Delegate` directly and only store the fields you need.

## Usage

### 1. Choose a delegate

#### Option A — `DefaultDelegate` (recommended)

`DefaultDelegate` stores every attribute in memory. No subclassing needed; the cluster's `Set*()` calls write straight through to it.

```cpp
#include <app/clusters/concentration-measurement-server/ConcentrationMeasurementDelegate.h>

// Medium, unit, and range are fixed at construction. Readings start null/zero.
ConcentrationMeasurement::DefaultDelegate gCO2Delegate(ConcentrationMeasurement::MeasurementMediumEnum::kAir,
                                                       ConcentrationMeasurement::MeasurementUnitEnum::kPpm,
                                                       chip::app::DataModel::MakeNullable(0.0f),    // MinMeasuredValue
                                                       chip::app::DataModel::MakeNullable(5000.0f), // MaxMeasuredValue
                                                       /* uncertainty = */ 0.0f);
```

#### Option B — Custom `Delegate` subclass

Override only the getters/setters for the features your sensor supports. Fields for unsupported features are never compiled in.

```cpp
#include <app/clusters/concentration-measurement-server/ConcentrationMeasurementDelegate.h>

class CO2SensorDelegate : public ConcentrationMeasurement::Delegate
{
public:
    ConcentrationMeasurement::MeasurementMediumEnum GetMeasurementMedium() override
    {
        return ConcentrationMeasurement::MeasurementMediumEnum::kAir;
    }

    ConcentrationMeasurement::MeasurementUnitEnum GetMeasurementUnit() override
    {
        return ConcentrationMeasurement::MeasurementUnitEnum::kPpm;
    }

    chip::app::DataModel::Nullable<float> GetMeasuredValue() override { return mValue; }
    chip::app::DataModel::Nullable<float> GetMinMeasuredValue() override { return mMin; }
    chip::app::DataModel::Nullable<float> GetMaxMeasuredValue() override { return mMax; }
    ConcentrationMeasurement::LevelValueEnum GetLevelValue() override { return mLevel; }

    void SetMeasuredValue(chip::app::DataModel::Nullable<float> v) override { mValue = v; }
    void SetLevelValue(ConcentrationMeasurement::LevelValueEnum v) override { mLevel = v; }

private:
    chip::app::DataModel::Nullable<float> mValue;
    chip::app::DataModel::Nullable<float> mMin      = chip::app::DataModel::MakeNullable(0.0f);
    chip::app::DataModel::Nullable<float> mMax      = chip::app::DataModel::MakeNullable(5000.0f);
    ConcentrationMeasurement::LevelValueEnum mLevel = ConcentrationMeasurement::LevelValueEnum::kUnknown;
};
```

### 2. Instantiate the delegate and cluster

Declare the delegate before the cluster — the cluster holds a reference to it and the delegate must outlive the cluster.

```cpp
#include <app/clusters/concentration-measurement-server/ConcentrationMeasurementCluster.h>
#include <app/server-cluster/ServerClusterInterfaceRegistry.h>

ConcentrationMeasurement::DefaultDelegate gCO2Delegate(ConcentrationMeasurement::MeasurementMediumEnum::kAir,
                                                       ConcentrationMeasurement::MeasurementUnitEnum::kPpm);

// One cluster instance per concentration type.
ConcentrationMeasurement::ConcentrationMeasurementCluster gCO2Cluster(
    /* endpointId = */ 1, CarbonDioxideConcentrationMeasurement::Id,
    chip::BitFlags<ConcentrationMeasurement::Feature>(ConcentrationMeasurement::Feature::kNumericMeasurement,
                                                      ConcentrationMeasurement::Feature::kLevelIndication),
    gCO2Delegate);

chip::app::ServerClusterRegistration gCO2Registration(gCO2Cluster);
```

### 3. Register with the data model

```cpp
#include <data-model-providers/codegen/CodegenDataModelProvider.h>

void ApplicationInit()
{
    auto & registry = chip::app::CodegenDataModelProvider::Instance().Registry();
    chip::LogErrorOnFailure(registry.Register(gCO2Registration));
    // repeat for each concentration type
}
```

### 4. Push sensor readings

Call `Set*()` on the cluster whenever your sensor produces a new value. The cluster checks that the relevant feature flag was enabled at construction, validates bounds, writes to the delegate, and notifies any subscribers.

```cpp
gCO2Cluster.SetMeasuredValue(chip::app::DataModel::MakeNullable(412.5f));
gCO2Cluster.SetLevelValue(ConcentrationMeasurement::LevelValueEnum::kMedium);

// Window values are in seconds; maximum is 604800 (7 days).
gCO2Cluster.SetPeakMeasuredValue(chip::app::DataModel::MakeNullable(450.0f));
gCO2Cluster.SetPeakMeasuredValueWindow(3600);
gCO2Cluster.SetAverageMeasuredValue(chip::app::DataModel::MakeNullable(415.0f));
gCO2Cluster.SetAverageMeasuredValueWindow(86400);
```

`Set*()` returns `CHIP_ERROR_INVALID_ARGUMENT` for any attribute whose feature flag was not set at construction.

## Feature Flags

| Feature | Attributes gated behind it |
|---|---|
| `kNumericMeasurement` | `MeasuredValue`, `MinMeasuredValue`, `MaxMeasuredValue`, `Uncertainty`, `MeasurementUnit` |
| `kPeakMeasurement` | `PeakMeasuredValue`, `PeakMeasuredValueWindow` |
| `kAverageMeasurement` | `AverageMeasuredValue`, `AverageMeasuredValueWindow` |
| `kLevelIndication` | `LevelValue` |
| `kMediumLevel` | sub-flag of `kLevelIndication`, no extra attributes |
| `kCriticalLevel` | sub-flag of `kLevelIndication`, no extra attributes |

`MeasurementMedium`, `FeatureMap`, and `ClusterRevision` are always present.

## Multi-Concentration-Type Example

The `air-quality-sensor-app` hosts all 10 aliased clusters on a single endpoint. The pattern is:

1. Declare delegates (one per concentration type) as member variables before clusters.
2. Declare clusters after their delegates.
3. Declare `ServerClusterRegistration` objects after their clusters.
4. In `Init()`, register each cluster and set initial values.
5. In sensor callbacks, call `Set*()` on the right cluster instance.

See [examples/air-quality-sensor-app/air-quality-sensor-common/include/air-quality-sensor-manager.h](../../../../examples/air-quality-sensor-app/air-quality-sensor-common/include/air-quality-sensor-manager.h)
and the corresponding `.cpp` for a full working example.

## Migrating from the Template-Based API

The old implementation used a heavily templated `ConcentrationMeasurementServer<NumericMeasurementEnabled, ...>` class from `concentration-measurement-server.h`.

| Old | New |
|---|---|
| `ConcentrationMeasurementServer<true, false, ...>` template instantiation | `ConcentrationMeasurementCluster` with `BitFlags<Feature>` |
| `MatterConcentrationMeasurementPluginServerInitCallback()` + ZAP glue | `registry.Register(myRegistration)` in application code |
| `GetInstance()->SetMeasuredValue()` | `cluster.SetMeasuredValue()` — same names, no singleton |
| Attribute storage baked into the template | Delegate owns storage; `DefaultDelegate` is a drop-in replacement |

Migration steps:

1. Remove includes of `concentration-measurement-server.h` and any ZAP-generated init callbacks for these clusters.
2. Add a `DefaultDelegate` member (or custom subclass) for each concentration type.
3. Construct a `ConcentrationMeasurementCluster` with the cluster ID and feature flags that match your old template parameters.
4. Add a `ServerClusterRegistration` member and call `registry.Register()` in init.
5. Replace `GetInstance()->Set*()` calls with direct calls on the cluster object.

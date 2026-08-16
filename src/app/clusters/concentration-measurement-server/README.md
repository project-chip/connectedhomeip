# Concentration Measurement Cluster

A single `ConcentrationMeasurementCluster` class handles all 10 aliased
concentration measurement clusters. They share an identical attribute structure;
the only difference is the cluster ID passed at construction:

| Cluster                                               | ID                                                          |
| ----------------------------------------------------- | ----------------------------------------------------------- |
| CarbonDioxideConcentrationMeasurement                 | `CarbonDioxideConcentrationMeasurement::Id`                 |
| CarbonMonoxideConcentrationMeasurement                | `CarbonMonoxideConcentrationMeasurement::Id`                |
| NitrogenDioxideConcentrationMeasurement               | `NitrogenDioxideConcentrationMeasurement::Id`               |
| OzoneConcentrationMeasurement                         | `OzoneConcentrationMeasurement::Id`                         |
| Pm1ConcentrationMeasurement                           | `Pm1ConcentrationMeasurement::Id`                           |
| Pm10ConcentrationMeasurement                          | `Pm10ConcentrationMeasurement::Id`                          |
| Pm25ConcentrationMeasurement                          | `Pm25ConcentrationMeasurement::Id`                          |
| RadonConcentrationMeasurement                         | `RadonConcentrationMeasurement::Id`                         |
| TotalVolatileOrganicCompoundsConcentrationMeasurement | `TotalVolatileOrganicCompoundsConcentrationMeasurement::Id` |
| FormaldehydeConcentrationMeasurement                  | `FormaldehydeConcentrationMeasurement::Id`                  |

## Overview

This directory contains a code-driven C++ implementation of the Matter
Concentration Measurement cluster server. The primary implementation lives in
`ConcentrationMeasurementCluster.h` and is designed to avoid the tight coupling
and binary-size overhead of the older template-based approach.

## Usage

### 1. Instantiate the cluster

Populate a `Config` struct and pass it to the constructor. All fixed sensor
characteristics (`medium`, `unit`, `minMeasured`, `maxMeasured`, `uncertainty`)
live in the config and cannot be changed after construction.

```cpp
#include <app/clusters/concentration-measurement-server/ConcentrationMeasurementCluster.h>
#include <app/server-cluster/ServerClusterInterfaceRegistry.h>

using namespace chip::app::Clusters::ConcentrationMeasurement;

ConcentrationMeasurementCluster::Config gCO2Config{
    /* clusterId  = */ chip::app::Clusters::CarbonDioxideConcentrationMeasurement::Id,
    /* features   = */ chip::BitFlags<Feature>(Feature::kNumericMeasurement,
                                               Feature::kLevelIndication),
    /* medium     = */ MeasurementMediumEnum::kAir,
    /* unit       = */ MeasurementUnitEnum::kPpm,
    /* minMeasured = */ chip::app::DataModel::MakeNullable(0.0f),
    /* maxMeasured = */ chip::app::DataModel::MakeNullable(5000.0f),
};

ConcentrationMeasurementCluster gCO2Cluster(/* endpointId = */ 1, gCO2Config);
chip::app::ServerClusterRegistration gCO2Registration(gCO2Cluster);
```

### 2. Register with the data model

```cpp
#include <data-model-providers/codegen/CodegenDataModelProvider.h>

void ApplicationInit()
{
    auto & registry = chip::app::CodegenDataModelProvider::Instance().Registry();
    chip::LogErrorOnFailure(registry.Register(gCO2Registration));
}
```

### 3. Update attribute values from firmware

Call `Set*()` on the cluster whenever the physical sensor produces a new
reading. The cluster validates bounds, stores the value, and notifies
subscribers automatically. These methods are for **device firmware only** —
remote Matter clients have no write access to these attributes.

```cpp
gCO2Cluster.SetMeasuredValue(chip::app::DataModel::MakeNullable(412.5f));
gCO2Cluster.SetLevelValue(LevelValueEnum::kMedium);

// Window values are in seconds; maximum is 604800 (7 days).
gCO2Cluster.SetPeakMeasuredValue(chip::app::DataModel::MakeNullable(450.0f));
gCO2Cluster.SetPeakMeasuredValueWindow(3600);
gCO2Cluster.SetAverageMeasuredValue(chip::app::DataModel::MakeNullable(415.0f));
gCO2Cluster.SetAverageMeasuredValueWindow(86400);
```

`Set*()` returns `CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE` for any attribute whose
feature flag was not set at construction, and
`CHIP_IM_GLOBAL_STATUS(ConstraintError)` for out-of-range values.

## Feature Flags

| Feature               | Attributes gated behind it                                                                |
| --------------------- | ----------------------------------------------------------------------------------------- |
| `kNumericMeasurement` | `MeasuredValue`, `MinMeasuredValue`, `MaxMeasuredValue`, `Uncertainty`, `MeasurementUnit` |
| `kPeakMeasurement`    | `PeakMeasuredValue`, `PeakMeasuredValueWindow`                                            |
| `kAverageMeasurement` | `AverageMeasuredValue`, `AverageMeasuredValueWindow`                                      |
| `kLevelIndication`    | `LevelValue`                                                                              |
| `kMediumLevel`        | sub-flag of `kLevelIndication`, no extra attributes                                       |
| `kCriticalLevel`      | sub-flag of `kLevelIndication`, no extra attributes                                       |

`MeasurementMedium`, `FeatureMap`, and `ClusterRevision` are always present.

`kPeakMeasurement` and `kAverageMeasurement` imply `kNumericMeasurement` — the
constructor sets `kNumericMeasurement` automatically when either is requested.
`kMediumLevel` and `kCriticalLevel` imply `kLevelIndication` similarly.

See
[examples/air-quality-sensor-app/air-quality-sensor-common/include/air-quality-sensor-manager.h](../../../../examples/air-quality-sensor-app/air-quality-sensor-common/include/air-quality-sensor-manager.h)
and the corresponding `.cpp` for a full working example.

## Backwards Compatibility

### The wrapper: `CodegenIntegration.h`

`CodegenIntegration.h` provides a compatibility shim for applications that used
the original template-based API. It defines:

```cpp
template <bool NumericMeasurementEnabled, bool LevelIndicationEnabled,
          bool MediumLevelEnabled,        bool CriticalLevelEnabled,
          bool PeakMeasurementEnabled,    bool AverageMeasurementEnabled>
class Instance { ... };
```

Internally, `Instance<>` owns a `ConcentrationMeasurementCluster` (via
`LazyRegisteredServerCluster`) and a `ServerClusterRegistration`. At
construction it translates the six boolean template parameters into a
`BitFlags<Feature>` and stores them in a `Config`. The cluster itself is not
created until `Init()` is called.

**Constructors** — two overloads exist:

```cpp
// Level-indication only (no MeasurementUnit parameter).
Instance(EndpointId, ClusterId, MeasurementMediumEnum);

// Numeric measurement (MeasurementUnit required).
Instance(EndpointId, ClusterId, MeasurementMediumEnum, MeasurementUnitEnum);
```

**Pre-init setters** — `SetMinMeasuredValue()`, `SetMaxMeasuredValue()`, and
`SetUncertainty()` configure the `Config` and **must be called before
`Init()`**. They return `CHIP_ERROR_INCORRECT_STATE` if called after `Init()`.

**`Init()`** — constructs the underlying cluster from the stored `Config`, calls
`CodegenDataModelProvider::Instance().Registry().Register(registration)`, and
remembers that registration so the destructor can unregister cleanly. This
preserves the original `Init()`-based ZAP callback pattern; no call-site changes
are needed.

**`Set*()` methods** — each method is guarded by `std::enable_if_t` on the
matching boolean template parameter, so calling e.g. `SetPeakMeasuredValue()` on
an instance where `PeakMeasurementEnabled = false` is a **compile-time error**,
not a runtime one. Internally they forward to the corresponding
`ConcentrationMeasurementCluster::Set*()`.

**`Registration()`** — returns the `ServerClusterRegistration` directly, so new
code that prefers explicit registration (rather than `Init()`) can also use
`Instance<>` without the `CodegenDataModelProvider` dependency:

```cpp
registry.Register(gMyInstance.Registration());
```

The old header `concentration-measurement-server.h` simply re-exports
`CodegenIntegration.h`, so existing `#include` sites continue to compile without
any changes.

### Legacy usage

```cpp
// concentration-measurement-server.h re-exports CodegenIntegration.h.
#include <app/clusters/concentration-measurement-server/concentration-measurement-server.h>

using namespace chip::app::Clusters::ConcentrationMeasurement;

// Template parameters map 1-to-1 to Feature flags:
//   NumericMeasurement, LevelIndication, MediumLevel,
//   CriticalLevel,      PeakMeasurement, AverageMeasurement
Instance<true, true, false, false, true, false> gCO2Instance(
    /* endpointId = */ 1,
    /* clusterId  = */ chip::app::Clusters::CarbonDioxideConcentrationMeasurement::Id,
    /* medium     = */ MeasurementMediumEnum::kAir,
    /* unit       = */ MeasurementUnitEnum::kPpm);

void ApplicationInit()
{
    // Optional: configure fixed sensor characteristics before Init().
    gCO2Instance.SetMinMeasuredValue(chip::app::DataModel::MakeNullable(0.0f));
    gCO2Instance.SetMaxMeasuredValue(chip::app::DataModel::MakeNullable(5000.0f));
    gCO2Instance.SetUncertainty(0.5f);

    // Registers with CodegenDataModelProvider internally.
    // Equivalent to: registry.Register(gCO2Instance.Registration())
    gCO2Instance.Init();
}

// Set*() availability is enforced at compile time via the template parameters.
// This compiles because PeakMeasurementEnabled = true (5th parameter):
gCO2Instance.SetPeakMeasuredValue(chip::app::DataModel::MakeNullable(450.0f));

// This would be a compile error because AverageMeasurementEnabled = false (6th parameter):
// gCO2Instance.SetAverageMeasuredValue(...);
```

This pattern still compiles and works unchanged.

### Recommended usage (new code)

Instantiate `ConcentrationMeasurementCluster` directly with a `Config` and
register it via `registry.Register()`, as shown in the **Usage** section above.

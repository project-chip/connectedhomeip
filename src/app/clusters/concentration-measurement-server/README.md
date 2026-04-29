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

```cpp
#include <app/clusters/concentration-measurement-server/ConcentrationMeasurementCluster.h>
#include <app/server-cluster/ServerClusterInterfaceRegistry.h>

using namespace chip::app::Clusters::ConcentrationMeasurement;

ConcentrationMeasurementCluster gCO2Cluster(
    /* endpointId   = */ 1,
    /* clusterId    = */ chip::app::Clusters::CarbonDioxideConcentrationMeasurement::Id,
    /* features     = */ chip::BitFlags<Feature>(Feature::kNumericMeasurement,
                                                 Feature::kLevelIndication),
    /* medium       = */ MeasurementMediumEnum::kAir,
    /* unit         = */ MeasurementUnitEnum::kPpm,
    /* minMeasured  = */ chip::app::DataModel::MakeNullable(0.0f),
    /* maxMeasured  = */ chip::app::DataModel::MakeNullable(5000.0f));

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

`MeasurementMedium`, `FeatureMap`, and `ClusterRevision` are always present. See
[examples/air-quality-sensor-app/air-quality-sensor-common/include/air-quality-sensor-manager.h](../../../../examples/air-quality-sensor-app/air-quality-sensor-common/include/air-quality-sensor-manager.h)
and the corresponding `.cpp` for a full working example.

## Backwards Compatibility

### The wrapper: `CodegenIntegration.h`

`CodegenIntegration.h` provides a compatibility shim for applications that used
the original template-based API. It defines:

```
template <bool NumericMeasurementEnabled, bool LevelIndicationEnabled,
          bool MediumLevelEnabled,        bool CriticalLevelEnabled,
          bool PeakMeasurementEnabled,    bool AverageMeasurementEnabled>
class Instance { ... };
```

Internally, `Instance<>` owns a `ConcentrationMeasurementCluster` and a
`ServerClusterRegistration`. At construction it translates the six boolean
template parameters into a `BitFlags<Feature>` and forwards everything to the
underlying cluster. The template booleans are therefore a compile-time view into
the same runtime feature map that `ConcentrationMeasurementCluster` uses.

**`Init()`** — calls
`CodegenDataModelProvider::Instance().Registry().Register(registration)` and
remembers that registration succeeded so the destructor can unregister cleanly.
This preserves the original `Init()`-based ZAP callback pattern; no call-site
changes are needed.

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

### Recommended usage (new code)

Instantiate `ConcentrationMeasurementCluster` directly and register it via
`registry.Register()`, as shown in the **Usage** section above.

### Legacy usage

Previously, applications used the template-based `Instance<>`:

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

This pattern still compiles and works unchanged. `Init()` calls
`CodegenDataModelProvider::Instance().Registry().Register()` internally, and all
`Set*()` methods forward directly to the underlying
`ConcentrationMeasurementCluster`.

## Migrating from the Template-Based API

1. Replace the `Instance<...>` declaration with
   `ConcentrationMeasurementCluster` and a `ServerClusterRegistration`.
2. Build the feature set with `BitFlags<Feature>` instead of template booleans.
3. Move fixed sensor characteristics (`medium`, `unit`, `minMeasured`,
   `maxMeasured`, `uncertainty`) from `SetMin/MaxMeasuredValue()` calls into the
   constructor.
4. Replace `instance.Init()` with an explicit `registry.Register(registration)`
   call in your application init.
5. All `Set*()` call sites are unchanged — method names and signatures are
   identical.

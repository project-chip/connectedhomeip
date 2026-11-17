# Pump Configuration and Control Cluster

`PumpConfigurationAndControlCluster` is a code-driven implementation of the Pump
Configuration and Control cluster. It is based on `DefaultServerCluster` and
keeps all attribute values in the cluster object instead of the Ember attribute
store.

## Files

| File                                    | Purpose                                                              |
| --------------------------------------- | -------------------------------------------------------------------- |
| `PumpConfigurationAndControlCluster.h`  | Cluster class, `Config` and `FixedAttributes`                        |
| `PumpConfigurationAndControlDelegate.h` | `Delegate` interface and `RemoteSensorType`                          |
| `CodegenIntegration.h/.cpp`             | Creates clusters from the ZAP configuration, `FindClusterOnEndpoint` |

## Attributes

| Attribute group                                  | Source of the value                                                           |
| ------------------------------------------------ | ----------------------------------------------------------------------------- |
| MaxPressure, MaxSpeed, MaxFlow, Min/Max\* limits | `Config::fixedAttributes`. Fixed, there are no setters.                       |
| OperationMode, ControlMode                       | Writable. Validated against the FeatureMap. Persisted.                        |
| EffectiveOperationMode, EffectiveControlMode     | Computed by the cluster when OperationMode or ControlMode changes. Persisted. |
| LifetimeRunningHours, LifetimeEnergyConsumed     | Writable by clients and the application. Persisted.                           |
| PumpStatus, Capacity, Speed, Power               | Set by the application.                                                       |

The Min/Max limit attributes follow their conformance: an attribute is present
when its feature is enabled, or when the Automatic feature is enabled and the
attribute is set in `Config::optionalAttributes`.

Optional attributes are enabled with the `Config::optionalAttributes` flags. A
struct of flags is used instead of `OptionalAttributeSet` because the
ControlMode attribute ID (0x21) does not fit in its 32-bit mask.

## Effective modes

| OperationMode    | EffectiveControlMode                                                                | Delegate call           |
| ---------------- | ----------------------------------------------------------------------------------- | ----------------------- |
| Normal           | Remote sensor mode if `GetRemoteSensorType` reports a sensor, otherwise ControlMode | `GetRemoteSensorType`   |
| Minimum, Maximum | ConstantSpeed                                                                       | `OnOperationModeMinMax` |
| Local            | ControlMode. The application may override it with `SetEffectiveControlMode`.        | none                    |

The RemotePressure, RemoteFlow and RemoteTemperature bits of PumpStatus are
updated with the same computation.

## ZAP based applications

`CodegenIntegration.cpp` creates one cluster per endpoint from the ZAP
configuration:

-   FeatureMap and the enabled optional attributes come from ZAP.
-   The fixed attributes, OperationMode and ControlMode use the ZAP default
    values. Configure the hardware limits of the pump as ZAP defaults.
-   When ControlMode is not enabled, the EffectiveControlMode default is used as
    the control mode of the pump.
-   `PumpConfigurationAndControl::CodegenDelegate` is installed. While
    OperationMode is Minimum or Maximum it sets the Level Control CurrentLevel
    to MinLevel (at least 1) or MaxLevel on the same endpoint.

## Migrating from the Ember implementation

The attributes are no longer stored by Ember, and the generated
`PumpConfigurationAndControl::Attributes::<Name>::Get/Set` functions in
`Accessors.h` are gone. `CodegenIntegration.h` declares functions with the same
signatures that forward to the cluster, so most application code only needs one
more include:

```cpp
#include <app/clusters/pump-configuration-and-control-server/CodegenIntegration.h>

// Unchanged application code:
PumpConfigurationAndControl::Attributes::PumpStatus::Set(endpointId, pumpStatus);
```

| Before                                                          | After                                                                          |
| --------------------------------------------------------------- | ------------------------------------------------------------------------------ |
| `Attributes::<Name>::Get/Set/SetNull` for non-fixed attributes  | Same call, with the `CodegenIntegration.h` include                             |
| `Attributes::Max*/Min*::Set` at runtime                         | Not available. Use ZAP default values, or `Config::fixedAttributes`            |
| `Attributes::<Name>::Set(endpoint, value, MarkAttributeDirty)`  | Not available. The setters report changes                                      |
| `MatterPostAttributeChangeCallback` for this cluster            | Not called. Use the `Delegate` hooks                                           |
| `LogEvent(PumpConfigurationAndControl::Events::X::Type{}, ...)` | `GenerateAlarmEvent(PumpConfigurationAndControl::Events::X::Type{})`           |
| Remote sensor detection (not supported before)                  | Subclass `CodegenDelegate`, override `GetRemoteSensorType`, call `SetDelegate` |

The setters return the status of the cluster setter. For example,
`OperationMode::Set` returns ConstraintError for a mode whose feature is not
enabled. Code that needs more than these accessors can use the cluster directly
through `PumpConfigurationAndControl::FindClusterOnEndpoint(endpointId)`.

# Pump Configuration and Control Cluster

`PumpConfigurationAndControlCluster` is a code-driven implementation of the Pump
Configuration and Control cluster. It is based on `DefaultServerCluster` and
keeps all attribute values in the cluster object instead of the Ember attribute
store.

## Files

| File                                    | Purpose                                                                                     |
| --------------------------------------- | ------------------------------------------------------------------------------------------- |
| `PumpConfigurationAndControlCluster.h`  | Cluster class, `Config`, `FixedAttributes` and `OptionalAttributes`                         |
| `PumpConfigurationAndControlDelegate.h` | `Delegate` interface and `RemoteSensorType`                                                 |
| `CodegenIntegration.h/.cpp`             | Creates clusters from the ZAP configuration, `FindClusterOnEndpoint`, `ApplicationDelegate` |

## Attributes

| Attribute group                                  | Source of the value                                                           |
| ------------------------------------------------ | ----------------------------------------------------------------------------- |
| MaxPressure, MaxSpeed, MaxFlow, Min/Max\* limits | `Config::fixedAttributes`. Fixed, there are no setters.                       |
| OperationMode, ControlMode                       | Writable. Validated against the FeatureMap. Persisted.                        |
| EffectiveOperationMode, EffectiveControlMode     | Computed by the cluster (see [Effective modes](#effective-modes)). Persisted. |
| LifetimeRunningHours, LifetimeEnergyConsumed     | Writable by clients and the application. Persisted.                           |
| PumpStatus, Capacity, Speed, Power               | Set by the application.                                                       |

Optional attributes are enabled with the `Config::optionalAttributes` bitmask. A
bitmask of `OptionalAttributes` is used instead of `OptionalAttributeSet`
because the ControlMode attribute ID (0x21) does not fit in its 32-bit mask.

The Min/Max limit attributes follow their conformance: an attribute is present
when its feature is enabled, or when the Automatic feature is enabled and its
bit is set in `Config::optionalAttributes`.

Setters of optional attributes that are not enabled return
`UnsupportedAttribute`.

When no ControlMode value is configured, or when the configured mode requires a
feature that is not enabled, the first supported mode in `ControlModeEnum` order
is used.

## Delegate

A `Delegate` is required. It is passed by reference in `Config` and must outlive
the cluster.

| Method                  | Called when                                                                          |
| ----------------------- | ------------------------------------------------------------------------------------ |
| `GetRemoteSensorType`   | The effective modes are computed while OperationMode is Normal                       |
| `OnOperationModeMinMax` | The effective modes are computed while OperationMode is Minimum or Maximum           |
| `OnAttributeChanged`    | Any attribute value changed: client write, application setter or computed by cluster |

## Effective modes

| OperationMode    | EffectiveControlMode                                                                | Delegate call           |
| ---------------- | ----------------------------------------------------------------------------------- | ----------------------- |
| Normal           | Remote sensor mode if `GetRemoteSensorType` reports a sensor, otherwise ControlMode | `GetRemoteSensorType`   |
| Minimum, Maximum | ConstantSpeed                                                                       | `OnOperationModeMinMax` |
| Local            | ControlMode. The application may override it with `SetEffectiveControlMode`.        | none                    |

The effective modes are computed when OperationMode or ControlMode changes. The
RemotePressure, RemoteFlow and RemoteTemperature bits of PumpStatus are updated
with the same computation.

Call `OnRemoteSensorChanged()` when the value returned by `GetRemoteSensorType`
changes. It only has an effect while OperationMode is Normal, so it never
overwrites the EffectiveControlMode set by the application in Local mode and
never calls `OnOperationModeMinMax`.

## ZAP based applications

`CodegenIntegration.cpp` creates one cluster per endpoint from the ZAP
configuration:

-   FeatureMap and the enabled optional attributes come from ZAP.
-   The fixed attributes, OperationMode and ControlMode use the ZAP default
    values. Configure the hardware limits of the pump as ZAP defaults.
-   When ControlMode is not enabled, the EffectiveControlMode default is used as
    the control mode of the pump.
-   The cluster gets a delegate owned by the integration. While OperationMode is
    Minimum or Maximum it sets the Level Control CurrentLevel to MinLevel (at
    least 1) or MaxLevel on the same endpoint.

Applications can install an `ApplicationDelegate` to report a remote sensor and
to be told about attribute changes:

```cpp
class MyPumpDelegate : public PumpConfigurationAndControl::ApplicationDelegate
{
public:
    PumpConfigurationAndControl::RemoteSensorType GetRemoteSensorType(EndpointId endpointId) override;
    void OnAttributeChanged(EndpointId endpointId, AttributeId attributeId) override;
};

MyPumpDelegate gPumpDelegate;

// Before or after server initialization:
LogErrorOnFailure(PumpConfigurationAndControl::SetApplicationDelegate(endpointId, &gPumpDelegate));

// When the remote sensor connects or disconnects:
if (auto * pump = PumpConfigurationAndControl::FindClusterOnEndpoint(endpointId); pump != nullptr)
{
    pump->OnRemoteSensorChanged();
}
```

`SetApplicationDelegate` queries the remote sensor again if the cluster already
exists. The `ApplicationDelegate` is removed when the cluster shuts down.

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

| Before                                                          | After                                                                  |
| --------------------------------------------------------------- | ---------------------------------------------------------------------- |
| `Attributes::<Name>::Get/Set/SetNull` for non-fixed attributes  | Same call, with the `CodegenIntegration.h` include                     |
| `Attributes::Max*/Min*::Set` at runtime                         | Not available. Use ZAP default values, or `Config::fixedAttributes`    |
| `Attributes::<Name>::Set(endpoint, value, MarkAttributeDirty)`  | Not available. The setters report changes                              |
| `MatterReportingAttributeChangeCallback` after a setter         | Not needed. The setters report changes                                 |
| `MatterPostAttributeChangeCallback` for this cluster            | Not called. Use `ApplicationDelegate::OnAttributeChanged`              |
| `LogEvent(PumpConfigurationAndControl::Events::X::Type{}, ...)` | `GenerateAlarmEvent(PumpConfigurationAndControl::Events::X::Type{})`   |
| Remote sensor detection (not supported before)                  | `ApplicationDelegate::GetRemoteSensorType` and `OnRemoteSensorChanged` |

The accessors return `UnsupportedEndpoint` if the endpoint has no cluster and
`UnsupportedAttribute` if the attribute is not enabled, as the generated
accessors did. Setters return the status of the cluster setter. For example,
`OperationMode::Set` returns ConstraintError for a mode whose feature is not
enabled. Code that needs more than these accessors can use the cluster directly
through `PumpConfigurationAndControl::FindClusterOnEndpoint(endpointId)`.

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

| Attribute group                                  | Source of the value                                                                                                                                                   |
| ------------------------------------------------ | --------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| MaxPressure, MaxSpeed, MaxFlow, Min/Max\* limits | `Config::fixedAttributes`. Fixed, there are no setters. Out-of-range values are replaced with null.                                                                   |
| OperationMode, ControlMode                       | Writable. Validated against the FeatureMap. Persisted.                                                                                                                |
| EffectiveOperationMode                           | Follows OperationMode, Local while LocalOverride is set (see [Effective modes](#effective-modes)). Not persisted: it is rebuilt from the persisted OperationMode.     |
| EffectiveControlMode                             | Computed by the cluster, or set by the application when owned by the application (see [Effective modes](#effective-modes)). Not persisted.                            |
| LifetimeRunningHours, LifetimeEnergyConsumed     | Writable by clients and the application. Persisted.                                                                                                                   |
| PumpStatus, Capacity, Speed, Power               | Set by the application. Capacity, Speed and Power use quieter reporting (`Q` quality): changes are marked reportable at most once per second and on null transitions. |

`LifetimeRunningHours` values stored in the 3-byte `int24u` format by earlier
implementations are loaded at startup and stored again in the current format.

Optional attributes are enabled with the `Config::optionalAttributes`.
`OptionalAttributes` is used instead of `OptionalAttributeSet` because the
ControlMode attribute ID (0x21) does not fit in its 32-bit mask.

The Min/Max limit attributes follow their conformance: an attribute is present
when its feature is enabled, or when the Automatic feature is enabled and its
bit is set in `Config::optionalAttributes`.

Setters of optional attributes that are not enabled return
`UnsupportedAttribute`.

When no ControlMode value is configured, or when the configured mode requires a
feature that is not enabled, the first supported mode in `ControlModeEnum` order
is used.

The spec requires at least one of the ConstantPressure, CompensatedPressure,
ConstantFlow, ConstantSpeed and ConstantTemperature features. `Startup` logs an
error when none of them is enabled.

## Delegate

A `Delegate` is required. It is passed by reference in `Config` and must outlive
the cluster.

| Method                  | Called when                                                                          |
| ----------------------- | ------------------------------------------------------------------------------------ |
| `GetRemoteSensorType`   | The effective modes are computed while OperationMode is Normal                       |
| `OnOperationModeMinMax` | OperationMode changes to Minimum or Maximum                                          |
| `OnAttributeChanged`    | Any attribute value changed: client write, application setter or computed by cluster |

## Effective modes

| OperationMode    | EffectiveControlMode                                                                                                                                                                                                      | Delegate call           |
| ---------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ----------------------- |
| Normal           | Remote sensor mode if `GetRemoteSensorType` reports a sensor, otherwise ControlMode. When no sensor is reported and ControlMode is Automatic or disabled, the application may override it with `SetEffectiveControlMode`. | `GetRemoteSensorType`   |
| Minimum, Maximum | ConstantSpeed                                                                                                                                                                                                             | `OnOperationModeMinMax` |
| Local            | ControlMode. The application may override it with `SetEffectiveControlMode`.                                                                                                                                              | none                    |

EffectiveOperationMode is set to OperationMode when OperationMode changes.
EffectiveControlMode is computed when OperationMode or ControlMode changes, and
when the active remote sensor changes. The RemotePressure, RemoteFlow and
RemoteTemperature bits of PumpStatus are owned by the cluster (ignored in
`SetPumpStatus`) and updated with the same computation. A remote sensor whose
control mode feature is not enabled is ignored.

`SetEffectiveControlMode` returns `InvalidInState` when the cluster computes
EffectiveControlMode: in Minimum and Maximum mode, while a remote sensor is
active, and in Normal mode when ControlMode is enabled and not Automatic.

`OnOperationModeMinMax` is called only when OperationMode changes, or when the
pump returns to OperationMode after LocalOverride (see below). A ControlMode
change while OperationMode is Minimum or Maximum does not call it again, because
ControlMode does not affect how the pump runs in these modes.

### LocalOverride

The LocalOverride bit of PumpStatus means that hardware, such as an external
STOP button or a local HMI, overrides the device control. As required by the
spec, while the application has this bit set with `SetPumpStatus`:

-   EffectiveOperationMode is Local.
-   OperationMode changes return `FAILURE`, for client writes and for
    `SetOperationMode`. Writing the current value succeeds.
-   `SetEffectiveOperationMode` returns `InvalidInState` for modes other than
    Local.

When the bit is cleared, EffectiveOperationMode is set back to OperationMode,
and `OnOperationModeMinMax` is called if that is Minimum or Maximum.

The Local operation mode requires the LocalOperation feature. Without it, the
LocalOverride bit is ignored in `SetPumpStatus` and an error is logged.

EffectiveControlMode, the application ownership of EffectiveControlMode and the
remote sensor handling depend on the OperationMode attribute, not on
LocalOverride.

Call `OnRemoteSensorChanged()` when the value returned by `GetRemoteSensorType`
changes. It only has an effect while the cluster is started, OperationMode is
Normal, and the active remote sensor differs from the one used for the last
computation. It never changes EffectiveOperationMode, never overwrites the
EffectiveControlMode set by the application in Local mode and never calls
`OnOperationModeMinMax`.

The effective modes are not persisted. At startup, they are computed from the
restored OperationMode and ControlMode:

-   EffectiveOperationMode is set to OperationMode.
-   In Normal mode, the delegate is asked for the remote sensor.
-   EffectiveControlMode is computed as described in the table above.

Application overrides (`SetEffectiveOperationMode`, `SetEffectiveControlMode`)
describe the runtime behavior of the pump. Set them again after a restart. A
value stored by the Ember implementation for these attributes is ignored.

Startup does not call `OnOperationModeMinMax` or `OnAttributeChanged`, because
the application and the Level Control cluster may not be ready yet.

## ZAP based applications

`CodegenIntegration.cpp` creates one cluster per endpoint from the ZAP
configuration:

-   FeatureMap and the enabled optional attributes come from ZAP.
-   The fixed attributes, OperationMode and ControlMode use the ZAP default
    values. Configure the hardware limits of the pump as ZAP defaults.
-   When ControlMode is not enabled, the EffectiveControlMode default is used as
    the control mode of the pump.
-   The cluster gets a delegate owned by the integration. When the pump enters
    Minimum or Maximum operation, it sets the Level Control CurrentLevel to
    MinLevel (at least 1) or MaxLevel on the same endpoint.

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

`SetApplicationDelegate` queries the remote sensor again if the cluster is
already started. If the new delegate reports the same remote sensor, nothing changes: an
EffectiveControlMode override set by the application is kept. The
`ApplicationDelegate` is preserved across temporary endpoint shutdowns and
removed when the endpoint is permanently removed.

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

| Before                                                                  | After                                                                                                                                                                                        |
| ----------------------------------------------------------------------- | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `Attributes::<Name>::Get/Set/SetNull` for non-fixed attributes          | Same call, with the `CodegenIntegration.h` include                                                                                                                                           |
| `Attributes::Max*/Min*::Set` at runtime                                 | Not available. Use ZAP default values, or `Config::fixedAttributes`                                                                                                                          |
| `Attributes::<Name>::Set(endpoint, value, MarkAttributeDirty)`          | Not available. The setters report changes                                                                                                                                                    |
| `MatterReportingAttributeChangeCallback` after a setter                 | Not needed. The setters report changes                                                                                                                                                       |
| `MatterPostAttributeChangeCallback` for this cluster                    | Not called. Use `ApplicationDelegate::OnAttributeChanged`                                                                                                                                    |
| `Attributes::EffectiveOperationMode::Set` for a physical local override | Same call (validated against the FeatureMap). The next OperationMode change sets it back to OperationMode, and it is not restored after a reboot. For a hardware override, prefer the LocalOverride bit of PumpStatus |
| `Attributes::EffectiveControlMode::Set`                                 | Same call (validated against the FeatureMap). Returns `InvalidInState` when the cluster computes EffectiveControlMode (see [Effective modes](#effective-modes)). Not restored after a reboot |
| `Attributes::ClusterRevision::Get`                                      | Not available. Use `PumpConfigurationAndControl::kRevision`                                                                                                                                  |
| `LogEvent(PumpConfigurationAndControl::Events::X::Type{}, ...)`         | `GenerateAlarmEvent(PumpConfigurationAndControl::Events::X::Type{})`                                                                                                                         |
| Remote sensor detection (not supported before)                          | `ApplicationDelegate::GetRemoteSensorType` and `OnRemoteSensorChanged`                                                                                                                       |

The accessors return `UnsupportedEndpoint` if the endpoint has no cluster and
`UnsupportedAttribute` if the attribute is not enabled, as the generated
accessors did. Setters return the status of the cluster setter. For example,
`OperationMode::Set` returns ConstraintError for a mode whose feature is not
enabled. Code that needs more than these accessors can use the cluster directly
through `PumpConfigurationAndControl::FindClusterOnEndpoint(endpointId)`.

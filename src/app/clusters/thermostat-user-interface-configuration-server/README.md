# Thermostat User Interface Configuration cluster

`ThermostatUserInterfaceConfigurationCluster` is a code-driven server cluster.
It stores attribute values in the cluster instance and exposes typed getters and
setters for application access. `Config` supplies initial values and selects
optional attributes; enable `ScheduleProgrammingVisibility` in
`Config::optionalAttributes` when constructing an instance that exposes it.

## Reacting to attribute changes

Implement `ThermostatUserInterfaceConfiguration::Delegate` and attach it using
`SetDelegate()`. Override only the callbacks your application needs:

-   `OnTemperatureDisplayModeChanged()`
-   `OnKeypadLockoutChanged()`
-   `OnScheduleProgrammingVisibilityChanged()`

Callbacks receive the new value after it has been stored. They run synchronously
for both Matter writes and application setter calls that change the value.
Invalid values and writes of the current value do not trigger callbacks.
Callbacks are notifications and cannot reject a change. Initial configuration
and attaching a delegate do not trigger callbacks; use the getters if the
application needs to initialize its UI from the current state.

The cluster does not invoke the legacy `MatterPostAttributeChangeCallback`.
Applications that handled this cluster there must move that handling to a
delegate.

## ZAP applications

After the cluster has been initialized on an endpoint, use
`FindClusterOnEndpoint()` from `CodegenIntegration.h` to attach the delegate:

```cpp
#include <app/clusters/thermostat-user-interface-configuration-server/CodegenIntegration.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

namespace {

class ThermostatUiDelegate : public chip::app::Clusters::ThermostatUserInterfaceConfiguration::Delegate
{
public:
    void OnTemperatureDisplayModeChanged(
        chip::app::Clusters::ThermostatUserInterfaceConfiguration::TemperatureDisplayModeEnum value) override
    {
        ChipLogProgress(AppServer, "Temperature display mode changed to %u", static_cast<unsigned>(value));
    }
};

ThermostatUiDelegate gThermostatUiDelegate;

} // namespace

CHIP_ERROR AttachThermostatUiDelegate(chip::EndpointId endpointId)
{
    auto * cluster = chip::app::Clusters::ThermostatUserInterfaceConfiguration::FindClusterOnEndpoint(endpointId);
    VerifyOrReturnError(cluster != nullptr, CHIP_ERROR_NOT_FOUND);
    cluster->SetDelegate(&gThermostatUiDelegate);
    return CHIP_NO_ERROR;
}
```

Call this from application initialization after cluster registration, on the
Matter thread or with the stack lock held. Reattach the delegate if the endpoint
and its cluster instance are recreated.

`CodegenIntegration.h` also declares live attribute `Get()` and `Set()`
functions. Generated `GetDefault()` accessors read startup defaults from the
ZAP/Ember attribute store, rather than the live cluster state.

## Applications that construct the cluster directly

Construct `ThermostatUserInterfaceConfigurationCluster` with the endpoint and
`Config`, attach the delegate with `SetDelegate()`, and register the instance
with the application's data model provider. No `CodegenIntegration` lookup is
needed. See the
[cluster development guide](../../../../docs/guides/writing_clusters.md) for the
registration and lifecycle model.

The application owns the delegate. Keep it alive while attached, or detach it
with `SetDelegate(nullptr)` before destroying it. Each cluster instance has one
delegate pointer. Use separate delegate instances when endpoint-specific state
is needed, since the callbacks receive the value but not the endpoint ID.

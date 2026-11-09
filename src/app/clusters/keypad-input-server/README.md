# Keypad Input cluster

`KeypadInputCluster` is a code-driven server cluster. It forwards all
application-specific behavior to a `KeypadInput::Delegate`:

| Request         | Delegate method                  |
| --------------- | -------------------------------- |
| Read FeatureMap | `GetFeatureMap(endpoint)`        |
| SendKey command | `HandleSendKey(helper, keyCode)` |

The cluster has no attribute storage of its own. `ClusterRevision` is a
constant.

## Delegate contract

-   `HandleSendKey` must answer through `helper` before returning, either with
    `helper.Success(response)` or with `helper.Failure(status)`. If the delegate
    returns without answering, the cluster replies with a `Failure` status.
-   `GetFeatureMap` is called on every `FeatureMap` read. The cluster does not
    report changes of the returned value to subscribers, so the value should
    stay constant while the cluster exists.
-   `GetFeatureMap` must not read the `FeatureMap` attribute of this cluster
    through the data model, because that read calls `GetFeatureMap` again. Use
    the generated `KeypadInput::Attributes::FeatureMap::GetDefault()` accessor
    to read the value configured in ZAP.

## ZAP applications

Set the delegate with `SetDefaultDelegate()` from `CodegenIntegration.h` (also
available through the legacy `keypad-input-server.h` include). The endpoint must
already be enabled, otherwise the call is ignored. The cluster is created when
the endpoint is enabled, so `emberAfKeypadInputClusterInitCallback()` is a valid
place for the call:

```cpp
#include <app/clusters/keypad-input-server/keypad-input-server.h>

chip::app::Clusters::KeypadInput::SetDefaultDelegate(endpoint, &gKeypadInputManager);
```

The delegate is kept while the endpoint is disabled and enabled again, and is
cleared when the endpoint is permanently removed. The delegate must stay valid
until it is replaced or the endpoint is permanently removed.

The delegate for an endpoint is resolved on every request, in this order:

| Order | Delegate                               | Applies to           |
| ----- | -------------------------------------- | -------------------- |
| 1     | `ContentApp::GetKeypadInputDelegate()` | ContentApp endpoints |
| 2     | Value given to `SetDefaultDelegate()`  | All other endpoints  |

ContentApp endpoints only exist when `CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED`
is set. Without a delegate, `FeatureMap` reports the value configured in ZAP and
`SendKey` returns a `Failure` status.

### Migrating from the Ember implementation

The cluster is listed in `CodeDrivenClusters`, so ZAP no longer generates
`KeypadInput::Attributes::FeatureMap::Get()` / `Set()` or
`ClusterRevision::Get()` / `Set()`. Delegates that called
`Attributes::FeatureMap::Get(endpoint, &featureMap)` should call
`Attributes::FeatureMap::GetDefault(endpoint, featureMap)` instead.

## Applications that construct the cluster directly

Construct `KeypadInputCluster` with the endpoint and a delegate that outlives
the cluster, then register the instance with the application's data model
provider. No `CodegenIntegration` code is needed. See the
[cluster development guide](../../../../docs/guides/writing_clusters.md) for the
registration and lifecycle model.

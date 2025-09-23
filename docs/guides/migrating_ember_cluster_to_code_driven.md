# Migrating EMBER clusters (Code generated clusters) to code driven clusters

It is recommended to read the guide of [Writing clusters](./writing_clusters.md) for
a general code-driven cluster description and how to implement them.

This document contains a list of steps that is suitable for converting an existing cluster
from an ember implementation to a code-driven implementation.

## Step 1: evaluate the implementation

Code generated clusters use a mix of the following:

### Pure ember implementation / simple values

When only simple values are required, the ember framework in `src/app/util` is
able to implement read/write of data. The application is then expected
to read/write those values in a types-safe manner existing
[Accessors.h](https://github.com/project-chip/connectedhomeip/blob/master/zzz_generated/app-common/app-common/zap-generated/attributes/Accessors.h)
For code-driven implementation, the data cannot be backed by ember anymore,
and must be members of the cluster. For the cluster or logic implementation,
you will need to:

- If ember/zap defaults need to be loaded:
  - mark the attribute as `RAM` in zap
  - Ensure CodegenIntegration loads the zap value and passes it to the cluster
    implementation (usually in the constructor)
- If attribute is persisted, ensure you load the value during Startup and
  persist it during writes

- if attribute is fixed or does not require persistence, it should be marked
as external/callback.

### `AttributeAccessInterface` and `CommandHandlerInterface`

When `AAI` / `CHI` is used, these generally can be converted into corresponding
`Read/WriteAttribute` and `InvokeCommand` calls. Some notes:

- Ensure that `NotifyAttributeIfChanged` is properly invoked. `AAI` does
  an unconditional update that is implied. `ServerClusterInterface` clusters
  need to be explicit on these
- Most `CHI` can return a CHIP_ERROR or Status and not call `handler->SetStatus`,
  which makes the code easier to read

### Ember command callbacks

Ember command handling is functions named `emberAf<Cluster>Cluster<Command>Callback`.
These should be translated to handlers in `InvokeCommand`. To check:

- InvokeCommand can return an `ActionReturnStatus` optional.
  - The API contract is to return a status/error code if returning a simple
    result (like error or success) `or std::nullopt` if the handler is used
    to set the status (typical for response values or asynchronous handling)
  - For readability, it is preferable to `return statusCode` instead of
    `handler->AddStatus(requestPath, code); return std::nullopt` if applicable.
    Use `std::nullopt` for response setting or for asynchonous processing (rare).

## Step 2: class layout

When converting clusters, optimizing for flash is desirable. For this reason
implement the `Cluster` without separating cluster and logic. Expect the following
files:

- `CodegenIntegration.cpp` - should contain ember/code generation logic and integration
- `<Name>Cluster.{h,cpp}` - actual cluster implementation
- `tests/Test<Name>Cluster.cpp` - contain unit tests for the cluster.
- Corresponding `BUILD.gn` and `tests/BUILD.gn` files
- `app_config_dependent_sources.{gni,cmake}`
  - `gni` should reference `CodegenIntegration.cpp`
  - `cmake` should reference the `CodegenIntegration.cpp` but also the `Cluster.{h,cpp}` files since
    `cmake` is unable to pull in dependencies like `gn` does

## Implementation notes

### Attribute & Command availability

Zap/Ember configurations can allow invalid selections. Code driven code should ensure that
metadata returned in `Attributes` and `Generated/AcceptedCommands` is always correct:

- mandatory items should always be returned
- elements controlled by features should be based on a feature map stored in the cluster
- purely optional elements should be based on a "enabled" member of the class:
  - use a `BitFlags` variable to contain possible values
  - if low IDs allow it, you may use an [OptionalAttributeSet](https://github.com/project-chip/connectedhomeip/blob/master/src/app/server-cluster/OptionalAttributeSet.h#L140)
  - otherwise a `struct` filled with bit-sized booleans is also possible. This will
    be needed for clusters with many attributes/attributes with high IDs

### Use `CodegenClusterIntegration`

Use [CodegenClusterIntegration](https://github.com/project-chip/connectedhomeip/blob/master/src/data-model-providers/codegen/ClusterIntegration.h)
in `CodegenIntegration.cpp` to minimize overhead of loading ember values. Specifically this helps
load optional feature maps and optional attribute lists

> IMPORTANT: Optional attribute bits for this class only work
> if their ID is up to 31. If this is not the case for
> your cluster (e.g. On/Off, Color Control, Door Lock etc.) a custom
> implementation for optional attributes will be needed

### Attribute persistence

Use the attribute persistence in the context.
For scalar values, use a persistence like `AttributePersistence persistence(mContext->attributeStorage);`.

### Event generation

Use `mContext->interactionContext.eventsGenerator.GenerateEvent` to replace
any use of `LogEvent`. This makes events unit testable.

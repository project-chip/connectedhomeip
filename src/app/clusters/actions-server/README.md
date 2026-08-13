# Actions Cluster Server

This directory implements the Matter **Actions cluster** (cluster ID `0x0025`).

## Scope constraint: Node singleton

The Matter spec classifies the Actions cluster as **"Scope: Node"**. This means:

-   The cluster **must appear on exactly one endpoint** on the node — typically
    an aggregator endpoint (e.g. endpoint 1 in bridge apps).
-   Only **one `ActionsServer` instance** should exist at a time. The
    constructor logs an error if this constraint is violated.
-   The `Delegate` interface has no `EndpointId` parameters for the same reason:
    it was designed for a single cluster instance.

## Architecture

There are two layers.

### `ActionsCluster` (preferred for new code)

`ActionsCluster` derives from `DefaultServerCluster` and is the canonical
implementation. It is instantiated once for the aggregator endpoint.

```text
ActionsCluster  (one instance, on the aggregator endpoint)
    └── Actions::Delegate  (supplied by the application)
```

Applications using the code-driven data model create an `ActionsCluster`
directly and register it with `RegisteredServerCluster<ActionsCluster>`.

### `ActionsServer` / `CodegenIntegration` (backwards-compatibility wrapper)

`ActionsServer` is a thin wrapper around `ActionsCluster` for applications
written against the older Ember/ZAP-generated API. It:

-   Owns a `RegisteredServerCluster<ActionsCluster>` for the aggregator
    endpoint.
-   Reads initial attribute state (e.g. `SetupURL`) from Ember RAM at
    construction time.
-   Exposes the legacy `ActionListModified(EndpointId)` /
    `EndpointListModified(EndpointId)` callbacks; calls for the wrong endpoint
    are silently ignored.

### ZAP-generated callbacks

The ZAP-generated plugin callbacks (`MatterActionsClusterInitCallback`, etc.)
are left as empty stubs. Applications instantiate `ActionsServer` directly and
register it with the codegen data model provider via `Init()`. This is the
standard code-driven cluster pattern where the application owns the cluster
lifecycle.

## Usage

### New code (code-driven data model)

```cpp
auto cluster = std::make_unique<ActionsCluster>(aggregatorEndpointId, myDelegate);
// Register via RegisteredServerCluster<ActionsCluster> and call Init().
```

### Legacy / backwards-compatible code

```cpp
// Typically called from emberAfActionsClusterInitCallback, guarded to run once:
sActionsDelegateImpl = std::make_unique<MyDelegate>();
sActionsServer       = std::make_unique<ActionsServer>(aggregatorEndpointId, *sActionsDelegateImpl);
sActionsServer->Init();

// When the action list changes:
sActionsServer->ActionListModified(aggregatorEndpointId);

// Shutdown is called automatically by the destructor.
```

`Init()` and `Shutdown()` are both idempotent.

## Delegate interface

The application supplies a concrete `Actions::Delegate`:

-   `ReadActionAtIndex` / `ReadEndpointListAtIndex` — iterate collections
    (return `CHIP_ERROR_PROVIDER_LIST_EXHAUSTED` to signal end of list).
-   `HaveActionWithId` — fast O(n) lookup by action ID.
-   `Handle*` — one method per Actions command (e.g. `HandleInstantAction`).
    Return `Status::Success` to accept, another status to reject.

## Asynchronous event generation

When an action's state changes after a command has been accepted, call
`GenerateEvent` on the `ActionsCluster` to emit the corresponding Matter event
to the fabric:

```cpp
cluster.GenerateEvent(Events::StateChanged::Type{ actionId, invokeId, newState });
cluster.GenerateEvent(Events::ActionFailed::Type{ actionId, invokeId, state, error });
```

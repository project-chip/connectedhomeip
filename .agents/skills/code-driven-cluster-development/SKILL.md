---
name: code-driven-cluster-development
description: >
    Guidelines for implementing Matter server clusters using the
    DefaultServerCluster base class (code-driven approach). Use this skill
    when writing a new cluster or adding features to an already-migrated one.
---

# Code-Driven Cluster Development

## What Is a Code-Driven Cluster?

A _code-driven_ cluster is a `ServerClusterInterface` implementation that lives in `src/app/clusters/<cluster-folder>/` and extends `DefaultServerCluster`. It stores its own attribute state in C++ member variables instead of the legacy Ember RAM store. The framework calls methods like `ReadAttribute` directly; ZAP-generated accessors (`emberAfReadAttribute`) must not be used inside the cluster class.

---

## Directory / File Layout

```
src/app/clusters/<name>-server/
├── <Name>Cluster.h             # Core class (extends DefaultServerCluster)
├── <Name>Cluster.cpp           # Core class implementation
├── CodegenIntegration.h        # Bridge declarations (ZAP ↔ Cluster)
├── CodegenIntegration.cpp      # ZAP callbacks + FindClusterOnEndpoint()
├── BUILD.gn                    # Core files (no codegen dependencies)
├── app_config_dependent_sources.gni   # Codegen-dependent files
└── tests/
    ├── BUILD.gn
    └── Test<Name>Cluster.cpp
```

- **`BUILD.gn`**: Includes cluster class and tests.
- **`app_config_dependent_sources.gni`**: Includes `CodegenIntegration.cpp` (it depends on application ZAP config).

---

## Core Cluster Class

### Header Pattern
- **Nested `Config`**: Use a nested `Config` struct for constructor arguments.
- **Builders**: Use `WithXxx()` methods for optional attributes to ensure flags and values are set together.
- **No `Init()`**: Use `Startup()` and `Shutdown()`.
- **No Redundant Members**: Do not store `EndpointId`, `mIsRegistered`, or `mContext`. These are handled by the base class.

### Implementation Pattern

#### `ReadAttribute`
- **Exhaustive**: Handle `ClusterRevision`, `FeatureMap`, and all attributes in one `switch`.
- **Direct Return**: Return `UnsupportedAttribute` in the `default` case. No redundant path-validity checks.

#### `WriteAttribute`
- **Delegate**: Return `DefaultServerCluster::WriteAttribute` in the `default` case to handle base behavior.
- **No Double-Notify**: If a setter (e.g., `SetAttributeValue`) already notifies, don't wrap it in `NotifyAttributeChangedIfSuccess`.

#### `InvokeCommand`
- **Auto-Status**: Any return except `std::nullopt` causes the framework to call `handler->AddStatus`.
- **Manual Response**: If you call `handler->AddResponse`, you MUST return `std::nullopt` to avoid dual responses.

---

## CodegenIntegration Layer

This is the **only** place where Ember/ZAP APIs (`Accessors::Get/Set`) are allowed.
1. Allocate via `LazyRegisteredServerCluster`.
2. Read ZAP defaults in an `IntegrationDelegate` to populate the `Config`.
3. Provide `FindClusterOnEndpoint()` for application access.

---

## Unit Testing with ClusterTester

The `ClusterTester` helper class (in `src/app/server-cluster/testing/ClusterTester.h`) is mandatory for testing. It abstracts TLV handling, memory management for views, and fabric context.

### Capabilities & Patterns
- **Direct Testing**: Instantiate the cluster class directly using `TestServerClusterContext`.
- **Automatic TLV**: Use `tester.ReadAttribute(Id, outValue)`, `tester.WriteAttribute(Id, inValue)`, and `tester.Invoke(requestStruct)`.
- **Side Effects**: Verify events via `tester.GetNextGeneratedEvent()` and dirty markings via `tester.GetDirtyList()`.
- **Fabric Scoping**: Use `tester.SetFabricIndex(idx)` to simulate actions from specific fabrics.
- **Coverage**:
    - Every feature combination in `Attributes()` and `AcceptedCommands()`.
    - Boundary cases (min, max, null sentinel rejection).
    - Dirty markings (reporting) for every state change.

---

## Ensuring Spec Compliance

Tests must prove the cluster adheres to the Matter Specification and Test Plans.

### Consult the Source of Truth
Use the `matter-specification-access` skill to obtain and read the latest specification and test plans.
- **Validate Semantics**: Ensure `ReadAttribute` and `WriteAttribute` handle constraints exactly as defined in the spec.
- **Mirror Test Plans**: Use the `.adoc` test plans (e.g., `src/cluster/<name>.adoc` in `chip-test-plans` repo) as the blueprint for your unit tests. If a test plan requires a specific error (e.g., `ConstraintError`), the unit test must assert it.

---

## Common Review Findings (Must-Not-Do)

1. **Namespace Pollution**: Do not use `using DataModel::X = ...` in headers.
2. **Forward Declarations**: Avoid them inside the cluster header; they often indicate poor coupling.
3. **Ember Leakage**: Never include `<app/util/util.h>` or generated `Accessors.h` in the core cluster `.cpp`.
4. **Member Waste**: Do not store `mEndpointId`. Use `GetPaths()[0].mEndpointId`.
5. **Double Notification**: Do not call `NotifyAttributeChanged` if `SetAttributeValue` was used.
6. **Feature Checks**: Do not check features inside `ReadAttribute`. `Attributes()` already gates them.
7. **Magic Numbers**: Use decimal for spec bounds (e.g., `9999`), not hex (e.g., `0x270F`).
8. **Test Gaps**: Always test the boundary cases (min, max, null) and feature-specific attribute lists.
9. **Build Issues**: Every `.h` and `.cpp` must be listed in a `BUILD.gn` or `app_config_dependent_sources` file.

---

## Reference Implementations

Study these clusters to understand specific implementation patterns:

| Pattern | Reference Cluster | PR |
|---|---|---|
| Simple Measurement | `relative-humidity-measurement-server` | [#71424](https://github.com/project-chip/connectedhomeip/pull/71424) |
| Command-heavy + Delegate | `actions-server` | [#43471](https://github.com/project-chip/connectedhomeip/pull/43471) |
| Multi-instance | `closure-dimension-server` | [#43720](https://github.com/project-chip/connectedhomeip/pull/43720) |
| Singleton (Node-scoped) | `basic-information` | [#40422](https://github.com/project-chip/connectedhomeip/pull/40422) |
| Runtime-only (no defaults) | `flow-measurement-server` | [#71552](https://github.com/project-chip/connectedhomeip/pull/71552) |
| Identify/Timer-driven | `identify-server` | [#41232](https://github.com/project-chip/connectedhomeip/pull/41232) |
| Writable scalar + features | `switch-server` | [#42968](https://github.com/project-chip/connectedhomeip/pull/42968) |

---

## References

- **Base Class**: `src/app/server-cluster/DefaultServerCluster.h`
- **Testing**: `src/app/server-cluster/testing/`
- **Integration Helper**: `src/data-model-providers/codegen/ClusterIntegration.h`

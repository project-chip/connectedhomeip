---
name: code-driven-cluster-development
description: >
    Guidelines for implementing Matter server clusters using the
    DefaultServerCluster base class (code-driven approach). Use this skill when
    working in src/app/clusters/ — writing a new cluster, adding features to an
    existing one, or reviewing any change in that directory.
---

# Code-Driven Cluster Development

## What Is a Code-Driven Cluster?

A _code-driven_ cluster is a `ServerClusterInterface` implementation that lives
in `src/app/clusters/<cluster-folder>/` and extends `DefaultServerCluster`. It
stores its own attribute state in C++ member variables instead of the legacy
Ember RAM store. The framework calls methods like `ReadAttribute` directly;
ZAP-generated accessors (`emberAfReadAttribute`) must not be used inside the
cluster class.

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

_Alternative: Legacy-Preserving Layout_ Some clusters (e.g., `on-off-server`)
use a layout that keeps the legacy Ember/ZAP implementation in a `codegen/`
subdirectory while placing the new code-driven implementation in the root.

-   **Not Typical:** This approach is not typical and is generally discouraged
    as it maintains two parallel implementation paths for the same cluster,
    increasing the burden on maintenance, testing, and validation.
-   **Last Resort:** This is only done as a last resort when complex cluster
    inter-dependencies make a full migration difficult without incurring
    significant code complexity or unacceptable resource (Flash/RAM) overhead.

**Build system rules:**

-   Codegen integration files (whether `CodegenIntegration.cpp` or files in
    `codegen/`) go in `app_config_dependent_sources.cmake` and
    `app_config_dependent_sources.gni` (these are the codegen-dependent files).
-   All other files (`<ClusterName>Cluster.h/cpp`, test files) go in `BUILD.gn`.
-   **Sources must belong to a single build target**: Files referenced in
    `BUILD.gn` and `app_config_dependent_sources.*` must be mutually exclusive.
-   `app_config_dependent_sources.cmake` and `app_config_dependent_sources.gni`
    must not contain non-codegen files.
-   The cluster should be added to the cluster list compiled in
    `src/app/clusters/BUILD.gn`.
-   Every source file must appear somewhere: either `BUILD.gn` (if no
    App-specific dependencies) or `app_config_dependent_sources.*` if depending
    on application ZAP configuration. Unlisted headers or cpp files are a review
    red flag.

---

## Core Cluster Class

### Header Pattern

-   **Nested `Config`**: Use a nested `Config` struct for constructor arguments.
-   **Builders**: Use `WithXxx()` methods for optional attributes to ensure
    flags and values are set together.
-   **No `Init()`**: Use `Startup()` and `Shutdown()`.
-   **No Redundant Members**: Do not store `EndpointId`, `mIsRegistered`, or
    `mContext`. These are handled by the base class.

### Implementation Pattern

**When to call the base class**: You MUST call the base class from `Startup()`
and `Shutdown()`. Do NOT call the base class from `ReadAttribute`,
`WriteAttribute`, or `InvokeCommand` — there is no base-class behavior for these
methods; return `UnsupportedAttribute` / `UnsupportedCommand` directly in the
`default` case instead.

#### `ReadAttribute`

-   **Exhaustive**: Handle `ClusterRevision`, `FeatureMap`, and all attributes
    in one `switch`.
-   **No Delegation**: Return `UnsupportedAttribute` directly in the `default`
    case. Do not call `DefaultServerCluster::ReadAttribute` — it has no base
    behavior.
-   **No pre-switch path checks**: The framework only calls `ReadAttribute` for
    paths in the `Attributes()` list; pre-switch validity checks add code size
    and are redundant.
-   **No feature-flag checks inside switch cases**: If optional attributes are
    already conditionally included in `Attributes()`, do not re-check their
    feature flags inside the switch — the framework has already pre-filtered.

#### `WriteAttribute`

-   **No Delegation**: Return `UnsupportedAttribute` directly in the `default`
    case. Do not call `DefaultServerCluster::WriteAttribute` — there is no base
    behavior.
-   **No Double-Notify**: If a setter (e.g., `SetAttributeValue`) already
    notifies, don't wrap it in `NotifyAttributeChangedIfSuccess`.

#### `Attributes`

```cpp
CHIP_ERROR FooCluster::Attributes(
    const ConcreteClusterPath & path,
    ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    AttributeListBuilder listBuilder(builder);

    const DataModel::AttributeEntry optionalAttrs[] = {
        Foo::Attributes::SomeOptional::kMetadataEntry,
    };

    return listBuilder.Append(Span(Foo::Attributes::kMandatoryMetadata),
                              Span(optionalAttrs),
                              mOptionalAttributeSet);
}
```

-   Use `AttributeListBuilder` from
    `<app/server-cluster/AttributeListBuilder.h>`.
-   `kMandatoryMetadata` is typically defined in
    `<clusters/<ClusterName>/Metadata.h>` (generated).
-   Pass the `OptionalAttributeSet` so optional attributes are only included
    when enabled.

#### Attribute mutation helpers

Use the inherited helpers to update values; they handle checking for changes and
notifying subscribers automatically:

```cpp
// Updates the value AND notifies subscribers automatically:
SetAttributeValue(mSomeField, newValue, Foo::Attributes::SomeField::Id);

// For nullable attributes:
SetAttributeValue(mNullableField, DataModel::NullNullable, Foo::Attributes::NullableField::Id);
```

`SetAttributeValue` returns `true` if the value actually changed (and thus a
notification was sent). `NotifyAttributeChanged` should only be used directly
for manually-complex cases (e.g. updating a list member) where it increments the
data version and notifies the IM engine.

#### Spec constraint validation

Return `CHIP_IM_GLOBAL_STATUS(ConstraintError)` (not a `VerifyOrDie`) for
out-of-range values coming from the application at runtime:

```cpp
CHIP_ERROR FooCluster::SetMeasuredValue(DataModel::Nullable<int16_t> value)
{
    if (!value.IsNull())
    {
        VerifyOrReturnError(value.Value() >= kMinAllowed && value.Value() <= kMaxAllowed,
                            CHIP_IM_GLOBAL_STATUS(ConstraintError));
    }
    SetAttributeValue(mMeasuredValue, value, Foo::Attributes::MeasuredValue::Id);
    return CHIP_NO_ERROR;
}
```

Use `VerifyOrDie` in the constructor for invariants that must hold at
construction time (programming errors), and `VerifyOrReturnError` for runtime
checks.

#### Writable attributes

Override `WriteAttribute` only when the cluster has spec-defined writable
attributes. Use `AttributeValueDecoder` to decode the incoming TLV:

```cpp
DataModel::ActionReturnStatus FooCluster::WriteAttribute(
    const DataModel::WriteAttributeRequest & request, AttributeValueDecoder & decoder)
{
    using namespace Foo::Attributes;
    switch (request.path.mAttributeId)
    {
    case WritableAttr::Id: {
        uint16_t value{};
        ReturnErrorOnFailure(decoder.Decode(value));
        return SetWritableAttr(value);
    }
    default:
        return Protocols::InteractionModel::Status::UnsupportedAttribute;
    }
}
```

Return `Protocols::InteractionModel::Status::UnsupportedAttribute` directly in
the `default` case. Do not delegate to `DefaultServerCluster::WriteAttribute` —
there is no base-class behavior for write operations.

#### Commands

```cpp
std::optional<DataModel::ActionReturnStatus> FooCluster::InvokeCommand(
    const DataModel::InvokeRequest & request,
    chip::TLV::TLVReader & input_arguments,
    CommandHandler * handler)
{
    using namespace Foo::Commands;
    switch (request.path.mCommandId)
    {
    case DoSomething::Id: {
        DoSomething::DecodableType req;
        ReturnErrorOnFailure(DataModel::Decode(input_arguments, req));
        return HandleDoSomething(req, handler);
    }
    default:
        return Protocols::InteractionModel::Status::UnsupportedCommand;
    }
}
```

##### Return Codes for `InvokeCommand`

The return type of `InvokeCommand` is
`std::optional<DataModel::ActionReturnStatus>`. Understanding what to return is
critical to avoid encoding duplicate responses:

-   **Any return except `std::nullopt` implies an automatic call to
    `handler->AddStatus`.**

    -   If you return `CHIP_NO_ERROR` (or
        `Protocols::InteractionModel::Status::Success`), the framework will
        automatically add a Success status response.
    -   If you return an error (e.g., `CHIP_ERROR_INVALID_ARGUMENT` or a
        specific IM status), the framework will automatically add the
        corresponding error status.

-   **If you manually add a response or status to the handler, you MUST return
    `std::nullopt`.**
    -   This applies if you call `handler->AddResponse(...)` or
        `handler->AddStatus(...)`.
    -   Returning anything else (even `CHIP_NO_ERROR`) will cause the framework
        to try to add another status, resulting in a bug (dual response
        encoding).

**Typical Patterns:**

1. **Command with data response:**

    ```cpp
    FooResponse::Type response;
    // fill response...
    handler->AddResponse(request.path, response);
    return std::nullopt; // Required because we used handler->AddResponse
    ```

2. **Command with success status (no data):**

    ```cpp
    // Do the work...
    return CHIP_NO_ERROR; // Framework will automatically call AddStatus(Success)
    ```

    _Note: `return Protocols::InteractionModel::Status::Success;` is also valid
    and equivalent._

3. **Command with error status:**
    ```cpp
    if (error) {
        return Protocols::InteractionModel::Status::ConstraintError; // Framework will AddStatus
    }
    ```

**Common Anti-Patterns (Bugs):**

-   `handler->AddResponse(path, response); return CHIP_NO_ERROR;` (Bug: encodes
    response AND success status)
-   `handler->AddStatus(path, Status::Success); return CHIP_NO_ERROR;` (Bug:
    encodes success status twice)

```cpp
CHIP_ERROR FooCluster::AcceptedCommands(
    const ConcreteClusterPath & path,
    ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder)
{
    static constexpr DataModel::AcceptedCommandEntry kCommands[] = {
        Foo::Commands::DoSomething::kMetadataEntry,
    };
    return builder.ReferenceExisting(Span(kCommands));
}
```

#### Events

```cpp
// Emit a spec-defined event using the cluster context:
Foo::Events::StateChanged::Type event{ /* fields */ };
mContext->interactionContext.eventsGenerator.GenerateEvent(event, mPath.mEndpointId);
```

Override `EventInfo` only when non-default read privileges are needed.

---

## CodegenIntegration Layer

This is the **only** place where Ember/ZAP APIs (`Accessors::Get/Set`) are
allowed. Typical pattern:

1. Allocate the cluster via `LazyRegisteredServerCluster`.
2. Read ZAP defaults in an `IntegrationDelegate` to populate the `Config`. The
   delegate is used only during init/lookup/shutdown, so it is safe — and
   preferred — to allocate it **on the stack**.
3. Register with the data model via `CodegenClusterIntegration::RegisterServer`.
4. Provide `FindClusterOnEndpoint()` for application access.

```cpp
void MatterFooPluginServerInitCallback()
{
    // IntegrationDelegate is only used during lookups — stack allocation is fine.
    IntegrationDelegate delegate;
    LazyRegisteredServerCluster<FooCluster> * cluster = /* allocate */;
    CodegenClusterIntegration::RegisterServer(*cluster, delegate);
}
```

See `src/data-model-providers/codegen/ClusterIntegration.h` for the full API.

---

## Unit Testing with ClusterTester

The `ClusterTester` helper class (in
`src/app/server-cluster/testing/ClusterTester.h`) is mandatory for testing. It
abstracts TLV handling, memory management for views, and fabric context. See
`src/app/server-cluster/testing/` for full documentation and examples.

### Capabilities & Patterns

-   **Direct Testing**: Instantiate the cluster class directly using
    `TestServerClusterContext`.
-   **Automatic TLV**: Use `tester.ReadAttribute(Id, outValue)`,
    `tester.WriteAttribute(Id, inValue)`, and `tester.Invoke(requestStruct)`.
-   **Side Effects**: Verify events via `tester.GetNextGeneratedEvent()` and
    dirty markings via `tester.GetDirtyList()`.
-   **Fabric Scoping**: Use `tester.SetFabricIndex(idx)` to simulate actions
    from specific fabrics.
-   **Coverage**:
    -   Every feature combination in `Attributes()` and `AcceptedCommands()`.
    -   Boundary cases (min, max, null sentinel rejection).
    -   Dirty markings (reporting) for every state change.
-   **No `sleep` in tests**: Unit tests must never call `sleep()` or rely on
    real-time delays. If the cluster has time-dependent behavior, use a mock
    clock. Test-plan steps that say "wait X seconds" should advance the mock
    clock, not sleep.

---

## Ensuring Spec Compliance

Tests must prove the cluster adheres to the Matter Specification and Test Plans.

### Consult the Source of Truth

Use the `matter-specification-access` skill to obtain and read the latest
specification and test plans.

-   **Validate Semantics**: Ensure `ReadAttribute` and `WriteAttribute` handle
    constraints exactly as defined in the spec.
-   **Mirror Test Plans**: Use the `.adoc` test plans (e.g.,
    `src/cluster/<name>.adoc` in `chip-test-plans` repo) as the blueprint for
    your unit tests. If a test plan requires a specific error (e.g.,
    `ConstraintError`), the unit test must assert it.

---

## README & API Compatibility

-   **Preserve API backwards compatibility** if converting an existing
    legacy/Ember cluster to code-driven.
-   If breaking compatibility is unavoidable, provide a `README.md` file
    explaining the upgrade steps in the cluster folder.
-   Providing a `README.md` every time is encouraged to explain the API and
    usage further.
-   If the cluster has notable architecture decisions, scope constraints (e.g.
    node singleton), or a delegate interface, add a `README.md` next to the
    cluster files. See `src/app/clusters/actions-server/README.md` or
    `src/app/clusters/air-quality-server/README.md` for good examples.

---

## Common Review Findings

These are patterns that reviewers have flagged repeatedly — avoid them:

1. **Unlisted headers** — every `.h` file must appear in a build file.
2. **Ember APIs in cluster core** — move them to `CodegenIntegration.cpp`.
3. **Missing null checks on pointer-returning singletons** — e.g.
   `Server::GetInstance().GetCASESessionManager()` may return null; check with
   `VerifyOrDie` or `VerifyOrReturnError` before use.
4. **Invalid ZAP defaults not handled gracefully** — e.g. if `min` > `max` in
   ZAP, the cluster should handle this safely (e.g. by nulling the range) rather
   than crashing.
5. **Incomplete optional-attribute test coverage** — test both with and without
   each optional attribute enabled.
6. **Typos in doc comments** — especially copy-paste errors from similar
   clusters (wrong cluster name in a comment).
7. **No setters or updates for fixed attributes** — Attributes that describe
   physical hardware or are fixed at construction (e.g., `MinMeasuredValue`,
   `MaxMeasuredValue`, `MeasurementUnit`) must not have setters in the cluster
   or the delegate. They should be read on-demand from the delegate to save RAM
   and code size.
8. **Global singletons in tests** — tests should use injected mocks/locals
   (`TestServerClusterContext`) rather than accessing `Server::GetInstance()`.
9. **`LogErrorOnFailure` omitted on fire-and-forget calls** — use
   `LogErrorOnFailure(cluster->SetValue(...))` rather than silently ignoring
   errors from setters.
10. **Unnecessary manual `AddStatus`** — Prefer returning the status directly
    from `InvokeCommand` for simple status returns (no data payload), letting
    the framework handle `AddStatus` automatically.
11. **Cross-directory source inclusion in build files** — Avoid listing source
    files from other clusters or directories directly in a target (e.g., in
    tests). Use proper library dependencies instead to avoid duplicate
    compilation and maintenance issues.
12. **Dependency on heavy singletons** — Some singletons, such as
    `Server::GetInstance()` and `InteractionModelEngine::GetInstance()`, are
    very large and difficult to mock or use in tests. Review whether smaller,
    more focused objects can be used or if additional decoupling is possible.
    Example considerations:
    - Instead of injecting a `Server` object, inject the specific objects needed
      by the cluster (e.g., `FabricTable` and `EndpointTable`).
    - Instead of using
      `InteractionModelEngine::GetInstance()->GetDataModelProvider()`, use the
      `DataModel::Provider` that is injected into the cluster context.
    - For complex code that truly requires `Server` or `InteractionModelEngine`,
      consider providing a delegate member and implementing the complex logic in
      `CodegenIntegration.h/cpp` when the goal is to avoid direct coupling to
      `Server` / `InteractionModelEngine` in the cluster itself when only a
      small subset of their functionality is needed.
13. **Namespace pollution in headers** — Do not add top-level
    `using DataModel::X` aliases in headers. Exception: within a class body,
    `using Feature = SomeConcreteCluster::Feature` is acceptable (and useful)
    for base-cluster type aliasing so that codegen-derived types are accessible
    through the base.
14. **Unnecessary forward declarations** — Avoid forward declarations in cluster
    headers; they often signal poor coupling. Exception: a delegate interface
    header may forward-declare the cluster class when delegate methods take the
    cluster as an argument (e.g.,
    `void OnFooChanged(BarCluster & cluster, Foo newValue)`).
15. **Storing mEndpointId** — Do not add a member `mEndpointId`. Use
    `mPath.mEndpointId` (inherited from `DefaultServerCluster`) directly.
16. **Numeric literal format** — Use whichever base is most readable for the
    value. Decimal is clearer for small bounds (e.g., `9999`), but hex is better
    for bitmasks, nullable sentinels (e.g., `0xFFFE`, `0xFF`), and range
    boundaries that are naturally expressed in hex (e.g., `0x3FFF`, `0x7FFF`).
    Do not mechanically convert hex to decimal just to avoid hex.

---

## Reference Implementations

Study these clusters to understand specific implementation patterns:

| Pattern                    | Reference Cluster                      | PR                                                                   |
| -------------------------- | -------------------------------------- | -------------------------------------------------------------------- |
| Simple Measurement         | `relative-humidity-measurement-server` | [#71424](https://github.com/project-chip/connectedhomeip/pull/71424) |
| Command-heavy + Delegate   | `actions-server`                       | [#43471](https://github.com/project-chip/connectedhomeip/pull/43471) |
| Multi-instance             | `closure-dimension-server`             | [#43720](https://github.com/project-chip/connectedhomeip/pull/43720) |
| Singleton (Node-scoped)    | `basic-information`                    | [#40422](https://github.com/project-chip/connectedhomeip/pull/40422) |
| Runtime-only (no defaults) | `flow-measurement-server`              | [#71552](https://github.com/project-chip/connectedhomeip/pull/71552) |
| Identify/Timer-driven      | `identify-server`                      | [#41232](https://github.com/project-chip/connectedhomeip/pull/41232) |
| Writable scalar + features | `switch-server`                        | [#42968](https://github.com/project-chip/connectedhomeip/pull/42968) |

---

## References

-   **Base Class**: `src/app/server-cluster/DefaultServerCluster.h`
-   **Testing**: `src/app/server-cluster/testing/`
-   **Integration Helper**:
    `src/data-model-providers/codegen/ClusterIntegration.h`

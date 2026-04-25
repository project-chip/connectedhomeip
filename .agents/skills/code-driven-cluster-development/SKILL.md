---
name: code-driven-cluster-development
description: >
    Guidelines for implementing or migrating Matter server clusters (code that
    resides in `src/app/clusters`) using the DefaultServerCluster base class
    (code-driven data model approach), as opposed to the legacy ZAP/Ember
    codegen approach.
---

# Code-Driven Cluster Development

## What Is a Code-Driven Cluster?

A _code-driven_ cluster is a `ServerClusterInterface` implementation that lives
in `src/app/clusters/<cluster-folder>/` and extends `DefaultServerCluster`. It
stores its own attribute state in C++ member variables instead of relying on the
Ember attribute RAM store. The framework calls the cluster's virtual methods
(`ReadAttribute`, `WriteAttribute`, `InvokeCommand`, …) directly; ZAP-generated
attribute accessors (`emberAfReadAttribute` etc.) must not be used inside the
cluster class itself.

Note that the `<cluster-folder>` naming is not standardized, but often starts
with the cluster name. It is a mapping defined in
`src/app/zap_cluster_list.json` and it is often (but not always)
`<cluster-name>-server`.

---

## Directory / File Layout

A common pattern for code-driven cluster directory layout is:

```
src/app/clusters/<cluster-name>-server/
├── <ClusterName>Cluster.h             # Core class (extends DefaultServerCluster)
├── <ClusterName>Cluster.cpp           # Core class implementation
├── CodegenIntegration.h               # App-specific Bridge: ZAP ↔ code-driven cluster
├── CodegenIntegration.cpp             # App-specific ZAP callbacks + FindClusterOnEndpoint()
├── BUILD.gn                           # Core files (does NOT include CodegenIntegration)
├── app_config_dependent_sources.cmake # Application code-generation dependencies
├── app_config_dependent_sources.gni   # Application code-generation dependencies
└── tests/
    ├── BUILD.gn
    └── Test<ClusterName>Cluster.cpp
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

### Header (`<ClusterName>Cluster.h`)

```cpp
#pragma once

#include <app/server-cluster/DefaultServerCluster.h>
#include <app/server-cluster/OptionalAttributeSet.h>
#include <clusters/<ClusterName>/Attributes.h>
#include <clusters/<ClusterName>/Metadata.h>

namespace chip::app::Clusters {

class FooCluster : public DefaultServerCluster
{
public:
    // Optional attributes are tracked as a compile-time bitset.
    using OptionalAttributeSet = app::OptionalAttributeSet<
        Foo::Attributes::SomeOptional::Id,
        Foo::Attributes::AnotherOptional::Id>;

    // Use a Config/StartupConfiguration class (or struct for simple cases) for
    // constructor arguments that may be optional or have defaults. Use a class
    // with private members and builder-style .WithXxx() setters to prevent
    // misconfiguration in non-trivial cases.
    class Config
    {
    public:
        Config & WithMinValue(DataModel::Nullable<int16_t> min) { mMinValue = min; return *this; }
        Config & WithMaxValue(DataModel::Nullable<int16_t> max) { mMaxValue = max; return *this; }
        Config & WithOptionalAttributes(OptionalAttributeSet attrs) { mOptionalAttributes = attrs; return *this; }

    private:
        friend class FooCluster;
        DataModel::Nullable<int16_t> mMinValue{};
        DataModel::Nullable<int16_t> mMaxValue{};
        OptionalAttributeSet mOptionalAttributes{};
    };

    FooCluster(EndpointId endpointId, const Config & config = {});

    // --- ServerClusterInterface overrides ---
    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override;
    CHIP_ERROR Attributes(const ConcreteClusterPath & path,
                          ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) override;

    // Application-facing API
    CHIP_ERROR SetMeasuredValue(DataModel::Nullable<int16_t> value);
    DataModel::Nullable<int16_t> GetMeasuredValue() const { return mMeasuredValue; }

protected:
    const BitFlags<Foo::Feature> mFeatureMap;
    OptionalAttributeSet mOptionalAttributeSet;
    DataModel::Nullable<int16_t> mMeasuredValue{};
    // ... other member variables
};

} // namespace chip::app::Clusters
```

Key points:

-   Inherit from `DefaultServerCluster`. Pass `{ endpointId, ClusterId }` to the
    base constructor.
-   Declare `OptionalAttributeSet` as a `using` alias so callers can refer to it
    via `FooCluster::OptionalAttributeSet`.
-   **Use a `Config` type:** For constructor arguments that may be optional or
    have defaults. Prefer a `class` with private members and builder-style
    `.WithXxx()` setters in non-trivial cases, and use a `struct` only for
    simple passive configuration bundles.
-   **Store Separate Variables:** Extract fields from the `Config` object into
    separate member variables in the cluster class. This allows marking
    immutable fields as `const` and prevents accidental runtime modification.
-   Validate constructor arguments with `VerifyOrDie` (programming errors that
    indicate a logic bug at call site, not a recoverable runtime error).
-   Expose application-facing setters/getters; keep attribute storage in
    `protected` or `private` members.

### Implementation (`<ClusterName>Cluster.cpp`)

#### `ReadAttribute`

```cpp
DataModel::ActionReturnStatus FooCluster::ReadAttribute(
    const DataModel::ReadAttributeRequest & request, AttributeValueEncoder & encoder)
{
    using namespace Foo::Attributes;
    switch (request.path.mAttributeId)
    {
    case ClusterRevision::Id:
        return encoder.Encode(Foo::kRevision);
    case FeatureMap::Id:
        return encoder.Encode(static_cast<uint32_t>(mFeatureMap.Raw()));
    case MeasuredValue::Id:
        return encoder.Encode(mMeasuredValue);
    // ... other attributes
    default:
        return Protocols::InteractionModel::Status::UnsupportedAttribute;
    }
}
```

-   **Return `Protocols::InteractionModel::Status::UnsupportedAttribute`
    directly in the `default` case.**
-   The framework pre-filters requests so `ReadAttribute` is only called for
    paths that are in the `Attributes()` list; returning `UnsupportedAttribute`
    for anything unrecognised is the correct and consistent pattern.
-   Always encode/handle `ClusterRevision` and `FeatureMap` explicitly.
-   **Do not add path-validity checks** before the switch — they add code size
    and are redundant because the framework guarantees the path exists.
-   **Do not add feature-flag checks inside the switch cases** for optional
    attributes if those attributes are already conditionally included in the
    `Attributes()` list. The framework pre-filters requests based on the
    supported attributes list.
-   Do not add returning `UnsupportedAttribute` inside attribute switch
    handling. Existent path checks ensure those code lines would never be used.

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
the `default` case to avoid the overhead of a virtual call to
`DefaultServerCluster::WriteAttribute`. This is consistent with the pattern used
in `ReadAttribute`.

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

`CodegenIntegration.h/cpp` (or equivalent files in the `codegen/` subdirectory)
is the **only** place where Ember/ZAP APIs are allowed. Its responsibilities
are:

1. Allocate cluster instances via `LazyRegisteredServerCluster<FooCluster>`.
2. Read ZAP attribute store defaults and construct `Config` structs.
3. Register/unregister clusters via `CodegenClusterIntegration::RegisterServer`.
4. Implement `FindClusterOnEndpoint()` and optional convenience setters.
5. Provide empty stubs for legacy plugin callbacks.

### Typical pattern

```cpp
// CodegenIntegration.cpp

#include <app/clusters/<name>-server/CodegenIntegration.h>
#include <app/clusters/<name>-server/FooCluster.h>
#include <app/static-cluster-config/Foo.h>
#include <data-model-providers/codegen/ClusterIntegration.h>
#include <data-model-providers/codegen/CodegenDataModelProvider.h>

namespace {

constexpr size_t kFixedCount = Foo::StaticApplicationConfig::kFixedClusterConfig.size();
constexpr size_t kMaxCount   = kFixedCount + CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT;

LazyRegisteredServerCluster<FooCluster> gServers[kMaxCount];

class IntegrationDelegate : public CodegenClusterIntegration::Delegate
{
public:
    ServerClusterRegistration & CreateRegistration(EndpointId endpointId,
                                                   unsigned clusterInstanceIndex,
                                                   uint32_t optionalAttributeBits,
                                                   uint32_t featureMap) override
    {
        FooCluster::Config config;
        config.optionalAttributes = FooCluster::OptionalAttributeSet(optionalAttributeBits);
        // Read defaults from Ember store. Tolerate failure (use neutral defaults).
        if (Foo::Attributes::SomeAttr::Get(endpointId, &config.someAttr)
            != Protocols::InteractionModel::Status::Success)
        {
             config.someAttr = kSomeDefaultAttrValue;
        }
        gServers[clusterInstanceIndex].Create(endpointId, config);
        return gServers[clusterInstanceIndex].Registration();
    }
    ServerClusterInterface * FindRegistration(unsigned index) override
    {
        VerifyOrReturnValue(gServers[index].IsConstructed(), nullptr);
        return &gServers[index].Cluster();
    }
    void ReleaseRegistration(unsigned index) override { gServers[index].Destroy(); }
};

} // namespace

void MatterFooClusterInitCallback(EndpointId endpointId)
{
    // Note: integration delegate is only used for lookups and it is OK
    //       for it to live on the stack. Typical pattern is to have
    //       this on the stack for all init/lookup/shutdown.
    IntegrationDelegate delegate;
    CodegenClusterIntegration::RegisterServer(
        { .endpointId = endpointId, .clusterId = Foo::Id,
          .fixedClusterInstanceCount = kFixedCount,
          .maxClusterInstanceCount   = kMaxCount,
          .fetchFeatureMap           = false,
          .fetchOptionalAttributes   = true },
        delegate);
}

void MatterFooClusterShutdownCallback(EndpointId endpointId, MatterClusterShutdownType shutdownType)
{
    IntegrationDelegate delegate;
    CodegenClusterIntegration::UnregisterServer(
        { .endpointId = endpointId, .clusterId = Foo::Id,
          .fixedClusterInstanceCount = kFixedCount,
          .maxClusterInstanceCount   = kMaxCount },
        delegate, shutdownType);
}

namespace chip::app::Clusters::Foo {

FooCluster * FindClusterOnEndpoint(EndpointId endpointId)
{
    IntegrationDelegate delegate;
    return static_cast<FooCluster *>(
        CodegenClusterIntegration::FindClusterOnEndpoint(
            { .endpointId = endpointId, .clusterId = Foo::Id,
              .fixedClusterInstanceCount = kFixedCount,
              .maxClusterInstanceCount   = kMaxCount },
            delegate));
}

// Optional convenience helper (common pattern):
CHIP_ERROR SetSomeValue(EndpointId endpointId, int16_t value)
{
    auto * cluster = FindClusterOnEndpoint(endpointId);
    VerifyOrReturnError(cluster != nullptr, CHIP_ERROR_NOT_FOUND);
    return cluster->SetSomeValue(value);
}

} // namespace chip::app::Clusters::Foo
```

**Key rules for `CodegenIntegration`:**

-   Always check return status / tolerate failure when reading Ember attribute
    defaults; use a safe fallback (null, zero, or a neutral default).
-   When both min/maxMeasuredValue are read from ZAP and form an invalid range
    (e.g., both 0 in a new ZAP config), treat both as null rather than crashing.
-   Do not add empty `MatterFooPluginServerInitCallback` / `ShutdownCallback`
    stubs unless they were generated by ZAP — only stubs that ZAP declares.
-   Pointer return values from singleton accessors (e.g.,
    `Server::GetInstance().GetCASESessionManager()`) must be null-checked before
    use via `VerifyOrDie` or `VerifyOrReturnError`.

### Direct registration (no ZAP integration)

For code-driven-only applications that never use ZAP:

```cpp
RegisteredServerCluster<FooCluster> gCluster(endpointId, config);
CodegenDataModelProvider::Instance().Registry().Register(gCluster.Registration());
```

---

## Unit Tests

Tests live in `tests/Test<ClusterName>Cluster.cpp` and use the Pigweed/GTest
framework.

### Standard structure

```cpp
#include <pw_unit_test/framework.h>
#include <app/clusters/<name>-server/FooCluster.h>
#include <app/server-cluster/testing/AttributeTesting.h>
#include <app/server-cluster/testing/ClusterTester.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>

namespace {
using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::Testing;

// Subclass to expose protected methods for testing
class TestableFooCluster : public FooCluster
{
public:
    using FooCluster::FooCluster;
    using FooCluster::SomeProtectedMethod;
};

struct TestFooCluster : public ::testing::Test
{
    static void SetUpTestSuite()   { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }
    TestServerClusterContext testContext;
};

} // namespace

TEST_F(TestFooCluster, AttributeList)
{
    FooCluster cluster(kRootEndpointId);
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    ReadOnlyBufferBuilder<DataModel::AttributeEntry> attrs;
    ASSERT_EQ(cluster.Attributes(ConcreteClusterPath(kRootEndpointId, Foo::Id), attrs),
              CHIP_NO_ERROR);
    // verify expected attribute set ...

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestFooCluster, ReadAttributes)
{
    FooCluster cluster(kRootEndpointId);
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
    ClusterTester tester(cluster);

    // Read mandatory attributes
    uint16_t revision{};
    ASSERT_EQ(tester.ReadAttribute(Foo::Attributes::ClusterRevision::Id, revision), CHIP_NO_ERROR);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}
```

### Test coverage checklist

-   [ ] Mandatory attributes are readable after construction.
-   [ ] Optional attributes appear in `Attributes()` only when enabled.
-   [ ] Setter returns `CHIP_NO_ERROR` for valid values.
-   [ ] Setter returns `CHIP_IM_GLOBAL_STATUS(ConstraintError)` for out-of-range
        values (including boundary values).
-   [ ] For nullable numeric attributes: verify that the reserved null sentinel
        (e.g. `0xFFFF` for `uint16`) is rejected when written via the data
        model.
-   [ ] `Startup` / `Shutdown` cycle works correctly.
-   [ ] Protected setters are exposed via a `Testable*` subclass when needed.

---

## Ember / ZAP Rules

| Location                     | Ember / ZAP APIs allowed?               |
| ---------------------------- | --------------------------------------- |
| `<ClusterName>Cluster.h/cpp` | **No** — never                          |
| `CodegenIntegration.h/cpp`   | **Yes** — exclusively here              |
| Tests                        | **No** — use `TestServerClusterContext` |

Forbidden in cluster core code: `EmberAfStatus`, `emberAfContainsServer`,
`emberAfReadAttribute`, `emberAfWriteAttribute`, ZAP-generated accessor
functions (`Foo::Attributes::Bar::Get/Set`).

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
3. **Missing `VerifyOrDie` / null checks on singleton pointers** — e.g.
   `Server::GetInstance().GetCASESessionManager()` may return null.
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

---

## References

-   Base class: `src/app/server-cluster/DefaultServerCluster.h`
-   Interface: `src/app/server-cluster/ServerClusterInterface.h`
-   Testing helpers: `src/app/server-cluster/testing/`
-   Build integration helper:
    `src/data-model-providers/codegen/ClusterIntegration.h`
-   Example simple cluster: `src/app/clusters/air-quality-server/`
-   Example cluster with commands: `src/app/clusters/on-off-server/`
-   Example cluster with delegate: `src/app/clusters/actions-server/`
-   Example cluster with config struct:
    `src/app/clusters/flow-measurement-server/`
-   Migration guide: `docs/guides/migrating_ember_cluster_to_code_driven.md`
-   Writing new clusters: `docs/guides/writing_clusters.md`

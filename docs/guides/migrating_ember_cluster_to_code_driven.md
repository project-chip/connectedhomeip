# Migrating Ember Clusters to Code-Driven Implementations

This document provides a step-by-step guide for converting an existing
Ember-based cluster to a modern, code-driven implementation. It is recommended
to first read the guide on [Writing Clusters](./writing_clusters.md) for a
general overview of the code-driven cluster architecture.

---

## Step 1: Evaluate the Existing Ember Implementation

Before writing new code, it's crucial to understand how the current Ember
cluster is implemented. Ember clusters typically use a mix of the following
patterns:

### Pure Ember Implementation (Simple Attributes)

For simple attributes, the Ember framework in `src/app/util` handles data
storage and access. The application interacts with these attributes via the
type-safe accessors in
`zzz_generated/app-common/app-common/zap-generated/attributes/Accessors.h`.

In a code-driven implementation, this data must be moved into member variables
within your new cluster class.

In some cases the cluster directory may not exist. In that case, create a new
directory and add the mapping in
[src/app/zap_cluster_list.json](https://github.com/project-chip/connectedhomeip/blob/master/src/app/zap_cluster_list.json)
under the `ServerDirectories` key.

### `AttributeAccessInterface` (`AAI`) and `CommandHandlerInterface` (`CHI`)

When more complex logic is needed, Ember clusters use these interfaces.

- `AAI` can be directly translated to the `ReadAttribute` and `WriteAttribute`
    methods in your new cluster class.
- `CHI` can be translated to the `InvokeCommand` method.

### Determine ember/zap storage

Ember storage should be moved from `persist/ram/callback` into `ram/callback`:

- if the value loaded from ZAP UI needs to be loaded, use `RAM` and have
    `CodegenIntegration.cpp` load the value from ZAP via `Accessors.h`. A common
    example here is `FeatureMap`

- if the value is internal to the cluster or does not need loading, set it as
    `External` by including it in the `attributeAccessInterfaceAttributes` in
    `zcl.json` and `zcl-with-test-extensions.json`. A common example here is
    `ClusterRevision`

- if the value used to be `persist` it is an indication that the cluster
    should handle persistence (load in `Startup` and store during writes).

## Step 2: Design the Code-Driven Implementation

### Class Layout and File Structure

When converting clusters, optimizing for flash usage is often a priority. For
this reason, it's common to implement the cluster without separating the logic
from the implementation class.

- **Recommendation:** Combine logic and data storage into a single
    `<Name>Cluster` class that implements the `ServerClusterInterface`.
- **Trade-off:** This approach prioritizes a smaller flash footprint over the
    modular, more testable layout described in the
    [Writing Clusters](./writing_clusters.md) guide. The combined approach is
    often suitable for simpler clusters or when resource constraints are tight.

You will need to create the following files:

- `src/app/clusters/<name>/<Name>Cluster.h`
- `src/app/clusters/<name>/<Name>Cluster.cpp`
- `src/app/clusters/<name>/CodegenIntegration.cpp`
- `src/app/clusters/<name>/tests/Test<Name>Cluster.cpp`
- Build files (`BUILD.gn`, `tests/BUILD.gn`,
    `app_config_dependent_sources.gni`, `app_config_dependent_sources.cmake`)

### Attribute and Command Availability

Your new class must accurately report which attributes and commands are
available based on the feature map and other configuration.

- Store the feature map in your cluster instance.
- For optional attributes, use a `BitFlags` variable, an
    `OptionalAttributeSet`, or a `struct` of booleans to track which are
    enabled.

---

## Step 3: Implement the Cluster Logic

This step involves translating the logic from the old Ember patterns into the
new code-driven class structure.

#### Attribute Storage and Persistence

- Attributes are now member variables of your cluster class.
- If attributes were configured as `RAM` in `zap` to load defaults, ensure
    your `CodegenIntegration.cpp` reads these defaults and passes them to your
    cluster's constructor.
- For persisted attributes, use the `AttributePersistence` helper, which is
    available via the `ServerClusterContext`. Load values in `Startup` and save
    them on writes.

#### Command Handling

- Translate `CommandHandlerInterface` calls `emberAf...Callback` functions into logic inside your
    `InvokeCommand` method:

  - ember calls of the form `emberAf<CLUSTER>Cluster<COMMAND>Callback` will be converted to
      a switch `case <CLUSTER>::Commands::<COMMAND>::Id: ...` implementation

      Example:

      ```cpp
      // This
      emberAfAccessControlClusterReviewFabricRestrictionsCallback(...);

      // Becomes this in the `InvokeCommand` implementation:
      switch (request.path.mCommandId) {
        // ...
        case AccessControl::Commands::ReviewFabricRestrictions::Id:
           // ...
      }
      ```

  - Command Handler Interface logic translates directly (same command switches)

- The `InvokeCommand` method can return an `ActionReturnStatus` optional. For
    better readability, prefer returning a status code directly (e.g.,
    `return Status::Success;`) rather than using the command handler to set the
    status, unless you need to return a response with a value or handle the
    command asynchronously.

#### Attribute Access

- Translate `AAI` logic into your `ReadAttribute` and `WriteAttribute`
    methods.
- **Important:** After successfully writing a new attribute value, you
    **must** explicitly call a notification function (e.g., via
    `interactionContext->dataModelChangeListener`) to inform the SDK of the
    change. This is required for subscriptions to work correctly.

#### Event Generation

- Replace any calls to `LogEvent` with
    `mContext->interactionContext.eventsGenerator.GenerateEvent`. This makes
    events unit-testable.

---

## Step 4: Update Build and Codegen Configuration

#### Build Files

Create the necessary build files (`BUILD.gn`, `tests/BUILD.gn`,
`app_config_dependent_sources.gni`, `app_config_dependent_sources.cmake`) to
integrate your new cluster files into the build system.

#### Codegen Integration

In `CodegenIntegration.cpp`, use the `CodegenClusterIntegration` helper class to
minimize the boilerplate needed to read configuration values (like feature maps
and optional attribute lists) from the generated code.

> **Note:** The `CodegenClusterIntegration` helper for optional attributes only
> supports attribute IDs up to 31. For clusters with higher attribute IDs (e.g.,
> `On/Off`, `Color Control`), you will need a custom implementation.

#### ZAP Configuration

You must update the ZAP configuration to inform the code generator that your
cluster is now code-driven. This prevents the Ember framework from managing its
data.

1. **Update `config-data.yaml`:** In
   `src/app/common/templates/config-data.yaml`, add your cluster to the
   `CommandHandlerInterfaceOnlyClusters` array. This disables Ember's command
   dispatch for this cluster.
2. **Update `zcl.json`:** In `src/app/zap-templates/zcl/zcl.json` and
   `zcl-with-test-extensions.json`, add all of your cluster's non-list
   attributes to the `attributeAccessInterfaceAttributes` list. This tells ZAP
   not to allocate RAM for these attributes, as your class now manages them.
3. Re-run ZAP regeneration, like

    ```bash
    ./scripts/run_in_build_env.sh 'scripts/tools/zap_regen_all.py'
    ```

---

## Step 5: Add Unit Tests

- Create `tests/Test<Name>Cluster.cpp` and add unit tests for your new
    implementation.
- Ensure you test the cluster's logic with various feature combinations and
    for all supported attributes and commands.

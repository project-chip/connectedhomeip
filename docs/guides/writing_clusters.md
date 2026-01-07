# Writing and Updating Clusters

This guide provides a comprehensive walkthrough for creating a new Matter
cluster implementation, referred to as a "code-driven" cluster.

## Overview of the Process

Writing a new cluster involves the following key stages:

1. **Define the Cluster:** Generate or update the cluster definition XML based
   on the Matter specification.
2. **Implement the Cluster:** Write the C++ implementation for the cluster's
   logic and data management.
3. **Integrate with Build System:** Add the necessary files to integrate the new
   cluster into the build process.
4. **Integrate with Application:** Connect the cluster to an application's code
   generation configuration.
5. **Test:** Add unit and integration tests to verify the cluster's
   functionality.

---

## Part 1: Cluster Definition (XML)

Clusters are defined based on the Matter specification. The C++ code for them is
generated from XML definitions located in
`src/app/zap-templates/zcl/data-model/chip`.

-   **Generate XML:** To create or update a cluster XML, use
    [Alchemy](https://github.com/project-chip/alchemy) to parse the
    specification's `asciidoc`. Manual editing of XML is discouraged, as it is
    error-prone.
-   **Run Code Generation:** Once the XML is ready, run the code generation
    script. It's often sufficient to run:

    ```bash
    ./scripts/run_in_build_env.sh 'scripts/tools/zap_regen_all.py'
    ```

    For more details, see the
    [code generation guide](../zap_and_codegen/code_generation.md).

---

## Part 2: C++ Implementation

### File Structure

Create a new directory for your cluster at
`src/app/clusters/<cluster-directory>/`. This directory will house the cluster
implementation and its unit tests.

For zap-based support, the directory mapping is defined in
[src/app/zap_cluster_list.json](https://github.com/project-chip/connectedhomeip/blob/master/src/app/zap_cluster_list.json)
under the `ServerDirectories` key. This maps the `UPPER_SNAKE_CASE` define of
the cluster to the directory name under `src/app/clusters`.

#### Naming conventions

Names vary, however to be consistent with most of the existing code use:

-   `cluster-name-server` for the cluster directory name
-   `ClusterNameSnakeCluster.h/cpp` for the `ServerClusterInterface`
    implementation
-   `ClusterNameSnakeLogic.h/cpp` for the `Logic` implementation if applicable

### Recommended Modular Layout

For better testability and maintainability, we recommend splitting the
implementation into logical components. The
[Software Diagnostics](https://github.com/project-chip/connectedhomeip/tree/master/src/app/clusters/software-diagnostics-server)
cluster is a good example of this pattern.

-   **`ClusterLogic`:**
    -   A type-safe class containing the core business logic of the cluster.
    -   Manages all attribute storage.
    -   Should be thoroughly unit-tested.
-   **`ClusterImplementation`:**
    -   Implements the `ServerClusterInterface` (often by deriving from
        `DefaultServerCluster`).
    -   Acts as a translation layer between the data model (encoders/decoders)
        and the `ClusterLogic`.
-   **`ClusterDriver` (or `Delegate`):**
    -   An optional interface providing callbacks to the application for cluster
        interactions. We recommend the term `Driver` to avoid confusion with the
        overloaded term `Delegate`.

### Choosing the Right Implementation Pattern

When implementing a cluster, you have two primary architectural choices: a
**combined implementation** and a **modular implementation**. The best choice
depends on the cluster's complexity and the constraints of the target device,
particularly flash and RAM usage.

-   **Combined Implementation (Logic and Data in One Class):**

    -   **Description:** In this pattern, the cluster's logic, data storage, and
        `ServerClusterInterface` implementation are all contained within a
        single class.
    -   **Pros:** Simpler to write and can result in a smaller flash footprint,
        making it ideal for simple clusters or resource-constrained devices.
    -   **Cons:** Can be harder to test and maintain as the cluster's complexity
        grows.
    -   **Example:** The
        [Basic Information](https://github.com/project-chip/connectedhomeip/tree/master/src/app/clusters/basic-information)
        cluster is a good example of a combined implementation.

-   **Modular Implementation (Logic Separated from Data Model):**
    -   **Description:** This pattern separates the core business logic into a
        `ClusterLogic` class, while the `ClusterImplementation` class handles
        the translation between the data model and the logic.
    -   **Pros:** Promotes better testability, as the `ClusterLogic` can be
        unit-tested in isolation. It is also more maintainable for complex
        clusters.
    -   **Cons:** May use slightly more flash and RAM due to the additional
        class and virtual function calls.
    -   **Example:** The
        [Software Diagnostics](https://github.com/project-chip/connectedhomeip/tree/master/src/app/clusters/software-diagnostics-server)
        cluster demonstrates a modular implementation.

**Recommendation:** Start with a combined implementation for simpler clusters.
If the cluster's logic is complex or if you need to maximize testability, choose
the modular approach.

### BUILD file layout

The description below will describe build files under
`src/app/clusters/<cluster-directory>/`. You are expected to have the following
items:

#### `BUILD.gn`

This file will contain a target that is named `<cluster-directory>`, usually a
`source_set`. This file gets referenced from
[src/app/chip_data_model.gni](https://github.com/project-chip/connectedhomeip/blob/master/src/app/chip_data_model.gni)
by adding a dependency as `deps += [ "${_app_root}/clusters/${cluster}" ]`, so
the default target name is important.

#### `app_config_dependent_sources`

There are two code generation integration support files: one for `GN` and one
for `CMake`. The way these work is that
`chip_data_model.gni`/`chip_data_model.cmake` will include these files and
bundle _ALL_ referenced sources into _ONE SINGLE SOURCE SET_, together with
ember code-generated settings (e.g. `endpoint_config.h` and similar files that
are application-specific)

As a result, there will be a difference between `.gni` and `.cmake`:

-   `app_config_dependent_sources.gni` will typically just contain
    `CodegenIntegration.cpp` and any other helper/compatibility layers (e.g.
    `CodegenIntegration.h` if applicable)
-   `app_config_dependent_sources.cmake` will contain all the files that the
    `.gni` file contains PLUS any dependencies that the `BUILD.gn` would pull in
    but cmake would not (i.e. dependencies not in the `libCHIP` builds). These
    extra files are often the `*.h/*.cpp` files that were in the `BUILD.gn`
    source set.

**EXAMPLE** taken from
([src/app/clusters/basic-information](https://github.com/project-chip/connectedhomeip/tree/master/src/app/clusters/basic-information)):

```
# BUILD.gn
import("//build_overrides/build.gni")
import("//build_overrides/chip.gni")

source_set("basic-information") {
   sources = [ ... ]
   public_deps = [ ... ]
}
```

```
# app_config_dependent_sources.gni
app_config_dependent_sources = [ "CodegenIntegration.cpp" ]
```

```
# app_config_dependent_sources.cmake
# This block adds the codegen integration sources, similar to app_config_dependent_sources.gni
TARGET_SOURCES(
  ${APP_TARGET}
  PRIVATE
    "${CLUSTER_DIR}/CodegenIntegration.cpp"
)

# These are the things that BUILD.gn dependencies would pull
TARGET_SOURCES(
  ${APP_TARGET}
  PRIVATE
    "${CLUSTER_DIR}/BasicInformationCluster.cpp"
    "${CLUSTER_DIR}/BasicInformationCluster.h"
)
```

### Implementation Details

#### Attribute and Feature Handling

Your implementation must correctly report which attributes and commands are
available based on the enabled features and optional items.

-   Use a feature map to control elements dependent on features.
-   Use boolean flags or `BitFlags` for purely optional elements.
-   Ensure your unit tests cover different combinations of enabled features and
    optional attributes/commands.

#### Attribute Accessors

Your cluster implementation must provide public getter and setter APIs for each
attribute to allow applications to interact with cluster state.

-   **Getter Methods:** Provide a getter method for every attribute (e.g.,
    `GetCurrentSensitivityLevel()`, `GetAlarmsActive()`). Applications need
    these to read the current cluster state.

    -   **Return by value (preferred):** Getters should return copies of data
        whenever practical. This avoids lifetime and ownership concerns.

    -   **Avoid returning pointers or references:** Returning pointers or
        references to internal cluster data creates lifetime risks—if the
        underlying memory is deallocated while the caller still holds the
        pointer, use-after-free bugs can occur. If you must return a pointer or
        reference, clearly document that the returned value is only valid for
        immediate use and must not be stored.

-   **Setter Methods:** Provide methods to modify all non-fixed (mutable)
    attributes in spec-compliant ways. For simple attributes, this may be a
    straightforward setter (e.g., `SetCurrentSensitivityLevel()`). However, spec
    compliance may require updating multiple attributes together atomically—in
    such cases, provide a higher-level API that encapsulates the required
    behavior rather than individual setters. When the application's driver state
    changes, these methods can be used to update the cluster's state
    accordingly. Setters are also responsible for triggering attribute change
    notifications (see
    [Attribute Change Notifications](#attribute-change-notifications)).

-   **Example:** The
    [Boolean State Configuration](https://github.com/project-chip/connectedhomeip/blob/master/src/app/clusters/boolean-state-configuration-server/BooleanStateConfigurationCluster.h)
    cluster demonstrates this pattern.

#### Attribute Change Notifications

For subscriptions to work correctly, you must notify the system whenever an
attribute's value changes.

-   The `Startup` method of your cluster receives a `ServerClusterContext`.
-   Use the context to call
    `interactionContext->dataModelChangeListener->MarkDirty(path)`. A
    `NotifyAttributeChanged` helper exists for paths managed by this cluster.

    -   For write implementations, you can use `NotifyAttributeChangedIfSuccess`
        together with a separate `WriteImpl` such that any successful attribute
        write will notify.

        Canonical example code would look like:

        ```cpp
        DataModel::ActionReturnStatus SomeCluster::WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                                          AttributeValueDecoder & decoder)
        {
                // Delegate everything to WriteImpl. If write succeeds, notify that the attribute changed.
                return NotifyAttributeChangedIfSuccess(request.path.mAttributeId, WriteImpl(request, decoder));
        }
        ```

    -   For the `NotifyAttributeChangedIfSuccess` ensure that WriteImpl is
        returning
        [ActionReturnStatus::FixedStatus::kWriteSuccessNoOp](https://github.com/project-chip/connectedhomeip/blob/master/src/app/data-model-provider/ActionReturnStatus.h)
        when no notification should be sent (e.g. write was a `noop` because
        existing value was already the same).

        Canonical example is:

        ```cpp
        VerifyOrReturnValue(mValue != value, ActionReturnStatus::FixedStatus::kWriteSuccessNoOp);
        ```

-   **OnClusterAttributeChanged Pattern:** Each cluster should implement a
    centralized helper method (e.g., `OnClusterAttributeChanged(AttributeId)`)
    that combines both network and application notifications.
    -   Call `NotifyAttributeChanged()` to notify network subscribers.
    -   Call delegate callbacks to notify the application layer.
    -   Invoke this method from `WriteAttribute`, `InvokeCommand`, and setter
        methods.
    -   **Example:** See
        [Boolean State Configuration](https://github.com/project-chip/connectedhomeip/blob/master/src/app/clusters/boolean-state-configuration-server/BooleanStateConfigurationCluster.h)
        which declares `OnClusterAttributeChanged(AttributeId)` as a private
        helper.

#### Persistent Storage

-   **Attributes:** For scalar attribute values, use `AttributePersistence` from
    `src/app/persistence/AttributePersistence.h`. The `ServerClusterContext`
    provides an `AttributePersistenceProvider`.
-   **General Storage:** For non-attribute data, the context provides a
    `PersistentStorageDelegate`.

#### Optimizing for Flash/RAM

For common or large clusters, you may need to optimize for resource usage.
Consider using `C++` templates to compile-time select features and attributes,
which can significantly reduce flash and RAM footprint.

### Advanced `ServerClusterInterface` Details

While `ReadAttribute`, `WriteAttribute`, and `InvokeCommand` are the most
commonly implemented methods, the `ServerClusterInterface` has other methods for
more advanced use cases.

#### List Attribute Writes (`ListAttributeWriteNotification`)

This method is an advanced callback for handling large list attributes that may
require special handling, such as persisting them to storage in chunks. A
typical example of a cluster that might use this is the **Binding cluster**. For
most clusters, the default implementation is sufficient.

#### Event Permissions (`EventInfo`)

You must implement the `EventInfo` method if your cluster emits any events that
require non-default permissions to be read. For example, an event might require
`Administrator` privileges. While not common, this should be verified for every
new cluster implementation and checked during code reviews to ensure event
access is correctly restricted.

#### Accepted vs. Generated Commands

The distinction between `AcceptedCommands` and `GeneratedCommands` can be
understood using a REST API analogy:

-   **`AcceptedCommands`**: These are the "requests" that the server cluster can
    process. In the Matter specification, these are commands sent from the
    client to the server (`client => server`).
-   **`GeneratedCommands`**: These are the "responses" that the server cluster
    can generate after processing an accepted command. In the spec, these are
    commands sent from the server back to the client (`server => client`).

These lists are built based on the cluster's definition in the Matter
specification.

### Unit Testing

-   Unit tests should reside in `src/app/clusters/<cluster-name>/tests/`.
-   At a minimum, `ClusterLogic` should be fully tested, including its behavior
    with different feature configurations.
-   `ClusterImplementation` can also be unit-tested if its logic is complex.
    Otherwise, integration tests should provide sufficient coverage.

---

## Part 3: Build and Application Integration

### Build System Integration

The build system maps cluster names to their source directories. Add your new
cluster to this mapping:

-   Edit `src/app/zap_cluster_list.json` and add an entry for your cluster,
    pointing to the directory you created.

### Application Integration (`CodegenIntegration.cpp`)

To integrate your cluster with an application's `.zap` file configuration, you
need to bridge the gap between the statically generated code and your C++
implementation.

1. **Create `CodegenIntegration.cpp`:** This file will contain the integration
   logic.
2. **Create Build Files:** Add `app_config_dependent_sources.gni` and
   `app_config_dependent_sources.cmake` to your cluster directory. These files
   should list `CodegenIntegration.cpp` and its dependencies. See existing
   clusters for examples.
3. **Use Generated Configuration:** The code generator creates a header file at
   `<app/static-cluster-config/<cluster-name>.h` that provides static,
   application-specific configuration. Use this to initialize your cluster
   correctly for each endpoint.
4. **Implement Callbacks:** Implement
   `Matter<Cluster>ClusterInitCallback(EndpointId)` and
   `Matter<Cluster>ClusterShutdownCallback(EndpointId)` in your
   `CodegenIntegration.cpp`.
5. **Update `config-data.yaml`:** To enable these callbacks, add your cluster to
   the `CodeDrivenClusters` array in
   `src/app/common/templates/config-data.yaml`.
6. **Update ZAP Configuration:** To prevent the Ember framework from allocating
   memory for your cluster's attributes (which are now managed by your
   `ClusterLogic`), you must:
    - In `src/app/common/templates/config-data.yaml`, consider adding your
      cluster to `CommandHandlerInterfaceOnlyClusters` if it does not need Ember
      command dispatch.
    - In `src/app/zap-templates/zcl/zcl.json` and
      `zcl-with-test-extensions.json`, add all non-list attributes of your
      cluster to `attributeAccessInterfaceAttributes`. This marks them as
      externally handled.
7. Once `config-data.yaml` and `zcl.json/zcl-with-test-extensions.json` are
   updated, run the ZAP regeneration command, like

    ```bash
    ./scripts/run_in_build_env.sh 'scripts/tools/zap_regen_all.py'
    ```

---

## Part 4: Example Application and Integration Testing

-   Write unit tests to ensure cluster test coverage
-   **Integrate into an Example:** Add your cluster to an example application,
    such as the `all-clusters-app`, to test it in a real-world scenario.
    -   use tools such as `chip-tool` or `matter-repl` to manually validate the
        cluster
-   **Add Integration Tests:** Write integration tests to validate the
    end-to-end functionality of your cluster against the example application.

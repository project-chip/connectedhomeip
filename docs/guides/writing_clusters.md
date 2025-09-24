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

Create a new directory for your cluster at `src/app/clusters/<cluster-folder>/`.
This directory will house the cluster implementation and its unit tests.

For zap-based support, the directory mapping is defined in
[src/app/zap_cluster_list.json](https://github.com/project-chip/connectedhomeip/blob/master/src/app/zap_cluster_list.json)
under the `ServerDirectories` key. This maps the `UPPER_SNAKE_CASE` define of
the cluster to the folder name under `src/app/clusters`.

#### Naming conventions

Names vary, however to be consistent with most of the existing code use:

-   `cluster-name-server` for the cluster folder name
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

### Implementation Details

#### Attribute and Feature Handling

Your implementation must correctly report which attributes and commands are
available based on the enabled features and optional items.

-   Use a feature map to control elements dependent on features.
-   Use boolean flags or `BitFlags` for purely optional elements.
-   Ensure your unit tests cover different combinations of enabled features and
    optional attributes/commands.

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
        [ActionReturnStatus::FixedStatus::kWriteSuccessNoOp](https://github.com/project-chip/connectedhomeip/blob/master/src/app/data-model-provider/ActionReturnStatus.h
        when no notification should be sent (e.g. write was a noop)

        Canonical example is:

        ```cpp
        VerifyOrReturnValue(mValue != value, ActionReturnStatus::FixedStatus::kWriteSuccessNoOp);
        ```

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

---

## Part 4: Example Application and Integration Testing

-   Write unit tests to ensure cluster test coverage
-   **Integrate into an Example:** Add your cluster to an example application,
    such as the `all-clusters-app`, to test it in a real-world scenario.
    -   use tools such as `chip-tool` or `matter-repl` to manually validate the
        cluster
-   **Add Integration Tests:** Write integration tests to validate the
    end-to-end functionality of your cluster against the example application.

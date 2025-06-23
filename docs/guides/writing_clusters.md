# Writing and updating clusters

The following checklist can be used to write a new cluster

-   Generate the XML based on the specification
-   Define a new `src/app/clusters/<cluster-name>` folder for the cluster code
    and integrate this into the build system
-   Implement cluster logic and unit tests under the new folder
-   Integrate the cluster into an example application
    -   Add codegen-integration support for the cluster
    -   integrate into an example app (e.g. all-clusters app)
-   Add integration tests for the new cluster

## Cluster definitions

Clusters are defined against the Matter specification. The underlying code for
them is code-generated, based on XML definitions from
[src/app/zap-templates/zcl/data-model/chip](https://github.com/project-chip/connectedhomeip/tree/master/src/app/zap-templates/zcl/data-model/chip)
In order to define a new cluster, use
[Alchemy](https://github.com/project-chip/alchemy) to parse the specification
`asciidoc` and generate/update the relevant XML files. Manual editing is
discouraged as we have found that mistakes are easy to make and hard to spot.

Once you have a new or updated XML, run
[code generation](../zap_and_codegen/code_generation.md). It is often sufficient
to `./scripts/run_in_build_env.sh 'scripts/tools/zap_regen_all.py'`

## Integrating into the build system

The build system maps cluster `UPPER_SNAKE_CASE` names into folder names. The
mapping is done in
[src/app/zap_cluster_list.json](https://github.com/project-chip/connectedhomeip/blob/master/src/app/zap_cluster_list.json)
and this file will need your new cluster added.

The mapping defines the folder under which the cluster resides, inside
[src/app/clusters](https://github.com/project-chip/connectedhomeip/tree/master/src/app/clusters)

## Cluster layout

This layout describes a "code-driven capable cluster" implementation. You can
see how an existing cluster implements this such as
[Software Diagnostics](https://github.com/project-chip/connectedhomeip/tree/master/src/app/clusters/software-diagnostics-server).

### Cluster implementation basic design

You will generally have 2 major classes:

-   `ClusterLogic` is intended to be type-safe implementation of the cluster.
    -   It contains all the logic for the cluster
    -   It contains all attribute storage for the cluster
    -   Is unit tested
-   `ClusterImplementation` that provides a translation between value
    encoders/decoders and a `ClusterLogic`

    -   This implements
        [DefaultServerCluster](https://github.com/project-chip/connectedhomeip/blob/master/src/app/server-cluster/DefaultServerCluster.h)
        or more generally the
        [ServerClusterInterface](https://github.com/project-chip/connectedhomeip/blob/master/src/app/server-cluster/ServerClusterInterface.h)
        interface.

-   (optional) a `ClusterDriver` that provides callbacks to an application for
    cluster interactions. Within the SDK the name `Delegate` is often used,
    however since the delegate term is often overloaded, we suggest using the
    term `Driver` for this.

Unit tests will reside in `src/app/clusters/<cluster>/tests` and will test
`ClusterLogic` at a minimum, including varying features, correctness for
attributes/commands and functionality.

`ClusterImplementation` can also be unit tested depending on the complexity of
its implementation. If its implementation is reasonably simple, the integration
tests should validate it.

### Implementation considerations

It is common that exposed attributes are optional or depend on feature enabling.
Ensure that your class always returns correct data depending on selected
features and functionality: this should be part of unit testing.

Consider if optimizing for flash/ram usage is required: common/large clusters
may need this, other application clusters may be able to accept an overhead for
maintainability. If compile-time flash/ram optimization is needed, use templates
to select available features/attributes and if they are enabled or not.

Ensure that every attribute update will notify via the context
`interactionContext->dataModelChangeListener`
(https://github.com/project-chip/connectedhomeip/blob/master/src/app/data-model-provider/Context.h).
This is required for subscriptions to work and should be unit tested:

-   `CHIP_ERROR ClusterServerInterface::Startup(ServerClusterContext & context)`
    will receive the context needed to communicate with the outside world
-   the `context` contains the
    [InteractionModelContext](https://github.com/project-chip/connectedhomeip/blob/master/src/app/server-cluster/ServerClusterContext.h)
    to use

### Persistent storage

> [!IMPORTANT] Attribute persistence support is not fully defined in the new
> cluster format. This will be available after
> [#37924](https://github.com/project-chip/connectedhomeip/issues/37924) is
> fixed.

For general storage, the cluster context
[provides](https://github.com/project-chip/connectedhomeip/blob/master/src/app/server-cluster/ServerClusterContext.h)
a `PersistentStorageDelegate`.

### Integration with application-specific code generation

When using code generation for applications (i.e. a `*.zap` file), every
application will have a source set that explicitly defines enabled items. To
integrate with the codegen data model provider/generated code, following changes
are needed:

-   create a `CodegenIntegration.cpp` file intended to make use of this static
    application configuration.
-   Add build system files: `app_config_dependent_sources.gni` and
    `app_config_dependent_sources.cmake` that contain this file and additional
    dependencies. See existing clusters for examples.
-   Make use of static configuration data as described below

#### Cluster-specific application configuration

These are generated files available for inclusion as
`<app/static-cluster-config/<cluster-name>.h`. They are generated from
[ServerClusterConfig.jinja](https://github.com/project-chip/connectedhomeip/blob/master/scripts/py_matter_idl/matter/idl/generators/cpp/application/ServerClusterConfig.jinja)
and provide the following information:

-   `chip::app::Clusters::<NAME>::kFixedClusterConfig` as an array of
    [ClusterConfiguration](https://github.com/project-chip/connectedhomeip/blob/master/src/app/util/cluster-config.h).
    Both initialization and static asserts can be done based on these
    configurations.

-   `chip::app::Clusters::<NAME>::IsAttributeEnabledOnSomeEndpoint` and
    `chip::app::Clusters::<NAME>::IsCommandEnabledOnSomeEndpoint` are available
    to check if a specific item is enabled on _any_ endpoint. This can be useful
    for dynamic cluster support for code generation (e.g. to define the maximal
    things supported by a cluster that could be instantiated on an endpoint)

Further defines are available through inclusion of
[src/app/util/config.h](https://github.com/project-chip/connectedhomeip/blob/master/src/app/util/config.h),
which will include `gen_config.h` and `endpoint_config.h` as generated files
through `ZAP`. These provide:

-   `CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT` as a count of dynamic endpoints
    for the ember framework
-   `MATTER_DM_<CLUSTER_DEFINE>_ENDPOINT_COUNT` for a count of static endpoints
    (same as the kFixedClusterConfig array size)
-   `MATTER_DM_<CLUSTER_DEFINE>_SERVER` definition as a flag if `CLUSTER` is in
    use by the application at all
-   `<CLUSTER_DEFINE>_ENABLE_<CMD_DEFINE>_CMD` to define if a specific command
    is enabled on a cluster

Beyond that, the following callbacks will be available to initialize and
shutdown clusters. Implement these as needed inside the `CodegenIntegration.cpp`
file:

-   `Matter<Cluster>ClusterServerInitCallback` - single callback for
    initializing the cluster
-   `emberAf<Cluster>ClusterInitCallback` and
    `Matter<Cluster>ServerShutdownCallback` are called on endpoint startup and
    shutdown.

Optional compatibility layers:

-   `Matter<Cluster>ClusterServerAttributeChangedCallback` is currently called
    by ember-clusters after attribute changes. Consider if this should be called
    by a `Driver` registered to the cluster.

#### Update code-generation configuration

To avoid duplication of implementations from ember, update
[src/app/common/templates/config-data.yaml](https://github.com/project-chip/connectedhomeip/blob/master/src/app/common/templates/config-data.yaml)
and set `CommandHandlerInterfaceOnlyClusters` since ember command dispatch will
not be needed

Update `attributeAccessInterfaceAttributes` in
[src/app/zap-templates/zcl/zcl.json](https://github.com/project-chip/connectedhomeip/blob/master/src/app/zap-templates/zcl/zcl.json)
and
[src/app/zap-templates/zcl/zcl-with-test-extensions.json](https://github.com/project-chip/connectedhomeip/blob/master/src/app/zap-templates/zcl/zcl-with-test-extensions.json)
to mark all attributes of the cluster as
`attribute access interface attributes`, so that ember does not reserve RAM for
them (`ClusterLogic` should contain this RAM now). List-typed attributes do not
need to be added in these lists.

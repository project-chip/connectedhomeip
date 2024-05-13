# Mode Base and its aliases

Mode Base is a pseudo cluster. It has no cluster ID. It exists only to be
derived from by other clusters.

# How to use a Mode Base derived cluster

To use a Mode Base derived cluster, you need to

-   Create a class that inherits the `ModeBase::Delegate` class. For simple
    examples that store all their data in memory, look at the
    `<alias name>-mode.*` files in the `src` and `include` directories in
    `examples/all-clusters-app/all-clusters-common`.
-   For this class implement the `GetModeLabelByIndex`, `GetModeValueByIndex`,
    `GetModeTagsByIndex` and `HandleChangeToMode` methods. Look at the
    documentation in `mode-base-server.h` for more information about these
    methods. Cluster-specific enums can be accessed from the cluster namespace.
-   Optionally implement the `Init` function.
-   In some translation unit (.c or .cpp file), instantiate your
    `ModeBase::Instance` inherited class. See the constructor documentation in
    `mode-base-server.h`.
-   Call the `.Init()` function of your instance after the root
    `Server::Init()`.
-   Alternatively, the last two steps can be done in the
    `emberAf<ClusterName>ClusterInitCallback` function.
-   Add `#define MATTER_DM_PLUGIN_MODE_BASE` to your
    `chip_device_project_config_include` file. In the examples, this file is
    `CHIPProjectAppConfig.h`.

**Note** Zap accessor functions for these clusters do not exist. Use the
instance's `Update...` and `Get...` functions to access the attributes.

# How to add new derived clusters

Once a Mode Base derived cluster has been defined in the spec, add the
implementation using the following steps

1.  Translate the spec as an XML in `src/app/zap-templates/zcl/data-model/chip`.
    You can look at similar files on how to do this.
2.  Regenerate the zap code.
3.  Extend the all-clusters-app example to include your new cluster.

# Mode Base and its aliases

Mode Base is a pseudo cluster. It has no cluster ID. It exists only to be derived
from by other clusters.

# How to use a ModeBase derived cluster

To use a ModeBase derived cluster, you need to

-   Create a class that inherits the `ModeBase::Instance` class. For simple examples
    that store all their data in memory,
    look at the `<alias name>-mode.*` files in the `src` and `include`
    directories in `examples/all-clusters-app/all-clusters-common`.
-   For this class implement the `NumberOfModes`, `GetModeLabelByIndex`,
    `GetModeValueByIndex` `GetModeTagsByIndex` and `HandleChangeToMode` methods.
    Look at the documentation in `mode-base-server.h` for more information about
    these methods. Cluster-specific enums can be accessed from the cluster
    namespace.
-   Optionally implement the `AppInit` function.
-   In your translation unit, instantiate your `ModeBase::Instance` inherited class.
    See the constructor documentation in `mode-base-server.h`.
-   Call the `.Init()` function of your instance.

**Note** Zap accessor functions for these clusters will not return correct values.
Use the instance's `Update...` and `Get...` functions to access the attributes.

# How to add new derived clusters

Once a ModeBase derived cluster has been defined in the spec, add the
implementation using the following steps

1.  Translate the spec as an XML in `src/app/zap-templates/zcl/data-model/chip`.
    You can look at similar files on how to do this.
2.  Add the cluster ID to `AliasedClusters` in `mode-base-cluster-objects.h`.
3.  Regenerate the zap code.
4.  Extend the all-clusters-app example to include your new cluster.

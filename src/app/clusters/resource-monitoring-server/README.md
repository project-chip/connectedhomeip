# Resource Monitoring and its aliases

Resource Monitoring is a pseudo cluster. It has no cluster ID. It exists only to
be aliased into other clusters.

# How to use a Resource Monitoring aliased cluster

To use a Resource Monitoring aliased cluster, you need to

-   Create a class that inherits the `ResourceMonitoring::Instance` class. For
    examples look at the files in the `src/instances` and `include/instances`
    directories in `examples/resource-monitoring-app/`.
-   For this class implement the `OnResetCondition` method. Look at the
    documentation in `resource-monitoring-server.h` for more information about
    these methods. Cluster-specific enums can be accessed from the cluster
    namespace.
-   Optionally implement the `AppInit` method.
-   In your main file, instantiate your `ResourceMonitoring::Instance` inherited
    class. See the constructor documentation in `resource-monitoring-server.h`.
-   Call the `.Init()` function of your instance.

**Note** Zap accessor functions for these clusters will not return true values.
Use the instance's `Update...` and `Get...` functions to access the attributes.

# How to add new aliased clusters

Once a Resource Monitoring aliased cluster has been defined in the spec, adding
the implementation to Resource Monitoring using the following steps

1.  Translate the spec as an XML in `src/app/zap-templates/zcl/data-model/chip`.
    You can look at similar files on how to do this.
2.  Add the cluster ID to `AliasedClusters` in
    `resource-monitoring-cluster-objects.h`.
3.  Regenerate the zap code.
4.  Extend the all-clusters-app/resource-monitoring example to include your new
    cluster.

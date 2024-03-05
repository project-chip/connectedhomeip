# The Operational State cluster and its derived clusters

The Operational State cluster is a normal cluster with ID 0x0060. It is also a
base cluster from with other operational state clusters are derived.

## How to use an Operational State cluster

All Operational State derived clusters have their own `Instance` class within
their respective namespace. This class is used to manage the SDK side of the
cluster. This class requires an `OperationalState::Delegate` where the
application specific logic is implemented.

To use an Operational State cluster

-   Create a class that inherits the `OperationalState::Delegate` class.
-   For this class, implement the necessary virtual methods.
-   In some translation unit (.c or .cpp file), instantiate the delegate class.
-   Instantiate the `Instance` class for your Operational State cluster using
    the delegate instance.
-   Call the `Init()` method of your instance after the root `Server::Init()`.
-   Alternatively, the last two steps can be done in the
    `emberAf<ClusterName>ClusterInitCallback` function.

**Note** Zap accessor functions for these clusters do not exist. Use the
instance's `Set...` and `Get...` functions to access the attributes.

## How to add new derived clusters

Once an Operational State derived cluster has been defined in the spec, add the
implementation using the following steps

1.  Translate the spec as an XML in `src/app/zap-templates/zcl/data-model/chip`.
    You can look at similar files on how to do this.
2.  Regenerate the zap code.
3.  Implement an `Instance` class in your cluster's namespace.
4.  Extend the all-clusters-app example to include your new cluster.

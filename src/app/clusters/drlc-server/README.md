# The Demand Response Load Control cluster and its derived clusters

A Demand Response Load Control (DRLC) cluster is fundamentally a pass thru interfacing between utility
and other EMS using Matter events, energy programs and attributes to guide the operation of, typically local,
devices that consume DRLC toward a global power economy.

The Demand Response Load Control cluster is a Matter defined cluster with ID 0x0096. It is a
base cluster from with other load control clusters can be derived.

## How to use an Demand Response Load Control cluster

All Demand Response Load Control derived clusters have their own `Instance` class within
their respective namespace. This class is used to manage the SDK side of the
cluster. This class requires an `OperationalState::Delegate` where the
application specific logic is implemented.  

To create a device specific Demand Response Load Control cluster

-   Create a class that inherits the `OperationalState::Delegate` class.
-   For this class, implement the necessary virtual methods.
-   In some translation unit (.c or .cpp file), instantiate the delegate class.
-   Instantiate the `Instance` class for your Demand Response Load Control cluster using
    the delegate instance.
-   Call the `Init()` method of your instance after the root `Server::Init()`.

A rudimentry delegate example has been implemented in DRLC.cpp.

## How to add new derived clusters

Once an Demand Response Load Control derived cluster has been defined in the spec, add the
implementation using the following steps

1.  Translate the spec as an XML in `src/app/zap-templates/zcl/data-model/chip`.
    You can look at similar files on how to do this.
2.  Regenerate the zap code. Example zap generated code is found in app-common/zap-generated.
3.  Implement an `Instance` class in your cluster's namespace.
4.  Extend the all-clusters-app example to include your new cluster.


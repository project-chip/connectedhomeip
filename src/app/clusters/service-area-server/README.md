# The Service Area Cluster

## Cluster design

The service area cluster is designed around three core classes: `Instance`,
`StorageDelegate`, and a second `Delegate` class.

The `Instance` class serves as the primary interface for safely accessing and
modifying cluster attributes, adhering to the constraints outlined in the
specification document. However, it does not handle the storage of list
attributes.

Storage management is delegated to the `StorageDelegate` class, which provides a
list of virtual methods for the user to implement. These methods are
straightforward and do not require input validation, but they should not be used
directly.

The business logic specific to the device is encapsulated in a second `Delegate`
class. This class can access the `Instance` through the `GetInstance()` method,
enabling it to utilize all the attribute access methods provided by the
`Instance` class. This design allows for a clear separation of concerns, with
each class handling a specific aspect of the cluster's functionality.

## How to use

-   Create a class that inherits the `ServiceArea::StorageDelegate` class.
-   For this class, implement the necessary virtual methods.
-   Create a class that inherits the `ServiceArea::Delegate` class.
-   For this class, implement the necessary virtual methods.
-   In some translation unit (.c or .cpp file), instantiate the delegate
    classes.
-   Instantiate the `Instance` class using the delegate instances.
-   Call the `Instance`'s `Init()` method after the root `Server::Init()`.
-   Alternatively, the last two steps can be done in the
    `emberAfServiceAreaClusterInitCallback` function.

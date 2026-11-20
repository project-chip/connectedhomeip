# The Service Area Cluster

## Cluster design

The service area cluster is designed around four core classes:
`ServiceAreaCluster`, `Instance`, `StorageDelegate`, and `Delegate`.

`ServiceAreaCluster` is the code-driven cluster implementation. It extends
`DefaultServerCluster` and implements the Matter data model (`ReadAttribute`,
`InvokeCommand`, and related methods). List attribute storage is delegated to
`StorageDelegate`; device-specific business logic lives in `Delegate`.

`Instance` is a backwards-compatible wrapper around `ServiceAreaCluster`.
Existing applications (such as `rvc-app`) can continue to use
`ServiceArea::Instance` and `Instance::Init()` without modification. New code
should prefer `ServiceAreaCluster` directly.

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

# Media File Management Cluster (0x0511)

Code-driven implementation of the Media File Management cluster, built on
`DefaultServerCluster`.

## Architecture

-   **`MediaFileManagementCluster`** (`MediaFileManagementCluster.h/.cpp`) — the
    `ServerClusterInterface` implementation. Owns the feature map and all
    attribute/command metadata. Delegates application data and command behavior
    to a `Delegate`.
-   **`Delegate`** (`MediaFileManagementDelegate.h`) — application-implemented
    interface providing storage stats, the `AvailableFiles` /
    `SupportedMimeTypes` lists (index-based), and command handlers. Command
    handlers return an IM `Status`; the two commands with data responses
    (`AddFile`, `GetSharedFile`) populate a response struct by reference.
-   **`MediaFileManagementServer`** (`CodegenIntegration.h/.cpp`) — ZAP/Ember
    integration wrapper. Reads the feature map from the ZAP attribute store and
    registers the cluster with the codegen data model provider. Applications
    using ZAP instantiate one `MediaFileManagementServer` per endpoint and call
    `Init()`.

New (non-ZAP) code can use `MediaFileManagementCluster` directly.

## Features

| Feature      | Bit | Effect                                                 |
| ------------ | --- | ------------------------------------------------------ |
| MediaSharing | 0   | Enables RequestSharedFiles / GetSharedFile / OfferFile |

When `MediaSharing` is not enabled, only `AddFile` and `DeleteFile` are
accepted, and only `AddFileResponse` is generated.

## Events

`SharedFilesAdded` is emitted via
`MediaFileManagementCluster::GenerateSharedFilesAddedEvent()` — call it from the
application when files become available for sharing in response to a
`RequestSharedFiles` command.

## Notes

-   This cluster is provisional in the Matter specification.
-   Storage attributes (`TotalStorage`, `AvailableStorage`) require Manage
    privilege for read, per the spec.

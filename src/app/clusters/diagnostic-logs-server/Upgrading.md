### DiagnosticLogsCluster API Changes

The Diagnostic Logs cluster is a device-wide singleton, not scoped per endpoint.
The API has been simplified to reflect this.

What Changed

#### SetDelegate() API Updated

Old API:

```
using chip::app::clusters::DiagnosticLogs;
DiagnosticLogsServer::Instance().SetDiagnosticLogsProviderDelegate(endpointId, delegate);
```

-   Required passing an `EndpointId`.
-   Misleading, since Diagnostic Logs is not endpoint-scoped.

New API:

```
using chip::app::Clusters;
DiagnosticLogsCluster::Instance().SetDelegate(delegate);
```

-   No `endpointId` required.
-   Clearer intent: sets the global delegate on the singleton cluster.

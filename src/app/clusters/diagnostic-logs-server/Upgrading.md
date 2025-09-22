### DiagnosticLogsCluster API Changes

The Diagnostic Logs cluster is a device-wide singleton. There is no need to
manage delegates on a per-endpoint basis. To reflect this, the API has been
simplified.

What Changed

#### SetDelegate() API Updated

Old API:

```
using chip::app::clusters::DiagnosticLogs;
DiagnosticLogsServer::Instance().SetDiagnosticLogsProviderDelegate(endpointId, delegate);
```

-   Required passing an EndpointId.
-   Misleading, since Diagnostic Logs is not endpoint-scoped.

New API:

```
using chip::app::Clusters;
DiagnosticLogsCluster::Instance().SetDelegate(delegate);
```

-   No endpointId required.
-   Clearer: directly sets the global delegate on the singleton cluster.

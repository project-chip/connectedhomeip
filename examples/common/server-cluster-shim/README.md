---
orphan: true
---

# Server Cluster Shim

## Overview

The `ServerClusterShim` provides a way to adapt legacy (codegen/Ember-based)
server cluster implementations to the new `ServerClusterInterface`. It acts as
an intermediary layer, translating between the new interface methods and legacy
`emberAf...` calls. This allows developers to isolate the dependency on code
generation to the cluster layer, instead of leaking it into the data model
provider layer.

This `ServerClusterShim` enables us to develop a new Data Model Provider (fully
code-driven) that doesn't depend on codegen/Ember and can be tested against real
clusters (while the clusters themselves are not all migrated to fully
code-driven yet). This is meant to be a temporary option, until all cluster
implementations have been migrated to implement the new interface. This is an
effort that's currently in progress in the SDK.

## Usage

To use the `ServerClusterShim`, you need to:

-   Have a ZAP file with all of the legacy clusters configured.
-   Instantiate a `ServerClusterShim` providing the endpoint and cluster ID into
    the constructor.
    -   You can either instantiate one `ServerClusterShim` for multiple
        endpoint/cluster ID pairs or one for each cluster.
-   Call `ServerClusterShim::Startup` to initialize the cluster with a
    `ServerClusterContext`.
-   Interact with the cluster by calling any of the `ServerClusterInterface`
    methods such as:
    -   `ReadAttribute`, `WriteAttribute`, `InvokeCommand`, etc.

See some other examples in `TestServerClusterShim.cpp`.

```
// Instantiate a `ServerClusterShim`
ServerClusterShim cluster({ { endpointId, clusterId } });

// Startup with a ServerClusterContext
ServerClusterContext clusterContext{.provider = &provider, .storage = nullptr, .interactionContext = &imContext};
cluster.Startup(&clusterContext);

// Read Attributes (for example, feature map)
ReadOperation readOperation(endpointId, clusterId, Attributes::FeatureMap::Id);
std::unique_ptr<AttributeValueEncoder> encoder = readOperation.StartEncoding();

ASSERT_EQ(cluster.ReadAttribute(readOperation.GetRequest(), *encoder), CHIP_NO_ERROR);
ASSERT_EQ(readOperation.FinishEncoding(), CHIP_NO_ERROR);

// Validate after read
std::vector<DecodedAttributeData> attribute_data;
ASSERT_EQ(readOperation.GetEncodedIBs().Decode(attribute_data), CHIP_NO_ERROR);
```

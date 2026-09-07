---
name: zap-matter-analysis
description:
    Guidelines and common jq/grep/awk queries for investigating ZAP (.zap) and
    Matter (.matter) files to understand endpoints, clusters, attributes, and
    commands.
---

# ZAP and Matter File Analysis Skill

This skill provides instructions and common commands for analyzing ZAP (.zap)
and `.matter` files in the Matter repository. These files define the data model
of the applications.

## 1. ZAP File Analysis (.zap)

ZAP files are JSON files that contain the configuration of endpoints, endpoint
types, enabled clusters, attributes, and commands.

### 1.1 High-level Endpoint Type Investigation

To get a quick overview of all endpoint types, their device types, and enabled
clusters (divided into server and client):

```bash
jq '.endpointTypes[] | {
  id,
  deviceTypeName,
  server_clusters: [.clusters[] | select(.side == "server" and .enabled == 1) | .name],
  client_clusters: [.clusters[] | select(.side == "client" and .enabled == 1) | .name]
}' <path_to_zap_file>
```

### 1.2 List All Endpoints

To see the list of all endpoints and which endpoint type index they map to:

```bash
jq '.endpoints[] | {endpointId, endpointTypeName, endpointTypeIndex}' <path_to_zap_file>
```

### 1.3 Locate a Cluster (by Code)

To find which endpoint types have a specific cluster enabled (e.g., OnOff
cluster with code `6`):

```bash
jq '.endpointTypes[] | {
  id,
  name,
  clusters: [.clusters[] | select(.code == <cluster_code> and .enabled == 1) | {name, side}]
}' <path_to_zap_file>
```

### 1.4 List Enabled Attributes for a Cluster

To list all enabled attributes for a specific cluster (by code and side) across
all endpoint types:

```bash
jq '.endpointTypes[] | {
  id,
  name,
  enabledAttributes: [.clusters[] | select(.code == <cluster_code> and .side == "<side>" and .enabled == 1) | .attributes?[] | select(.included == 1) | .name]
}' <path_to_zap_file>
```

_Example for OnOff (6) server attributes:_ Use `.code == 6`,
`.side == "server"`.

### 1.5 List Enabled Commands for a Cluster

To list all enabled commands for a specific cluster (by code and side) across
all endpoint types:

```bash
jq '.endpointTypes[] | {
  id,
  name,
  enabledCommands: [.clusters[] | select(.code == <cluster_code> and .side == "<side>" and .enabled == 1) | .commands?[] | select(.isEnabled == 1) | .name]
}' <path_to_zap_file>
```

_Example for OnOff (6) server commands:_ Use `.code == 6`, `.side == "server"`.

### 1.6 ZAP File Format Schema

Since `.zap` files are standard JSON, they can be parsed, validated, and queried
programmatically. Below is the schema/structure of a `.zap` file:

-   **Top-Level Structure**:

    -   `fileFormat` (number): Version of the ZAP file format.
    -   `featureLevel` (number): ZAP feature level.
    -   `creator` (string): Tool that created it (usually `"zap"`).
    -   `keyValuePairs` (array): Key-value settings for the session (e.g.,
        default response policy).
    -   `package` (array): ZCL and generation templates used by this file.
    -   `endpointTypes` (array): Definitions of endpoint types (clusters,
        attributes, commands).
    -   `endpoints` (array): Instances of endpoints mapping to `endpointTypes`.

-   **`endpointTypes` Object**:

    -   `id` (number): Unique ID for the endpoint type.
    -   `name` (string): Name of the endpoint type.
    -   `deviceTypeName` (string): Device type name (e.g., `"MA-onofflight"`).
    -   `deviceTypeCode` (number): Device type code (e.g., `256`).
    -   `clusters` (array): Clusters enabled on this endpoint type.

-   **`clusters` Object (inside `endpointTypes`)**:

    -   `name` (string): Cluster name (e.g., `"On/Off"`).
    -   `code` (number): Cluster ID (e.g., `6`).
    -   `side` (string): `"client"` or `"server"`.
    -   `enabled` (number): `1` if enabled, `0` if disabled.
    -   `attributes` (array, optional): Configuration of attributes for this
        cluster.
    -   `commands` (array, optional): Configuration of commands for this
        cluster.
    -   `events` (array, optional): Configuration of events for this cluster.

-   **`attributes` Object (inside `clusters`)**:

    -   `name` (string): Attribute name.
    -   `code` (number): Attribute ID.
    -   `included` (number): `1` if enabled on this endpoint, `0` otherwise.
    -   `storageOption` (string): `"RAM"`, `"External"`, etc.
    -   `defaultValue` (string/null): Default value.

-   **`commands` Object (inside `clusters`)**:

    -   `name` (string): Command name.
    -   `code` (number): Command ID.
    -   `source` (string): `"client"` or `"server"`.
    -   `isEnabled` (number): `1` if enabled, `0` otherwise.

-   **`endpoints` Object**:
    -   `endpointId` (number): The endpoint number (e.g., `1`).
    -   `endpointTypeName` (string): References the name of the `endpointType`.
    -   `endpointTypeIndex` (number): 0-based index of the referenced
        `endpointType` in the `endpointTypes` array.
    -   `profileId` (number): Profile ID (typically `259` for Matter).

---

## 2. Matter IDL File Analysis (.matter)

Matter IDL (.matter) files are human-readable representations of the data model.
They are typically generated from ZAP files.

### 2.1 List All Endpoints and Device Types

To quickly list all endpoints defined in the IDL and their device types:

```bash
rg -A 2 "endpoint [0-9]+" <path_to_matter_file>
```

### 2.2 Extract Endpoint Block and List Server Clusters

To extract the definition block of a specific endpoint (e.g., `endpoint 1`) and
list all its server clusters:

```bash
sed -n '/endpoint[[:space:]]\+<endpoint_id>[[:space:]]*{/,/^}/p' <path_to_matter_file> | grep "server cluster"
```

_Example for endpoint 1:_ `sed -n '/endpoint 1 {/,/^}/p' ...`

### 2.3 Extract Endpoint Block and List Binding (Client) Clusters

To extract the definition block of a specific endpoint (e.g., `endpoint 1`) and
list all its binding (client) clusters:

```bash
sed -n '/endpoint[[:space:]]\+<endpoint_id>[[:space:]]*{/,/^}/p' <path_to_matter_file> | grep "binding cluster"
```

### 2.4 Detect Duplicate Cluster Definitions (Generator Bugs)

Sometimes, if a cluster is enabled as both client and server in the same app,
the generator might bug out and generate duplicate top-level cluster
definitions. Use this command to detect them:

```bash
awk '/^cluster [A-Za-z0-9_]+ = (0x[0-9A-Fa-f]+|[0-9]+) \{/ { count[$0]++; } END { for (line in count) { if (count[line] > 1) print count[line], line; } }' <path_to_matter_file>
```

This will output the count and the cluster definition line for any duplicates
found.

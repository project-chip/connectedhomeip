# Matter ESP32 Bridge App Example

Please
[setup ESP-IDF and Matter Environment](../../../docs/platforms/esp32/setup_idf_chip.md)
and refer
[building and commissioning](../../../docs/platforms/esp32/build_app_and_commission.md)
guides to get started.

---

-   [Introduction](#introduction)
-   [Usage Guide](#usage-guide)
-   [Dynamic Endpoints](#dynamic-endpoints)
-   [Cluster control](#cluster-control)

---

## Introduction

A prototype application that demonstrates a Matter bridge with dynamic endpoint
management. The bridge allows you to add and remove non-Matter devices as
endpoints at runtime using shell commands. Each bridged device appears as an
On/Off Light to Matter controllers.

## Usage Guide

### Shell Commands

After flashing and commissioning the bridge, use these shell commands to manage
bridged devices:

| Command                        | Description                    |
| ------------------------------ | ------------------------------ |
| `bridge add [name] [location]` | Add a new bridged light device |
| `bridge remove <endpoint>`     | Remove a device by endpoint ID |
| `bridge list`                  | List all bridged devices       |
| `bridge toggle [endpoint]`     | Toggle device(s) on/off        |
| `bridge max`                   | Show endpoint limits           |
| `bridge removeall`             | Remove all bridged devices     |

### Adding Devices

Add bridged devices with optional name and location:

```
bridge add                     # Adds "Light 1" in "Room"
bridge add "Kitchen Light"     # Adds with custom name
bridge add "Lamp" "Bedroom"    # Adds with name and location
```

Each device is assigned a unique endpoint ID automatically.

### Removing Devices

Remove a device by its endpoint ID:

```
bridge remove 3
```

### Listing Devices

View all bridged devices:

```
bridge list
```

Output example:

```
Bridged devices (2/16):
  "Kitchen Light" @ Kitchen (endpoint 3, ON)
  "Bedroom Lamp" @ Bedroom (endpoint 4, OFF)
```

### Toggling Devices

Toggle a specific device or all devices:

```
bridge toggle 3      # Toggle device at endpoint 3
bridge toggle        # Toggle all devices
```

### Controlling from Matter Controller

After commissioning, use chip-tool to control bridged devices:

```bash
# Turn on device at endpoint 3
chip-tool onoff on <node-id> 3

# Turn off device at endpoint 3
chip-tool onoff off <node-id> 3

# Toggle device at endpoint 3
chip-tool onoff toggle <node-id> 3
```

## Dynamic Endpoints

The Bridge Example makes use of Dynamic Endpoints. Current SDK support is
limited for dynamic endpoints, since endpoints are typically defined (along with
the clusters and attributes they contain) in a .zap file which then generates
code and static structures to define the endpoints.

To support endpoints that are not statically defined, the ZCL attribute storage
mechanisms will hold additional endpoint information for `NUM_DYNAMIC_ENDPOINTS`
additional endpoints. These additional endpoint structures must be defined by
the application and can change at runtime.

To facilitate the creation of these endpoint structures, several macros are
defined:

`DECLARE_DYNAMIC_ATTRIBUTE_LIST_BEGIN(attrListName)`
`DECLARE_DYNAMIC_ATTRIBUTE(attId, attType, attSizeBytes, attrMask)`
`DECLARE_DYNAMIC_ATTRIBUTE_LIST_END(clusterRevision)`

-   These three macros are used to declare a list of attributes for use within a
    cluster. The declaration must begin with the
    `DECLARE_DYNAMIC_ATTRIBUTE_LIST_BEGIN` macro which will define the name of
    the allocated attribute structure. Each attribute is then added by the
    `DECLARE_DYNAMIC_ATTRIBUTE` macro. Finally,
    `DECLARE_DYNAMIC_ATTRIBUTE_LIST_END` macro should be used to close the
    definition.

-   All attributes defined with these macros will be configured as
    `MATTER_ATTRIBUTE_FLAG_EXTERNAL_STORAGE` in the ZCL database and therefore
    will rely on the application to maintain storage for the attribute.
    Consequently, reads or writes to these attributes must be handled within the
    application by the `emberAfExternalAttributeWriteCallback` and
    `emberAfExternalAttributeReadCallback` functions. See the bridge
    application's `main.cpp` for an example of this implementation.

`DECLARE_DYNAMIC_CLUSTER_LIST_BEGIN(clusterListName)`
`DECLARE_DYNAMIC_CLUSTER(clusterId, clusterAttrs, role, incomingCommands, outgoingCommands)`
`DECLARE_DYNAMIC_CLUSTER_LIST_END`

-   These three macros are used to declare a list of clusters for use within a
    endpoint. The declaration must begin with the
    `DECLARE_DYNAMIC_CLUSTER_LIST_BEGIN` macro which will define the name of the
    allocated cluster structure. Each cluster is then added by the
    `DECLARE_DYNAMIC_CLUSTER` macro referencing attribute list previously
    defined by the `DECLARE_DYNAMIC_ATTRIBUTE...` macros and the lists of
    incoming/outgoing commands terminated by kInvalidCommandId (or nullptr if
    there aren't any commands in the list). Finally,
    `DECLARE_DYNAMIC_CLUSTER_LIST_END` macro should be used to close the
    definition.

`DECLARE_DYNAMIC_ENDPOINT(endpointName, clusterList)`

-   This macro is used to declare an endpoint and its associated cluster list,
    which must be previously defined by the `DECLARE_DYNAMIC_CLUSTER...` macros.

### Cluster control

#### onoff

To use the Client to send Matter commands, run the built executable and pass it
the target cluster name, the target command name as well as an endpoint id.

```
$ ./out/debug/chip-tool onoff on <NODE ID> <ENDPOINT>
```

The client will send a single command packet and then exit.

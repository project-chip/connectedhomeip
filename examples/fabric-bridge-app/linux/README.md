# Matter Linux Fabric Bridge Example

An example application to implement the Aggregator device type with Fabric
Synchronization condition met and demonstrates the end-to-end Fabric
Synchronization feature using dynamic endpoints.

Fabric Synchronization feature will facilitate the commissioning of end devices
from one fabric to another without requiring user intervention for every end
device. It defines mechanisms that can be used by multiple
ecosystems/controllers to communicate with one another to simplify the
experience for users.

This doc is tested on **Ubuntu 22.04 LTS (aarch64)**

<hr>

-   [Matter Linux Fabric Bridge Example](#matter-linux-fabric-bridge-example)
    -   [Theory of Operation](#theory-of-operation)
    -   [Building](#building)
    -   [Running the Complete Example on Ubuntu](#running-the-complete-example-on-ubuntu)

<hr>

## Theory of Operation

### Dynamic Endpoints

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
    `ATTRIBUTE_MASK_EXTERNAL_STORAGE` in the ZCL database and therefore will
    rely on the application to maintain storage for the attribute. Consequently,
    reads or writes to these attributes must be handled within the application
    by the `emberAfExternalAttributeWriteCallback` and
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

## Building

-   Install tool chain

    ```sh
    sudo apt-get install git gcc g++ python pkg-config libssl-dev libdbus-1-dev libglib2.0-dev ninja-build python3-venv python3-dev unzip
    ```

-   Build the example application:

    ### For Linux host example:

    ```
    source scripts/activate.sh
    ./scripts/build/build_examples.py --target linux-x64-fabric-bridge-rpc-no-ble build
    ```

    ### For Raspberry Pi 4 example:

    Pull Docker Images

    ```
    docker pull ghcr.io/project-chip/chip-build-crosscompile:112
    ```

    Run docker

    ```
    docker run -it -v ~/connectedhomeip:/var/connectedhomeip ghcr.io/project-chip/chip-build-crosscompile:112 /bin/bash
    ```

    Build

    ```
    cd /var/connectedhomeip

    git config --global --add safe.directory /var/connectedhomeip

    ./scripts/run_in_build_env.sh \
     "./scripts/build/build_examples.py \
        --target linux-arm64-fabric-bridge-no-ble-clang-rpc \
        build"
    ```

    Transfer the fabric-bridge-app binary to a Raspberry Pi

    ```
    scp ./fabric-bridge-app ubuntu@xxx.xxx.xxx.xxx:/home/ubuntu
    ```

## Running the Complete Example on Ubuntu

-   Building

    Follow [Building](#building) section of this document.

-   Run Linux Fabric Bridge Example App

    ```sh
    cd ~/connectedhomeip/examples/fabric-bridge-app/linux
    sudo out/debug/fabric-bridge-app
    ```

-   Test the device using FabricAdmin on your laptop / workstation etc.

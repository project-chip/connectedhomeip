# Matter Linux Fabric Sync Example

An example application to implement the Fabric Synchronization feature and
demonstrates the end-to-end Fabric Synchronization feature using dynamic
endpoints.

Fabric Synchronization feature will facilitate the commissioning of end devices
from one fabric to another without requiring user intervention for every end
device. It defines mechanisms that can be used by multiple
ecosystems/controllers to communicate with one another to simplify the
experience for users.

This doc is tested on **Ubuntu 22.04 LTS (aarch64)**

<hr>

-   [Matter Linux Fabric Sync Example](#matter-linux-fabric-sync-example)
    -   [Theory of Operation](#theory-of-operation)
    -   [Building](#building)
    -   [Running the Complete Example on Ubuntu](#running-the-complete-example-on-ubuntu)

<hr>

## Theory of Operation

### Dynamic Endpoints

The Fabric-Sync Example makes use of Dynamic Endpoints. Current SDK support is
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

## Building

-   For Linux host example:

    ```sh
    source scripts/activate.sh
    ./scripts/build/build_examples.py --target linux-x64-fabric-sync-no-ble build
    ```

-   For Raspberry Pi 4 example:

    Pull Docker Images

    ```sh
    docker pull ghcr.io/project-chip/chip-build-crosscompile:119
    ```

    Run docker

    ```sh
    docker run -it -v ~/connectedhomeip:/var/connectedhomeip ghcr.io/project-chip/chip-build-crosscompile:119 /bin/bash
    ```

    Build

    ```sh
    cd /var/connectedhomeip

    git config --global --add safe.directory /var/connectedhomeip

    ./scripts/run_in_build_env.sh \
     "./scripts/build/build_examples.py \
        --target linux-arm64-fabric-sync-no-ble-clang \
        build"
    ```

    Transfer the fabric-bridge-app binary to a Raspberry Pi

    ```sh
    scp ./fabric-sync ubuntu@xxx.xxx.xxx.xxx:/home/ubuntu
    ```

## Running the Complete Example on Ubuntu

-   Building

    Follow [Building](#building) section of this document.

-   Run Linux Fabric Sync Example App on two Linux machine E1 and E2

    ```sh
    sudo rm -rf /tmp/chip_*
    cd ~/connectedhomeip/
    out/debug/fabric-sync
    ```

-   Initiate the FS Setup Process from E1 to E2

    ```sh
    > app add-bridge 1 20202021 192.168.86.246 5540
    Done
    > New device with Node ID: 0000000000000001 has been successfully added.
    A new device has been added on Endpoint: 2.
    ```

-   Verify Reverse Commissioning of the Fabric-Bridge from E1 on E2

    ```sh
    > New device with Node ID: 0000000000000002 has been successfully added.
    ```

-   Pair Light Example to E2

    Since Fabric-Bridge also functions as a Matter server, running it alongside
    the Light Example app on the same machine would cause conflicts. Therefore,
    you need to run the Matter Light Example app on a separate physical machine
    from the one hosting Fabric-Sync.

    ```sh
    > app add-device 3 <setup-pin-code> <device-remote-ip> <device-remote-port>
    ```

    After the device is successfully added, you will observe the following
    message on E2 with the newly assigned Node ID:

    ```sh
    > New device with Node ID: 0x3 has been successfully added.
    ```

    Additionally, you should also get notified when a new device is added to E2
    from the E1:

    ```sh
    > A new device is added on Endpoint 3.
    ```

-   Synchronize Light Example to E1

    After the Light Example is successfully paired in E2, we can start to
    synchronize the light device to E1 using the new assigned dynamic endpointid
    on Ecosystem 2.

    ```sh
    > app sync-device <endpointid>
    ```

# Matter Linux Bridge Example

An example demonstrating a simple lighting bridge and the use of dynamic
endpoints. The document will describe the theory of operation and how to build
and run Matter Linux Bridge Example on Raspberry Pi. This doc is tested on
**Ubuntu for Raspberry Pi Server 20.04 LTS (aarch64)** and **Ubuntu for
Raspberry Pi Desktop 20.10 (aarch64)**

<hr>

-   [Matter Linux Bridge Example](#matter-linux-bridge-example)
    -   [Theory of Operation](#theory-of-operation)
    -   [Building](#building)
    -   [Running the Complete Example on Raspberry Pi 4](#running-the-complete-example-on-raspberry-pi-4)

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

### Limitations

Because code generation is dependent upon the clusters and attributes defined in
the .zap file (for static endpoint generation), it is necessary to include a
defined endpoint within the .zap that contains _all_ the clusters that may be
used on dynamic endpoints. On the bridge example, this is done on endpoint 1,
which is used as a 'dummy' endpoint that will be disabled at runtime. Endpoint 0
is also defined in the .zap and contains the bridge basic and configuration
clusters as well as the root descriptor cluster.

### Bridge Implementation Example

The example demonstrates the use of dynamic endpoints and the concept of adding
and removing endpoints at runtime. First, the example declares a
`bridgedLightEndpoint` data structure for a Light endpoint with `OnOff`,
`Descriptor`, `BridgedDeviceBasicInformation`, and `FixedLabel` clusters.

Using this declared endpoint structure, three endpoints for three bridged lights
are dynamically added at endpoint ID's `2`, `3`, and `4`, representing
`Light 1`, `Light 2`, and `Light 3` respectively.

Then, endpoint `3` is removed, simulating the deletion of `Light 2`.

A fourth light, `Light 4`, is then added occupying endpoint ID `5`.

Finally, `Light 2` is re-added, and will occupy endpoint ID `6`.

All endpoints populate the `Bridged Device Basic Information` and `Fixed Label`
clusters. In the `Bridged Device Basic Information` cluster, the `reachable`
attribute is simulated. In the `Fixed Label` cluster, the `LabelList` attribute
is simulated with the value/label pair `"room"`/`[light name]`.

## Building

-   Install tool chain

    ```sh
    sudo apt-get install git gcc g++ python pkg-config libssl-dev libdbus-1-dev libglib2.0-dev ninja-build python3-venv python3-dev unzip
    ```

-   Build the example application:

    ```sh
    cd ~/connectedhomeip/examples/bridge-app/linux
    git submodule update --init
    source third_party/connectedhomeip/scripts/activate.sh
    gn gen out/debug
    ninja -C out/debug
    ```

-   To delete generated executable, libraries and object files use:

    ```sh
    cd ~/connectedhomeip/examples/bridge-app/linux
    rm -rf out/
    ```

## Running the Complete Example on Raspberry Pi 4

-   Prerequisites

    1. A Raspberry Pi 4 board
    2. A USB Bluetooth Dongle, Ubuntu desktop will send Bluetooth advertisement,
       which will block CHIP from connecting via BLE. On Ubuntu server, you need
       to install `pi-bluetooth` via APT.
    3. Ubuntu 20.04 or newer image for ARM64 platform.

-   Building

    Follow [Building](#building) section of this document.

-   Running

    -   [Optional] Plug USB Bluetooth dongle

        -   Plug USB Bluetooth dongle and find its bluetooth device number. The
            number after `hci` is the bluetooth device number, `1` in this
            example.

            ```sh
            $ hciconfig
            hci1:	Type: Primary  Bus: USB
                BD Address: 00:1A:7D:AA:BB:CC  ACL MTU: 310:10  SCO MTU: 64:8
                UP RUNNING PSCAN ISCAN
                RX bytes:20942 acl:1023 sco:0 events:1140 errors:0
                TX bytes:16559 acl:1011 sco:0 commands:121 errors:0

            hci0:	Type: Primary  Bus: UART
                BD Address: B8:27:EB:AA:BB:CC  ACL MTU: 1021:8  SCO MTU: 64:1
                UP RUNNING PSCAN ISCAN
                RX bytes:8609495 acl:14 sco:0 events:217484 errors:0
                TX bytes:92185 acl:20 sco:0 commands:5259 errors:0
            ```

        -   Run Linux Bridge Example App

            ```sh
            cd ~/connectedhomeip/examples/bridge-app/linux
            sudo out/debug/chip-bridge-app --ble-device [bluetooth device number]
            # In this example, the device we want to use is hci1
            sudo out/debug/chip-bridge-app --ble-device 1
            ```

        -   Test the device using ChipDeviceController on your laptop /
            workstation etc.

# CHIP ESP32 Bridge App Example

A prototype application that demonstrates dynamic endpoint with device
commissioning and cluster control. It adds the non-chip device as endpoints on a
bridge(Matter device). In this example four light devices supporting on-off
cluster have been added as endpoints

1. Light1 at endpoint 2
2. Light2 at endpoint 6
3. Light3 at endpoint 4
4. Light4 at endpoint 5

---

-   [CHIP ESP32 Bridge App Example](#chip-esp32-bridge-app-example)
    -   [Dynamic Endpoints](#dynamic-endpoints)
    -   [Building the Example Application](#building-the-example-application)
    -   [Commissioning and cluster control](#commissioning-and-cluster-control)
        -   [Setting up chip-tool](#setting-up-chip-tool)
        -   [Commissioning over BLE](#commissioning-over-ble)
        -   [Cluster control](#cluster-control)

---

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
    `ATTRIBUTE_MASK_EXTERNAL_STORAGE` in the ZCL database and therefore will
    rely on the application to maintain storage for the attribute. Consequently,
    reads or writes to these attributes must be handled within the application
    by the `emberAfExternalAttributeWriteCallback` and
    `emberAfExternalAttributeReadCallback` functions. See the bridge
    application's `main.cpp` for an example of this implementation.

`DECLARE_DYNAMIC_CLUSTER_LIST_BEGIN(clusterListName)`
`DECLARE_DYNAMIC_CLUSTER(clusterId, clusterAttrs, incomingCommands, outgoingCommands)`
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

## Building the Example Application

Building the example application requires the use of the Espressif ESP32 IoT
Development Framework and the xtensa-esp32-elf toolchain.

The VSCode devcontainer has these components pre-installed, so you can skip this
step. To install these components manually, follow these steps:

-   Clone the Espressif ESP-IDF and checkout
    [v4.4.1 release](https://github.com/espressif/esp-idf/releases/tag/v4.4.1)

          ```
          $ mkdir ${HOME}/tools
          $ cd ${HOME}/tools
          $ git clone https://github.com/espressif/esp-idf.git
          $ cd esp-idf
          $ git checkout v4.4.1
          $ git submodule update --init
          $ ./install.sh
          ```

-   Install ninja-build

          ```
          $ sudo apt-get install ninja-build
          ```

Currently building in VSCode _and_ deploying from native is not supported, so
make sure the IDF_PATH has been exported(See the manual setup steps above).

-   Setting up the environment

        ```
        $ cd ${HOME}/tools/esp-idf
        $ ./install.sh
        $ . ./export.sh
        $ cd {path-to-connectedhomeip}
        ```

    To download and install packages.

        ```
        $ source ./scripts/bootstrap.sh
        $ source ./scripts/activate.sh
        ```

    If packages are already installed then simply activate them.

        ```
        $ source ./scripts/activate.sh
        ```

-   Configuration Options

    This application uses `ESP32-DevKitC` as a default device type. To use other
    ESP32 based device types, please refer
    [examples/all-clusters-app/esp32](https://github.com/project-chip/connectedhomeip/tree/master/examples/all-clusters-app/esp32)

-   Enable Ccache for faster IDF builds

    It is recommended to have Ccache installed for faster builds

    ```
    $ export IDF_CCACHE_ENABLE=1
    ```

-   To build the demo application.

          ```
          $ idf.py build
          ```

-   After building the application, to flash it outside of VSCode, connect your
    device via USB. Then run the following command to flash the demo application
    onto the device and then monitor its output. If necessary, replace
    `/dev/tty.SLAB_USBtoUART`(MacOS) with the correct USB device name for your
    system(like `/dev/ttyUSB0` on Linux). Note that sometimes you might have to
    press and hold the `boot` button on the device while it's trying to connect
    before flashing. For ESP32-DevKitC devices this is labeled in the
    [functional description diagram](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/hw-reference/esp32/get-started-devkitc.html#functional-description).

          ```
          $ idf.py -p /dev/tty.SLAB_USBtoUART flash monitor
          ```

    Note: Some users might have to install the
    [VCP driver](https://www.silabs.com/products/development-tools/software/usb-to-uart-bridge-vcp-drivers)
    before the device shows up on `/dev/tty`.

-   Quit the monitor by hitting `Ctrl+]`.

    Note: You can see a menu of various monitor commands by hitting
    `Ctrl+t Ctrl+h` while the monitor is running.

-   If desired, the monitor can be run again like so:

          ```
          $ idf.py -p /dev/tty.SLAB_USBtoUART monitor
          ```

## Commissioning and cluster control

Commissioning can be carried out using WiFi or BLE.

1.  Set the `Rendezvous Mode` for commissioning using menuconfig; the default
    Rendezvous mode is BLE.

         ```
         $ idf.py menuconfig
         ```

Select the Rendezvous Mode via `Demo -> Rendezvous Mode`.

2.  Now flash the device with the same command as before. (Use the right `/dev`
    device)

         ```
         $ idf.py -p /dev/tty.SLAB_USBtoUART flash monitor
         ```

3.  The device should boot up. When device connects to your network, you will
    see a log like this on the device console.

         ```
         I (5524) chip[DL]: SYSTEM_EVENT_STA_GOT_IP
         I (5524) chip[DL]: IPv4 address changed on WiFi station interface: <IP_ADDRESS>...
         ```

4.  Use
    [python based device controller](https://github.com/project-chip/connectedhomeip/tree/master/src/controller/python)
    or  
    [standalone chip-tool](https://github.com/project-chip/connectedhomeip/tree/master/examples/chip-tool)
    or  
    [iOS chip-tool app](https://github.com/project-chip/connectedhomeip/tree/master/src/darwin/CHIPTool)
    or  
    [Android chip-tool app](https://github.com/project-chip/connectedhomeip/tree/master/src/android/CHIPTool)
    to communicate with the device.

Note: The ESP32 does not support 5GHz networks. Also, the Device will persist
your network configuration. To erase it, simply run.

    ```
    $ idf.py -p /dev/tty.SLAB_USBtoUART erase_flash
    ```

-   Once ESP32 is up and running, we need to set up a device controller to
    perform commissioning and cluster control.

### Setting up chip-tool

See [the build guide](../../../docs/guides/BUILDING.md#prerequisites) for
general background on build prerequisites.

Building the example:

```
$ cd examples/chip-tool

$ rm -rf out

$ gn gen out/debug

$ ninja -C out/debug
```

which puts the binary at `out/debug/chip-tool`

### Commission a device using chip-tool

To initiate a client commissioning request to a device, run the built executable
and choose the pairing mode.

#### Commissioning over BLE

Run the built executable and pass it the discriminator and pairing code of the
remote device, as well as the network credentials to use.

The command below uses the default values hard-coded into the debug versions of
the ESP32 all-clusters-app to commission it onto a Wi-Fi network:

    ```
    $ ./out/debug/chip-tool pairing ble-wifi 12344321 ${SSID} ${PASSWORD} 20202021 3840
    ```

Parameters:

1. Discriminator: 3840
2. Setup-pin-code: 20202021
3. Node-id: 12344321 (you can assign any node id)

### Cluster control

#### onoff

To use the Client to send Matter commands, run the built executable and pass it
the target cluster name, the target command name as well as an endpoint id.

    ```
    $ ./out/debug/chip-tool onoff on 12344321 2
    ```

The client will send a single command packet and then exit.

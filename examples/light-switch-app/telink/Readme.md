![Telink B91 EVK](http://wiki.telink-semi.cn/wiki/assets/Hardware/B91_Generic_Starter_Kit_Hardware_Guide/connection_chart.png)

## Build and flash

1. Pull docker image from repository:

    ```bash
    $ docker pull connectedhomeip/chip-build-telink:latest
    ```

1. Run docker container:

    ```bash
    $ docker run -it --rm -v ${CHIP_BASE}:/root/chip -v /dev/bus/usb:/dev/bus/usb --device-cgroup-rule "c 189:* rmw" connectedhomeip/chip-build-telink:latest
    ```

    here `${CHIP_BASE}` is directory which contains CHIP repo files **!!!Pay
    attention that OUTPUT_DIR should contains ABSOLUTE path to output dir**

1. Activate the build environment:

    ```bash
    $ source ./scripts/activate.sh
    ```

1. In the example dir run:

    ```bash
    $ west build
    ```

1. Flash binary:

    ```
    $ west flash --erase
    ```

## Usage

### UART

To get output from device, connect UART to following pins:

| Name | Pin                           |
| :--: | :---------------------------- |
|  RX  | PB3 (pin 17 of J34 connector) |
|  TX  | PB2 (pin 16 of J34 connector) |
| GND  | GND                           |

### Buttons

The following buttons are available on **tlsr9518adk80d** board:

| Name     | Function               | Description                                                                                            |
| :------- | :--------------------- | :----------------------------------------------------------------------------------------------------- |
| Button 1 | Factory reset          | Perform factory reset to forget currently commissioned Thread network and back to uncommissioned state |
| Button 2 | Light Switch control   | Manually triggers the light switch state                                                               |
| Button 3 | Thread start           | Commission thread with static credentials and enables the Thread on device                             |
| Button 4 | Open commission window | The button is opening commissioning window to perform commissioning over BLE                           |

### LEDs

**Red** LED indicates current state of Thread network. It ables to be in
following states:

| State                       | Description                                                                  |
| :-------------------------- | :--------------------------------------------------------------------------- |
| Blinks with short pulses    | Device is not commissioned to Thread, Thread is disabled                     |
| Blinls with frequent pulses | Device is commissioned, Thread enabled. Device trying to JOIN thread network |
| Blinks with whde pulses     | Device commissioned and joined to thread network as CHILD                    |

### CHIP tool commands

1. Build
   [chip-tool cli](https://github.com/project-chip/connectedhomeip/blob/master/examples/chip-tool/README.md)
2. Pair with device

    ```
    ${CHIP_TOOL_DIR}/chip-tool pairing code ${NODE_ID_TO_ASSIGN} MT:D8XA0CQM00KA0648G00
    ```

    here:

    - \${NODE_ID_TO_ASSIGN} is the node id to assign to the light switch

### Binding cluster and endpoints

Binding links clusters and endpoints on both devices, which enables them to
communicate with each other.

To perform binding, you need a controller that can write the binding table to
the light switch device and write proper ACL to the endpoint light bulb on the
Lighting Example application. For example, you can use the CHIP Tool as the
controller. The ACL should contain information about all clusters that can be
called by the light switch application. See the section about interacting with
ZCL clusters in the CHIP Tool's user guide for more information about ACLs.

You can perform the binding process to a single remote endpoint (unicast
binding) or to a group of remote endpoints (group multicast).

> **Note:** To use a light switch without brightness dimmer, apply only the
> first binding command with cluster no. 6.

#### Unicast binding to a remote endpoint using the CHIP Tool

In this scenario, commands are provided for a light switch device with the
`nodeId = <light-switch-node-id>` and a light bulb device with
`nodeId = <lighting-node-id>`, both commissioned to the same Matter network.

To perform the unicast binding process, complete the following steps:

1.  Add an ACL to the development kit that is programmed with the Lighting
    Application Example by running the following command:

    ```bash
    $ ./chip-tool accesscontrol write acl '[{"fabricIndex": 1, "privilege": 5, "authMode": 2, "subjects": [112233], "targets": null}, {"fabricIndex": 1, "privilege": 3, "authMode": 2, "subjects": [<light-switch-node-id>], "targets": [{"cluster": 6, "endpoint": 1, "deviceType": null}, {"cluster": 8, "endpoint": 1, "deviceType": null}]}]' <lighting-node-id> 0
    ```

    In this command:

    -   `{"fabricIndex": 1, "privilege": 5, "authMode": 2, "subjects": [112233], "targets": null}`
        is an ACL for the communication with the CHIP Tool.
    -   `{"fabricIndex": 1, "privilege": 5, "authMode": 2, "subjects": [<light-switch-node-id>], "targets": [{"cluster": 6, "endpoint": 1, "deviceType": null}, {"cluster": 8, "endpoint": 1, "deviceType": null}]}`
        is an ACL for binding (cluster no. 6 is the On/Off cluster and the
        cluster no. 8 is the Level Control cluster).

    This command adds permissions on the lighting application device that allows
    it to receive commands from the light switch device.

2.  Add a binding table to the Light Switch binding cluster:

    ```bash
    $ ./chip-tool binding write binding '[{"fabricIndex": 1, "node": <lighting-node-id>, "endpoint": 1, "cluster": 6}, {"fabricIndex": 1, "node": <lighting-node-id>, "endpoint": 1, "cluster": 8}]' <light-switch-node-id> 1
    ```

    In this command:

    -   `{"fabricIndex": 1, "node": <lighting-node-id>, "endpoint": 1, "cluster": 6}`
        is a binding for the On/Off cluster.
    -   `{"fabricIndex": 1, "node": <lighting-node-id>, "endpoint": 1, "cluster": 8}`
        is a binding for the Level Control cluster.

#### Group multicast binding to the group of remote endpoints using the CHIP Tool

The group multicast binding lets you control more than one lighting device at a
time using a single light switch.

The group multicast binding targets all development kits that are programmed
with the Lighting Application Example and added to the same multicast group.
After the binding is established, the light switch device can send multicast
requests, and all of the devices in the bound groups can run the received
command.

In this scenario, commands are provided for a light switch device with the
`nodeId = <light-switch-node-id>` and a light bulb device with
`nodeId = <lighting-node-id>`, both commissioned to the same Matter network.

To perform the unicast binding process, complete the following steps:

1.  Add an ACL to the lighting endpoint permissions by running the following
    command:

    ```bash
    $ ./chip-tool accesscontrol write acl '[{"fabricIndex": 1, "privilege": 5, "authMode": 2, "subjects": [112233], "targets": null}, {"fabricIndex": 1, "privilege": 3, "authMode": 2, "subjects": [<light-switch-node-id>], "targets": [{"cluster": 6, "endpoint": 1, "deviceType": null}, {"cluster": 8, "endpoint": 1, "deviceType": null}]}]' <lighting-node-id> 0
    ```

    In this command:

    -   `{"fabricIndex": 1, "privilege": 5, "authMode": 2, "subjects": [112233], "targets": null}`
        is an ACL for the communication with the CHIP Tool.
    -   `{"fabricIndex": 1, "privilege": 5, "authMode": 2, "subjects": [<light-switch-node-id>], "targets": [{"cluster": 6, "endpoint": 1, "deviceType": null}, {"cluster": 8, "endpoint": 1, "deviceType": null}]}`
        is an ACL for binding (cluster `no. 6` is the On/Off cluster and the
        cluster `no. 8` is the Level Control cluster).

    This allows the lighting application device to receive commands from the
    light switch device.

2.  Add the light switch device to the multicast group by running the following
    command:

    ```bash
    $ ./chip-tool tests TestGroupDemoConfig --nodeId <light-switch-node-id>
    ```

3.  Add all light bulbs to the same multicast group by applying command below
    for each of the light bulbs, using the appropriate `<lighting-node-id>` (the
    user-defined ID of the node being commissioned except
    `<light-switch-node-id>` due to use this `<light-switch-node-id>` for
    light-switch) for each of them:

    ```bash
    $ ./chip-tool tests TestGroupDemoConfig --nodeId <lighting-node-id>
    ```

4.  Add Binding commands for group multicast:

    ```bash
    $ ./chip-tool binding write binding '[{"fabricIndex": 1, "group": 257}]' <light-switch-node-id> 1
    ```

### Testing the communication

To test the communication between the light switch device and the bound devices,
use [light switch buttons](#buttons).

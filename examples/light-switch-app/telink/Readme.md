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
    ${CHIP_TOOL_DIR}/chip-tool pairing qrcode ${NODE_ID_TO_ASSIGN} MT:D8XA0CQM00KA0648G00
    ```

    here:

    - \${NODE_ID_TO_ASSIGN} is the node id to assign to the light switch

3. Add an ACL to the development kit that is programmed with the Lighting
   Application Example by running the following command:

    ```
    ${CHIP_TOOL_DIR}/chip-tool accesscontrol write acl '[{"fabricIndex": 1, "privilege": 5, "authMode": 2, "subjects": [112233], "targets": null}, {"fabricIndex": 1, "privilege": 3, "authMode": 2, "subjects": [<light-switch-node-id>], "targets": [{"cluster": 6, "endpoint": 1, "deviceType": null}, {"cluster": 8, "endpoint": 1, "deviceType": null}]}]' <lighting-node-id> 0
    ```

    In this command:

    - `{"fabricIndex": 1, "privilege": 5, "authMode": 2, "subjects": [112233], "targets": null}`
      is an ACL for the communication with the CHIP Tool.
    - `{"fabricIndex": 1, "privilege": 5, "authMode": 2, "subjects": [<light-switch-node-id>], "targets": [{"cluster": 6, "endpoint": 1, "deviceType": null}, {"cluster": 8, "endpoint": 1, "deviceType": null}]}`
      is an ACL for binding (cluster no. 6 is the On/Off cluster and the cluster
      no. 8 is the Level Control cluster).

    This command adds permissions on the lighting application device that allows
    it to receive commands from the light switch device.

4. Add a binding table to the Light Switch binding cluster:

    ```
    ${CHIP_TOOL_DIR}/chip-tool binding write binding '[{"fabricIndex": 1, "node": <lighting-node-id>, "endpoint": 1, "cluster": 6}, {"fabricIndex": 1, "node": <lighting-node-id>, "endpoint": 1, "cluster": 8}]' <light-switch-node-id> 1
    ```

    In this command:

    - `{"fabricIndex": 1, "node": <lighting-node-id>, "endpoint": 1, "cluster": 6}`
      is a binding for the On/Off cluster.
    - `{"fabricIndex": 1, "node": <lighting-node-id>, "endpoint": 1, "cluster": 8}`
      is a binding for the Level Control cluster.

### Testing the communication

To test the communication between the light switch device and the bound devices,
use [light switch buttons](#buttons).

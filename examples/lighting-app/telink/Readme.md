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
| Button 2 | Lighting control       | Manually triggers the lighting state                                                                   |
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

**Blue** LED shows current state of lightbulb

### CHIP tool commands

1. Build
   [chip-tool cli](https://github.com/project-chip/connectedhomeip/blob/master/examples/chip-tool/README.md)
1. Pair with device

    ```
    ${CHIP_TOOL_DIR}/chip-tool pairing code ${NODE_ID_TO_ASSIGN} MT:D8XA0CQM00KA0648G00
    ```

    here:

    - \${NODE_ID_TO_ASSIGN} is the node id to assign to the lightbulb

1. Switch on the light:

    ```
    ${CHIP_TOOL_DIR}/chip-tool onoff on 1
    ```

    here:

    - **onoff** is name of cluster
    - **on** command to the cluster
    - **1** ID of endpoint

1. Switch off the light:

    ```
    ${CHIP_TOOL_DIR}/chip-tool onoff off 1
    ```

    here:

    - **onoff** is name of cluster
    - **off** command to the cluster
    - **1** ID of endpoint

1. Read the light state:

    ```
    ${CHIP_TOOL_DIR}/chip-tool onoff read on-off 1
    ```

    here:

    - **onoff** is name of cluster
    - **read** command to the cluster
    - **on-off** attribute to read
    - **1** ID of endpoint

1. Change brightness of light:

    ```
    ${CHIP_TOOL_DIR}/chip-tool levelcontrol move-to-level 32 0 0 0 1
    ```

    here:

    - **levelcontrol** is name of cluster
    - **move-to-level** command to the cluster
    - **32** brightness value
    - **0** transition time
    - **0** option mask
    - **0** option override
    - **1** ID of endpoint

1. Reag brightness level:
    ```
    ./chip-tool levelcontrol read current-level 1
    ```
    here:
    - **levelcontrol** is name of cluster
    - **read** command to the cluster
    - **current-level** attribute to read
    - **1** ID of endpoint

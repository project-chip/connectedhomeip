# Matter CYW30739 Light Switch Example

An example showing the use of Matter on the Infineon CYW30739 platform.

---

## Table of Contents

-   [Matter CYW30739 Light Switch Example](#matter-cyw30739-light-switch-example)
    -   [Table of Contents](#table-of-contents)
    -   [Introduction](#introduction)
    -   [Building](#building)
    -   [Building Options](#building-options)
        -   [DAC / DAC Key / PAI Certificate / Certificate Declaration](#dac--dac-key--pai-certificate--certificate-declaration)
    -   [Flashing the Application](#flashing-the-application)
        -   [Enter Recovery Mode](#enter-recovery-mode)
        -   [Run Flash Script](#run-flash-script)
    -   [Running the Complete Example](#running-the-complete-example)

## Introduction

The CYW30739 light switch example provides a baseline demonstration of a on-off
light switch device, built using Matter and the Infineon Modustoolbox SDK. It
can be controlled by a Matter controller over Openthread network.

The CYW30739 device can be commissioned over Bluetooth Low Energy where the
device and the Matter controller will exchange security information with the
Rendez-vous procedure. Target Thread Network information including the active
dataset and CASE credentials are then provided.

## Building

-   Build the example application:

    ```bash
    $ cd ~/connectedhomeip
    $ git submodule update --init
    $ ./scripts/examples/gn_build_example.sh examples/light-switch-app/infineon/cyw30739 out/light-switch-app
    ```

-   To delete generated executable, libraries and object files use:

    ```bash
    $ cd ~/connectedhomeip
    $ rm -rf ./out/
    ```

-   OR use GN/Ninja directly

    ```bash
    $ cd ~/connectedhomeip/examples/light-switch-app/infineon/cyw30739
    $ git submodule update --init
    $ source third_party/connectedhomeip/scripts/activate.sh
    $ gn gen out/debug
    $ ninja -C out/debug
    ```

-   To delete generated executable, libraries and object files use:

    ```bash
    $ cd ~/connectedhomeip/examples/light-switch-app/infineon/cyw30739
    $ rm -rf out/
    ```

## Building Options

### DAC / DAC Key / PAI Certificate / Certificate Declaration

Infineon CYW30739 examples use test certifications, keys, and CD by default. For
a production build, manufacturers can provision certifications, keys, and CD by
the following arguments:

-   `matter_dac`, `matter_dac_key`, `matter_pai`, `matter_cd`

    ```bash
    $ ./scripts/examples/gn_build_example.sh examples/light-switch-app/infineon/cyw30739 out/light-switch-app \
    'matter_dac="/path/to/dac.der"' \
    'matter_dac_key="/path/to/dac_key.der"' \
    'matter_pai="/path/to/pai.der"' \
    'matter_cd="/path/to/cd.der"'
    ```

## Flashing the Application

### Enter Recovery Mode

Put the CYW30739 in to the recovery mode before running the flash script.

1. Press and hold the `RECOVERY` button on the board.
2. Press and hold the `RESET` button on the board.
3. Release the `RESET` button.
4. After one second, release the `RECOVERY` button.

### Run Flash Script

-   On the command line:

    ```bash
    $ cd ~/connectedhomeip/examples/light-switch-app/infineon/cyw30739
    $ python3 out/debug/chip-cyw30739-light-switch-example.flash.py
    ```

## Running the Complete Example

-   It is assumed here that you already have an OpenThread border router
    configured and running. If not see the following guide
    [Openthread_border_router](https://github.com/project-chip/connectedhomeip/blob/master/docs/guides/openthread_border_router_pi.md)
    for more information on how to setup a border router on a raspberryPi.

-   For this example to work, it is necessary to have a second CYW30739 device
    running the lighting app example commissioned on the same OpenThread network

-   If the CYW30739 device is running the light switch example, then

    **Push USER Button** - Sends a Toggle command to bound light app.

    **LED1** - Indicates the current button state.

    **_OnOff Cluster_** - As following commands are app shell commands.

        -  'switch local on'            : Lights On LED1 of light-switch device
        -  'switch local off'           : Lights Off LED1 of light-switch device
        -  'switch local toggle'        : Makes Toggle LED1 of light-switch device

        -  'switch onoff on'            : Sends unicast On command to bound device
        -  'switch onoff off'           : Sends unicast Off command to bound device
        -  'switch onoff toggle'        : Sends unicast Toggle command to bound device

        -  'switch groups onoff on'     : Sends On group command to bound group
        -  'switch groups onoff off'    : Sends On group command to bound group
        -  'switch groups onoff toggle' : Sends On group command to bound group

-   Here is an example with the CHIPTool for unicast commands only:

    ```bash
    chip-tool pairing ble-thread 1 hex:<operationalDataset> 20202021 3840

    chip-tool accesscontrol write acl '[{"fabricIndex": 1, "privilege": 5, "authMode": 2, "subjects": [<chip-tool-node-id>], "targets": null}, {"fabricIndex": 1, "privilege": 3, "authMode": 2, "subjects": [<light-switch-node-id>], "targets": [{"cluster": 6, "endpoint": 1, "deviceType": null}]}]' <lighting-node-id> 0

    chip-tool binding write binding '[{"fabricIndex": 1, "node": <lighting-node-id>, "endpoint": 1, "cluster": 6}]' <light-switch-node-id> 1
    ```

    Example: After pairing successfully [lighting-node-id : 1,
    light-switch-node-id : 2]

    ```bash
    chip-tool accesscontrol write acl '[{"fabricIndex": 1, "privilege": 5, "authMode": 2, "subjects": [112233], "targets": null}, {"fabricIndex": 1,
    "privilege": 3, "authMode": 2, "subjects": [2], "targets": [{"cluster": 6, "endpoint": 1, "deviceType": null}]}]' 1 0

    chip-tool binding write binding '[{"fabricIndex": 1, "node": 1, "endpoint": 1, "cluster": 6}]' 2 1
    ```

-   Here is an example with the CHIPTool for groups commands only:

    Pairing the device

    ```bash
    chip-tool pairing ble-thread 1 hex:<operationalDataset> 20202021 3840
    ```

    You can use a series of commands after pairing successfully. Here is a
    lighting device (node 1) and a light-switch device (node 2) for connection
    demonstration.

    ```bash
    chip-tool groupkeymanagement key-set-write '{"groupKeySetID": 417, "groupKeySecurityPolicy": 0, "epochKey0":"a0a1a2a3a4a5a6a7a8a9aaabacadaeaf", "epochStartTime0": 1110000,"epochKey1":"b0b1b2b3b4b5b6b7b8b9babbbcbdbebf", "epochStartTime1":1110001,"epochKey2":"c0c1c2c3c4c5c6c7c8c9cacbcccdcecf", "epochStartTime2": 1110002 }' 1 0

    chip-tool groupkeymanagement write group-key-map '[{"groupId": 257, "groupKeySetID": 417, "fabricIndex": 1}]' 1 0

    chip-tool groups add-group 257 demo 1 1

    chip-tool accesscontrol write acl '[{"fabricIndex": 1, "privilege": 5, "authMode": 2, "subjects": null, "targets": null}, {"fabricIndex": 1, "privilege": 3, "authMode": 3, "subjects": [257], "targets": null}]' 1 0

    chip-tool groupkeymanagement key-set-write '{"groupKeySetID": 417, "groupKeySecurityPolicy": 0, "epochKey0":"a0a1a2a3a4a5a6a7a8a9aaabacadaeaf", "epochStartTime0": 1110000,"epochKey1":"b0b1b2b3b4b5b6b7b8b9babbbcbdbebf", "epochStartTime1":1110001,"epochKey2":"c0c1c2c3c4c5c6c7c8c9cacbcccdcecf", "epochStartTime2": 1110002 }' 2 0

    chip-tool groupkeymanagement write group-key-map '[{"groupId": 257, "groupKeySetID": 417, "fabricIndex": 1}]' 2 0

    chip-tool groups add-group 257 demo 2 1

    chip-tool binding write binding '[{"fabricIndex": 1, "group": 257}]' 2 1
    ```

    Or you can use TestGroupDemoConfig after pairing successfully

    ```bash
    chip-tool tests TestGroupDemoConfig --nodeId <light-switch-node-id>

    chip-tool tests TestGroupDemoConfig --nodeId <lighting-node-id>

    chip-tool binding write binding '[{"fabricIndex": 1, "group": 257}]' <light-switch-node-id> 1
    ```

    Example: After pairing successfully [lighting-node-id : 1,
    light-switch-node-id : 2]

    ```bash
    chip-tool tests TestGroupDemoConfig --nodeId 2

    chip-tool tests TestGroupDemoConfig --nodeId 1

    chip-tool binding write binding '[{"fabricIndex": 1, "group": 257}]' 2 1
    ```

    To run the example with unicast and groups commands, run the group
    configuration commands and replace the last one with binding this command

    ```bash
    chip-tool binding write binding '[{"fabricIndex": 1, "group": 257},{"fabricIndex": 1, "node": <lighting-node-id>, "endpoint": 1, "cluster":6} ]' <light-switch-node-id> 1
    ```

    To acquire the chip-tool node id, read the acl table right after
    commissioning

    ```bash
    chip-tool accesscontrol read acl <nodeid> 0
    ```

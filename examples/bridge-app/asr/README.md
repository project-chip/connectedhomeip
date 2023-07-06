# Matter ASR Bridge Example

This example demonstrates the Matter Bridge application on ASR platform.

---

-   [Matter ASR Bridge Example](#matter-asr-bridge-example)
    -   [Introduction](#introduction)
    -   [Building and Commissioning](#building-and-commissioning)
    -   [Testing the example](#testing-the-example)

---

## Introduction

A prototype application that demonstrates dynamic endpoint with device
commissioning and cluster control. It adds the non-chip device as endpoints on a
bridge(Matter device). In this example four light devices supporting on-off
cluster have been added as endpoints

1. Light1 at endpoint 3
2. Light2 at endpoint 4
3. Light3 at endpoint 5
4. Light4 at endpoint 6

## Building and Commissioning

Please refer
[Building and Commissioning](../../../docs/guides/asr_getting_started_guide.md#building-the-example-application)
guides to get started

```
./scripts/build/build_examples.py --target asr-$ASR_BOARD-bridge build
```

## Testing the example

-   An additional light-switch device is required to complete this example.
-   Commission bridge device with node-id `1`
-   Commission light-switch device with node-id `2`
-   After bridge device and light-switch device successful commissioning, use
    the GUI tool `DOGO` to input AT command `subdevice sync` for the bridge
    device, and then use chip-tool to write ACL to the bridge device.
    ```
    ./chip-tool accesscontrol write acl '[{"fabricIndex": 1, "privilege": 5, "authMode": 2, "subjects": [112233], "targets": null },{"fabricIndex": 1, "privilege": 3, "authMode": 2, "subjects": [2], "targets": null }]' 1 0
    ```
-   After successful commissioning, use the `chip-tool` for binding
    light-switch's endpoint 1 with bridge device's endpoint 3 respectively.
    ```
    ./chip-tool binding write binding '[{"fabricIndex": 1, "node":1, "endpoint":3, "cluster":6}]' 2 1
    ```
-   Light switch button

    This demo uses button to test changing the `Light1`, and the bridge device
    will output log information:

    |  Name  |  Pin  |
    | :----: | :---: |
    | BUTTON | PAD12 |

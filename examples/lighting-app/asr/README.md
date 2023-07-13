# Matter ASR Lighting Example

This example demonstrates the Matter Lighting application on ASR platform.

---

-   [Matter ASR Lighting Example](#matter-asr-lighting-example)
    -   [Building and Commissioning](#building-and-commissioning)
    -   [Cluster Control](#cluster-control)
    -   [Indicate current state of lightbulb](#indicate-current-state-of-lightbulb)

---

## Building and Commissioning

Please refer
[Building and Commissioning](../../../docs/guides/asr_getting_started_guide.md#building-the-example-application)
guides to get started

```
./scripts/build/build_examples.py --target asr-$ASR_BOARD-lighting build
```

## Cluster Control

After successful commissioning, use `chip-tool` to control the board

-   OnOff Cluster
    ```
    ./chip-tool onoff on <NODE ID> 1
    ./chip-tool onoff off <NODE ID> 1
    ./chip-tool onoff toggle <NODE ID> 1
    ```
-   LevelControl Cluster

    ```
    ./chip-tool levelcontrol move-to-level 128 10 0 0 <NODE ID> 1
    ```

-   ColorControl Cluster
    ```
    ./chip-tool colorcontrol move-to-hue-and-saturation 240 100 0 0 0 <NODE ID> 1
    ```

## Indicate current state of lightbulb

By default, a 3-color LED(RGB) module is used to show current state of lightbulb
Connect the module to following pins:

| Name  |  Pin  |
| :---: | :---: |
|  Red  | PAD7  |
| Green | PAD6  |
| Blue  | PAD10 |

These default pins are defined at `examples/platform/asr/LEDWidget.h`

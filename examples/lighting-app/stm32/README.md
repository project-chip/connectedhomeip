# Matter STM32 Lighting Example over thread

This example demonstrates the Matter Lighting application on stm32 platform.

---

-   [Matter STM32 Lighting Example over thread](#matter-stm32-lighting-example-over-thread)
    -   [Building and Commissioning](#building-and-commissioning)
    -   [Cluster Control](#cluster-control)
    -   [Indicate current state of lightbulb](#indicate-current-state-of-lightbulb)

---

## Building and Commissioning

Please refer
[Building and Commissioning](../../../docs/guides/stm32_getting_started_guide.md#building-the-example-application)
guides to get started

```
./scripts/build/build_examples.py --target stm32-$stm32_BOARD-lighting build
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

The LCD screen displays "BLE connected" when the BLE rendezvous started. The LCD
screen displays "Network Join" when the board joins thread network. The LCD
screen displays "LED ON" + the light level when the controller enable the light
ON and the line is erased when the controller disable the light.

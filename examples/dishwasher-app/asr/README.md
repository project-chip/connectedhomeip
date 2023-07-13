# CHIP ASR Lighting Example

This example demonstrates the Matter Lighting application on ASR platform.

---

-   [CHIP ASR Lighting Example](#chip-asr-lighting-example)
    -   [Supported Chips](#supported-chips)
    -   [Building the Example Application](#building-the-example-application)
    -   [Commissioning](#commissioning)
        -   [BLE mode](#ble-mode)
        -   [IP mode](#ip-mode)
    -   [Cluster Control](#cluster-control)

---

## Supported Chips

The CHIP demo application is supported on:

-   ASR582X
-   ASR595X

## Building the Example Application

-   [Setup CHIP Environment](../../../docs/guides/BUILDING.md)

-   Setup toolchain for ASR582X,download gcc-arm-none-eabi-9-2019-q4-major,then
    export `ASR_TOOLCHAIN_PATH`:
    ```
    export ASR_TOOLCHAIN_PATH={path-to-toolchain}/gcc-arm-none-eabi-9-2019-q4-major/bin/
    ```
    for ASR595X,download asr_riscv_gnu_toolchain_10.2_ubuntu,then export
    `ASR_TOOLCHAIN_PATH`:
    ```
    export ASR_TOOLCHAIN_PATH={path-to-toolchain}/asr_riscv_gnu_toolchain_10.2_ubuntu-16.04/bin/
    ```
-   Setup Chip environment
    -   for ASR582X:
        ```
        export ASR_BOARD=asr582x
        ```
    -   for ASR595X:
        ```
        export ASR_BOARD=asr595x
        ```
-   Building the Application
    ```
    ./scripts/build/build_examples.py --target asr-$ASR_BOARD-lighting build
    ```
-   The output image files are stored in the subdirectories under `out`, the
    subdirectory name is the same as the argument specified after the option
    `--target` when build the examples.

-   After building the application, `DOGO` tool is used to flash it to the
    board.

## Commissioning

There are two commissioning modes supported by ASR platform:

### BLE mode

1. Build and Flash
2. The example will run automatically after booting the ASR board.
3. Restore factory settings using command `recovery`
4. Commissioning with
   [Chip-Tool](https://github.com/project-chip/connectedhomeip/tree/master/examples/chip-tool)

### IP mode

1. Build and Flash
2. The example will run automatically after booting the ASR board.
3. Restore factory settings using command `recovery`
4. Connect to AP using command `wifi_open sta [ssid] [password]`
5. Commissioning with
   [Chip-Tool](https://github.com/project-chip/connectedhomeip/tree/master/examples/chip-tool)

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

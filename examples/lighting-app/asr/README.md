# CHIP ASR Lighting Example

This example demonstrates the Matter Lighting application on ASR platform.

---

- [CHIP ASR Lighting Example](#chip-asr-lighting-example)
  - [Supported Chips](#supported-chips)
  - [Building the Example Application](#building-the-example-application)
  - [Commissioning](#commissioning)
    - [BLE mode](#ble-mode)
    - [IP mode](#ip-mode)

---

## Supported Chips

The CHIP demo application is supported on:
-   ASR582X
-   ASR595X

## Building the Example Application

-   [Setup CHIP Environment](../../../docs/guides/BUILDING.md)

-   Setup ASR toolchian
    for ASR582X,download gcc-arm-none-eabi-9-2019-q4-major,then export `TOOLCHAIN_PATH`:
    ```
    export TOOLCHAIN_PATH={toolchain path}/gcc-arm-none-eabi-9-2019-q4-major/bin/
    ```
    for ASR595X,download asr_riscv_gnu_toolchain_10.2_ubuntu,then export `TOOLCHAIN_PATH`:
    ```
    export TOOLCHAIN_PATH={toolchain path}/compiler/asr_riscv_gnu_toolchain_10.2_ubuntu-16.04/bin/
    ```
-   Setup ASR IC
    for ASR582X:
    ```
    export ASR_IC=asr582x
    ```
    for ASR595X:
    ```
    export ASR_IC=asr595x
    ```
-   To build the demo application:
    ```
    ./scripts/examples/gn_asr_example.sh ./examples/lighting-app/asr out/example_app
    ```
-   The output image files are stored in `out/example_app` folder.

-   After building the application, **DOGO** is used to flash it to the board.


## Commissioning

There are two commissioning modes supported by ASR platform:

### BLE mode

1. Build and Flash
2. The example will run automatically after booting the ASR board.
3. Restore factory settings using command `recovery`
4. Test with
   [Chip-Tool](https://github.com/project-chip/connectedhomeip/tree/master/examples/chip-tool)

### IP mode

1. Build and Flash
2. The example will run automatically after booting the ASR board.
3. Restore factory settings using command `recovery`
4. Connect to AP using command `wifi_open sta [ssid] [password]` 
5. Test with
   [Chip-Tool](https://github.com/project-chip/connectedhomeip/tree/master/examples/chip-tool)


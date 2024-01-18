# ASR Getting Started Guide

The ASR platform is a [Matter](https://github.com/project-chip/connectedhomeip)
platform that uses ASR FreeRTOS SDK.

---

-   [ASR Getting Started Guide](#asr-getting-started-guide)
    -   [Supported Chips](#supported-chips)
    -   [Matter Example Applications](#matter-example-applications)
    -   [Building the Example Application](#building-the-example-application)
    -   [Commissioning](#commissioning)
        -   [BLE mode](#ble-mode)
        -   [IP mode](#ip-mode)
    -   [Shell](#shell)
    -   [OTA](#ota)
    -   [Factory](#factory)

---

## Supported Chips

The ASR platform is supported on:

-   [ASR582X](http://www.asrmicro.com/en/goods/proinfo/36.html)
-   [ASR595X](http://www.asrmicro.com/en/goods/proinfo/42.html)
-   [ASR550X](http://www.asrmicro.com/en/goods/proinfo/14.html)

## Matter Example Applications

Sample Matter applications are provided for the ASR platform. They can be used
to speed up development. You can find them in the samples with `/asr` subfolder.

## Building the Example Application

-   [Setup Matter Environment](./BUILDING.md)

-   Setup toolchain
    -   for ASR582X and ASR550X
        ```
        wget https://developer.arm.com/-/media/Files/downloads/gnu-rm/9-2019q4/RC2.1/gcc-arm-none-eabi-9-2019-q4-major-x86_64-linux.tar.bz2
        tar -jxvf gcc-arm-none-eabi-9-2019-q4-major-x86_64-linux.tar.bz2
        export ASR_TOOLCHAIN_PATH={abs-path-to-toolchain}/gcc-arm-none-eabi-9-2019-q4-major/bin/
        ```
    -   for ASR595X
        ```
        git clone --depth=1 https://github.com/asriot/Tools.git
        cd Tools/toolchain
        cat asr_riscv_gnu_toolchain_10.2_ubuntu-16.04.tar.bz2.part* > asr_riscv_gnu_toolchain_10.2_ubuntu-16.04.tar.bz2
        mkdir -p asr_riscv_gnu_toolchain_10.2_ubuntu-16.04
        tar -jxvf asr_riscv_gnu_toolchain_10.2_ubuntu-16.04.tar.bz2 -C asr_riscv_gnu_toolchain_10.2_ubuntu-16.04/
        export ASR_TOOLCHAIN_PATH={abs-path-to-toolchain}/asr_riscv_gnu_toolchain_10.2_ubuntu-16.04/bin/
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
    -   for ASR550X:
        ```
        export ASR_BOARD=asr550x
        ```
-   Building the Application

    If the lighting example is to be built:

    ```
    ./scripts/build/build_examples.py --target asr-$ASR_BOARD-lighting build
    ```

-   The output image files are stored in the subdirectories under `out`, the
    subdirectory name is the same as the argument specified after the option
    `--target` when build the examples.

-   After building the application, `DOGO` tool is used to flash it to the
    board, please refer to the
    [DOGO Tool User Guide](https://asriot.readthedocs.io/en/latest/ASR550X/Download-Tool/ASR_IoT_DOGO_Tool_User_Guide.html).

## Commissioning

There are two commissioning modes supported by ASR platform:

### BLE mode

1. Build and Flash
2. The example will run automatically after booting the ASR board.
3. Restore factory settings using command `recovery`
4. Commissioning with
   [Chip-Tool](https://github.com/project-chip/connectedhomeip/tree/master/examples/chip-tool),for
   example:
    ```
    ./chip-tool pairing ble-wifi <node_id> <ssid> <password> <pin_code> <discriminator>
    ```
5. Only ASR582X and ASR595X support BLE

### IP mode

1. Build and Flash
2. The example will run automatically after booting the ASR board.
3. Restore factory settings using command `recovery`
4. Connect to AP using command `wifi_open sta [ssid] [password]`
5. Commissioning with
   [Chip-Tool](https://github.com/project-chip/connectedhomeip/tree/master/examples/chip-tool),for
   example:
    ```
    ./chip-tool pairing onnetwork-long <node_id> <pin_code> <discriminator>
    ```

## Shell

1. To build the example with Matter shell, exactly the same steps as above but
   add argument `-shell` when using the `build_examples.py` script. For example:
   `./scripts/build/build_examples.py --target asr-$ASR_BOARD-lighting-shell build`
2. The example will run automatically after booting the ASR board.
3. Use `help` command to list all supported commands:
    ```
    help
    base64          Base64 encode / decode utilities
    exit            Exit the shell application
    help            List out all top level commands
    version         Output the software version
    ble             BLE transport commands
    wifi            Usage: wifi <subcommand>
    config          Manage device configuration. Usage to dump value: config [param_name] and to set some values (discriminator): config [param_name] [param_value].
    device          Device management commands
    onboardingcodes Dump device onboarding codes. Usage: onboardingcodes none|softap|ble|onnetwork [qrcode|qrcodeurl|manualpairingcode]
    dns             Dns client commands
    OnOff           OnOff commands. Usage: OnOff [on|off]
    Done
    ```

## OTA

1. To build the example with the Matter OTA Requestor functionality, exactly the
   same steps as above but add argument `-ota` when using the
   `build_examples.py` script. For example:
   `./scripts/build/build_examples.py --target asr-$ASR_BOARD-lighting-ota build`
2. For more usage details, please refer to the
   [OTA example](../../examples/ota-requestor-app/asr/README.md)

## Factory

1. To build the example with the ASR Factory Data Provider, exactly the same
   steps as above but add argument `-factory` when using the `build_examples.py`
   script. For example:
   `./scripts/build/build_examples.py --target asr-$ASR_BOARD-lighting-factory build`
2. For more usage details, please refer to the
   [Factory Tool User Guide](https://github.com/asriot/Tools/blob/main/factory_tool/README.md)

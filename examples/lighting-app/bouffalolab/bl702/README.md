# CHIP BL702 Lighting App Example

## Supported Hardware

Current supported develop boards:

-   BL702-IoT-DVK
-   BL706-IoT-DVK
-   BL706-NIGHT-LIGHT
    > Note, please make sure both of flash and `PSRAM` shipped with develop
    > board/device are at lease 2MB.

## Build

### Prerequisite

-   Clone connectedhomeip github repo and update all submodule;
-   Install all tools (likely already present for CHIP developers).
-   Setup connectedhomeip environment:
    -   First time, or environment is out of date. Please execute the following
        script under root folder of connectedhomeip repo to install and setup
        environment, which will take some minutes; meanwhile, Bouffalolab flash
        tool, `bflb-iot-tool`, will be installed.
        ```shell
        source scripts/bootstrap.sh
        ```
        > Note, `scripts/bootstrap.sh` only installs `bflb-iot-tool` under
        > connectedhomeip build context.
    -   Others, please execute the following script before any build commands
        executed; and `bflb-iot-tool` will import under this environment.
        ```shell
        source scripts/activate.sh
        ```

## Build CHIP BL702 Lighting App example

-   Build with script `gn_bouffalolab_example.sh` under root folder of
    connectedhomeip repo

    -   Command format:

        ```shell
        ./scripts/examples/gn_bouffalolab_example.sh <Example folder> <Output folder> <Bouffalolab_board_name> [<Build options>]
        ```

        -   Build lighting-app for board BL706-IoT-DVK.

            ```shell
            ./scripts/examples/gn_bouffalolab_example.sh lighting-app out/debug BL706-IoT-DVK
            ```

        -   Build lighting-app for board BL702-IOT-DVK module_type is
            `BL706C-22` by default. Please execute the following command to
            build lighting-app for BL702-IoT-DVK with module `BL702`

        ```shell
        ./scripts/examples/gn_bouffalolab_example.sh lighting-app out/debug BL702-IoT-DVK module_type="BL702"
        ```

        -   Build lighting-app for board BL706-NIGHT-LIGHT

        ```shell
        ./scripts/examples/gn_bouffalolab_example.sh lighting-app out/debug BL706-NIGHT-LIGHT module_type="BL702"
        ```

        > Note, please check which module is used on the board.

    -   With UART shell command support:

        ```shell
        ./scripts/examples/gn_bouffalolab_example.sh lighting-app out/debug BL706-IoT-DVK chip_build_libshell=true
        ```

    -   With pigweed RPC support:
        ```shell
        ./scripts/examples/gn_bouffalolab_example.sh lighting-app out/debug BL706-IoT-DVK 'import("//with_pw_rpc.gni")'
        ```
        > Note, UART shell command and pigweed RPC can not build together.

-   Build with `build_examples.py`

    -   Build for BL702-IoT-DVK, BL706-IoT-DVK and BL706-NIGHT-LIGHT as
        following commands.

        ```shell
        ./scripts/build/build_examples.py --target bouffalolab-BL702-IoT-DVK-light build
        ./scripts/build/build_examples.py --target bouffalolab-BL706-IoT-DVK-BL706C-22-light build
        ./scripts/build/build_examples.py --target bouffalolab-BL706-NIGHT-LIGHT-light build
        ```

    -   Build with pigweed RPC support as following commands.
        ```shell
        ./scripts/build/build_examples.py --target bouffalolab-BL702-IoT-DVK-light-rpc build
        ./scripts/build/build_examples.py --target bouffalolab-BL706-IoT-DVK-BL706C-22-light-rpc build
        ```

-   Download image

    -   Using script `chip-bl702-lighting-example.flash.py`.

        After building gets done, a python
        `chip-bl702-lighting-example.flash.py` will generated under build output
        folder. Such as chip-bl702-lighting-example.flash.py for lighting-app
        example. Please check `help` option of script for more detail.

        -   Hold BOOT pin and reset chip, let it be in download mode.
        -   Download image as following execution under build output folder:

            ```shell
            ./chip-bl702-lighting-example.flash.py --port /dev/tty.usbserial-3
            ```

            > Note, where `/dev/tty.usbserial-3` is UART port of device.

        -   Build Bouffalolab OTA image as following execution under build
            output folder:
            ```shell
            ./chip-bl702-lighting-example.flash.py --build
            ```
            After script executed, a folder `ota_images` and image
            `FW_OTA.bin.xz.hash` will generated. `FW_OTA.bin.xz.hash` is
            compressed with hash verification with
            `chip-bl702-lighting-example.bin`.

        > Note, `chip-bl702-lighting-example.flash.py` uses Python module
        > `bflb-iot-tool` to flash device. Please make sure current terminal is
        > under matter build environment, otherwise, Python module
        > `bflb-iot-tool` should be installed with default Python.

    -   Using `Bouffalo Lab` flash tool`BLDevCube`
        -   Hold BOOT pin and reset chip, let it be in download mode.
        -   Select `DTS` file
            `<connectedhomeip_repo_path>/examples/platform/bouffalolab/bl702/flash_config/bl_factory_params_IoTKitA_32M.dts`;
        -   Select Partition Table
            `<connectedhomeip_repo_path>/examples/platform/bouffalolab/bl702/flash_config/partition_cfg_2M.toml`;
        -   Select Firmware Bin chip-bl702-lighting-example.bin;
        -   Select Chip Erase if need;
        -   Choose Target COM port.
        -   Then click Create & Download.
            > Where `connectedhomeip_repo_path` is the root path of repo
            > connectedhomeip.

-   Firmware Behavior

    -   IOT Dev board Status LED: TX0<br> Lighting LED: RX1<br> Factory Reset:
        Short `IO11` to `GND` over 3 seconds<br>

    -   Night Light Unprovisioned state: light shows yellow. Provisioned state:
        light show white. Factory Reset: Power cycle 3 times before light is on;
        at 3rd time, light shows green and does factory reset after 3 seconds
        later. And factory reset can be cancelled during 3 seconds wait time.

-   UART baudrate for log and shell command
    -   By default, UART baudrate is 2000000
    -   To change UART baudrate, please run script `gn_bouffalolab_example.sh`
        with `baudrate=[uart baudrate]` followed, such as
        `./scripts/examples/gn_bouffalolab_example.sh lighting-app out/debug BL706-NIGHT-LIGHT module_type="BL702" baudrate=115200`

## Test with chip-tool

### Setup Raspberry PI OTBR as [guide](../../../../docs/guides/openthread_border_router_pi.md)

### Build and use chip-tool as [chip_tool_guide](../../../../docs/guides/chip_tool_guide.md) and [guide](../../../chip-tool/README.md)

### Commissioning

-   Power on BL702 with chip lighting app
-   BLE commissioning with chip-tool
    ```shell
    ./chip-tool pairing ble-thread <node_id> hex:<thread_operational_dataset> 20202021 3840
    ```
    `node_id` is matter node id, such as 10; `<thread_operational_dataset>` is
    Border Router Dataset, which to get with command
    `sudo ot-ctl dataset active -x` on Rasp PI border router.

### Toggle Light

-   After BLE commissioning gets successfully,
    ```
    $ ./chip-tool onoff toggle <node_id> 1
    ```

### Identify Light

-   After BLE commissioning gets successfully,
    ```shell
    ./chip-tool identify identify <identify_duration> <node_id> 1
    ```

which `<identify_duration>` is how many seconds to execute identify command.

## OTA software upgrade with ota-provider-app

### Build ota-provider-app as [guide](../../../ota-provider-app/linux/README.md)

### Create the Matter OTA with Bouffalolab OTA bin `FW_OTA.bin.xz.hash`

-   Under connectedhomeip repo path

    ```shell
    $ ./src/app/ota_image_tool.py create -v 0xFFF1 -p 0x8005 -vn 1 -vs "1.0" -da sha256 <FW_OTA.bin.xz.hash> lighting-app.ota

    ```

-   lighting-app.ota should have greater software version which is defined by
    macro CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION in CHIPProjectConfig.h

### Start ota-provider-app

-   Start ota-provider-app for lighting-app.ota
    ```shell
    $ rm -r /tmp/chip_*
    $ ./chip-ota-provider-app -f <path_to_ota_bin>/lighting-app.ota
    ```
    where `<path_to_ota_bin>` is the folder for lighting-app.ota.
-   Provision ota-provider-app with assigned node id to 1
    ```shell
    $ ./chip-tool pairing onnetwork 1 20202021
    $ ./chip-tool accesscontrol write acl '[{"fabricIndex": 1, "privilege": 5, "authMode": 2, "subjects": [112233], "targets": null}, {"fabricIndex": 1, "privilege": 3, "authMode": 2, "subjects": null, "targets": null}]' 1 0
    ```

### Start ota software upgrade

-   BLE commission BL702 lighting if not commissioned.
-   Start OTA software upgrade process
    ```shell
    ./chip-tool otasoftwareupdaterequestor announce-ota-provider 1 0 0 0 <node_id_to_lighting_app> 0
    ```
    where `<node_id_to_lighting_app>` is node id of BL702 lighting app.
-   After OTA software upgrade gets done, BL702 will get reboot automatically.

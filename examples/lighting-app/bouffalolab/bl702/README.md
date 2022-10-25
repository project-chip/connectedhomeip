# CHIP BL70X Lighting App Example

BL70X is highly integrated BLE and IEEE 802.15.4 combo chip for IoT
applications, and BL702 is a general name for BL70X family.

This example is powered by BL706 and functions as a Thread light bulb device
type, with on/off, level and color capabilities. The steps were verified with
following boards:

-   `XT-ZB6-DevKit`
-   `BL706-IoT-DVK`
-   `BL706-NIGHT-LIGHT`

## Prerequisite

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
        > Note, `bflb-iot-tool` is only installed under connectedhomeip build
        > context by matter environment setup `scripts/bootstrap.sh`.
    -   Others, please execute the following script before any build commands
        executed; and `bflb-iot-tool` imports under this environment.
        ```shell
        source scripts/activate.sh
        ```

## Build CHIP BL702 Lighting App example

-   Build for `XT-ZB6-DevKit` and BL706-NIGHT-LIGHT as following commands.

    ```shell
    ./scripts/build/build_examples.py --target bouffalolab-xt-zb6-devkit-light build
    ./scripts/build/build_examples.py --target bouffalolab-bl706-night-light-light build
    ```

-   Build target name with `-115200` appended for UART baudrate 115200 command
    enabled as following commands.

    ```shell
    ./scripts/build/build_examples.py --target bouffalolab-xt-zb6-devkit-light-rpc build
    ```

    > UART baudrate is 2000000 by default.

-   Build target name with `-shell` appended for UART shell command enabled as
    following commands.

    ```shell
    ./scripts/build/build_examples.py --target bouffalolab-xt-zb6-devkit-light-shell build
    ```

-   Build target name with `-rpc` appended for rpc enabled as following
    commands.
    `shell ./scripts/build/build_examples.py --target bouffalolab-xt-zb6-devkit-light-rpc build`
    > For multiple build options, such as UART baudrate 115200 + rpc, please try
    > build command as
    > `./scripts/build/build_examples.py --target bouffalolab-xt-zb6-devkit-light-rpc-115200 build`

## Download image

-   Using script `chip-bl702-lighting-example.flash.py`.

    After building gets done, a python `chip-bl702-lighting-example.flash.py`
    will generated under build output folder. Such as
    chip-bl702-lighting-example.flash.py for lighting-app example. Please check
    `--help` option of script for more detail.

    -   Hold BOOT pin and reset chip, let it be in download mode.
    -   Enter build out folder, download image as following execution under
        build output folder:

        ```shell
        ./chip-bl702-lighting-example.flash.py --port /dev/tty.usbserial-3
        ```

        > Note, where `/dev/tty.usbserial-3` is UART port of device.

    -   Build Bouffalolab OTA image as following execution under build output
        folder:
        ```shell
        ./chip-bl702-lighting-example.flash.py --build
        ```
        After script executed, a folder `ota_images` and an image
        `FW_OTA.bin.xz.hash` will be generated. `FW_OTA.bin.xz.hash` is
        compressed with hash verification for `chip-bl702-lighting-example.bin`.

    > Note, `chip-bl702-lighting-example.flash.py` uses Python module
    > `bflb-iot-tool` to flash device. Please make sure current terminal is
    > under matter build environment; otherwise, Python module `bflb-iot-tool`
    > should be installed under default Python environment using command
    > `pip install bflb-iot-tool`.

-   Using `Bouffalo Lab` flash tool`BLDevCube`
    -   Hold BOOT pin and reset chip, let it be in download mode.
    -   Select `DTS` file;
    -   Select Partition Table
        `<connectedhomeip_repo_path>/examples/platform/bouffalolab/bl702/flash_config/partition_cfg_2M.toml`;
    -   Select Firmware Bin chip-bl702-lighting-example.bin;
    -   Select Chip Erase if need;
    -   Choose Target COM port.
    -   Then click Create & Download.
        > Where `connectedhomeip_repo_path` is the root path of repo
        > connectedhomeip.

## Firmware Behavior

-   `XT-ZB6-DevKit` board

    -   Lighting LED, controlled by on/off command, level command and identify
        command.
    -   Factory Reset: Press down boot pin over 4 seconds, and release boot pin

-   `Night Light` board
    -   Unprovisioned state: light shows yellow.
    -   Provisioned state: light show white.
    -   Factory Reset: Power cycle 3 times before light is on; at 3rd time,
        light shows green and does factory reset after 3 seconds later. And
        factory reset can be cancelled during 3 seconds wait time.

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
    $ ./src/app/ota_image_tool.py create -v 0xFFF1 -p 0x8005 -vn 10 -vs "1.0" -da sha256 <FW_OTA.bin.xz.hash> lighting-app.ota

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

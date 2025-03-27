# Checking out the Matter code

## Checking out all Platforms

To check out the Matter repository with all platforms, run the following
command:

```
git clone --recurse-submodules https://github.com/project-chip/connectedhomeip.git
```

## Checking out `Bouffalo Lab` Platform

-   Checking out matter top level repo with command below:

    ```
    git clone --depth=1 https://github.com/project-chip/connectedhomeip.git
    ```

-   Check out necessary submodules

    Checkout `BL_IOT_SDK` for `BL602`, `BL702` and `BL702L` platform:

    ```
    ./scripts/checkout_submodules.py --shallow --recursive --platform bouffalolab
    ```

    Checkout `bouffalo_sdk` for `BL616` platform:

    ```
    ./scripts/checkout_submodules.py --shallow --recursive --platform bouffalo_sdk
    ```

    > Please contact `Bouffalo Lab` for `BL616` SDK access.

    If you want to checkout Matter Linux example and development tools, please
    try as follows:

    ```
    ./scripts/checkout_submodules.py --shallow --recursive --platform linux bouffalolab
    ```

    Or if you want to checkout Matter Darwin example and development tools,
    please try as follows:

    ```
    ./scripts/checkout_submodules.py --shallow --recursive --platform darwin bouffalolab
    ```

# Setup build environment

Please refer to section `Prerequisites` in
[BUILDING.md](../../guides/BUILDING.md) to install build software.

## Install toolchains for `Bouffalo Lab` SoC

-   Enter to cloned Matter project in terminal and run the following script

    ```
    ./integrations/docker/images/stage-2/chip-build-bouffalolab/setup.sh
    ```

    Script `setup.sh` requires to select install path, and please execute
    following command to export `BOUFFALOLAB_SDK_ROOT` before building.

    ```
    export BOUFFALOLAB_SDK_ROOT="Your install path"
    ```

## Setup Matter build environment

-   Enter to cloned Matter project in terminal and run the following script

    ```
    source scripts/activate.sh -p bouffalolab
    ```

    > Please refer to `scripts/bootstrap.sh` and `scripts/activate.sh` for more
    > detail.

# Build examples

## Build options for `Bouffalo Lab` SoC

With `source scripts/activate.sh -p bouffalolab` under terminal, please try the
following command to list supports options.

```
./scripts/build/build_examples.py targets
```

The output with `bouffalolab` started likes below:

```
bouffalolab-{bl602dk,bl704ldk,bl706dk,bl602-night-light,bl706-night-light,bl602-iot-matter-v1,xt-zb6-devkit}-{light,contact-sensor}-{ethernet,wifi,thread,thread-ftd,thread-mtd}-{easyflash,littlefs}[-shell][-mfd][-rotating_device_id][-rpc][-cdc]
```

-   supported board options, select one of the following options to build

    -   `-bl602dk`
    -   `-bl616dk`
    -   `-bl704ldk`
    -   `-bl706dk`

    -   `-bl602-night-light`
    -   `-bl706-night-light`
    -   `-bl602-iot-matter-v1`
    -   `-xt-zb6-devkit`

-   supported example options, select one of the following options to build

    -   `-light`
    -   `-contact-sensor`, currently, only BL704L with Thread MTD and low power
        supported

-   connectivity options, select one of the following options to build

    -   `-wifi`, specifies to use Wi-Fi for Matter application.

    -   `-ethernet`, specifies to use Ethernet for Matter application.

    -   `-thread`, specifies to use Thread FTD for Matter application.

    -   `-thread-ftd`, specifies to use Thread FTD for Matter application.

    -   `-thread-mtd`, specifies to use Thread MTD for Matter application.

-   storage options, select one of the following options to build

    -   `-littlefs`, specifies to use `littlefs` for flash access.

    -   `-easyflash`, specifies to use `easyflash` for flash access.

        > `littlefs` has different format with `easyflash`, please uses
        > `-easyflash` for your in-field production

-   `-rotating_device_id`, enable rotating device id

-   `-mfd`, enable Matter factory data feature, which load factory data from
    `MFD` partition

    -   Please refer to
        [Bouffalo Lab Matter factory data guide](./matter_factory_data.md) or
        contact to `Bouffalo Lab` for support.

-   `-shell`, enable command line

-   `-rpc`, enable Pigweed RPC feature, which will use `baudrate` 115200 for PRC

-   `-cdc`, enable USB CDC feature, only support for BL706, and can't work with
    Ethernet Board

> `Bouffalo Lab` Matter project uses UART `baudrate` 2000000 for logging output
> by default. If you want other `baudrate` for your test station, please change
> variable `baudrate` in `BUILD.gn` under example project.

## Build an example

Taking lighting app with `littlefs` supported as example :

-   BL602DK with Wi-Fi

    ```
    ./scripts/build/build_examples.py --target bouffalolab-bl602dk-light-wifi-littlefs build
    ```

-   BL616DK with Wi-Fi

    ```shell
    ./scripts/build/build_examples.py --target bouffalolab-bl616dk-light-wifi-littlefs build
    ```

-   BL616 with Thread

    ```
    ./scripts/build/build_examples.py --target bouffalolab-bl616dk-light-thread-littlefs build
    ```

-   BL704L with Thread

    ```
    ./scripts/build/build_examples.py --target bouffalolab-bl704ldk-light-thread-littlefs build
    ```

-   BL706 with Thread

    ```
    ./scripts/build/build_examples.py --target bouffalolab-bl706dk-light-thread-littlefs build
    ```

-   BL706 with Ethernet

    ```
    ./scripts/build/build_examples.py --target bouffalolab-bl706dk-light-ethernet-littlefs build
    ```

-   BL706 with Wi-Fi

    ```
    ./scripts/build/build_examples.py --target bouffalolab-bl706dk-light-ethernet-littlefs build
    ```

    > This BL706 + BL602 Wi-Fi solution: BL602 runs WLAN part and BL706 runs
    > TCP/IP stack which uses SPI for communication between these two parts.

# Download image

Take build target `bouffalolab-bl602dk-light-wifi-littlefs` as example to
introduce image downloading steps.

After example compiled, a python script `chip-bl602-lighting-example.flash.py`
will be generated out under `./out/bouffalolab-bl602dk-light-wifi-littlefs/` and
is used to download image to `Bouffalo Lab` SoC.

And download image as following steps:

-   Connect the board to your build machine with USB cable

-   Put the SoC to the download mode:

    -   Press and hold the **BOOT** button.
    -   Click the **RESET** or **EN** button.
    -   Release the **BOOT** button.

-   Type following command for image download. Please set serial port
    accordingly, here we use /dev/ttyACM0 as a serial port example.

    ```shell
    ./out/bouffalolab-bl602dk-light-wifi-littlefs/chip-bl602-lighting-example.flash.py --port /dev/ttyACM0
    ```

    If needs to download image with the whole flash erased, please append
    `--erase` option.

    ```shell
    ./out/bouffalolab-bl602dk-light-wifi-littlefs/chip-bl602-lighting-example.flash.py --port /dev/ttyACM0 --erase
    ```

    > Note, better to append --erase option to download image for BL602 develop
    > board at first time.

-   Then, open serial console `/dev/ttyACM0` with `baudrate` 2000000. The log
    will output if the **RESET** or **EN** button clicked.

# Test Commission and Control with chip-tool

Please follow
[chip_tool_guide](../../development_controllers/chip-tool/chip_tool_guide.md) to
build and [guide](../../../examples/chip-tool/README.md) use chip-tool for test.

## Prerequisite for Thread Protocol

Thread wireless protocol could runs on BL704L/BL706/BL616, which needs a Thread
border router to connect Thread network to Wi-Fi/Ethernet network. Please follow
this [guide](../openthread/openthread_border_router_pi.md) to setup a raspberry
Pi border router.

After Thread border router setup, please type following command on Thread border
router to get Thread network credential.

```shell
sudo ot-ctl dataset active -x
```

## Commissioning over BLE

-   Reset the board or factory reset the board

-   Enter build out folder of chip-tool and running the following command to do
    BLE commission

    -   Wi-Fi

        ```shell
        ./out/linux-x64-chip-tool/chip-tool pairing ble-wifi <device_node_id> <wifi_ssid> <wifi_passwd> 20202021 3840
        ```

    -   Thread

        ```shell
        ./out/linux-x64-chip-tool/chip-tool pairing ble-thread <device_node_id> hex:<thread_operational_dataset> 20202021 3840
        ```

    -   Ethernet
        `./out/linux-x64-chip-tool/chip-tool pairing onnetwork <device_node_id> 20202021`
        > `<device_node_id>`, which is node ID assigned to device with
        > chip-tool;<br>`<wifi_ssid>`, Wi-Fi network SSID;<br>`<wifi_passwd>`,
        > Wi-FI network password;<br>`<thread_operational_dataset>`, Thread
        > network credential which running `sudo ot-ctl dataset active -x`
        > command on border router to get.

## Read a cluster attribute

```
./out/linux-x64-chip-tool/chip-tool basicinformation read vendor-name <device_node_id> 0
```

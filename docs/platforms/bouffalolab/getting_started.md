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

    Checkout `bouffalo_sdk` for `BL61X` platform:

    ```
    ./scripts/checkout_submodules.py --shallow --recursive --platform bouffalo_sdk
    ```

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

`Bouffalo Lab` Matter platforms currently use different build systems based on
the SDK family:

| SDK integration         | Platforms                  | Build system                                                                                    | Recommended build entry                 |
| ----------------------- | -------------------------- | ----------------------------------------------------------------------------------------------- | --------------------------------------- |
| `bouffalo_sdk` + Matter | BL61X, such as BL616/BL618 | CMake build system from `bouffalo_sdk`, with Matter integrated into the SDK application project | CMake/Makefile in the example directory |
| `BL_IOT_SDK` + Matter   | BL602, BL702/BL702L        | Matter ninja build system                                                                       | `scripts/build/build_examples.py`       |

For `bouffalo_sdk` + Matter, the application is built as a `bouffalo_sdk` CMake
project. The example Makefile wraps the SDK CMake flow and is the recommended
entry point for BL61X product application development. `build_examples.py` is
kept compatible for these BL61X targets: it dispatches to the same CMake build
and then exports artifacts into the standard Matter `out/<target>` directory.

For `BL_IOT_SDK` + Matter, the examples use the Matter ninja build system. Use
`build_examples.py` to select the board, application, connectivity, storage, and
optional features; the script generates the Matter build configuration and runs
the ninja build.

With `source scripts/activate.sh -p bouffalolab` under terminal, please try the
following command to list supported options.

```
./scripts/build/build_examples.py targets
```

The output with `bouffalolab` started likes below:

```
bouffalolab-{bl602-night-light,bl602dk,bl616cl,bl616dk,bl704ldk,bl706-night-light,bl706dk}-{contact-sensor,light}-{ethernet,thread,thread-ftd,thread-mtd,wifi}-{easyflash,littlefs}[-cdc][-coredump][-memmonitor][-mfd][-rotating_device_id][-rpc][-shell]
```

-   supported board options, select one of the following options to build

    -   `-bl602dk`
    -   `-bl616dk`
    -   `-bl616cl`
    -   `-bl704ldk`
    -   `-bl706dk`

    -   `-bl602-night-light`
    -   `-bl706-night-light`

-   supported example options, select one of the following options to build

    -   `-light`
    -   `-contact-sensor`, availability depends on board/connectivity. BL61X
        supports Wi-Fi contact sensor builds; BL704L supports Thread MTD
        low-power contact sensor builds.

-   connectivity options, select one of the following options to build

    -   `-wifi`, specifies to use Wi-Fi for Matter application.

    -   `-ethernet`, specifies to use Ethernet for Matter application.

    -   `-thread`, specifies to use Thread FTD for Matter application.

    -   `-thread-ftd`, specifies to use Thread FTD for Matter application.

    -   `-thread-mtd`, specifies to use Thread MTD for Matter application.

-   storage options, select one of the following options to build

    -   `-littlefs`, specifies to use `littlefs` for flash access.

    -   `-easyflash`, specifies to use `easyflash` for flash access.

        > `littlefs` uses a different format than `easyflash` and they are not
        > compatible. Please use the `-easyflash` flag if existing in-field
        > devices were already deployed with `easyflash`.

        > BL61X `bouffalo_sdk` builds support `littlefs`. Do not use
        > `-easyflash` with BL61X `build_examples.py` targets.

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
> by default. If you want another baudrate for a `BL_IOT_SDK` Matter ninja
> build, please change variable `baudrate` in `BUILD.gn` under the example
> project. For BL61X CMake-based builds, change the UART baudrate through the
> `bouffalo_sdk` configuration used by the example.

## Build an example

### BL61X using CMake (recommended for product development)

The BL61X `bouffalo_sdk` + Matter build uses the CMake build system through a
Makefile wrapper. Product-side application development with `bouffalo_sdk` +
Matter should use this CMake flow because it follows the SDK project layout and
configuration model directly.

**Activate the build environment:**

```shell
source scripts/activate.sh -p bouffalolab
```

**Lighting app:**

| Command                                                                                     | Description         |
| ------------------------------------------------------------------------------------------- | ------------------- |
| `make -C examples/lighting-app/bouffalolab CONFIG_WIFI=y`                                   | Wi-Fi (default)     |
| `make -C examples/lighting-app/bouffalolab CONFIG_THREAD=y`                                 | Thread / OpenThread |
| `make -C examples/lighting-app/bouffalolab CONFIG_THREAD=y CONFIG_OT_FTD=0 CONFIG_OT_MTD=1` | Thread MTD          |
| `make -C examples/lighting-app/bouffalolab CONFIG_ETHERNET=y`                               | Ethernet            |
| `make -C examples/lighting-app/bouffalolab CONFIG_WIFI=y CONFIG_SHELL=y`                    | Wi-Fi + shell       |

**Contact sensor app (Wi-Fi only):**

| Command                                           | Description         |
| ------------------------------------------------- | ------------------- |
| `make -C examples/contact-sensor-app/bouffalolab` | Default Wi-Fi build |

**Optional feature flags (both apps):**

| Flag                                 | Default | Description                       |
| ------------------------------------ | ------- | --------------------------------- |
| `CONFIG_MFD=y/n`                     | `y`     | Enable factory/manufacturing data |
| `CONFIG_CHIP_ROTATING_DEVICE_ID=y/n` | `n`     | Enable rotating device ID         |
| `CONFIG_CHIP_HEAP_MONITOR=y/n`       | `n`     | Enable heap monitoring            |
| `CONFIG_COREDUMP=y/n`                | `n`     | Enable SDK `coredump` capture     |
| `CONFIG_SHELL=y/n`                   | `n`     | Enable Matter interactive shell   |

**Thread-specific flags (lighting-app only):**

| Flag                | Default | Description                       |
| ------------------- | ------- | --------------------------------- |
| `CONFIG_OT_FTD=1/0` | `1`     | Enable or disable Thread FTD mode |
| `CONFIG_OT_MTD=1/0` | `0`     | Enable or disable Thread MTD mode |

**Board selection:**

| Flag            | Default     | Description                                          |
| --------------- | ----------- | ---------------------------------------------------- |
| `CHIP=<chip>`   | `bl616`     | BL61X chip name, for example `bl616` or `bl616cl`    |
| `BOARD=<board>` | `${CHIP}dk` | SDK board name, for example `bl616dk` or `bl616cldk` |

**Clean and rebuild:**

```shell
make -C examples/<app>/bouffalolab clean
make -C examples/<app>/bouffalolab CONFIG_WIFI=y
```

**Flash with optional MFD:**

```shell
make -C examples/<app>/bouffalolab CONFIG_WIFI=y flash MFD_FILE=/path/to/mfd.bin
```

> The CMake build for BL61X is the primary and recommended build method for
> product application development. `build_examples.py` is compatible with BL61X
> and dispatches to the same CMake/Makefile build internally. For the full list
> of available build options, please refer to the `Makefile` in the example
> directory (`examples/<app>/bouffalolab/Makefile`).

### Using build_examples.py

`build_examples.py` remains supported for `Bouffalo Lab` platforms. For BL61X
`bouffalo_sdk` + Matter targets, it invokes the same CMake/Makefile build and
then places generated artifacts and flashing scripts under the standard Matter
output directory, `out/<target>`. For `BL_IOT_SDK` + Matter targets,
`build_examples.py` drives the Matter ninja build system.

Taking lighting app with `littlefs` supported as example:

-   BL602DK with Wi-Fi

    ```shell
    ./scripts/build/build_examples.py --target bouffalolab-bl602dk-light-wifi-littlefs build
    ```

-   BL616DK with Wi-Fi

    ```shell
    ./scripts/build/build_examples.py --target bouffalolab-bl616dk-light-wifi-littlefs build
    ```

    > Recommended product-development build:
    > `make -C examples/lighting-app/bouffalolab CONFIG_WIFI=y`

-   BL616 with Thread

    ```shell
    ./scripts/build/build_examples.py --target bouffalolab-bl616dk-light-thread-littlefs build
    ```

    > Recommended product-development build:
    > `make -C examples/lighting-app/bouffalolab CONFIG_THREAD=y`

-   BL616 with Ethernet

    ```shell
    ./scripts/build/build_examples.py --target bouffalolab-bl616dk-light-ethernet-littlefs build
    ```

    > Recommended product-development build:
    > `make -C examples/lighting-app/bouffalolab CONFIG_ETHERNET=y`

-   BL616 contact sensor with Wi-Fi

    ```shell
    ./scripts/build/build_examples.py --target bouffalolab-bl616dk-contact-sensor-wifi-littlefs build
    ```

    > Recommended product-development build:
    > `make -C examples/contact-sensor-app/bouffalolab`

-   BL704L with Thread

    ```shell
    ./scripts/build/build_examples.py --target bouffalolab-bl704ldk-light-thread-littlefs build
    ```

-   BL706 with Thread

    ```shell
    ./scripts/build/build_examples.py --target bouffalolab-bl706dk-light-thread-littlefs build
    ```

-   BL706 with Ethernet

    ```shell
    ./scripts/build/build_examples.py --target bouffalolab-bl706dk-light-ethernet-littlefs build
    ```

-   BL706 with Wi-Fi

    ```shell
    ./scripts/build/build_examples.py --target bouffalolab-bl706dk-light-wifi-littlefs build
    ```

    > This BL706 + BL602 Wi-Fi solution: BL602 runs WLAN part and BL706 runs
    > TCP/IP stack which uses SPI for communication between these two parts.

# Partition table

`Bouffalo Lab` provides reference partition table files for each platform under
`examples/platform/bouffalolab/<platforms>/flash_config`.

Final products may have different flash layout requirements, such as:

-   Firmware size requirements;
-   Over-the-air upgrade support with or without compressed image;
-   Supports more Matter fabrics;
-   Custom application partitions;
-   And other specific needs.

Developers should design the flash layout accordingly for their final products.
For guidance, refer to the ‌ flash tool documentation or contact `Bouffalo Lab`
for support.

> ‌**Note:**‌ The partition table is typically ‌not editable over-the-air.
> Ensure sufficient margin in the initial design to accommodate future
> requirements.

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

# Matter ESP32 Lighting Example

This example demonstrates the Matter Lighting application on ESP platforms.

---

-   [Matter ESP32 Lighting Example](#matter-esp32-lighting-example)
    -   [Supported Devices](#supported-devices)
    -   [Building the Example Application](#building-the-example-application)
    -   [Commissioning over BLE using chip-tool](#commissioning-over-ble-using-chip-tool)
    -   [Cluster Control](#cluster-control)
-   [Steps to Try Lighting app OTA Requestor](#steps-to-try-lighting-app-ota-requestor)
-   [Flash Encryption](#flash-encryption)

---

## Supported Devices

The demo application is intended to work on following categories of ESP32
devices:

-   [ESP32-DevKitC](https://www.espressif.com/en/products/hardware/esp32-devkitc/overview)
-   [ESP32-WROVER-KIT_V4.1](https://www.espressif.com/en/products/hardware/esp-wrover-kit/overview)
-   [ESP32C3-DevKitM](https://docs.espressif.com/projects/esp-idf/en/latest/esp32c3/hw-reference/esp32c3/user-guide-devkitm-1.html)
-   [ESP32S3-DevKitM](https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/hw-reference/esp32s3/user-guide-devkitm-1.html)
-   [ESP32-H2](https://www.espressif.com/en/news/ESP32_H2).

## Building the Example Application

Building the example application requires the use of the Espressif IoT
Development Framework ([ESP-IDF](https://github.com/espressif/esp-idf)).

The VSCode devcontainer has these components pre-installed, so you can skip this
step. To install these components manually, follow these steps:

-   Clone the Espressif ESP-IDF and checkout
    [v4.4.1 release](https://github.com/espressif/esp-idf/releases/tag/v4.4.1)

        $ mkdir ${HOME}/tools
        $ cd ${HOME}/tools
        $ git clone https://github.com/espressif/esp-idf.git
        $ cd esp-idf
        $ git checkout v4.4.1
        $ git submodule update --init
        $ ./install.sh

-   For ESP32H2, you can checkout commit id
    [10f3aba770](https://github.com/espressif/esp-idf/tree/10f3aba770)

        $ cd esp-idf
        $ git checkout 10f3aba770
        $ git submodule update --init
        $ ./install.sh

-   Install ninja-build

        $ sudo apt-get install ninja-build

Currently building in VSCode _and_ deploying from native is not supported, so
make sure the IDF_PATH has been exported(See the manual setup steps above).

-   Setting up the environment

        $ cd ${HOME}/tools/esp-idf
        $ ./install.sh
        $ . ./export.sh
        $ cd {path-to-connectedhomeip}

    To download and install packages.

        $ source ./scripts/bootstrap.sh
        $ source ./scripts/activate.sh

    If packages are already installed then simply activate them.

        $ source ./scripts/activate.sh

-   Enable Ccache for faster IDF builds

    It is recommended to have Ccache installed for faster builds

    ```
    $ export IDF_CCACHE_ENABLE=1
    ```

-   Target Set

        $ idf.py set-target esp32
        or
        $ idf.py set-target esp32c3
        or
        $ idf.py set-target esp32s3
        or
        $ idf.py --preview set-target esp32h2

-   Using ESP32 Factory Data Provider (Optional)

    By default this application uses test-mode CommissionableDataProvider and
    Example DeviceAttestationCredentialsProvider.

    Enable config option `CONFIG_ENABLE_ESP32_FACTORY_DATA_PROVIDER` to use
    ESP32 specific implementation of CommissionableDataProvider and
    DeviceAttestationCredentialsProvider.

    ESP32 implementation reads factory data from nvs partition, chip-factory
    data must be flashed into the configure nvs partition. Factory partition can
    be configured using `CONFIG_CHIP_FACTORY_NAMESPACE_PARTITION_LABEL` option,
    default is "nvs".

    `scripts/tools/generate_esp32_chip_factory_bin.py` script generates the
    chip-factory NVS image `partition.bin`.

    Script has dependency on
    [spake2p](https://github.com/project-chip/connectedhomeip/tree/master/src/tools/spake2p)
    for generating spake2p parameters.

    Build spake2p:

    ```
    cd path/to/connectedhomeip
    gn gen out/host
    ninja -C out/host
    cd -
    ```

    Add spake2p to \$PATH environment variable

    ```
    export PATH=$PATH:path/to/connectedhomeip/out/host
    ```

-   To erase flash of the chip.

        $ idf.py -p /dev/tty.SLAB_USBtoUART erase-flash

    Below mentioned command generates the nvs image with test DAC with
    VID:0xFFF2 and PID:8001

    ```
    cd third_party/connectedhomeip/scripts/tools

    ./generate_esp32_chip_factory_bin.py -d 3434 -p 99663300 \
                                        --dac-cert ../../credentials/test/attestation/Chip-Test-DAC-FFF2-8001-0008-Cert.der \
                                        --dac-key ../../credentials/test/attestation/Chip-Test-DAC-FFF2-8001-0008-Key.der \
                                        --pai-cert ../../credentials/test/attestation/Chip-Test-PAI-FFF2-8001-Cert.der \
                                        --cd ../../credentials/test/certification-declaration/Chip-Test-CD-FFF2-8001.der
    cd -
    ```

    This project uses VID:0xFFF1 and PID:0x8000, if you are planning to use the
    above command as is please change the VID/PID using menuconfig options.

    Use the following command to flash the NVS image. `0x9000` is default
    address for `nvs` partition.

    ```
    esptool.py -p <port> write_flash 0x9000 third_party/connectedhomeip/scripts/tools/partition.bin
    ```

-   To build the demo application.

        $ idf.py build

-   After building the application, to flash it outside of VSCode, connect your
    device via USB. Then run the following command to flash the demo application
    onto the device and then monitor its output. If necessary, replace
    `/dev/tty.SLAB_USBtoUART`(MacOS) with the correct USB device name for your
    system(like `/dev/ttyUSB0` on Linux). Note that sometimes you might have to
    press and hold the `boot` button on the device while it's trying to connect
    before flashing.

        $ idf.py -p /dev/tty.SLAB_USBtoUART flash monitor

    Note: Some users might have to install the
    [VCP driver](https://www.silabs.com/products/development-tools/software/usb-to-uart-bridge-vcp-drivers)
    before the device shows up on `/dev/tty`.

-   Quit the monitor by hitting `Ctrl+]`.

    Note: You can see a menu of various monitor commands by hitting
    `Ctrl+t Ctrl+h` while the monitor is running.

-   If desired, the monitor can be run again like so:

        $ idf.py -p /dev/tty.SLAB_USBtoUART monitor

-   Commissioning over ble after flashing script, change the passcode, replace
    `20202021` with `99663300`.

## Commissioning over BLE using chip-tool

-   Please build the standalone chip-tool as described [here](../../chip-tool)
-   Commissioning the WiFi Lighting devices(ESP32, ESP32C3, ESP32S3)

        $ ./out/debug/chip-tool pairing ble-wifi 12345 <ssid> <passphrase> 20202021 3840

-   For ESP32-H2, firstly start OpenThread Border Router, you can either use
    [Raspberry Pi OpenThread Border Router](https://github.com/project-chip/connectedhomeip/blob/master/docs/guides/openthread_border_router_pi.md)
    OR
    [ESP32 OpenThread Border Router](https://github.com/espressif/esp-idf/tree/master/examples/openthread/ot_br)
-   Get the active operational dataset.

        $ ot-ctl> dataset active -x

-   Commissioning the Thread Lighting device(ESP32H2)

         $ ./out/debug/chip-tool pairing ble-thread 12345 hex:<operational-dataset> 20202021 3840

    NOTE: If using ESP32 factory data provider then commission the device using
    discriminator and passcode provided while building the factory NVS binary.

## Cluster Control

-   After successful commissioning, use the OnOff cluster command to control the
    OnOff attribute. This allows you to toggle a parameter implemented by the
    device to be On or Off.

        $ ./out/debug/chip-tool onoff on 12345 1

-   On
    [ESP32C3-DevKitM](https://docs.espressif.com/projects/esp-idf/en/latest/esp32c3/hw-reference/esp32c3/user-guide-devkitm-1.html)
    or
    [ESP32S3-DevKitM](https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/hw-reference/esp32s3/user-guide-devkitm-1.html)
    board, there is an on-board RGB-LED. Use ColorControl cluster command to
    control the color attributes:

        $ ./out/debug/chip-tool colorcontrol move-to-hue-and-saturation 240 100 0 0 0 12345 1

# Steps to Try Lighting app OTA Requestor

Before moving ahead, make sure your device is commissioned and running.

-   Build the Linux OTA Provider

```
scripts/examples/gn_build_example.sh examples/ota-provider-app/linux out/debug chip_config_network_layer_ble=false
```

-   Run the Linux OTA Provider with OTA image.

```
./out/debug/chip-ota-provider-app -f hello-world.bin
```

hello-world.bin can be obtained from compiling the hello-world ESP-IDF example.

-   Commission the Linux OTA Provider using chip-tool

```
./out/debug/chip-tool pairing onnetwork 12346 20202021
```

## Query for an OTA Image

After commissioning is successful, press Enter in requestor device console and
type below query.

```
>matter ota query 1 12346 0
```

## Apply update

Once the transfer is complete, OTA requestor sends ApplyUpdateRequest command to
OTA provider for applying the image. Device will restart on successful
application of OTA image.

# Flash encryption

Below is the quick start guide for encrypting the application and factory
partition but before proceeding further please READ THE DOCS FIRST.
Documentation References:

-   [Flash Encryption](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/security/flash-encryption.html)
-   [NVS Encryption](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/storage/nvs_flash.html#nvs-encryption)

## Enable flash encryption and some factory settings using `idf.py menuconfig`

-   Enable the Flash encryption [Security features → Enable flash encryption on
    boot]
-   Use `partitions_encrypted.csv` partition table [Partition Table → Custom
    partition CSV file]
-   Enable ESP32 Factory Data Provider [Component config → CHIP Device Layer →
    Commissioning options → Use ESP32 Factory Data Provider]
-   Enable ESP32 Device Instance Info Provider [Component config → CHIP Device
    Layer → Commissioning options → Use ESP32 Device Instance Info Provider]

## Generate the factory partition using `generate_esp32_chip_factory_bin.py` script

-   Please provide `-e` option along with other options to generate the
    encrypted factory partition
-   Two partition binaries will be generated `factory_partition.bin` and
    `keys/nvs_key_partition.bin`

## Flashing the application, factory partition, and nvs keys

-   Flash the application using `idf.py flash`, NOTE: If not flashing for the
    first time you will have to use `idf.py encrypted-flash`

-   Flash the factory partition, this SHALL be non encrypted write as NVS
    encryption works differently

```
esptool.py -p (PORT) write_flash 0x9000 path/to/factory_partition.bin
```

-   Encrypted flash the nvs keys partition

```
esptool.py -p (PORT) write_flash --encrypt 0x317000 path/to/nvs_key_partition.bin
```

NOTE: Above command uses the default addressed printed in the boot logs

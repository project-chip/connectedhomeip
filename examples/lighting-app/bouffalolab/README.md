# Matter `Bouffalo Lab` Lighting app Example

This example functions as a light bulb device type, with on/off and level
capabilities and uses a test Vendor ID (VID) and a Product ID (PID)
of **0x8005**.

Please refer to the following documents for more information

-   [Bouffalo Lab - Platform overview](../../../docs/platforms/bouffalolab/platform_overview.md)
-   [Bouffalo Lab - Getting Started](../../../docs/platforms/bouffalolab/getting_started.md)
-   [Bouffalo Lab - OTA upgrade](../../../docs/platforms/bouffalolab/ota_upgrade.md)
-   [Bouffalo Lab - Matter factory data generation](../../../docs/platforms/bouffalolab/matter_factory_data.md)
-   [Bouffalo Lab - RPC console](../../../docs/platforms/bouffalolab/rpc_console.md)

## Build example

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

## Test with chip-tool

### Commissioning over BLE

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

### Cluster control

After successful commissioning, cluster commands available to control the board.

-   OnOff cluster

    The following command shows to toggle the LED on the board

    ```
    $ ./chip-tool onoff toggle <device_node_id> 1
    ```

-   Level cluster

    The following command shows to move level to 128.

    ```
    $ ./chip-tool levelcontrol move-to-level 128 10 0 0 <device_node_id> 1
    ```

-   Color cluster

    The following command shows to change hue and saturation to 240 and 100

    ```
    $ ./chip-tool colorcontrol move-to-hue-and-saturation 240 100 0 0 0 <device_node_id> 1
    ```

-   Identify Light

    The following command shows to identify the board 10 seconds

    ```shell
    ./chip-tool identify identify 10 <device_node_id> 1
    ```

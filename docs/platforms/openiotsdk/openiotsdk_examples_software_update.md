# Matter Open IoT SDK Example Device Firmware Upgrade

Matter Open IoT SDK examples can support over-the-air (`OTA`) Device Firmware
Upgrade (`DFU`) based on Matter-compliant OTA update protocol that uses the
Matter operational network for querying and downloading a new firmware image.

## Device Firmware Upgrade over Matter OTA cluster

The `DFU` over Matter OTA clusters requires two kinds of nodes:

-   `OTA Provider` is an application that implements the OTA provider cluster
    service. It can respond to the OTA Requestors' queries about available
    software updates and share the update image with them.
-   `OTA Requestor` is an application that implements the OTA requestor cluster
    service. It can communicate with the OTA Provider to fetch applicable
    software updates and apply them.

The last required element is a Matter controller. This application controls both
nodes and manages the entire software update process.

In the procedure described below, the `OTA Provider` will be a
[Linux application](../../examples/ota-provider-app/linux/README.md) and the
Open IoT SDK example with
[DFU support](./openiotsdk_examples.md#device-firmware-update) will work as the
OTA Requestor. The [chip-tool](../../examples/chip-tool/README.md) application
used as the Matter controller. Each application should be launched in a separate
terminal.

List of `OIS` examples that currently support the `DFU` over Matter:

-   ota-requestor-app

### Device Firmware Upgrade over Matter procedure

1.  Navigate to the CHIP root directory.

2.  Build the `OTA Provider` application:

    ```
    scripts/examples/gn_build_example.sh examples/ota-provider-app/linux out/ota-provider chip_config_network_layer_ble=false
    ```

    More details about the `OTA provider` application can be found
    [here](../../examples/ota-provider-app/linux/README.md).

3.  Build `chip-tool`:

    ```
    scripts/examples/gn_build_example.sh examples/chip-tool out/chip-tool
    ```

    More details about the `chip-tool` application can be found
    [here](../../examples/chip-tool/README.md).

4.  Build `OIS` example application

    ```
    scripts/examples/openiotsdk_example.sh -v 1 -V 0.0.1 <example_name>
    ```

    This is first version of the application that can be updated.

5.  Build `OIS` update image

    ```
    scripts/examples/openiotsdk_example.sh -p out/update -v 2 -V 0.0.2 <example_name>
    ```

    Pass the build path (`-p out/update`) in this step to not override the
    example application. In that directory you should find the update image file
    with `.ota` extension (`chip-openiotsdk-<example_name>-example.ota`).

6.  Setup `OIS` network environment

    ```
    sudo ./scripts/setup/openiotsdk/network_setup.sh -n OISupdate up
    ```

    More details about `OIS` network environment can be found
    [here](./openiotsdk_examples.md#networking-setup).

7.  In `terminal 1`: run the `OTA Provider` application in the network
    environment and provide a path to the update image:

    ```
    scripts/run_in_ns.sh OISupdate out/ota-provider/chip-ota-provider-app --KVS /tmp/chip-ota-provider --discriminator 3841 --secured-device-port 5580 --filepath out/update/chip-openiotsdk-<example_name>-example.ota
    ```

    The node details should be printed:

    ```
    ...
    CHIP:DL: Device Configuration:
    CHIP:DL:   Serial Number: TEST_SN
    CHIP:DL:   Vendor Id: 65521 (0xFFF1)
    CHIP:DL:   Product Id: 32769 (0x8001)
    CHIP:DL:   Hardware Version: 0
    CHIP:DL:   Setup Pin Code (0 for UNKNOWN/ERROR): 20202021
    CHIP:DL:   Setup Discriminator (0xFFFF for UNKNOWN/ERROR): 3841 (0xF01)
    CHIP:DL:   Manufacturing Date: (not set)
    CHIP:DL:   Device Type: 65535 (0xFFFF)
    CHIP:SVR: SetupQRCode: [MT:-24J0IRV01KA0648G00]
    CHIP:SVR: Copy/paste the below URL in a browser to see the QR Code:
    CHIP:SVR: https://project-chip.github.io/connectedhomeip/qrcode.html?data=MT%3A-24J0IRV01KA0648G00
    CHIP:SVR: Manual pairing code: [34970112332]
    CHIP:SWU: Using OTA file: out/update/chip-openiotsdk-ota-requestor-app-example.ota
    ...
    ```

    > 💡 **Notes**:
    >
    > Set the custom Key Value Store path for `OTA provider`.
    >
    > The `OTA provider` discriminator must be different from the value used by
    > the example application. `OIS` examples use the default discriminator
    > value `3840`.
    >
    > The `OTA provider` UDP port must be different from the value used by the
    > example application. `OIS` examples use the default discriminator value
    > `5540`.

8.  In `terminal 2`: run the `OIS` example application in the network
    environment:

    ```
    scripts/run_in_ns.sh OISupdate scripts/examples/openiotsdk_example.sh -C run -n OISupdatetap <example_name>
    ```

    The node details should be printed to terminal:

    ```
    ...
    [INF] [DL] Device Configuration:
    [INF] [DL]   Serial Number: TEST_SN
    [INF] [DL]   Vendor Id: 65521 (0xFFF1)
    [INF] [DL]   Product Id: 32769 (0x8001)
    [INF] [DL]   Hardware Version: 0
    [INF] [DL]   Setup Pin Code (0 for UNKNOWN/ERROR): 20202021
    [INF] [DL]   Setup Discriminator (0xFFFF for UNKNOWN/ERROR): 3840 (0xF00)
    [INF] [DL]   Manufacturing Date: (not set)
    [INF] [DL]   Device Type: 65535 (0xFFFF)
    [INF] [SVR] SetupQRCode: [MT:-24J0AFN00KA0648G00]
    [INF] [SVR] Copy/paste the below URL in a browser to see the QR Code:
    [INF] [SVR] https://project-chip.github.io/connectedhomeip/qrcode.html?data=MT%3A-24J0AFN00KA0648G00
    [INF] [SVR] Manual pairing code: [34970112332]
    [INF] [-] Current software version: [1] 0.0.1
    [INF] [SWU] Stopping the watchdog timer
    [INF] [SWU] Starting the periodic query timer, timeout: 86400 seconds
    [INF] [-] Open IoT SDK ota-requestor-app example application run
    ...
    ```

9.  In `terminal 3`: commission the `OTA Provider` application into the Matter
    network:

    ```
    scripts/run_in_ns.sh OISupdate ./out/chip-tool/chip-tool pairing onnetwork-long 123 20202021 3841
    ```

    Set node ID to `123`.

    The confirmation of commissioning success will be printed on the terminal:

    ```
    CHIP:SVR: Commissioning completed successfully
    ```

10. In `terminal 3`: commission the `OIS` example application into the Matter
    network:

    ```
    scripts/run_in_ns.sh OISupdate ./out/chip-tool/chip-tool pairing onnetwork-long 321 20202021 3840
    ```

    Set node ID to `321`.

    The confirmation of commissioning success will be printed on the terminal:

    ```
    [INF] [SVR] Commissioning completed successfully
    ```

11. In `terminal 3`: configure the `OTA Provider` with the access control list
    (ACL) that grants _Operate_ privileges to all nodes in the fabric. This is
    necessary to allow the nodes to send cluster commands to the `OTA Provider`:

    ```
    scripts/run_in_ns.sh OISupdate ./out/chip-tool/chip-tool accesscontrol write acl '[{"fabricIndex": 1, "privilege": 5, "authMode": 2, "subjects": [321], "targets": null}, {"fabricIndex": 1, "privilege": 3, "authMode": 2, "subjects": null, "targets": null}]' 123 0
    ```

12. Initiate the `DFU` procedure:

    In `terminal 3`: send the `Announce OTA Provider` command to the example
    application. The numeric arguments are: provider node ID, provider vendor
    ID, announcement reason, provider endpoint ID, requestor node ID and
    requestor Endpoint Id, respectively.

    ```
    scripts/run_in_ns.sh OISupdate ./out/chip-tool/chip-tool otasoftwareupdaterequestor announce-otaprovider 123 0 2 0 321 0
    ```

    The example application is notified of the `OTA Provider` node and it
    automatically queries for a new firmware image. If a new version of firmware
    is available, the downloading step starts.

13. When the firmware image download is complete, the device is automatically
    rebooted to apply the update.

    The new version of firmware will be printed in the log `terminal 2`:

    ```
    ...
    [INF] [-] Current software version: [2] 0.0.2
    ...
    ```

14. Now, you can manually close the nodes applications and terminals sessions:

    -   `OTA Provider` - `terminal 1`
    -   `OIS` example - `terminal 2`. More details about terminating `OIS`
        example can be found [here](./openiotsdk_examples.md#running).

15. Cleanup after update (disable network environment, remove `KVS` storage
    files):

    ```
    sudo ./scripts/setup/openiotsdk/network_setup.sh -n OISupdate down
    sudo rm /tmp/chip-ota-provider
    ```

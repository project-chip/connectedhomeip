# Matter Telink Pump Controller Example Application

The Telink Pump Controller Example demonstrates how to remotely control a pump
device. It uses buttons to test changing the pump state and device states and
LEDs to show the state of these changes. This example is inherited from the
"lock-app" example but modified to simulate a pump device and can be used as a
reference for creating your own pump application.

![Telink B91 EVK](http://wiki.telink-semi.cn/wiki/assets/Hardware/B91_Generic_Starter_Kit_Hardware_Guide/connection_chart.png)

## Supported devices

The example supports building and running on the following devices:

| Board/SoC                                                                                                                                                              | Build target                                                  | Zephyr Board Info                                                                                              |
| ---------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ------------------------------------------------------------- | -------------------------------------------------------------------------------------------------------------- |
| [B91](https://wiki.telink-semi.cn/wiki/Hardware/B91_Generic_Starter_Kit_Hardware_Guide) [TLSR9518ADK80D](https://wiki.telink-semi.cn/wiki/chip-series/TLSR951x-Series) | `tlsr9518adk80d`, `tlsr9518adk80d-mars`, `tlsr9518adk80d-usb` | [TLSR9518ADK80D](https://github.com/telink-semi/zephyr/blob/develop/boards/riscv/tlsr9518adk80d/doc/index.rst) |
| [B92](https://wiki.telink-semi.cn/wiki/Hardware/B92_Generic_Starter_Kit_Hardware_Guide) [TLSR9528A](https://wiki.telink-semi.cn/wiki/chip-series/TLSR952x-Series)      | `tlsr9528a`, `tlsr9528a_retention`                            | [TLSR9528A](https://github.com/telink-semi/zephyr/blob/develop/boards/riscv/tlsr9528a/doc/index.rst)           |
| [W91](https://wiki.telink-semi.cn/wiki/Hardware/W91_Generic_Starter_Kit_Hardware_Guide) [TLSR9118BDK40D](https://wiki.telink-semi.cn/wiki/chip-series/TLSR911x-Series) | `tlsr9118bdk40d`                                              | [TLSR9118BDK40D](https://github.com/telink-semi/zephyr/blob/develop/boards/riscv/tlsr9118bdk40d/doc/index.rst) |

## Build and flash

1. Run the Docker container:

    ```bash
    $ docker run -it --rm -v $PWD:/host -w /host ghcr.io/project-chip/chip-build-telink:$(wget -q -O - https://raw.githubusercontent.com/project-chip/connectedhomeip/master/.github/workflows/examples-telink.yaml 2> /dev/null | grep chip-build-telink | awk -F: '{print $NF}')
    ```

    You can find the compatible Docker image version in the file:

    ```bash
    $ .github/workflows/examples-telink.yaml
    ```

2. Activate the build environment:

    ```bash
    $ source ./scripts/activate.sh -p all,telink
    ```

3. Build the example (replace _<build_target>_ with your board name, see
   [Supported devices](#supported-devices)):

    ```bash
    $ west build -b <build_target>
    ```

    Also use key `-DFLASH_SIZE`, if your board has memory size different from 2
    MB, for example, `-DFLASH_SIZE=1m` or `-DFLASH_SIZE=4m`:

    ```bash
    $ west build -b <build_target> -- -DFLASH_SIZE=4m
    ```

    You can find the target built file called **_zephyr.bin_** under the
    **_build/zephyr_** directory.

4. Flash binary:

    ```
    $ west flash --erase
    ```

## Usage

### UART

To get output from device, connect UART to following pins:

| Name | Pin                           |
| :--: | :---------------------------- |
|  RX  | PB3 (pin 17 of J34 connector) |
|  TX  | PB2 (pin 16 of J34 connector) |
| GND  | GND                           |

Baud rate: 115200 bits/s

### Buttons

The following buttons are available on **tlsr9518adk80d** board:

| Name     | Function               | Description                                                                                                                                       |
| :------- | :--------------------- | :------------------------------------------------------------------------------------------------------------------------------------------------ |
| Button 1 | Factory reset          | Perform factory reset to forget currently commissioned Thread network and return to a decommissioned state (to activate, push the button 3 times) |
| Button 2 | Lock control           | Manually triggers the bolt lock state                                                                                                             |
| Button 3 | Thread start           | Commission thread with static credentials and enables the Thread on device                                                                        |
| Button 4 | Open commission window | The button is opening commissioning window to perform commissioning over BLE                                                                      |

### LEDs

#### Indicate current state of Thread network

**Red** LED indicates current state of Thread network. It is able to be in
following states:

| State                       | Description                                                                  |
| :-------------------------- | :--------------------------------------------------------------------------- |
| Blinks with short pulses    | Device is not commissioned to Thread, Thread is disabled                     |
| Blinks with frequent pulses | Device is commissioned, Thread enabled. Device trying to JOIN thread network |
| Blinks with wide pulses     | Device commissioned and joined to thread network as CHILD                    |

#### Indicate identify of device

**Green** LED used to identify the device. The LED starts blinking when the
Identify command of the Identify cluster is received. The command's argument can
be used to specify the the effect. It is able to be in following effects:

| Effect                          | Description                                                                  |
| :------------------------------ | :--------------------------------------------------------------------------- |
| Blinks (200 ms on/200 ms off)   | Blink (`Clusters::Identify::EffectIdentifierEnum::kBlink`)                   |
| Breathe (during 1000 ms)        | Breathe (`Clusters::Identify::EffectIdentifierEnum::kBreathe`)               |
| Blinks (50 ms on/950 ms off)    | Okay (`Clusters::Identify::EffectIdentifierEnum::kOkay`)                     |
| Blinks (1000 ms on/1000 ms off) | Channel Change ( `Clusters::Identify::EffectIdentifierEnum::kChannelChange`) |
| Blinks (950 ms on/50 ms off)    | Finish ( `Clusters::Identify::EffectIdentifierEnum::kFinishEffect`)          |
| LED off                         | Stop (`Clusters::Identify::EffectIdentifierEnum::kStopEffect`)               |

#### Indicate current Pump state

**White** LED shows current state of Pump (running/stopped)

### CHIP tool commands

1. Build
   [chip-tool cli](https://github.com/project-chip/connectedhomeip/blob/master/examples/chip-tool/README.md)

2. Pair with device

    ```
    ${CHIP_TOOL_DIR}/chip-tool pairing ble-thread ${NODE_ID} hex:${DATASET} ${PIN_CODE} ${DISCRIMINATOR}
    ```

    Example:

    ```
    ./chip-tool pairing ble-thread 1234 hex:0e080000000000010000000300000f35060004001fffe0020811111111222222220708fd61f77bd3df233e051000112233445566778899aabbccddeeff030e4f70656e54687265616444656d6f010212340410445f2b5ca6f2a93a55ce570a70efeecb0c0402a0fff8 20202021 3840
    ```

### OTA with Linux OTA Provider

OTA feature enabled by default only for ota-requestor-app example. To enable OTA
feature for another Telink example:

-   set CONFIG_CHIP_OTA_REQUESTOR=y in corresponding "prj.conf" configuration
    file.

After build application with enabled OTA feature, use next binary files:

-   merged.bin - main binary to flash PCB (Use at least 2MB PCB).
-   matter.ota - binary for OTA Provider

All binaries has the same SW version. To test OTA “matter.ota” should have
higher SW version than base SW. Set CONFIG_CHIP_DEVICE_SOFTWARE_VERSION=2 in
corresponding “prj.conf” conﬁguration file.

Usage of OTA:

-   Build the [Linux OTA Provider](../../ota-provider-app/linux)

    ```
    ./scripts/examples/gn_build_example.sh examples/ota-provider-app/linux out/ota-provider-app chip_config_network_layer_ble=false
    ```

-   Run the Linux OTA Provider with OTA image.

    ```
    ./chip-ota-provider-app -f matter.ota
    ```

-   Provision the Linux OTA Provider using chip-tool

    ```
    ./chip-tool pairing onnetwork ${OTA_PROVIDER_NODE_ID} 20202021
    ```

    here:

    -   \${OTA_PROVIDER_NODE_ID} is the node id of Linux OTA Provider

-   Configure the ACL of the ota-provider-app to allow access

    ```
    ./chip-tool accesscontrol write acl '[{"fabricIndex": 1, "privilege": 5, "authMode": 2, "subjects": [112233], "targets": null}, {"fabricIndex": 1, "privilege": 3, "authMode": 2, "subjects": null, "targets": null}]' ${OTA_PROVIDER_NODE_ID} 0
    ```

    here:

    -   \${OTA_PROVIDER_NODE_ID} is the node id of Linux OTA Provider

-   Use the chip-tool to announce the ota-provider-app to start the OTA process

    ```
    ./chip-tool otasoftwareupdaterequestor announce-otaprovider ${OTA_PROVIDER_NODE_ID} 0 0 0 ${DEVICE_NODE_ID} 0
    ```

    here:

    -   \${OTA_PROVIDER_NODE_ID} is the node id of Linux OTA Provider
    -   \${DEVICE_NODE_ID} is the node id of paired device

Once the transfer is complete, OTA requestor sends ApplyUpdateRequest command to
OTA provider for applying the image. Device will restart on successful
application of OTA image.

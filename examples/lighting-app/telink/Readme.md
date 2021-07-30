![Telink B91 EVK](http://wiki.telink-semi.cn/wiki/assets/Hardware/B91_Generic_Starter_Kit_Hardware_Guide/connection_chart.png)

## Build

1. Pull docker image from repository:

    ```
    $ docker pull connectedhomeip/chip-build-telink:latest
    ```

1. Run docker container:

    ```
    $ docker run -it -v ${CHIP_BASE}:/root/chip connectedhomeip/chip-build-telink:latest
    ```

    here `${CHIP_BASE}` is directory which contains CHIP repo files **!!!Pay
    attention that OUTPUT_DIR should conatins ABSOLUTE path to output dir**

1. Run build script:
    ```
    cd /root/chip/scripts/examples && ./telink_example.sh lighting-app tlsr9518adk80d
    ```
1. Exit docker container and collect build artifacts. Firmware binary would be
   located in
   **\${CHIP_BASE}/examples/lighting-app/telink/build/tlsr9518adk80d/zephyr/zephyr.bin**

## Flash

### Presetup

To make `west flash` command works following steps should be performed just
once:

1. Download toolchain:
    ```
    $ wget http://wiki.telink-semi.cn/tools_and_sdk/Tools/IDE/telink_riscv_linux_toolchain.zip
    $ unzip telink_riscv_linux_toolchain.zip
    ```
1. Add TELINK_TOOLCHAIN_BASE variable to environment:
    ```
    $ export TELINK_TOOLCHAIN_BASE=${PATH_TO_TOOLCHAIN}
    ```
1. Setup dependencies:

    ```
    $ sudo dpkg --add-architecture i386
    $ sudo apt-get update
    $ sudo apt-get install -y libc6:i386 libncurses5:i386 libstdc++6:i386
    ```

### Flashing

1. Go to example directory and flash board with west command
    ```
    $ cd ${TELINK_LIGHTING_EXAMPLE_DIR} && west flash
    ```

## Border Router

### Build

Use following manual to build your own border router:
https://openthread.io/guides/border-router/build Pay attention that border
router should be configured as Access Point i.e next command should be executed
in step 3:

```
$ BORDER_ROUTING=0 NETWORK_MANAGER=1 ./script/setup
```

### Setup IPv6

Pay attention that border router shoud be configured as IPv6 access point.

1. To do so perform the following command:
    ```
    ip -6 addr add 2001:db8:1::1/64 dev wlan0
    ```
2. Add following lines in **/etc/dnsmasq.conf** file:
    ```
    enable-ra
    dhcp-range=2001:db8:1::, ra-only, 64, 12h
    ```

### Config network

Use [Web GUI](https://openthread.io/guides/border-router/web-gui) to config
Thread network **tlsr9518adk80d** board supports only static comissioning with
predefined Thread credentials shown in table below:

| Item                   |              Value               |
| :--------------------- | :------------------------------: |
| Network name           |          OpenThreadDemo          |
| Network ExtendedPAN ID |         1111111122222222         |
| PAN ID                 |              0x1234              |
| Passphrase             |              123456              |
| Master Key             | 00112233445566778899aabbccddeeff |
| Channel                |                15                |
| On-Mesh Prefix         |            fd11:22::             |

## Usage

### UART

To get output from device, connect UART to following pins:

| Name | Pin                           |
| :--: | :---------------------------- |
|  RX  | PB3 (pin 17 of J34 connector) |
|  TX  | PB2 (pin 16 of J34 connector) |
| GND  | GND                           |

### Buttons

The following buttons are available on **tlsr9518adk80d** board:

| Name     | Function         | Description                                                                                           |
| :------- | :--------------- | :---------------------------------------------------------------------------------------------------- |
| Button 1 | Factory reset    | Perform factory reset to forget currently commissioned Thread network and back to uncommisioned state |
| Button 2 | Lighting control | Manually triggers the lighting state                                                                  |
| Button 3 | Thread start     | Comission thread with static credentials and enables the Thread on device                             |

### LEDs

**Red** LED indicates current state of Thread network. It ables to be in
following states:

| State                       | Description                                                                  |
| :-------------------------- | :--------------------------------------------------------------------------- |
| Blinks with short pulses    | Device is not commisioned to Thread, Thred is disabled                       |
| Blinls with frequent pulses | Device is commissioned, Thread enabled. Device trying to JOIN thread network |
| Blinks with whde pulses     | Device commissioned and joined to thread network as CHILD                    |

**Blue** LED shows current state of lightbulb

### Check connection

1. With your client device (PC, Laptop etc.) connect to BorderRouterAP WiFi
2. Press Button 3 on **tlsr9518adk80d** board and wait till it joins to Thread
   network
3. Find ajusted IPv6 address in UART output of **tlsr9518adk80d**
4. Perform following command on your client device:
    ```
    ping -6 ${IP_ADDRESS_OF_CHIP_DEVICE}
    ```
    here `${IP_ADDRESS_OF_CHIP_DEVICE}` is address which you got from UART
    output of **tlsr9518adk80d** board If everything is ok then `ping` command
    should perform transactions without losses

### CHIP tool commands

1. Build
   [chip-tool cli](https://github.com/project-chip/connectedhomeip/blob/master/examples/chip-tool/README.md)
1. Pair with device

    ```
    ${CHIP_TOOL_DIR}/chip-tool pairing bypass ${IP_ADDRESS_OF_CHIP_DEVICE} 5540
    ```

    here:

    - `${IP_ADDRESS_OF_CHIP_DEVICE}` is IPv6 address of CHIP device
    - **5540** is the standard CHIP TCP port

1. Switch on the light:

    ```
    ${CHIP_TOOL_DIR}/chip-tool onoff on 1
    ```

    here:

    - **onoff** is name of cluster
    - **on** command to the cluster
    - **1** ID of endpoint

1. Switch off the light:

    ```
    ${CHIP_TOOL_DIR}/chip-tool onoff off 1
    ```

    here:

    - **onoff** is name of cluster
    - **off** command to the cluster
    - **1** ID of endpoint

1. Read the light state:

    ```
    ${CHIP_TOOL_DIR}/chip-tool onoff read on-off 1
    ```

    here:

    - **onoff** is name of cluster
    - **read** command to the cluster
    - **on-off** attribute to read
    - **1** ID of endpoint

1. Change brightness of light:

    ```
    ${CHIP_TOOL_DIR}/chip-tool levelcontrol move-to-level 32 0 0 0 1
    ```

    here:

    - **levelcontrol** is name of cluster
    - **move-to-level** command to the cluster
    - **32** brightness value
    - **0** transition time
    - **0** option mask
    - **0** option override
    - **1** ID of endpoint

1. Reag brightness level:
    ```
    ./chip-tool levelcontrol read current-level 1
    ```
    here:
    - **levelcontrol** is name of cluster
    - **read** command to the cluster
    - **current-level** attribute to read
    - **1** ID of endpoint

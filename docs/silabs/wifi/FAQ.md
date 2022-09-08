# Frequently Asked Questions and Troubleshooting for Matter over Wi-Fi

## Troubleshooting

<br>

### 1. Bluetooth connection fails when trying to commission the system through the ChipTool:

**Command leading to error:**

`$ out/standalone/chip-tool pairing ble-wifi 1122 mySSID myPassword 20202021 3840`

Where `mySSID` is **your AP's SSID** and `mypassword` is **your AP's password**.

<br>

**Error example:**

```log
    [1659464425.856025][34818:34823] CHIP:DL: HandlePlatformSpecificBLEEvent 16386
    [1659464425.856035][34818:34823] CHIP:IN: Clearing BLE pending packets.
    [1659464425.856055][34818:34823] CHIP:IN: BleConnection Error: ../../examples/chip-tool/third_party/connectedhomeip/src/platform/Linux/bluez/Helper.cpp:1775: CHIP Error 0x000000AC: Internal error
```

> This error indicates that the Bluetooth connection between your system and
> laptop is failing. Follow the given procedure and then retry the ChipTool
> commissioning command.

<br>

**Procedure:**

1. Stop Bluetooth service:

    `$ systemctl stop bluetooth.service`

2. Wait 20 seconds
3. Restart Bluetooth service:

    `$ sudo service bluetooth restart`

4. Unblock Bluetooth service:

    `$ rfkill unblock all`

5. Enable Bluetooth service:

    `$ sudo systemctl enable bluetooth`

6. Issue the pairing command:

    `$ out/standalone/chip-tool pairing ble-wifi 1122 mySSID mypassword 20202021 3840`

    > Where `mySSID` is **your AP's SSID** and `mypassword` is **your AP's
    > password**.

<br> 

### 2. Unsupported certificate format Error:

<br>

> When trying to commission the system, if an `Unsupported certificate format`
> error (example below) is encountered, follow the procedure stated below.

<br>

**Error example:**

```log
[1659631352.672826][5076:5076] CHIP:TOO: Run command failure: ../../examples/chip-tool/third_party/connectedhomeip/src/controller/CHIPDeviceController.cpp:1275: CHIP Error 0x00000050: Unsupported certificate format
```

<br>

**Procedure:**

-   Delete the existing certificates on your laptop with the following command
    run from the `/connectedhomeip` directory:

    `$ /bin/rm /tmp/chip_*`

-   Issue the commissioning command

<br>

### 3. WLAN connection fails from RS9116 during commissioning when channel 13 is selected on the AP:

<br>

```c
The required channel becomes available for connection when the WLAN connection
region is configured during compilation to one that supports the channel. For
e.g. Japan for channel 13. Refer below build instructions.

Instructions :

Before building, make sure the WLAN connection region is configured correctly by reviewing/modifying the following lines under

/third_party/silabs/matter_support/wifi/rs911x/rsi_wlan_config.h

//Make sure this is set to RSI_ENABLE

#define RSI_SET_REGION_SUPPORT RSI_ENABLE

Note that the channels available for WLAN connection depend on the region selected
// Make sure this is set to 1 to configure from RSI_REGION_CODE value below

#define RSI_SET_REGION_FROM_USER_OR_BEACON 1

1: region configurations taken from user
0: region configurations taken from beacon
#define RSI_REGION_CODE 3

0 : Default Region domain
1 : US
2 : EUROPE
3 : JAPAN
```

<br>

### 4 . Incorrect firmware version

<br>

```shell
cd ./third_party/silabs/wiseconnect-wifi-bt-sdk/firmware
```

You will get appropriate firmware in the above mentioned path.

Note:

1.  How to check the current using firmware version?

    > You can find the currently using firmware version in the DUT log.

2.  How to check whether you are using correct firmware vesrsion or not?

    > Compare last 5 digits of fimware version mention in the above path with
    > the currently using firmware version.

<br>

### 5 . Homepod associated failures

<br>

`Error: if commissioning failure with error :3000001` (homepod is on the
network)

> Remove the homepod and retry to do the commissioning.

<br>

### 6 . Commissioning failure at step 18

<br>

> 1. Verify router configuration specifically related to IPV6 SLAAC, Internet
>    Group Management Protocol (IGMP) snooping.
> 2. Delete all the files of chip-tool /tmp folder. (`rm -rf /tmp/chip_*`)
> 3. Factory reset of your access point after checking the router configuration.

<br>

### 7. Commissioning failure at step 16

> Verify the access point settings, SSID, PSK, secirity type, REGION, CHANNEL.

<br>

### 8 . Inconsistent logs

> Verify external power is supplied to rs911x

<br>

### 9 . To enable different security options on AP/Router 

1. Get the router address by entering `route -n` or ifconfig of ipconfig.

2. Enter router address in the browser and enter the appropriate username and
    password.

3. Select appropriate band.

4. In security, select type(WPA/WPA2/WPA3). 

---

[Table of Contents](../README.md) | [Thread Demo](../thread/DEMO_OVERVIEW.md) |
[Wi-Fi Demo](./DEMO_OVERVIEW.md)
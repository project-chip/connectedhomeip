# Frequently Asked Questions and Troubleshooting for Matter over Wi-Fi

## Troubleshooting

<br>

### 1. Bluetooth connection fails when trying to commission the system through the ChipTool:

<br>

**Command leading to error:**

    $ out/standalone/chip-tool pairing ble-wifi 1122 mySSID myPassword 20202021 3840

> Where `mySSID` is **your AP's SSID** and `mypassword` is **your AP's
> password**.

<br>

**Error example:**

    [1659464425.856025][34818:34823] CHIP:DL: HandlePlatformSpecificBLEEvent 16386

    [1659464425.856035][34818:34823] CHIP:IN: Clearing BLE pending packets.

    [1659464425.856055][34818:34823] CHIP:IN: BleConnection Error: ../../examples/chip-tool/third_party/connectedhomeip/src/platform/Linux/bluez/Helper.cpp:1775: CHIP Error 0x000000AC: Internal error

> This error indicates that the Bluetooth connection between your system and
> laptop is failing. Follow the given procedure and then retry the ChipTool
> commissioning command.

<br>

**Procedure:**

1. Stop Bluetooth service: `$ systemctl stop bluetooth.service`
2. Wait 20 seconds
3. Restart Bluetooth service: `$ sudo service bluetooth restart`
4. Unblock Bluetooth service: `$ rfkill unblock all`
5. Enable Bluetooth service: `$ sudo systemctl enable bluetooth`
6. Issue the pairing command:
   `$ out/standalone/chip-tool pairing ble-wifi 1122 mySSID mypassword 20202021 3840`
   <br>

    > Where `mySSID` is **your AP's SSID** and `mypassword` is **your AP's
    > password**.

<br> <br>

### 2. Unsupported certificate format Error:

<br>

When trying to commission the system, if an `Unsupported certificate format`
error (example below) is encountered, follow the procedure stated below.

<br>

**Error example:**

    [1659631352.672826][5076:5076] CHIP:TOO: Run command failure: ../../examples/chip-tool/third_party/connectedhomeip/src/controller/CHIPDeviceController.cpp:1275: CHIP Error 0x00000050: Unsupported certificate format

<br>

**Procedure:**

-   Delete the existing certificates on your laptop with the following command
    run from the `/connectedhomeip` directory:

    > $ /bin/rm/tmp/chip\_\*

-   Issue the commissioning command

<br>

---

[Table of Contents](../README.md) | [Thread Demo](../thread/DEMO_OVERVIEW.md) |
[Wi-Fi Demo](./DEMO_OVERVIEW.md)

# IP commissioning

## Devices

### ESP32 (M5 stack) - all-clusters-app

The M5 doesn’t have an ethernet port, but we can simulate IP connection by
giving the device wifi credentials. The device will start up advertising as a
commissionable device.

Compile the device image as follows

```
Demo->Device Type = <as required>
Component config-> Chip Device Layer -> WiFi Station Options - set up ssid and password
```

### linux builds

```bash
gn gen out/debug
ninja -C out/debug
```

These devices start in commissioning mode if they do not have stored Matter
credentials. Linux devices store credentials in the /tmp directory, so to
re-commission the device from scratch, remove the chip\_\* files from /tmp.

## Controller

### chip-device-ctrl

There are two ways to connect via IP:

**Discover then connect ip**

```
discover -all
```

or

```
discover -qr “[qrcode]”
```

then

```
connect -ip <address from above> <setup pin code> [<node id>]
```

It is no longer necessary to run resolve after connect - this is done as part of
the commissioning process

**Connect using the QR code**

```
connect -qr “[qr code]”
```

This will discover, connect over IP with the code from the QR and resolve.

### chip-tool

You can connect using chip-tool either using the setup codes or by discovering
the device on the network using one of the mDNS sub types. To get a list of the
available options run

```
chip-tool pairing
```

For example, to discover, connect and commission using the long discriminator

```
chip-tool pairing onnetwork-long <nodeid> <setuppincode> <long-discriminator>
```

where the node id is set to a value of your choice (used to id devices for
future communication), and the setup pin code and long discriminator are set by
the device and are normally logged at startup.

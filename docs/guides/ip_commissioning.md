# IP commissioning

## Devices

### ESP32 (M5 stack) - all-clusters-app

The M5 doesn’t have an ethernet port, but we can simulate IP connection by
giving the device wifi credentials. Currently, devices with wifi credentials
assume they have been provisioned, thus it is necessary to put the device back
into commissioning mode explicitly.

Compile the device image as follows

```
Demo->Device Type = <as required>
Demo->Rendezvous Mode = “BLE / On-Network” or “On-Network”
Component config-> Chip Device Layer -> WiFi Station Options - set up ssid and password
Component config-> Chip Device Layer -> Commissioning Options -> Use full IP-based commissioning = 1
```

Demo->Rendezvous Mode = “BLE / On-Network” or “On-Network”

-   this just controls the QR code flags and is only required if you wish to
    connect using the QR code

Component config-> Chip Device Layer -> WiFi Station Options - set up ssid and
password

-   gives the device wifi access to simulate an on-network setup

Component config-> Chip Device Layer -> Commissioning Options -> Use full
IP-based commissioning = 1

-   instructs the device to wait for the commissioning complete command before
    doing rendezvous cleanup and recording

Once the device is running, go to Setup -> Force wifi commissioning to put the
device into commissioning mode.

### linux builds with an ethernet connection

```bash
gn gen out/debug --args='chip_ip_commissioning=true'
ninja -C out/debug
```

As with the M5, This will set up the QR code with the proper flags and will
instruct the device to wait for the commissioning complete command to cleanup
and record.

These devices should start in commissioning mode naturally and requires no
manual intervention

## Controller

### chip-device-ctrl

The controller builds with IP commissioning support by default, but you can turn
it on or off using

```
scripts/build_python.sh --clusters_for_ip_commissioning <true/false>
```

There are two ways to connect via IP: **Discover then connect ip**

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

This will discover, connect over IP with the code from the QR and resolve. It
does, however, require that the QR code flags are set properly.

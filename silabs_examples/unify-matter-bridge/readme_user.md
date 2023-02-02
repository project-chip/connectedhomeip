# Unify Matter Bridge User's Guide

The Unify Matter Bridge is a Unify IoT Service that enables interaction with
Unify devices from a Matter fabric. For a more thorough description see the
[Unify Matter Bridge Overview](../../silabs_examples/unify-matter-bridge/readme_overview.md).

As a prerequisite for the Matter Bridge to work, at least one Unify protocol
controller should be set up and running. This guide assumes that you have set up
the Z-Wave Protocol Controller (uic-zpc) to run on a Raspberry Pi 4 and
connected it to an MQTT broker in your network. Read the
[Unify Host SDK's Getting Started Guide](https://siliconlabs.github.io/UnifySDK/doc/getting_started.html)
for information on how to set this up.

Once a protocol controller is running, the Matter Bridge can be started.

The following documentation assumes that you have built the Unify Matter Bridge
application by following the _[Build Guide](./readme_building.md)_ and have
transferred the _`unify-matter-bridge`_ to your Raspberry Pi 4 (RPi4) running
the 64-bit version of Raspberry Pi OS Bullseye.

> **Note:**
>
> The requirements for using the Matter Bridge with the Google Home App (GHA)
> are:
>
> -   Inclusion in the
>     [Matter Early Access Program (EAP)](https://developers.home.google.com/matter/eap/).
> -   A project properly configured in the
>     [Google Home Developer Console](https://console.home.google.com/projects).
> -   Use of the specified version of the Google Home App for Android.
>
> This guide will use the CLI tool called _`chip-tool`_ for commissioning and
> device control.

- [Unify Matter Bridge User's Guide](#unify-matter-bridge-users-guide)
  - [Running the Matter Bridge](#running-the-matter-bridge)
    - [Important Configuration Settings](#important-configuration-settings)
    - [Starting the Matter Bridge](#starting-the-matter-bridge)
  - [Commissioning the Bridge to a Network](#commissioning-the-bridge-to-a-network)
    - [Using the chip-tool to Commission](#using-the-chip-tool-to-commission)
    - [Toggle an OnOff device](#toggle-an-onoff-device)
  - [Toggle a Group of OnOff Devices](#toggle-a-group-of-onoff-devices)
  - [Running the matter bridge in none specification compliant mode](#running-the-matter-bridge-in-none-specification-compliant-mode)
  - [Command Line Arguments](#command-line-arguments)

## Running the Matter Bridge

At start-up, the Matter Bridge needs to connect to the Matter Fabric as well as
the MQTT Broker. It is therefore critical that you have access to port 1883, the
default MQTT Broker's port, as well as a network setup that allows mDNS through.

A few important runtime configurations must be considered, along with some other
configuration options. A full list of command-line parameters is provided in the
[Command line arguments](#command-line-arguments) section.

### Important Configuration Settings

-   #### Network Interface

    Specify the network interface on which the Matter Fabric runs. In a regular
    RPi4 setup it would be `wlan0` for WiFi and `eth0` for ethernet. Specify
    this with the '`--interface`' argument, as such:

    ```bash
    ./unify-matter-bridge --interface eth0
    ```

-   #### Key-Value store (KVS)

    The Matter Bridge uses a Key-Value store for persisting various run-time
    configurations. Make sure to have read/write access to the default path
    '`/var/chip_unify_bridge.kvs`' or provide the path to where writing this
    data is allowed. If this file is deleted before start-up, everything is
    reset and the bridge will not belong to any Matter Fabric until it has again
    been commissioned.

    ```bash
    ./unify-matter-bridge --kvs ./matter-bridge.kvs
    ```

-   #### MQTT Host

    If you have followed the
    [Unify Host SDK's Getting Started Guide](https://siliconlabs.github.io/UnifySDK/doc/getting_started.html),
    your MQTT Broker should now be running on '`localhost`'. If you have decided
    to run the MQTT broker on a different host, you can tell the Unify Matter
    Bridge to connect to a different host.

    ```bash
    ./unify-matter-bridge --mqtt.host 10.0.0.42
    ```

-   #### Vendor and Product ID

    If you have access to the EAP and you want to use the Google Home App, you
    need to set a specific VID and PID for the Matter Bridge.

    ```bash
    ./unify-matter-bridge --vendor fff1 --product 8001
    ```

### Starting the Matter Bridge

Once the configuration parameters are set it is time to start the bridge
application.

```bash
./unify-matter-bridge --interface eth0 --kvs ./matter-bridge.kvs --mqtt.host localhost --mqtt.port 1337
```

## Commissioning the Bridge to a Network

To include the bridge in the Matter network, it must first be commissioned. The
first time the bridge starts it will automatically go into commissioning mode.
After 10 minutes the bridge will exit commissioning mode. If the bridge has not
been commissioned within this window, the application must be restarted to open
the commissioning window again or the window can be opened by writing
`commission` in the CLI when running the bridge. The commission command may also
be used for multi-fabric commissioning.

The Unify Matter Bridge uses the "On Network" commissioning method. For now,
there is no Bluetooth commissioning support.

The commissioning procedure requires use of a pairing code. This pairing code is
written to the console when running the Matter Bridge. Look for something
similar to '`MT:-24J029Q00KA0648G00`', used as the pairing code in the following
example. This code can be used when commissioning with the CLI commissioning
tool `chip-tool`.

```bash
[1659615301.367669][1967:1967] CHIP:SVR: SetupQRCode: [MT:-24J029Q00KA0648G00]
```

A simpler, more user-friendly way of commissioning is to use the Google Home App
with a QR code containing the pairing code. The QR code can be obtained by
following the link seen in the console, similar to the line below:

```bash
[1659615301.367723][1967:1967] CHIP:SVR: https://dhrishi.github.io/connectedhomeip/qrcode.html?data=MT%3A-24J029Q00KA0648G00
```

It should be noted that the commissioner **must** be on the same network as the
Raspberry Pi. Note that by default the bridge binds to the eth0 interface. If
another interface is to be used, see the description of the command line
arguments for setting [Network Interface](#network-interface).

### Using the chip-tool to Commission

In the following procedure make sure to use the pairing code taken from the
console output, as described above. To commission the Matter Bridge with the
`chip-tool` and assign the bridge the Node ID 1:

```bash
chip-tool pairing code 1 MT:-24J0AFN00KA0648G00
```

### Toggle an OnOff device

To send an OnOff cluster Toggle command to a bridged endpoint with id 2, via
Matter Fabric Node ID 1:

```bash
chip-tool onoff toggle 1 2
```

For more information on how to use the `chip-tool` see the
[chip-tool manual](../../docs/guides/chip_tool_guide.md) on the Matter website.

## Toggle a Group of OnOff Devices

The Matter Bridge has support for forwarding group messages from the Matter
Fabric to Unify Nodes. The protocol controllers will send the group messages as
actual group cast messages on the destination network (Z-Wave/Zigbee).

To send a group command, first set up the group keys in the bridge. This example
assumes the bridge to be Node ID 1, and GroupKeySetID 42 is added to Group ID 1:

```bash
chip-tool accesscontrol write acl '[{"fabricIndex": 1, "privilege": 5, "authMode": 2, "subjects": [112233], "targets": null },{"fabricIndex": 1, "privilege": 4, "authMode": 3, "subjects": [1], "targets": null }]' 1 0
chip-tool groupkeymanagement key-set-write '{"groupKeySetID": 42, "groupKeySecurityPolicy": 0, "epochKey0": "d0d1d2d3d4d5d6d7d8d9dadbdcdddedf", "epochStartTime0": 2220000,"epochKey1": "d1d1d2d3d4d5d6d7d8d9dadbdcdddedf", "epochStartTime1": 2220001,"epochKey2": "d2d1d2d3d4d5d6d7d8d9dadbdcdddedf", "epochStartTime2": 2220002 }' 1 0
chip-tool groupkeymanagement write group-key-map '[{"groupId": 1, "groupKeySetID": 42, "fabricIndex": 1}]' 1 0
```

Next, add bridge endpoint 2 to Group ID 0x0001

```bash
chip-tool groups add-group 0x0001 grp1 1 2
```

Next, program the chip-tool:

```bash
chip-tool groupsettings add-group grp1 0x0002
chip-tool groupsettings add-keysets 0x0042 0 0x000000000021dfe0 hex:d0d1d2d3d4d5d6d7d8d9dadbdcdddedf
chip-tool groupsettings bind-keyset 0x0001 0x0042
```

Finally, a multicast command may be sent using the chip-tool.

```bash
// Send actual multicast command
./chip-tool onoff toggle 0xffffffffffff0001 1
```

## Running the matter bridge in none specification compliant mode

By default, the matter bridge application only maps devices from the Unify
Controller Language protocol to the Matter protocol that complies with the
Matter specification.

If you want to try and map devices that does not necessarily conform to the
Matter specificaiton. You can run the bridge with the command line argument
`./unify-matter-bridge --spec-compliance false`.

## Command Line Arguments

The Unify Matter Bridge provides the following command line arguments:

Using _--help_ displays the following text.

```text
Usage: ./unify_matter_bridge [Options]

Options:
  --conf arg (=/etc/uic/uic.cfg)        Config file in YAML format. UIC_CONF
                                        env variable can be set to override the
                                        default config file path
  --help                                Print this help message and quit
  --dump-config                         Dump the current configuration in a
                                        YAML config file format that can be
                                        passed to the --conf option
  --version                             Print version information and quit
```

The following options can also be in a config file. Options and values passed on
the command line take precedence over the options and values in the config file.

```text
 --log.level arg (=i)                  Log Level (d,i,w,e,c)
 --log.tag_level arg                   Tag-based log level
                                       Format: <tag>:<severity>,
                                       <tag>:<severity>, ...
 --interface arg (=en0)                Ethernet interface to use
 --kvs arg (=/var/chip_unify_bridge.kvs)
                                       Matter key value store path
 --vendor arg (=65521)                 Vendor ID
 --product arg (=32769)                Product ID
 --mqtt.host arg (=localhost)          MQTT broker hostname or IP
 --mqtt.port arg (=1883)               MQTT broker port
 --mqtt.cafile arg                     Path to file containing the PEM-encoded
                                       CA certificate to connect to Mosquitto
                                       MQTT broker for TLS encryption
 --mqtt.certfile arg                   Path to file containing the PEM-encoded
                                       client certificate to connect to
                                       Mosquitto MQTT broker for TLS
                                       encryption
 --mqtt.keyfile arg                    Path to a file containing the PEM-
                                       encoded unencrypted private key for
                                       this client
 --mqtt.client_id arg (=unify_matter_bridge_71460)
                                       Set the MQTT client ID of the
                                       application.
```

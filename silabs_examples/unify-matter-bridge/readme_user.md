# Matter Bridge User's Guide

The Unify Matter Bridge is a Unify IoT Service that allows interaction with Unify devices from a Matter fabric.
For a more thorough description, have a look at the [Unify Matter Bridge Overview](../../silabs_examples/unify-matter-bridge/readme_overview.md).

As a prerequisite for the Matter Bridge to work, at least one Unify protocol controller should be set up and running.
For now we will assume that you have setup the Z-Wave Protocol Controller (uic-zpc) to run on a Raspberry Pi 4 and connected it to an MQTT broker in your network.
Read the [Unify Host SDK's Getting Started Guide](https://siliconlabs.github.io/UnifySDK/doc/getting_started.html) for information on how to set this up.

Once a protocol controller is running, the Matter Bridge can be started.

The following assumes that you have build the Unify Matter Bridge application by following the _[Build Guide](./readme_building.md)_ and have transferred the _`unify-matter-bridge`_ to your RPi4 running the 32-bit version of Debian Buster.

> **Note:**
> 
> The requirements for using the Matter Bridge with the Google Home App (GHA) are:
> 
> - Inclusion to the [Matter Early Access Program (EAP)](https://developers.home.google.com/matter/eap/) is required.
> - A project properly configured in the [Google Home Developer Console](https://console.home.google.com/projects).
> - Only a specific version of the Google Home App for Android is supported.
>
> 
> This guide will use the CLI tool called _`chip-tool`_ for commissioning and device control.

<br>

- [Running the Matter Bridge](#running-the-matter-bridge)
  - [Important configurations](#important-configurations)
- [Commissioning the bridge to a network](#commissioning-the-bridge-to-a-network)
- [Testing the bridge using the chip tool](#testing-the-bridge-using-the-chip-tool)
- [Sending a group command](#sending-a-group-command)
- [Command line arguments](#command-line-arguments)

## Running the Matter Bridge

At start up, the Matter Bridge needs to connect to the Matter Fabric as well as the MQTT Broker.
It is therefore crucial that you have access to port 1883, the default MQTT Broker's port, as well as a network setup that allows mDNS through.

There are a few important runtime configurations that needs to be considered as well as some other configuration options.
A full list of commandline parameters is listed are the [Command line arguments](#command-line-arguments) section.

### Important configurations
- #### Network Interface
  Choosing the network interface on which the Matter Fabric runs - ie. on a regular RPi4 setup it would be `wlan0` for WiFi and `eth0` for ethernet.
  Specify this with the '`--interface`' argument, as such:
  ```bash
  ./unify-matter-bridge --interface eth0
  ```

- #### Key-Value store (KVS)
  The Matter Bridge uses a Key-Value store for persisting various run-time configurations.
  Make sure to have read/write access to the default path '`/var/chip_unify_bridge.kvs`' or provide the path to where it is allowed to write this data to.
  If you delete this file prior to start up, you will reset everything and the bridge will not belong to any Matter Fabric until it has yet again been comissioned.
  ```bash
  ./unify-matter-bridge --kvs ./matter-bridge.kvs
  ```

- #### MQTT Host
  If you have followed the [Unify Host SDK's Getting Started Guide](https://siliconlabs.github.io/UnifySDK/doc/getting_started.html) to the letter, your MQTT Broker should be running on '`localhost`'.
  In the case where you have decided to run the MQTT broker on a different host, you can tell the Unify Matter Bridge to connect to a different host.
  ```bash
  ./unify-matter-bridge --mqtt.host 10.0.0.42
  ```

- #### Vendor and Product ID
  If you do have access to the EAP, and you want to use the Google Home App, you need to set a specific VID and PID for the Matter Bridge.
  This can be done in the following way:
  ```bash
  ./unify-matter-bridge --vendor fff1 --product 8001
  ```

### Starting the Matter Bridge
Now that you have decided on the configuration parameters it is time to start the bridge application.

```bash
./unify-matter-bridge --interface eth0 --kvs ./matter-bridge.kvs --mqtt.host localhost --mqtt.port 1337
```

## Commissioning the bridge to a network

For the bridge to be included into the Matter network, it needs to be commissioned.
The first time the bridge starts it will automatically go into commissioning mode, after 10 minutes the bridge will exit commissioning mode again.
If the bridge has not been commissioned within this window, the application must be restarted to open the commissioning window again or you can 
write `commission` in the CLI when running the bridge.

The Unify Matter Bridge uses the "On Network" commissioning method - for now there is no Bluetooth commissioning support.

The commisioning procedure requires a pairing code to be used.
This pairing code is written to the console when running the Matter Bridge.
Look for something similar to the following example with a pairing code of '`MT:-24J029Q00KA0648G00`':
This code can be used when commissioning with the CLI commissioning tool `chip-tool`.

```bash
[1659615301.367669][1967:1967] CHIP:SVR: SetupQRCode: [MT:-24J029Q00KA0648G00]
```

For a simpler, more userfriendly way of commmissioning, one could use the Google Home App with a QR code containing the pairing code.
The QR code can be obtained by following the link seen in the console, similar to the line below:

```bash
[1659615301.367723][1967:1967] CHIP:SVR: https://dhrishi.github.io/connectedhomeip/qrcode.html?data=MT%3A-24J029Q00KA0648G00
```

It should be noted that the commissioner **must** be on the same network as the Raspberry Pi.
Please note that by default the bridge binds to the eth0 interface.
If another interface is to be used have a look at the description of the commandline arguments for setting [Network Interface](#network-interface).

### Using the chip-tool to commission

In the following make sure to use the pairing code taken from the console output, as described above.
To commission the Matter Bridge with the `chip-tool` and assign the bridge the Node ID 1:

```bash
chip-tool pairing code 1 MT:-24J0AFN00KA0648G00
```

### Toggle an OnOff device

To send a command OnOff cluster Toggle command to a bridged endpoint with id 2, via Matter Fabric Node ID 1:
```bash
chip-tool onoff toggle 1 2
```

For further information on how to use the `chip-tool` see the [chip-tool manual](../../docs/guides/chip_tool_guide.md) on the Matter website.

## Toggle a group of OnOff devices

The Matter Bridge has support for forwarding group messages from the Matter Fabric to Unify Nodes.
The protocol controllers will send the group messages as actual group cast messages on the destination network(Z-Wave/ZigBee).

To send a group command, the group keys must first be set up in the bridge, again here the bridge is assumed to be Node ID 1, we add the GroupKeySetID 42 to Group ID 1:

```bash
chip-tool accesscontrol write acl '[{"fabricIndex": 1, "privilege": 5, "authMode": 2, "subjects": [112233], "targets": null },{"fabricIndex": 1, "privilege": 4, "authMode": 3, "subjects": [1], "targets": null }]' 1 0
chip-tool groupkeymanagement key-set-write '{"groupKeySetID": 42, "groupKeySecurityPolicy": 0, "epochKey0": "d0d1d2d3d4d5d6d7d8d9dadbdcdddedf", "epochStartTime0": 2220000,"epochKey1": "d1d1d2d3d4d5d6d7d8d9dadbdcdddedf", "epochStartTime1": 2220001,"epochKey2": "d2d1d2d3d4d5d6d7d8d9dadbdcdddedf", "epochStartTime2": 2220002 }' 1 0
chip-tool groupkeymanagement write group-key-map '[{"groupId": 1, "groupKeySetID": 42, "fabricIndex": 1}]' 1 0
```

Now we can add bridge endpoint 2 to Group ID 0x0001

```bash
chip-tool groups add-group 0x0001 grp1 1 2
```

Now we need to program the chip-tool:

```bash
chip-tool groupsettings add-group grp1 0x0002
chip-tool groupsettings add-keysets 0x0042 0 0x000000000021dfe0 hex:d0d1d2d3d4d5d6d7d8d9dadbdcdddedf
chip-tool groupsettings bind-keyset 0x0001 0x0042
```

As Matter is based on IPv6 and the group message will be sent as a multicast message,
we need to set up a multicast route to tell the Linux kernel what interface the
group message needs to be sent to.

```bash
sudo route add -6 ff35:40:fd00::/24 dev eth0
```

Finally, a multicast command may be sent using the chip-tool

```bash
// Send actual multicast command
./chip-tool onoff toggle 0xffffffffffff0001 1
```

## Command line arguments

The Unify Matter Bridge provides the following command line arguments:

Using the _--help_ the following help text appear.

```bash
Usage: ./unify_matter_bridge [Options]

Options:
  --conf arg (=/etc/uic/uic.cfg)        Config file in YAML format. UIC_CONF
                                        env variable can be set to override the
                                        default config file path
  --help                                Print this help message and quit
  --dump-config                         Dumps the current configuration on a
                                        YAML config file format that can be
                                        passed to --conf option
  --version                             Print version information and quit

Following options can also be in a Config file.
 Options and values passed on command line here take precedence over the options and values in config file:
  --log.level arg (=i)                  Log Level (d,i,w,e,c)
  --log.tag_level arg                   Tag based log level
                                        Format: <tag>:<severity>, 
                                        <tag>:<severity>, ...
  --interface arg (=en0)                Ethernet interface to use
  --kvs arg (=/var/chip_unify_bridge.kvs)
                                        Matter key value store path
  --vendor arg (=65521)                 Vendor ID
  --product arg (=32769)                Product ID
  --mqtt.host arg (=localhost)          MQTT broker hostname or IP
  --mqtt.port arg (=1883)               MQTT broker port
  --mqtt.cafile arg                     Path to file containing the PEM encoded
                                        CA certificate to connect to Mosquitto
                                        MQTT broker for TLS encryption
  --mqtt.certfile arg                   Path to file containing the PEM encoded
                                        client certificate to connect to
                                        Mosquitto MQTT broker for TLS
                                        encryption
  --mqtt.keyfile arg                    Path to a file containing the PEM
                                        encoded unencrypted private key for
                                        this client
  --mqtt.client_id arg (=unify_matter_bridge_71460)
                                        Set the MQTT client ID of the
                                        application.
```

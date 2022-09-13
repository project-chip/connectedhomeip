# Matter Bridge User's Guide

This guide describes how to use the unify matter bridge.

The unify matter bridge is a Unify IoT service that allows control of Unify
devices from a matter fabric. The bridge translates matter cluster commands and
attributes accesses into the corresponding Unify MQTT publish messages. The
bridge also caches the state of Unify node attributes and make those attribute
readable on the matter fabric.

## Command line arguments

Using the _--help_ the following help text appear

```bash
Usage: ./applications/matter_bridge/unify_matter_bridge [Options]

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
  --kvs arg (=/tmp/chip_unify_bridge.kvs)
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

## How it works

The Unify Matter Bridge acts as a Unify IoT service. The Unify data model
is largely based on the same data model as Matter, making the job of the Unify
Matter Bridge relatively simple. There is almost a 1-1 correspondence between
the Matter commands and attributes and the Unify command and attributes.

## Using the matter bridge

As a prerequisite for the matter bridge to work at least one Unify protocol
controller should be set up and running. Read the [Unify User Guide](../../doc/user_guide.md)
for information on how to set this up.

Once a protocol controller is running on the system the matter bridge can be
started, by executing the following command

```bash
sudo systemctl start uic-matter-bridge
````

To monitor the log output of the bridge use this command:

```bash
journalctl -f -u uic-matter-bridge
```

### Commissioning the bridge to a network

The first time the bridge starts it will automatically go into commissioning mode, after
10 minutes the bridge will exit commissioning mode again. If the bridge has not been
commissioned within this window the application must be restarted to open the commissioning window again.

The Unify Matter Bridge uses the "On Network" commissioning method, ie there is no
Bluetooth or WiFI involved.

To obtain the QR commissioning code run the following command:

```bash
journalctl  -u uic-matter-bridge | grep qrcode | tail -1
Aug 04 14:15:01 raspberrypi unify_matter_bridge[1967]: [1659615301.367723][1967:1967] CHIP:SVR: https://dhrishi.github.io/connectedhomeip/qrcode.html?data=MT%3A-24J029Q00KA0648G00
```

Open the printed link in a browser to get the QR code. It should be noted that the commissioner
must be on the same network as the raspberry pi. Also, note that by default the bridge binds
to the eth0 interface. If another interface is to be used this must be set using the
`--interface` command argument.

In case of commission with the matter chip tool, the raw QR code string should be obtained:

```bash
journalctl  -u uic-matter-bridge | grep QRCode | tail -1
Aug 04 14:15:01 raspberrypi unify_matter_bridge[1967]: [1659615301.367669][1967:1967] CHIP:SVR: SetupQRCode: [MT:-24J029Q00KA0648G00]
```

### Testing the bridge using the chip tool

To commission the matter bridge with the `chip-tool` and assign the bridge node id 1
run the following command:

```bash
chip-tool pairing qrcode 1 MT:-24J0AFN00KA0648G00
```

To send a command OnOff cluster Toggle command to an endpoint on the bridge

```bash
chip-tool onoff toggle 1 2
```

Here the bridge is node matter fabric node id 1 and the bridged endpoint is 2

For further information on how to use the `chip-tool` see the [chip-tool manual](https://github.com/project-chip/connectedhomeip/blob/master/docs/guides/chip_tool_guide.md)
on the Matter website.

### Sending a group command

The matter bridge has support for forwarding group messages from the matter fabric to Unify
Nodes. The protocol controllers will send the group messages as actual group cast messages
on the destination network(Z-Wave/ZigBee).

To send a group command, the group keys must first be set up in the bridge, again here
the bridge is assumed to be node id 1, we add the keyset id 42 to group id 1:

```bash
chip-tool accesscontrol write acl '[{"fabricIndex": 1, "privilege": 5, "authMode": 2, "subjects": [112233], "targets": null },{"fabricIndex": 1, "privilege": 4, "authMode": 3, "subjects": [1], "targets": null }]' 1 0
chip-tool groupkeymanagement key-set-write '{"groupKeySetID": 42, "groupKeySecurityPolicy": 0, "epochKey0": "d0d1d2d3d4d5d6d7d8d9dadbdcdddedf", "epochStartTime0": 2220000,"epochKey1": "d1d1d2d3d4d5d6d7d8d9dadbdcdddedf", "epochStartTime1": 2220001,"epochKey2": "d2d1d2d3d4d5d6d7d8d9dadbdcdddedf", "epochStartTime2": 2220002 }' 1 0
chip-tool groupkeymanagement write group-key-map '[{"groupId": 1, "groupKeySetID": 42, "fabricIndex": 1}]' 1 0
```

Now we can add bridge endpoint 2 to group id 0x0001

```bash
chip-tool groups add-group 0x0001 grp1 1 2
```

Now we need to program the chip tool:

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

## Supported Clusters

The Unify Matter bridge currently supports the mapping of the following clusters

| Cluster             |
|---------------------|
| Bridged Device Info |
| Group               |
| Identify            |
| Level               |
| OnOff               |

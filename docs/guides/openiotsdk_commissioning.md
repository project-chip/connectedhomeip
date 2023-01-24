# Commissioning Open IoT SDK devices

Matter devices based on Open IoT SDK reset into a ready for commissioning state.
This allows a controller to connect to them and set configuration options before
the device becomes available on the Matter network.

Open IoT SDK Matter devices, due to the connectivity setup, start already
connected to the IP network and do not require credentials provisioning.

## Building and installing the Python Device Controller

To make provisioning possible and to control the Matter device with a Python
application, you can build and run the Python controller application. Please
read the guide
[Python Device Controller guide](python_chip_controller_building.md) for further
instructions.

## Device commissioning

Run chip-device-ctrl and use the interactive prompt to commission the device.

After the device boots, it's in ready for commissioning mode and starts the mDNS
advertisement. This can be discovered by the controller using:

```
discover -all
```

This will list the devices and their addresses. To commission the device use:

```
connect -ip <address from above> <setup pin code> [<node id>]
```

The setup pin code is printed in the log of the device. The `<node id>` can be
chosen by the user, if left blank it will be automatically picked.

## Sending ZCL commands

If the commissioning process was successful, it is possible to send a ZCL
command to the device which initiates a certain action.

`zcl <Cluster> <Command> <NodeId> <EndpointId> <GroupId> [arguments]`

Example:

    chip-device-ctrl > zcl LevelControl MoveWithOnOff 12344321 1 0 moveMode=1 rate=2

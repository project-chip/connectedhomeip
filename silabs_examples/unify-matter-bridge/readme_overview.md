# Unify Matter Bridge Overview

_[The Unify Host SDK](https://github.com/SiliconLabs/UnifySDK) provides the fastest path for bridging Matter and other protocols as well as a development environment for connecting to ecosystems such as Google Home._ - [https://www.silabs.com/wireless/matter]

"_The Unify Framework provides software source code and binary packages for Raspberry Pi 4 to help build an IoT gateway product.
The Unify Framework enables an IoT service to control and manage end nodes in several wireless PHY radios (e.g., Z-Wave, Zigbee, and so on) supported by Silicon Labs.
The Unify Framework uses the Unify Controller Language (UCL) as an internal abstraction layer to seamlessly control various end nodes that are enabled with multiple wireless PHY radios.
The detailed architecture of the Unify Framework is described in the Unify Framework Specification._...<br>
...
_A Unify gateway consists of a Message Queuing Telemetry Transport (MQTT) broker and a number of MQTT clients. The Unify Framework uses the Mosquitto MQTT broker."_ - [https://siliconlabs.github.io/UnifySDK/doc/UnifySDK.html].

## Matter Bridge as an IoT Service

The Unify Matter Bridge is an Unify IoT Service that allows for control of Unify devices from a Matter fabric.
It translates Matter cluster commands and attributes accesses into the corresponding Unify MQTT publish messages.
Unify node attributes are readable from the Matter Fabric, eg. in your Google Home App, as the Unify Matter Bridge also caches the state of those attributes.

The Unify data model is largely based on the same data model as Matter, making the job of the Unify Matter Bridge relatively simple.
There is almost a 1-1 relationship between the Matter commands and attributes and the Unify command and attributes.

> **Note:** Currently there is no control going the other way, that is, the Unify Matter Bridge can not '_see_' what else is on the Matter Fabric, let alone control end devices in the Matter Fabric.

See the [release notes for the Unify Matter Bridge](release_notes.md) for details on feature additions, bug fixes and known issues.

## Supported Clusters/Devices

The Unify Matter bridge currently supports the mapping of the following clusters/device types.

| Cluster             |
|---------------------|
| Bridged Device Info |
| Group               |
| Identify            |
| Level               |
| OnOff               |

## Further Reading

- [Unify Host SDK Documentation](https://siliconlabs.github.io/UnifySDK/doc/UnifySDK.html)
- [Building the Matter Bridge](./readme_building.md)
- [Getting Started](./readme_user.md#running-the-matter-bridge)
- [Control a Z-Wave OnOff device](./readme_user.md#toggle-an-onoff-device)
- [Toggle a group of OnOff devices](./readme_user.md#toggle-a-group-of-onoff-devices)

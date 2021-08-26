# Quick Start

## Demo Overview

The CHIP reference implementation contains support for a number examples and
platforms.

## WiFi Nodes

| <div style="width:200px">Controller / Admin</div>                                                                                                                                     | <div style="width:200px">Node</div>                                                                                                                                                                                                                                 | Description                                                                                                                                                                                                                                                                                                                                                                                                                             |
| ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| [**chip-tool**](https://github.com/project-chip/connectedhomeip/blob/master/examples/chip-tool/README.md) (Linux / Mac)<br> Includes docs for all the cluster commands supported <br> | **all-clusters-app** <li> [M5Stack](https://github.com/project-chip/connectedhomeip/blob/master/examples/all-clusters-app/esp32/README.md) (ESP)<li>[Linux](https://github.com/project-chip/connectedhomeip/tree/master/examples/all-clusters-app/linux) simulation | Use the command line tool on a laptop to pair with and control an embedded WiFi platform. This demo supports the “all-clusters-app”, so it provides the basic onoff light test and more. Two incremental modes of operation are supported for testing (configurable via Kconfig tool when building M5): <ol><li> Unsecured (WiFi only) <br> BLE Rendezvous Mode Bypass = 1 <li> PASE secured <br> (BLE pairing, WiFi operational) </ol> |
| [**chip-device-ctrl.py**](https://github.com/project-chip/connectedhomeip/blob/master/src/controller/python/README.md)                                                                | **all-clusters-app**<li> [M5Stack](https://github.com/project-chip/connectedhomeip/blob/master/examples/all-clusters-app/esp32/README.md) (ESP)<li> [Linux](https://github.com/project-chip/connectedhomeip/tree/master/examples/all-clusters-app/linux) simulation | Same as above, but uses the pychip tool as Controller Node, and focuses on BLE flow rather than Bypass.                                                                                                                                                                                                                                                                                                                                 |

## Thread Nodes

Use one of the controllers listed above and then a Border Router and Node
combination listed below.

| <div style="width:200px">Border Router</div>                                                                   | <div style="width:200px">Node</div>                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                            | Description                                                                                                                                                         |
| -------------------------------------------------------------------------------------------------------------- | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| [**ot-br**](https://openthread.io/guides/border-router/build)<br>Thread Border Router <li>RasPi <li>BeagleBone | **lighting-app** <li>[Nordic nRF5x](https://github.com/project-chip/connectedhomeip/tree/master/examples/lighting-app/nrfconnect/README.md) <li> [NXP K32W](https://github.com/project-chip/connectedhomeip/tree/master/examples/lighting-app/k32w/README.md) <li> [Qorvo QPG6100](https://github.com/project-chip/connectedhomeip/tree/master/examples/lighting-app/qpg) <li> [Silicon Labs EFR32](https://github.com/project-chip/connectedhomeip/tree/master/examples/lighting-app/efr32/README.md)                                                                                                         | The Lighting example is supported by many of the available Thread platforms. See the chip-tool controller instructions for how to actuate the light on/off cluster. |
| [**ot-br**](https://openthread.io/guides/border-router/build)<br>Thread Border Router <li>RasPi <li>BeagleBone | **lock-app** <li>[Nordic nRF5x](https://github.com/project-chip/connectedhomeip/tree/master/examples/lock-app/nrfconnect/README.md) <li> [NXP K32W](https://github.com/project-chip/connectedhomeip/tree/master/examples/lock-app/k32w/README.md) <li> [Qorvo QPG6100](https://github.com/project-chip/connectedhomeip/tree/master/examples/lock-app/qpg) <li> [Silicon Labs EFR32](https://github.com/project-chip/connectedhomeip/tree/master/examples/lock-app/efr32/README.md) <li> [TI CC13x2x7](https://github.com/project-chip/connectedhomeip/tree/master/examples/lock-app/cc13x2x7_26x2x7/README.md) | The Lock example is supported by many of the available Thread and WiFi platforms.                                                                                   |

## Controllers

### chip-tool

This section summarizes how to run some common scenarios with the
[**chip-tool**](https://github.com/project-chip/connectedhomeip/blob/master/examples/chip-tool/README.md)
controller.

#### IP Pairing

`chip-tool pairing onnetwork 0 20202021 3840 ::1 5540` will use PASE over IP to
pair a device.

NOTE: to run both the Node and Controller as separate processes on the same
Linux or Mac machine, build the all-clusters-app with BLE disabled as follows:

`scripts/examples/gn_build_example.sh examples/all-clusters-app/linux out/debug/standalone/ chip_config_network_layer_ble=false`

#### Automated CASE tests

`chip-tool tests Test_TC_OO_1_1` will run a suite of tests that use CASE To
communicate with a paired `all-clusters-app` peer node.

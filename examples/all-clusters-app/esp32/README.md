# Matter ESP32 All Clusters Example

A prototype application that demonstrates device commissioning and cluster
control.

Please
[setup ESP-IDF and CHIP Environment](../../../docs/guides/esp32/setup_idf_chip.md)
and refer
[building and commissioning](../../../docs/guides/esp32/build_app_and_commission.md)
guides to get started.

---

-   [Cluster control](#cluster-control)
-   [Matter OTA guide](../../../docs/guides/esp32/ota.md)
-   [RPC console and Device Tracing](../../../docs/guides/esp32/rpc_console.md)
-   [Multiple Network Interfaces](#multiple-network-interfaces)

---

### Cluster control

#### onoff

To use the Client to send Matter commands, run the built executable and pass it
the target cluster name, the target command name as well as an endpoint id.

```
$ ./out/debug/chip-tool onoff on <NODE ID> <ENDPOINT>
```

The client will send a single command packet and then exit.

#### levelcontrol

```bash
Usage:
  $ ./out/debug/chip-tool levelcontrol move-to-level Level=10 TransitionTime=0 OptionMask=0 OptionOverride=0 <NODE ID> <ENDPOINT>
```

### Multiple Network Interfaces

The data model of this example includes a secondary NetworkCommissioning
Endpoint with another NetworkCommissioning cluster. The Endpoint Id for the
secondary NetworkCommissioning Endpoint is 65534. The secondary
NetworkCommissioning Endpoint can be used to manage the driver of extra network
interface.

For ESP32-C6 DevKits, if `CHIP_DEVICE_CONFIG_ENABLE_WIFI` and
`CHIP_DEVICE_CONFIG_ENABLE_THREAD` are both enabled, please set
`CONFIG_THREAD_NETWORK_ENDPOINT_ID` to 0 and set
`CONFIG_WIFI_NETWORK_ENDPOINT_ID` to 65534, which presents that the
NetworkCommissioning cluster in Endpoint 0 will be used for Thread network
driver and the same cluster on Endpoint 65534 will be used for Wi-Fi network
driver. Or vice versa.

For ESP32-Ethernet-Kits, if `CHIP_DEVICE_CONFIG_ENABLE_WIFI` and
`CHIP_DEVICE_CONFIG_ENABLE_ETHERNET` are both enabled, please set
`CONFIG_ETHERNET_NETWORK_ENDPOINT_ID` to 0 and set
`CONFIG_WIFI_NETWORK_ENDPOINT_ID` to 65534, which presents that the
NetworkCommissioning cluster in Endpoint 0 will be used for Ethernet network
driver and the same cluster on Endpoint 65534 will be used for Wi-Fi network
driver. Or vice versa.

---

This demo app illustrates controlling OnOff cluster (Server) attributes of an
endpoint. For `ESP32-DevKitC`, `ESP32-WROVER-KIT_V4.1` and `ESP32C3-DevKitM`, a
GPIO (configurable through `STATUS_LED_GPIO_NUM` in `main/main.cpp`) is updated
through the on/off/toggle commands from the `python-controller`. For `M5Stack`,
a virtual Green LED on the display is used for the same.

If you wish to see the actual effect of the commands on `ESP32-DevKitC`,
`ESP32-WROVER-KIT_V4.1`, you will have to connect an external LED to GPIO
`STATUS_LED_GPIO_NUM`. For `ESP32C3-DevKitM`, the on-board LED will show the
actual effect of the commands.

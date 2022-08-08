# CHIP ESP32 All Clusters Example

A prototype application that demonstrates device commissioning and cluster
control.

Refer to
[setup ESP-IDF and CHIP Environment](#setup-esp-idf-and-chip-environment) and
[Build, Flash, Monitor, and Commissioning](#build-flash-monitor-and-commissioning)
guides to get started.

---

-   [Cluster control](#cluster-control)
-   [Note](#note)
-   Please refer [Matter OTA guide](../../../docs/guides/esp32/ota.md) for
    trying out OTA
-   Please refer
    [RPC console and Device Tracing](../../../docs/guides/esp32/rpc_console.md)
    for using RPC and device tracing

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

### Note

This demo app illustrates controlling OnOff cluster (Server) attributes of an
endpoint. For `ESP32-DevKitC`, `ESP32-WROVER-KIT_V4.1` and `ESP32C3-DevKitM`, a
GPIO (configurable through `STATUS_LED_GPIO_NUM` in `main/main.cpp`) is updated
through the on/off/toggle commands from the `python-controller`. For `M5Stack`,
a virtual Green LED on the display is used for the same.

If you wish to see the actual effect of the commands on `ESP32-DevKitC`,
`ESP32-WROVER-KIT_V4.1`, you will have to connect an external LED to GPIO
`STATUS_LED_GPIO_NUM`. For `ESP32C3-DevKitM`, the on-board LED will show the
actual effect of the commands.

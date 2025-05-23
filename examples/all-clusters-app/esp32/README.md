# Matter ESP32 All Clusters Example

A prototype application that demonstrates device commissioning and cluster
control.

Please
[setup ESP-IDF and CHIP Environment](../../../docs/platforms/esp32/setup_idf_chip.md)
and refer
[building and commissioning](../../../docs/platforms/esp32/build_app_and_commission.md)
guides to get started.

---

-   [Cluster control](#cluster-control)
-   [Matter OTA guide](../../../docs/platforms/esp32/ota.md)
-   [RPC console and Device Tracing](../../../docs/platforms/esp32/rpc_console.md)
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

### Matter Stack Shutdown and Restart

This document describes how to cleanly shutdown and restart the Matter stack on ESP32.
Shutting down the Matter stack typically frees up approximately 100 KB of heap memory.

To use the Matter shutdown and restart functionality, make sure the following configs are disabled in `menuconfig`.
```
CONFIG_ENABLE_CHIP_SHELL=n
CONFIG_USE_BLE_ONLY_FOR_COMMISSIONING=n #(if the device is not commissioned)
```

#### Matter Stack Shutdown Flow
The code snippet below shows the shutdown flow for the Matter stack:
```
static void Shutdown(intptr_t context)
{
    {
        DeviceLayer::StackLock lock;
        Esp32AppServer::Shutdown();  // Shutdown Matter AppServer
    }

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
    if (DeviceLayer::Internal::ESP32Utils::ShutdownWiFiStack() != CHIP_NO_ERROR)
    {
        ESP_LOGE(TAG, "Failed to shutdown the Wi-Fi stack");
        return;
    }
#endif

    ESPOpenThreadShutdown();         // Shutdown OpenThread (if used)
    GetAppTask().StopAppTask();      // Stop the application task

    CHIPDeviceManager & deviceMgr = CHIPDeviceManager::GetInstance();
    CHIP_ERROR error = deviceMgr.Shutdown(&EchoCallbacks);  // Shutdown CHIPDeviceManager
    if (error != CHIP_NO_ERROR)
    {
        ESP_LOGE(TAG, "CHIPDeviceManager Shutdown Failed: %" CHIP_ERROR_FORMAT, error.Format());
        return;
    }

#if CONFIG_ENABLE_CHIP_SHELL
    chip::StopShell();               // Stop the Matter shell (if enabled)
#endif
}
```
We do not deinitialize `esp_event_loop` and `nvs_flash` during shutdown, as they are still required for core ESP32 functionality.

#### Restarting the Matter Stack

To restart the Matter stack, perform the same sequence of initializations as done in `app_main()`. This includes:

Initializing the Wi-Fi stack (if enabled)
Reinitializing the Matter stack (via CHIPDeviceManager::Init())
Starting the application task
Initializing OpenThread (if used)
Scheduling the Matter server initialization with PlatformMgr().ScheduleWork()

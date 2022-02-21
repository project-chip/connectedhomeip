#pragma once

#include "BluetoothWidget.h"
#include "Button.h"
#include "CHIPDeviceManager.h"
#include "DeviceCallbacks.h"
#include "Display.h"
#include "Globals.h"
#include "LEDWidget.h"
#include "ListScreen.h"
#include "OpenThreadLaunch.h"
#include "QRCodeScreen.h"
#include "ScreenManager.h"
#include "ShellCommands.h"
#include "StatusScreen.h"
#include "WiFiWidget.h"
#include "esp_heap_caps_init.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_spi_flash.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "platform/PlatformManager.h"
#include "shell_extension/launch.h"

#include <cmath>
#include <cstdio>
#include <ctype.h>
#include <string>
#include <vector>

#include <app-common/zap-generated/att-storage.h>
#include <app-common/zap-generated/attribute-id.h>
#include <app-common/zap-generated/attribute-type.h>
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-id.h>
#include <app/clusters/door-lock-server/door-lock-server.h>
#include <app/clusters/network-commissioning/network-commissioning.h>
#include <app/clusters/on-off-server/on-off-server.h>
#include <app/clusters/ota-requestor/BDXDownloader.h>
#include <app/clusters/ota-requestor/OTARequestor.h>
#include <app/server/AppDelegate.h>
#include <app/server/Dnssd.h>
#include <app/server/OnboardingCodesUtil.h>
#include <app/server/Server.h>
#include <app/util/af-types.h>
#include <app/util/af.h>
#include <binding-handler.h>
#include <credentials/DeviceAttestationCredsProvider.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>
#include <lib/shell/Engine.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/ErrorStr.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/ESP32/NetworkCommissioningDriver.h>
#include <platform/ESP32/OTAImageProcessorImpl.h>
#include <platform/GenericOTARequestorDriver.h>
#include <setup_payload/ManualSetupPayloadGenerator.h>
#include <setup_payload/QRCodeSetupPayloadGenerator.h>

#if CONFIG_DEVICE_TYPE_M5STACK
#define BUTTON_1_GPIO_NUM ((gpio_num_t) 39)                           // Left button on M5Stack
#define BUTTON_2_GPIO_NUM ((gpio_num_t) 38)                           // Middle button on M5Stack
#define BUTTON_3_GPIO_NUM ((gpio_num_t) 37)                           // Right button on M5Stack
#define STATUS_LED_GPIO_NUM ((gpio_num_t) CONFIG_STATUS_LED_GPIO_NUM) // No status LED on M5Stack

void SetupPretendDevices();
void InitM5Stack(std::string qrCodeText);
#endif

#if CONFIG_HAVE_DISPLAY
void InitDeviceDisplay();
#endif

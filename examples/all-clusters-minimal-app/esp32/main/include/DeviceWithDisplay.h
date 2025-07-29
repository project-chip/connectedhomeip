/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
 *    All rights reserved.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#pragma once

#include "Button.h"
#include "DeviceCallbacks.h"
#include "Globals.h"
#if CONFIG_HAVE_DISPLAY
#include "Display.h"
#include "ListScreen.h"
#include "QRCodeScreen.h"
#include "ScreenManager.h"
#include "StatusScreen.h"
#endif
#include "esp_check.h"
#include "esp_log.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string>
#include <vector>

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/clusters/door-lock-server/door-lock-server.h>
#include <app/server/Dnssd.h>
#include <app/server/Server.h>
#include <setup_payload/OnboardingCodesUtil.h>

#if CONFIG_DEVICE_TYPE_M5STACK
#define BUTTON_1_GPIO_NUM ((gpio_num_t) 39) // Left button on M5Stack
#define BUTTON_2_GPIO_NUM ((gpio_num_t) 38) // Middle button on M5Stack
#define BUTTON_3_GPIO_NUM ((gpio_num_t) 37) // Right button on M5Stack

void SetupPretendDevices();
esp_err_t InitM5Stack(std::string qrCodeText);
#endif

#if CONFIG_HAVE_DISPLAY
void InitDeviceDisplay();
#endif

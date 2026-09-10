/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include "DeviceDisplay.h"
#include "Button.h"
#include "DeviceInfoScreen.h"
#include "DeviceSelectionScreen.h"
#include "Display.h"
#include "ListScreen.h"
#include "QRCodeScreen.h"
#include "ScreenManager.h"
#include <esp_log.h>
#include <lib/support/CHIPMem.h>
#include <setup_payload/OnboardingCodesUtil.h>
#include <setup_payload/QRCodeSetupPayloadGenerator.h>

#include <app/server/Server.h>
#include <platform/PlatformManager.h>

static const char TAG[] = "DeviceDisplay";

Button gButtons[BUTTON_NUMBER] = { Button(BUTTON_1_GPIO_NUM), Button(BUTTON_2_GPIO_NUM), Button(BUTTON_3_GPIO_NUM) };

void PushFactoryResetScreen()
{
    auto * model = chip::Platform::New<SimpleListModel>();
    if (model == nullptr)
    {
        return;
    }
    model->Title("Factory Reset?")
        ->Item("< Cancel", []() { ScreenManager::PopScreen(); })
        ->Item("No", []() { ScreenManager::PopScreen(); })
        ->Item("Yes, Reset", []() {
            ESP_LOGI(TAG, "Factory reset requested from UI, resetting...");
            LogErrorOnFailure(chip::DeviceLayer::PlatformMgr().ScheduleWork(
                [](intptr_t) { chip::Server::GetInstance().ScheduleFactoryReset(); }));
        });

    auto * screen = chip::Platform::New<ListScreen>(model);
    if (screen == nullptr)
    {
        chip::Platform::Delete(model);
        return;
    }
    ScreenManager::PushScreen(screen);
}

void InitDeviceDisplay()
{
    // Create buffer for QR code that can fit max size and null terminator.
    char qrCodeBuffer[chip::QRCodeBasicSetupPayloadGenerator::kMaxQRCodeBase38RepresentationLength + 1];
    chip::MutableCharSpan qrCodeText(qrCodeBuffer);

    CHIP_ERROR chipErr = GetQRCode(qrCodeText, chip::RendezvousInformationFlags(CONFIG_RENDEZVOUS_MODE));
    if (chipErr != CHIP_NO_ERROR)
    {
        ESP_LOGE(TAG, "GetQRCode() failed: %" CHIP_ERROR_FORMAT, chipErr.Format());
        return;
    }

    esp_err_t err = InitDisplay();
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "InitDisplay() failed: %s", esp_err_to_name(err));
        return;
    }

    ScreenManager::Init();

#if CONFIG_DEVICE_TYPE_M5STACK
    err = gpio_install_isr_service(0);
    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE)
    {
        ESP_LOGE(TAG, "gpio_install_isr_service failed: %s", esp_err_to_name(err));
    }
    for (int i = 0; i < BUTTON_NUMBER; ++i)
    {
        gButtons[i].Init();
    }
#endif // CONFIG_DEVICE_TYPE_M5STACK

    ESP_LOGI(TAG, "Initializing screen hierarchy");

    auto * rootModel = chip::Platform::New<SimpleListModel>();
    if (rootModel != nullptr)
    {
        rootModel->Title("All Devices")
            ->Item("Status & Info", []() { PushStatusOrQRCodeScreen(); })
            ->Item("Select Device", []() { PushDeviceSelectionScreen(); })
            ->Item("Factory Reset", []() { PushFactoryResetScreen(); });

        auto * rootScreen = chip::Platform::New<ListScreen>(rootModel);
        if (rootScreen != nullptr)
        {
            ScreenManager::PushScreen(rootScreen);
        }
        else
        {
            chip::Platform::Delete(rootModel);
        }
    }

    // Push QR Code screen initially so it is visible immediately on boot
    auto * qrScreen = chip::Platform::New<QRCodeScreen>(qrCodeText.data());
    if (qrScreen != nullptr)
    {
        ScreenManager::PushScreen(qrScreen);
    }
}

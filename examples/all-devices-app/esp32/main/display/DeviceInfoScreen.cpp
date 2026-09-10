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

#include "DeviceInfoScreen.h"

#if CONFIG_HAVE_DISPLAY

#include "QRCodeScreen.h"
#include "ScreenManager.h"
#include <app/server/Server.h>
#include <esp_heap_caps.h>
#include <esp_netif.h>
#include <lib/support/CHIPMem.h>
#include <setup_payload/OnboardingCodesUtil.h>
#include <setup_payload/QRCodeSetupPayloadGenerator.h>

const std::string & GetActiveDeviceType();

namespace {

void PushQRCodeScreenHelper()
{
    char qrCodeBuffer[chip::QRCodeBasicSetupPayloadGenerator::kMaxQRCodeBase38RepresentationLength + 1];
    chip::MutableCharSpan qrCodeText(qrCodeBuffer);
    CHIP_ERROR err = GetQRCode(qrCodeText, chip::RendezvousInformationFlags(CONFIG_RENDEZVOUS_MODE));
    if (err == CHIP_NO_ERROR)
    {
        ScreenManager::PushScreen(chip::Platform::New<QRCodeScreen>(qrCodeText.data()));
    }
}

} // namespace

DeviceInfoListModel::DeviceInfoListModel()
{
    mItems.push_back({ "< Back", []() { ScreenManager::PopScreen(); } });

    auto & fabricTable  = chip::Server::GetInstance().GetFabricTable();
    uint8_t fabricCount = fabricTable.FabricCount();
    if (fabricCount > 0)
    {
        mItems.push_back({ "Commissioned: Yes (" + std::to_string(fabricCount) + " fab)", nullptr });
    }
    else
    {
        mItems.push_back({ "Commissioned: No", nullptr });
    }

    esp_netif_t * netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
    esp_netif_ip_info_t ipInfo;
    if (netif != nullptr && esp_netif_get_ip_info(netif, &ipInfo) == ESP_OK && ipInfo.ip.addr != 0)
    {
        char ipStr[32];
        snprintf(ipStr, sizeof(ipStr), "IP: " IPSTR, IP2STR(&ipInfo.ip));
        mItems.push_back({ std::string(ipStr), nullptr });
    }
    else
    {
        mItems.push_back({ "IP: Not Connected", nullptr });
    }

    mItems.push_back({ "Dev: " + GetActiveDeviceType(), nullptr });

    unsigned int freeHeapKb = heap_caps_get_free_size(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL) / 1024;
    mItems.push_back({ "RAM: " + std::to_string(freeHeapKb) + " KB free", nullptr });

    mItems.push_back({ "View QR Code", []() { PushQRCodeScreenHelper(); } });
}

std::string DeviceInfoListModel::GetItemText(int i)
{
    if (i >= 0 && i < static_cast<int>(mItems.size()))
    {
        return mItems[i].text;
    }
    return "";
}

void DeviceInfoListModel::ItemAction(int i)
{
    if (i >= 0 && i < static_cast<int>(mItems.size()) && mItems[i].action)
    {
        mItems[i].action();
    }
}

void PushStatusOrQRCodeScreen()
{
    auto & fabricTable = chip::Server::GetInstance().GetFabricTable();
    if (fabricTable.FabricCount() == 0)
    {
        PushQRCodeScreenHelper();
    }
    else
    {
        ScreenManager::PushScreen(chip::Platform::New<ListScreen>(chip::Platform::New<DeviceInfoListModel>()));
    }
}

#endif // CONFIG_HAVE_DISPLAY

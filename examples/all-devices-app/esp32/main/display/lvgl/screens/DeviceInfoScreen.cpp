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
#include "DeviceTypeSelection.h"

#include <app/server/Server.h>
#include <esp_heap_caps.h>
#include <esp_netif.h>

#include <cstdio>
#include <string>

namespace {

void OnRefreshClicked(lv_event_t * event)
{
    auto * parent = static_cast<lv_obj_t *>(lv_event_get_user_data(event));
    if (parent != nullptr)
    {
        UpdateDeviceInfo(parent);
    }
}

} // namespace

void ShowDeviceInfo(lv_obj_t * parent)
{
    lv_obj_set_flex_flow(parent, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_all(parent, 10, LV_PART_MAIN);
    lv_obj_set_style_pad_row(parent, 6, LV_PART_MAIN);
    lv_obj_set_flex_align(parent, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);

    // Title
    lv_obj_t * title = lv_label_create(parent);
    lv_label_set_text_static(title, "Device Status");
    lv_obj_set_style_text_color(title, lv_palette_lighten(LV_PALETTE_BLUE, 2), LV_PART_MAIN);

    // Commissioning status
    auto & fabricTable  = chip::Server::GetInstance().GetFabricTable();
    uint8_t fabricCount = fabricTable.FabricCount();
    char statusBuf[64];
    if (fabricCount > 0)
    {
        snprintf(statusBuf, sizeof(statusBuf), "Commissioned: Yes (%u fab)", fabricCount);
    }
    else
    {
        snprintf(statusBuf, sizeof(statusBuf), "Commissioned: No");
    }
    lv_obj_t * commLabel = lv_label_create(parent);
    lv_label_set_text(commLabel, statusBuf);

    // IP Address
    esp_netif_t * netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
    esp_netif_ip_info_t ipInfo;
    char ipBuf[48];
    if (netif != nullptr && esp_netif_get_ip_info(netif, &ipInfo) == ESP_OK && ipInfo.ip.addr != 0)
    {
        snprintf(ipBuf, sizeof(ipBuf), "IP: " IPSTR, IP2STR(&ipInfo.ip));
    }
    else
    {
        snprintf(ipBuf, sizeof(ipBuf), "IP: Not Connected");
    }
    lv_obj_t * ipLabel = lv_label_create(parent);
    lv_label_set_text(ipLabel, ipBuf);

    // Active Device Type
    std::string devStr  = "Type: " + GetActiveDeviceType();
    lv_obj_t * devLabel = lv_label_create(parent);
    lv_label_set_text(devLabel, devStr.c_str());

    // Memory stats
    unsigned int freeInternalKb = heap_caps_get_free_size(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL) / 1024;
    unsigned int freePsramKb    = heap_caps_get_free_size(MALLOC_CAP_SPIRAM) / 1024;
    char memBuf[64];
    snprintf(memBuf, sizeof(memBuf), "RAM: %u KB | PSRAM: %u KB", freeInternalKb, freePsramKb);
    lv_obj_t * memLabel = lv_label_create(parent);
    lv_label_set_text(memLabel, memBuf);

    // Refresh button
    lv_obj_t * refreshBtn = lv_button_create(parent);
    lv_obj_set_width(refreshBtn, LV_PCT(100));
    lv_obj_set_height(refreshBtn, 32);
    lv_obj_t * btnLabel = lv_label_create(refreshBtn);
    lv_label_set_text_static(btnLabel, "Refresh Status");
    lv_obj_center(btnLabel);
    lv_obj_add_event_cb(refreshBtn, OnRefreshClicked, LV_EVENT_CLICKED, parent);
}

void UpdateDeviceInfo(lv_obj_t * parent)
{
    lv_obj_clean(parent);
    ShowDeviceInfo(parent);
}

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
#include "DisplayBoard.h"
#include "Ui.h"

#include <app/server/Server.h>
#include <esp_heap_caps.h>
#include <esp_log.h>
#include <esp_netif.h>
#include <platform/CHIPDeviceLayer.h>

#include <cstdio>
#include <string>

namespace {

constexpr char TAG[] = "DeviceInfoScreen";

// The commissioning label waiting for a fabric count, or null when no such label exists. The
// screen is rebuilt on every refresh, so a reply can outlive the label it was asked for; the
// label clears this on delete and a late reply then finds nothing to write to. Only touched
// with the display lock held.
lv_obj_t * sCommissioningLabel = nullptr;

void OnCommissioningLabelDeleted(lv_event_t * event)
{
    if (sCommissioningLabel == lv_event_get_target(event))
    {
        sCommissioningLabel = nullptr;
    }
}

// The fabric table belongs to the CHIP thread, so it is read there and the result is handed
// back under the display lock. Taking the display lock from the CHIP thread is the direction
// DisplayNotificationHub already uses; the reverse would deadlock.
void RequestCommissioningStatus()
{
    chip::DeviceLayer::SystemLayer().ScheduleLambda([] {
        const uint8_t fabricCount = chip::Server::GetInstance().GetFabricTable().FabricCount();

        if (!DisplayBoard::Lock())
        {
            ESP_LOGE(TAG, "Could not acquire display lock for the commissioning status");
            return;
        }

        if (sCommissioningLabel != nullptr)
        {
            if (fabricCount > 0)
            {
                lv_label_set_text_fmt(sCommissioningLabel, "%u fabric%s", fabricCount, fabricCount == 1 ? "" : "s");
            }
            else
            {
                lv_label_set_text_static(sCommissioningLabel, "Not commissioned");
            }
        }

        DisplayBoard::Unlock();
    });
}

void OnRefreshClicked(lv_event_t * event)
{
    auto * parent = static_cast<lv_obj_t *>(lv_event_get_user_data(event));
    if (parent != nullptr)
    {
        UpdateDeviceInfo(parent);
    }
}

// Icon, muted key and a value label; returns the value label so callers can update it.
lv_obj_t * AddRow(lv_obj_t * card, const char * symbol, const char * key, const char * value)
{
    lv_obj_t * row = lv_obj_create(card);
    lv_obj_remove_style_all(row);
    lv_obj_set_width(row, LV_PCT(100));
    lv_obj_set_height(row, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(row, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(row, Ui::Px(12, 6), LV_PART_MAIN);
    lv_obj_clear_flag(row, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(row, LV_OBJ_FLAG_CLICKABLE);

    Ui::CreateIconBadge(row, symbol, Ui::Accent(), Ui::Px(40, 24));

    lv_obj_t * text = lv_obj_create(row);
    lv_obj_remove_style_all(text);
    lv_obj_set_flex_grow(text, 1);
    lv_obj_set_height(text, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(text, LV_FLEX_FLOW_COLUMN);
    lv_obj_clear_flag(text, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(text, LV_OBJ_FLAG_CLICKABLE);

    Ui::CreateCaption(text, key);
    lv_obj_t * valueLabel = lv_label_create(text);
    lv_label_set_text(valueLabel, value);
    lv_label_set_long_mode(valueLabel, LV_LABEL_LONG_DOT);
    lv_obj_set_width(valueLabel, LV_PCT(100));
    return valueLabel;
}

} // namespace

void ShowDeviceInfo(lv_obj_t * parent)
{
    Ui::LayoutColumn(parent);

    lv_obj_t * card = lv_obj_create(parent);
    lv_obj_set_width(card, LV_PCT(100));
    lv_obj_set_height(card, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(card, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(card, Ui::Px(12, 4), LV_PART_MAIN);
    lv_obj_clear_flag(card, LV_OBJ_FLAG_SCROLLABLE);

    // Filled in once the CHIP thread reports the fabric count
    lv_obj_t * commLabel = AddRow(card, LV_SYMBOL_HOME, "Fabrics", "...");
    lv_obj_add_event_cb(commLabel, OnCommissioningLabelDeleted, LV_EVENT_DELETE, nullptr);
    sCommissioningLabel = commLabel;
    RequestCommissioningStatus();

#if CONFIG_LWIP_IPV4
    esp_netif_t * netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
    esp_netif_ip_info_t ipInfo;
    char ipBuf[48];
    if (netif != nullptr && esp_netif_get_ip_info(netif, &ipInfo) == ESP_OK && ipInfo.ip.addr != 0)
    {
        snprintf(ipBuf, sizeof(ipBuf), IPSTR, IP2STR(&ipInfo.ip));
    }
    else
    {
        snprintf(ipBuf, sizeof(ipBuf), "Not connected");
    }
    AddRow(card, LV_SYMBOL_WIFI, "IP address", ipBuf);
#endif // CONFIG_LWIP_IPV4

    AddRow(card, LV_SYMBOL_EDIT, "Device type", GetActiveDeviceType().c_str());

    unsigned int freeInternalKb = heap_caps_get_free_size(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL) / 1024;
    unsigned int freePsramKb    = heap_caps_get_free_size(MALLOC_CAP_SPIRAM) / 1024;
    char memBuf[64];
    snprintf(memBuf, sizeof(memBuf), "%u KB internal, %u KB PSRAM", freeInternalKb, freePsramKb);
    AddRow(card, LV_SYMBOL_CHARGE, "Free memory", memBuf);

    lv_obj_t * refreshBtn = lv_button_create(parent);
    lv_obj_set_width(refreshBtn, LV_PCT(100));
    lv_obj_t * btnLabel = lv_label_create(refreshBtn);
    lv_label_set_text_static(btnLabel, LV_SYMBOL_REFRESH "  Refresh");
    lv_obj_center(btnLabel);
    lv_obj_add_event_cb(refreshBtn, OnRefreshClicked, LV_EVENT_CLICKED, parent);
}

void UpdateDeviceInfo(lv_obj_t * parent)
{
    lv_obj_clean(parent);
    ShowDeviceInfo(parent);
}

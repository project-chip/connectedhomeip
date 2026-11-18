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

#include "CommissioningMonitor.h"
#include "DisplayBoard.h"
#include "DisplayWake.h"
#include "NavigationStack.h"
#include "Ui.h"

#include <app/server/Server.h>
#include <credentials/FabricTable.h>
#include <esp_log.h>
#include <platform/CHIPDeviceLayer.h>
#include <transport/SecureSession.h>

namespace CommissioningMonitor {
namespace {

const char TAG[] = "Commissioning";

constexpr uint32_t kSuccessDwellMs = 2500;

const char kIdleUncommissioned[] = "Waiting for a controller";
const char kIdleCommissioned[]   = "Commissioned";

enum class Phase
{
    kIdle,
    kInProgress,
    kSuccess,
    kFailure,
};

// Shared between the CHIP thread and the LVGL task, always under the display lock.
const char * sStatusText = kIdleUncommissioned;
Phase sPhase             = Phase::kIdle;
uint8_t sFabricCount     = 0;

lv_obj_t * sStatusView        = nullptr;
lv_obj_t * sIconSlot          = nullptr;
lv_obj_t * sLabel             = nullptr;
lv_timer_t * sReturnHomeTimer = nullptr;

// CHIP thread only.
bool sWifiConnected = false;
bool sBleConnected  = false;
// Once credentials are in, a commissioner may drop BLE and finish over the operational
// network, so a BLE close is only a failure before that point.
bool sNetworkEnabled = false;

// Set from the first sign of a controller until the verdict, so the BLE link closing
// after a successful run is not reported as a failure.
bool sInProgress = false;

lv_color_t PhaseColor(Phase phase)
{
    switch (phase)
    {
    case Phase::kSuccess:
        return Ui::Success();
    case Phase::kFailure:
        return Ui::Danger();
    case Phase::kInProgress:
        return Ui::Text();
    case Phase::kIdle:
        break;
    }
    return Ui::TextMuted();
}

void RenderIcon()
{
    lv_obj_clean(sIconSlot);
    const int32_t size = Ui::Px(44, 26);

    switch (sPhase)
    {
    case Phase::kInProgress: {
        lv_obj_t * spinner = lv_spinner_create(sIconSlot);
        lv_obj_set_size(spinner, size, size);
        lv_spinner_set_anim_params(spinner, 1000, 200);
        lv_obj_center(spinner);
        break;
    }
    case Phase::kSuccess:
        Ui::PopIn(Ui::CreateIconBadge(sIconSlot, LV_SYMBOL_OK, Ui::Success(), size));
        break;
    case Phase::kFailure:
        Ui::PopIn(Ui::CreateIconBadge(sIconSlot, LV_SYMBOL_WARNING, Ui::Danger(), size));
        break;
    case Phase::kIdle:
        Ui::CreateIconBadge(sIconSlot, sFabricCount != 0 ? LV_SYMBOL_HOME : LV_SYMBOL_BLUETOOTH,
                            sFabricCount != 0 ? Ui::Success() : Ui::Accent(), size);
        break;
    }
}

void RenderStatus()
{
    if (sStatusView == nullptr)
    {
        return;
    }
    lv_label_set_text_static(sLabel, sStatusText);
    lv_obj_set_style_text_color(sLabel, PhaseColor(sPhase), LV_PART_MAIN);
    RenderIcon();

    if (sPhase == Phase::kIdle && sFabricCount == 0)
    {
        Ui::StartPulse(sLabel);
    }
    else
    {
        Ui::StopPulse(sLabel);
    }
}

void RenderIndicators()
{
    NavigationStack::SetIndicators(sWifiConnected, sBleConnected, sFabricCount);
}

// Ends the success dwell. Only pops to Home while the QR screen is still the one showing.
void ReturnHome(lv_timer_t * timer)
{
    sReturnHomeTimer = nullptr;
    sStatusText      = kIdleCommissioned;
    sPhase           = Phase::kIdle;
    RenderStatus();
    if (sStatusView != nullptr)
    {
        NavigationStack::ResetToRoot();
    }
}

// CHIP thread. Takes the display lock, never the other way round.
void SetStatus(const char * text, Phase phase)
{
    ESP_LOGI(TAG, "%s", text);

    if (!DisplayBoard::Lock())
    {
        ESP_LOGE(TAG, "Could not acquire display lock for the commissioning status");
        return;
    }

    sStatusText = text;
    sPhase      = phase;
    RenderStatus();
    RenderIndicators();
    WakeDisplay();

    if (sReturnHomeTimer != nullptr)
    {
        lv_timer_delete(sReturnHomeTimer);
        sReturnHomeTimer = nullptr;
    }
    if (phase == Phase::kSuccess)
    {
        sReturnHomeTimer = lv_timer_create(ReturnHome, kSuccessDwellMs, nullptr);
        lv_timer_set_repeat_count(sReturnHomeTimer, 1);
    }

    DisplayBoard::Unlock();
}

// CHIP thread. Connectivity changes outside a commissioning session only touch the bar.
void UpdateIndicators()
{
    if (!DisplayBoard::Lock())
    {
        ESP_LOGE(TAG, "Could not acquire display lock for the connectivity indicators");
        return;
    }
    RenderIndicators();
    DisplayBoard::Unlock();
}

// CHIP thread. Fabrics come and go outside commissioning too, e.g. when a controller
// removes the node, and the QR screen hides the codes based on this count.
void RefreshFabricCount(const chip::FabricTable & table)
{
    if (!DisplayBoard::Lock())
    {
        ESP_LOGE(TAG, "Could not acquire display lock for the fabric count");
        return;
    }
    sFabricCount = table.FabricCount();
    if (sPhase == Phase::kIdle)
    {
        sStatusText = sFabricCount != 0 ? kIdleCommissioned : kIdleUncommissioned;
    }
    // The codes screen picks the QR code or the commissioned notice when it is built.
    if (sStatusView != nullptr)
    {
        NavigationStack::RefreshCurrent();
    }
    else
    {
        RenderStatus();
    }
    RenderIndicators();
    DisplayBoard::Unlock();
}

class FabricWatcher : public chip::FabricTable::Delegate
{
public:
    void OnFabricRemoved(const chip::FabricTable & table, chip::FabricIndex) override { RefreshFabricCount(table); }
    void OnFabricCommitted(const chip::FabricTable & table, chip::FabricIndex) override { RefreshFabricCount(table); }
};

FabricWatcher sFabricWatcher;

void OnViewDeleted(lv_event_t * event)
{
    if (lv_event_get_target_obj(event) == sStatusView)
    {
        sStatusView = nullptr;
        sIconSlot   = nullptr;
        sLabel      = nullptr;
    }
}

void OnSecureSession(const chip::DeviceLayer::ChipDeviceEvent & event)
{
    using chip::Transport::SecureSession;
    switch (static_cast<SecureSession::Type>(event.SecureSessionEstablished.SecureSessionType))
    {
    case SecureSession::Type::kPASE:
        sInProgress = true;
        SetStatus("Passcode verified, secure session open", Phase::kInProgress);
        break;
    case SecureSession::Type::kCASE:
        if (sInProgress)
        {
            SetStatus("Operational session established", Phase::kInProgress);
        }
        break;
    }
}

void OnPlatformEvent(const chip::DeviceLayer::ChipDeviceEvent * event, intptr_t)
{
    using namespace chip::DeviceLayer;

    switch (event->Type)
    {
    case DeviceEventType::kCHIPoBLEConnectionEstablished:
        sInProgress     = true;
        sNetworkEnabled = false;
        sBleConnected   = true;
        SetStatus("Controller connected over BLE", Phase::kInProgress);
        break;

    case DeviceEventType::kSecureSessionEstablished:
        OnSecureSession(*event);
        break;

    case DeviceEventType::kOperationalNetworkEnabled:
        if (sInProgress)
        {
            sNetworkEnabled = true;
            SetStatus("Credentials received, joining Wi-Fi", Phase::kInProgress);
        }
        break;

    case DeviceEventType::kWiFiConnectivityChange:
        sWifiConnected = event->WiFiConnectivityChange.Result == kConnectivity_Established;
        if (sInProgress && sWifiConnected)
        {
            SetStatus("Joined Wi-Fi, waiting for the controller", Phase::kInProgress);
        }
        else
        {
            UpdateIndicators();
        }
        break;

    case DeviceEventType::kCommissioningComplete:
        sInProgress = false;
        SetStatus("Commissioning successful", Phase::kSuccess);
        break;

    case DeviceEventType::kFailSafeTimerExpired:
        if (sInProgress)
        {
            sInProgress = false;
            SetStatus("Commissioning failed: timed out", Phase::kFailure);
        }
        break;

    case DeviceEventType::kCHIPoBLEConnectionClosed:
        sBleConnected = false;
        if (sInProgress && !sNetworkEnabled)
        {
            sInProgress = false;
            SetStatus("Commissioning failed: controller disconnected", Phase::kFailure);
        }
        else
        {
            UpdateIndicators();
        }
        break;

    default:
        break;
    }
}

} // namespace

void Init()
{
    if (!DisplayBoard::Lock())
    {
        ESP_LOGE(TAG, "Could not acquire display lock for the initial commissioning state");
    }
    else
    {
        sFabricCount   = chip::Server::GetInstance().GetFabricTable().FabricCount();
        sWifiConnected = chip::DeviceLayer::ConnectivityMgr().IsWiFiStationConnected();
        sStatusText    = sFabricCount != 0 ? kIdleCommissioned : kIdleUncommissioned;
        RenderIndicators();
        DisplayBoard::Unlock();
    }

    CHIP_ERROR err = chip::DeviceLayer::PlatformMgr().AddEventHandler(OnPlatformEvent, 0);
    if (err != CHIP_NO_ERROR)
    {
        ESP_LOGE(TAG, "Failed to register for platform events: %" CHIP_ERROR_FORMAT, err.Format());
    }

    err = chip::Server::GetInstance().GetFabricTable().AddFabricDelegate(&sFabricWatcher);
    if (err != CHIP_NO_ERROR)
    {
        ESP_LOGE(TAG, "Failed to register for fabric changes: %" CHIP_ERROR_FORMAT, err.Format());
    }
}

lv_obj_t * CreateStatusView(lv_obj_t * parent)
{
    lv_obj_t * row = lv_obj_create(parent);
    lv_obj_remove_style_all(row);
    lv_obj_set_width(row, LV_PCT(100));
    lv_obj_set_height(row, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(row, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(row, Ui::Px(12, 6), LV_PART_MAIN);
    lv_obj_clear_flag(row, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(row, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(row, OnViewDeleted, LV_EVENT_DELETE, nullptr);

    const int32_t slot = Ui::Px(48, 28);
    sIconSlot          = lv_obj_create(row);
    lv_obj_remove_style_all(sIconSlot);
    lv_obj_set_size(sIconSlot, slot, slot);
    lv_obj_clear_flag(sIconSlot, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(sIconSlot, LV_OBJ_FLAG_CLICKABLE);

    sLabel = lv_label_create(row);
    lv_label_set_long_mode(sLabel, LV_LABEL_LONG_WRAP);
    lv_obj_set_flex_grow(sLabel, 1);
    lv_obj_set_style_text_font(sLabel, Ui::BodyFont(), LV_PART_MAIN);

    sStatusView = row;
    RenderStatus();
    return row;
}

uint8_t FabricCount()
{
    return sFabricCount;
}

} // namespace CommissioningMonitor

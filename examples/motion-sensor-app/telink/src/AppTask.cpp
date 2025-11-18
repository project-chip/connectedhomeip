/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#include "AppTask.h"
#include "LEDManager.h"

#include <app-common/zap-generated/attributes/Accessors.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_DECLARE(app, CONFIG_CHIP_APP_LOG_LEVEL);

AppTask AppTask::sAppTask;

// sequence of lux values for button toggling
static const uint16_t kLuxSteps[] = { 100, 300, 600, 1000, 1500, 2000 };
static constexpr size_t kLuxCount = sizeof(kLuxSteps) / sizeof(uint16_t);

CHIP_ERROR AppTask::Init()
{
    SetExampleButtonCallbacks(MotionActionEventHandler);
    InitCommonParts();

    LedManager::getInstance().setLed(LedManager::EAppLed_App0, MotionSensorMgr().IsMotionDetected());

    MotionSensorMgr().SetCallback(OnMotionStateChanged);

    UpdateDeviceState();
    return CHIP_NO_ERROR;
}

void AppTask::OnMotionStateChanged(MotionSensorManager::State aState)
{
    bool detected = (aState == MotionSensorManager::State::kMotionDetected);

    LedManager::getInstance().setLed(LedManager::EAppLed_App0, detected);

    if (sAppTask.IsSyncClusterToButtonAction())
        sAppTask.UpdateClusterState();
}

void AppTask::PostMotionActionRequest(MotionSensorManager::Action aAction)
{
    AppEvent event;
    event.Type               = AppEvent::kEventType_DeviceAction;
    event.DeviceEvent.Action = static_cast<uint8_t>(aAction);
    event.Handler            = MotionActionEventHandler;

    sAppTask.PostEvent(&event);
}

void AppTask::MotionActionEventHandler(AppEvent * aEvent)
{
    MotionSensorManager::Action action;

    if (aEvent->Type == AppEvent::kEventType_Button)
    {
        // toggle occupancy
        if (MotionSensorMgr().IsMotionDetected())
            action = MotionSensorManager::Action::kSetUndetected;
        else
            action = MotionSensorManager::Action::kSetDetected;

        // toggle lux
        static size_t idx = 0;
        idx               = (idx + 1) % kLuxCount;
        sAppTask.mLux     = kLuxSteps[idx];

        sAppTask.SetSyncClusterToButtonAction(true);
    }
    else
    {
        action = static_cast<MotionSensorManager::Action>(aEvent->DeviceEvent.Action);
    }

    MotionSensorMgr().InitiateAction(action);
    sAppTask.SetIlluminance(sAppTask.mLux);
}

void AppTask::SetIlluminance(uint16_t lux)
{
    chip::app::Clusters::IlluminanceMeasurement::Attributes::MeasuredValue::Set(2, lux);
    ChipLogProgress(NotSpecified, "Illuminance SET: %u", lux);
}

void AppTask::UpdateClusterStateInternal(intptr_t)
{
    using Bitmap = chip::app::Clusters::OccupancySensing::OccupancyBitmap;

    chip::BitMask<Bitmap> mask;
    if (MotionSensorMgr().IsMotionDetected())
        mask.Set(Bitmap::kOccupied);
    else
        mask.ClearAll();

    chip::app::Clusters::OccupancySensing::Attributes::Occupancy::Set(1, mask);

    chip::app::Clusters::IlluminanceMeasurement::Attributes::MeasuredValue::Set(2, sAppTask.mLux);
}

void AppTask::UpdateClusterState()
{
    PlatformMgr().ScheduleWork(UpdateClusterStateInternal, 0);
}

void AppTask::UpdateDeviceStateInternal(intptr_t)
{
    chip::BitMask<chip::app::Clusters::OccupancySensing::OccupancyBitmap> mask;
    chip::app::Clusters::OccupancySensing::Attributes::Occupancy::Get(1, &mask);

    bool occupied = mask.Raw() & 0x01;
    LedManager::getInstance().setLed(LedManager::EAppLed_App0, occupied);

    chip::app::DataModel::Nullable<uint16_t> lux;
    chip::app::Clusters::IlluminanceMeasurement::Attributes::MeasuredValue::Get(2, lux);

    if (!lux.IsNull())
        ChipLogProgress(NotSpecified, "Lux: %u", lux.Value());
}

void AppTask::UpdateDeviceState()
{
    PlatformMgr().ScheduleWork(UpdateDeviceStateInternal, 0);
}

void AppTask::LinkLeds(LedManager & ledManager)
{
#if CONFIG_CHIP_ENABLE_APPLICATION_STATUS_LED
    ledManager.linkLed(LedManager::EAppLed_Status, 0);
    ledManager.linkLed(LedManager::EAppLed_App0, 1);
#else
    ledManager.linkLed(LedManager::EAppLed_App0, 0);
#endif
}

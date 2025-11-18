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

#pragma once

#include "AppTaskCommon.h"
#include "MotionSensorManager.h"

class AppTask : public AppTaskCommon
{
public:
    void PostMotionActionRequest(MotionSensorManager::Action aAction);

    void UpdateClusterState();
    void UpdateDeviceState();

    bool IsSyncClusterToButtonAction();
    void SetSyncClusterToButtonAction(bool value);

    void SetIlluminance(uint16_t lux);

private:
    friend AppTask & GetAppTask();
    friend class AppTaskCommon;

    CHIP_ERROR Init();
    void LinkLeds(LedManager & ledManager);

    static void OnMotionStateChanged(MotionSensorManager::State aState);

    static void UpdateClusterStateInternal(intptr_t arg);
    static void UpdateDeviceStateInternal(intptr_t arg);

    static void MotionActionEventHandler(AppEvent * aEvent);

    bool mSyncClusterToButtonAction = false;
    uint16_t mLux                   = 100; // initial brightness

    static AppTask sAppTask;
};

inline AppTask & GetAppTask()
{
    return AppTask::sAppTask;
}

inline bool AppTask::IsSyncClusterToButtonAction()
{
    return mSyncClusterToButtonAction;
}

inline void AppTask::SetSyncClusterToButtonAction(bool value)
{
    mSyncClusterToButtonAction = value;
}

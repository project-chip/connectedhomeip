/*
 *
 *    Copyright (c) 2022-2023 Project CHIP Authors
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
#include "ContactSensorManager.h"

// Application-defined error codes in the CHIP_ERROR space.
#define APP_ERROR_UNHANDLED_EVENT CHIP_APPLICATION_ERROR(0x03)

class AppTask : public AppTaskCommon
{
public:
    void PostContactActionRequest(ContactSensorManager::Action aAction);
    void UpdateClusterState(void);
    void UpdateDeviceState(void);

    bool IsSyncClusterToButtonAction(void);
    void SetSyncClusterToButtonAction(bool value);

private:
    friend AppTask & GetAppTask(void);
    friend class AppTaskCommon;

    CHIP_ERROR Init(void);

    static void OnStateChanged(ContactSensorManager::State aState);

    static void UpdateClusterStateInternal(intptr_t arg);
    static void UpdateDeviceStateInternal(intptr_t arg);

    static void ContactActionEventHandler(AppEvent * aEvent);

    bool mSyncClusterToButtonAction = false;

    static AppTask sAppTask;
};

inline AppTask & GetAppTask(void)
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

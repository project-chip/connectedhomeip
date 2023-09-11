/*
 *
 * SPDX-FileCopyrightText: 2022-2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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

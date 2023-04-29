/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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
#include "BoltLockManager.h"

#include <app-common/zap-generated/attributes/Accessors.h>

LOG_MODULE_DECLARE(app, CONFIG_CHIP_APP_LOG_LEVEL);

using namespace ::chip::app::Clusters::DoorLock;

namespace {
#if CONFIG_CHIP_ENABLE_APPLICATION_STATUS_LED
LEDWidget sLockLED;
#endif
} // namespace

AppTask AppTask::sAppTask;

CHIP_ERROR AppTask::Init(void)
{
#if APP_USE_EXAMPLE_START_BUTTON
    SetExampleButtonCallbacks(LockActionEventHandler);
#endif
    InitCommonParts();

#if CONFIG_CHIP_ENABLE_APPLICATION_STATUS_LED
    sLockLED.Init(LOCK_STATE_LED);
    sLockLED.Set(BoltLockMgr().IsLocked());
#endif

    BoltLockMgr().Init(LockStateChanged);

    // Disable auto-relock time feature.
    DoorLockServer::Instance().SetAutoRelockTime(kExampleEndpointId, 0);

    CHIP_ERROR err = ConnectivityMgr().SetBLEDeviceName("Telink Lock");
    if (err != CHIP_NO_ERROR)
    {
        LOG_ERR("SetBLEDeviceName fail");
        return err;
    }

    return CHIP_NO_ERROR;
}

void AppTask::LockActionEventHandler(AppEvent * aEvent)
{
    if (BoltLockMgr().IsLocked())
    {
        BoltLockMgr().Unlock(BoltLockManager::OperationSource::kButton);
    }
    else
    {
        BoltLockMgr().Lock(BoltLockManager::OperationSource::kButton);
    }
}

void AppTask::LockStateChanged(BoltLockManager::State state, BoltLockManager::OperationSource source)
{
    switch (state)
    {
    case BoltLockManager::State::kLockingInitiated:
        LOG_INF("Lock action initiated");
#if CONFIG_CHIP_ENABLE_APPLICATION_STATUS_LED
        sLockLED.Blink(50, 50);
#endif
        break;
    case BoltLockManager::State::kLockingCompleted:
        LOG_INF("Lock action completed");
#if CONFIG_CHIP_ENABLE_APPLICATION_STATUS_LED
        sLockLED.Set(true);
#endif
        break;
    case BoltLockManager::State::kUnlockingInitiated:
        LOG_INF("Unlock action initiated");
#if CONFIG_CHIP_ENABLE_APPLICATION_STATUS_LED
        sLockLED.Blink(50, 50);
#endif
        break;
    case BoltLockManager::State::kUnlockingCompleted:
        LOG_INF("Unlock action completed");
#if CONFIG_CHIP_ENABLE_APPLICATION_STATUS_LED
        sLockLED.Set(false);
#endif
        break;
    }

    // Handle changing attribute state in the application
    sAppTask.UpdateClusterState(state, source);
}

void AppTask::UpdateClusterState(BoltLockManager::State state, BoltLockManager::OperationSource source)
{
    DlLockState newLockState;

    switch (state)
    {
    case BoltLockManager::State::kLockingCompleted:
        newLockState = DlLockState::kLocked;
        break;
    case BoltLockManager::State::kUnlockingCompleted:
        newLockState = DlLockState::kUnlocked;
        break;
    default:
        newLockState = DlLockState::kNotFullyLocked;
        break;
    }

    SystemLayer().ScheduleLambda([newLockState, source] {
        chip::app::DataModel::Nullable<chip::app::Clusters::DoorLock::DlLockState> currentLockState;
        chip::app::Clusters::DoorLock::Attributes::LockState::Get(kExampleEndpointId, currentLockState);

        if (currentLockState.IsNull())
        {
            // Initialize lock state with start value, but not invoke lock/unlock.
            chip::app::Clusters::DoorLock::Attributes::LockState::Set(kExampleEndpointId, newLockState);
        }
        else
        {
            LOG_INF("Updating LockState attribute");

            if (!DoorLockServer::Instance().SetLockState(kExampleEndpointId, newLockState, source))
            {
                LOG_ERR("Failed to update LockState attribute");
            }
        }
    });
}

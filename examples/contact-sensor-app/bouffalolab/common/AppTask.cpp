/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include <app-common/zap-generated/attributes/Accessors.h>

#include <app/clusters/boolean-state-server/CodegenIntegration.h>
#include <app/server/Dnssd.h>
#include <app/server/Server.h>
#include <credentials/DeviceAttestationCredsProvider.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>
#include <platform/bouffalolab/common/DiagnosticDataProviderImpl.h>
#include <setup_payload/OnboardingCodesUtil.h>
#include <system/SystemClock.h>

#if HEAP_MONITORING
#include "MemMonitoring.h"
#endif

#if CHIP_ENABLE_OPENTHREAD
#include <platform/OpenThread/OpenThreadUtils.h>
#include <platform/ThreadStackManager.h>
#include <platform/bouffalolab/common/ThreadStackManagerImpl.h>
#include <utils_list.h>
#endif

#include <plat.h>

#if CHIP_DEVICE_LAYER_TARGET_BFLB
extern "C" {
#include <pm_manager.h>
}
#endif

#include "AppTask.h"
#include "mboard.h"

using namespace ::chip;
using namespace ::chip::app;
using namespace ::chip::Credentials;
using namespace ::chip::DeviceLayer;

AppTask AppTask::sAppTask;
StackType_t AppTask::appStack[APP_TASK_STACK_SIZE / sizeof(StackType_t)];
StaticTask_t AppTask::appTaskStruct;

void StartAppTask(void)
{
    GetAppTask().sAppTaskHandle =
        xTaskCreateStatic(GetAppTask().AppTaskMain, APP_TASK_NAME, MATTER_ARRAY_SIZE(GetAppTask().appStack), NULL,
                          APP_TASK_PRIORITY, GetAppTask().appStack, &GetAppTask().appTaskStruct);
    if (GetAppTask().sAppTaskHandle == NULL)
    {
        ChipLogError(NotSpecified, "Failed to create app task");
        appError(APP_ERROR_EVENT_QUEUE_FAILED);
    }
}

void AppTask::PostEvent(app_event_t event)
{
    if (xPortIsInsideInterrupt())
    {
        BaseType_t higherPrioTaskWoken = pdFALSE;
        xTaskNotifyFromISR(sAppTaskHandle, event, eSetBits, &higherPrioTaskWoken);
    }
    else
    {
        xTaskNotify(sAppTaskHandle, event, eSetBits);
    }
}

void AppTask::ButtonEventHandler(int pin, bool is_pin_high)
{
    if (CHIP_RESET_PIN == pin)
    {
        if (is_pin_high)
        {
            GetAppTask().mButtonPressedTime = System::SystemClock().GetMonotonicMilliseconds64().count();
        }
        else
        {
            if (GetAppTask().mButtonPressedTime)
            {
                uint64_t pressed_time =
                    System::SystemClock().GetMonotonicMilliseconds64().count() - GetAppTask().mButtonPressedTime;

                if (pressed_time > APP_BUTTON_PRESS_LONG)
                {
                    GetAppTask().PostEvent(APP_EVENT_FACTORY_RESET);
                }
                else if (APP_BUTTON_PRESS_JITTER < pressed_time && pressed_time < APP_BUTTON_PRESS_SHORT)
                {
                    GetAppTask().PostEvent(APP_EVENT_BUTTON_PRESSED);
                }

                GetAppTask().mButtonPressedTime = 0;
            }
        }
    }
    else if (CHIP_CONTACT_PIN == pin)
    {
        if (is_pin_high)
        {
            GetAppTask().PostEvent(APP_EVENT_CONTACT_SENSOR_TRUE);
        }
        else
        {
            GetAppTask().PostEvent(APP_EVENT_CONTACT_SENSOR_FALSE);
        }
    }
}

void AppTask::AppTaskMain(void * pvParameter)
{
    app_event_t appEvent;
    bool stateValueAttrValue = false;
    uint64_t currentHeapFree = 0;

    app_pds_init(GetAppTask().ButtonEventHandler);

    ChipLogProgress(NotSpecified, "Starting Platform Manager Event Loop");
    CHIP_ERROR ret = PlatformMgr().StartEventLoopTask();
    if (ret != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified, "PlatformMgr().StartEventLoopTask() failed");
        appError(ret);
    }

    vTaskSuspend(NULL);
#if CHIP_DEVICE_LAYER_TARGET_BFLB
    pm_enable_tickless();
#endif
#if CHIP_DEVICE_LAYER_TARGET_BL702L && CHIP_DETAIL_LOGGING
    Server::GetInstance().GetICDManager().RegisterObserver(&sAppTask);
#endif

    TEMPORARY_RETURN_IGNORED DiagnosticDataProviderImpl::GetDefaultInstance().GetCurrentHeapFree(currentHeapFree);
    ChipLogProgress(NotSpecified, "App Task started, with SRAM heap %lld left", currentHeapFree);

    while (true)
    {
        appEvent                 = APP_EVENT_NONE;
        BaseType_t eventReceived = xTaskNotifyWait(0, APP_EVENT_ALL_MASK, (uint32_t *) &appEvent, portMAX_DELAY);

        if (eventReceived)
        {
            PlatformMgr().LockChipStack();

            if (APP_EVENT_BUTTON_PRESSED & appEvent)
            {
                ChipLogProgress(NotSpecified, "APP_EVENT_BUTTON_PRESSED");
            }

            if (APP_EVENT_FACTORY_RESET & appEvent)
            {
                DeviceLayer::ConfigurationMgr().InitiateFactoryReset();
            }

            if (APP_EVENT_CONTACT_SENSOR_TRUE & appEvent)
            {
                stateValueAttrValue = 1;

                auto booleanState = app::Clusters::BooleanState::FindClusterOnEndpoint(1);
                if (booleanState != nullptr)
                {
                    booleanState->SetStateValue(stateValueAttrValue);
                }
            }

            if (APP_EVENT_CONTACT_SENSOR_FALSE & appEvent)
            {
                stateValueAttrValue = 0;

                auto booleanState = app::Clusters::BooleanState::FindClusterOnEndpoint(1);
                if (booleanState != nullptr)
                {
                    booleanState->SetStateValue(stateValueAttrValue);
                }
            }

            PlatformMgr().UnlockChipStack();
        }
    }
}

#if CHIP_DEVICE_LAYER_TARGET_BL702L && CHIP_DETAIL_LOGGING
void AppTask::OnEnterActiveMode()
{
    ChipLogProgress(NotSpecified, "App ICD enter active mode.");
}

void AppTask::OnEnterIdleMode()
{
    ChipLogProgress(NotSpecified, "App ICD enter idle mode.");
}

void AppTask::OnTransitionToIdle()
{
    ChipLogProgress(NotSpecified, "App ICD transit to idle.");
}

void AppTask::OnICDModeChange()
{
    ChipLogProgress(NotSpecified, "App ICD mode change.");
}
#endif

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

class AppTask : public AppTaskCommon
{
public:
#ifdef CONFIG_CHIP_ENABLE_POWER_ON_FACTORY_RESET
    void PowerOnFactoryReset(void);
#endif /* CONFIG_CHIP_ENABLE_POWER_ON_FACTORY_RESET */
    void SetInitiateAction(PWMDevice::Action_t aAction, int32_t aActor, uint8_t * value);
    void UpdateClusterState(void);
    PWMDevice & GetPWMDevice(void) { return mPwmRgbBlueLed; }

private:
    friend AppTask & GetAppTask(void);
    friend class AppTaskCommon;

    CHIP_ERROR Init(void);

    static void ActionInitiated(PWMDevice::Action_t aAction, int32_t aActor);
    static void ActionCompleted(PWMDevice::Action_t aAction, int32_t aActor);

    static void LightingActionEventHandler(AppEvent * aEvent);
#ifdef CONFIG_CHIP_ENABLE_POWER_ON_FACTORY_RESET
    static void PowerOnFactoryResetEventHandler(AppEvent * aEvent);
    static void PowerOnFactoryResetTimerEvent(struct k_timer * dummy);

    static unsigned int sPowerOnFactoryResetTimerCnt;
    static k_timer sPowerOnFactoryResetTimer;
#endif /* CONFIG_CHIP_ENABLE_POWER_ON_FACTORY_RESET */
    PWMDevice mPwmRgbBlueLed;
#if USE_RGB_PWM
    PWMDevice mPwmRgbGreenLed;
    PWMDevice mPwmRgbRedLed;
#endif

    static AppTask sAppTask;
};

inline AppTask & GetAppTask(void)
{
    return AppTask::sAppTask;
}

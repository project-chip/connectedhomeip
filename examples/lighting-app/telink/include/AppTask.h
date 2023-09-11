/*
 *
 * SPDX-FileCopyrightText: 2022-2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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

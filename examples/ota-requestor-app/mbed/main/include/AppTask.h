/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 * SPDX-FileCopyrightText: 2019 Google LLC.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "AppEvent.h"
#include <DFUManager.h>
#include <lib/core/CHIPCallback.h>
#include <lib/support/Span.h>
#include <rtos/EventFlags.h>
class AppTask
{
public:
    int StartApp();

    void PostEvent(AppEvent * aEvent);

    void ButtonEventHandler(uint32_t id, bool pushed);

    enum FunctionButton
    {
        kFunction_Button_1 = 0,
        kFunction_Button_2,
        kFunction_Button_last
    };

    enum FunctionButtonAction
    {
        kFunction_Button_release = 0,
        kFunction_Button_push
    };

    enum UserResponseType
    {
        kUser_Response_confirm = 0x1,
        kUser_Response_reject  = 0x2
    };

private:
    AppTask() : mOnUpdateAvailableCallback(OnUpdateAvailableHandler, this), mOnUpdateApplyCallback(OnUpdateApplyHandler, this)
    {
        mTimerCallbacks[kFunction_Button_1] = mbed::callback(this, &AppTask::TimerButton1EventHandler);
        mTimerCallbacks[kFunction_Button_2] = mbed::callback(this, &AppTask::TimerButton2EventHandler);
    }

    friend AppTask & GetAppTask(void);

    int Init();

    void DispatchEvent(const AppEvent * event);

    static void FunctionTimerEventHandler(AppEvent * aEvent);
    static void ButtonHandler(AppEvent * aEvent);

    void FunctionButton1PressEventHandler(void);
    void FunctionButton1ReleaseEventHandler(void);
    void FunctionButton2PressEventHandler(void);
    void FunctionButton2ReleaseEventHandler(void);

    void StartTimer(uint8_t index, uint32_t aTimeoutInMs);
    void CancelTimer(uint8_t index);
    void TimerButton1EventHandler();
    void TimerButton2EventHandler();

    static bool OnUpdateAvailableHandler(void * context, uint32_t softwareVersion, chip::CharSpan softwareVersionString);
    static bool OnUpdateApplyHandler(void * context);

    chip::Callback::Callback<OnUpdateAvailable> mOnUpdateAvailableCallback;
    chip::Callback::Callback<OnUpdateApply> mOnUpdateApplyCallback;

    enum Function_t
    {
        kFunction_NoneSelected = 0,
        kFunction_ConfirmResponse,
        kFunction_RejectResponse,
        kFunction_FactoryReset,
        kFunction_CommissioningReset,
        kFunction_Invalid
    };

    Function_t mFunction[kFunction_Button_last];
    bool mFunctionTimerActive[kFunction_Button_last];
    mbed::Callback<void()> mTimerCallbacks[kFunction_Button_last];
    rtos::EventFlags mUserResponseFlag;
    static AppTask sAppTask;
};

inline AppTask & GetAppTask(void)
{
    return AppTask::sAppTask;
}

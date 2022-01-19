/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
 *    Copyright (c) 2019 Google LLC.
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

#include "AppEvent.h"
#include <rtos/EventFlags.h>

class AppTask
{
public:
    int StartApp();

    void PostEvent(AppEvent * aEvent);

    void ButtonEventHandler(uint32_t id, bool pushed);

private:
    friend AppTask & GetAppTask(void);

    int Init();

    void DispatchEvent(const AppEvent * event);

    static void FunctionTimerEventHandler(AppEvent * aEvent);
    static void FunctionHandler(AppEvent * aEvent);
    static void BleHandler(AppEvent * aEvent);

    void BleButtonPressEventHandler(void);
    void FunctionButtonPressEventHandler(void);
    void FunctionButtonReleaseEventHandler(void);

    void StartTimer(uint32_t aTimeoutInMs);
    void CancelTimer(void);
    void TimerEventHandler(void);

    enum Function_t
    {
        kFunction_NoneSelected   = 0,
        kFunction_SoftwareUpdate = 0,
        kFunction_FactoryReset,

        kFunction_Invalid
    };

    Function_t mFunction;
    bool mFunctionTimerActive;
    static AppTask sAppTask;
};

inline AppTask & GetAppTask(void)
{
    return AppTask::sAppTask;
}

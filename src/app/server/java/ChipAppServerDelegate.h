/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include <app/server/AppDelegate.h>
#include <jni.h>
#include <lib/core/CHIPError.h>
#include <lib/support/JniReferences.h>

class ChipAppServerDelegate : public AppDelegate
{
public:
    void OnCommissioningSessionEstablishmentStarted() override;
    void OnCommissioningSessionStarted() override;
    void OnCommissioningSessionEstablishmentError(CHIP_ERROR err) override;
    void OnCommissioningSessionStopped() override;
    void OnCommissioningWindowOpened() override;
    void OnCommissioningWindowClosed() override;

    CHIP_ERROR InitializeWithObjects(jobject appDelegateObject);

private:
    chip::JniGlobalReference mChipAppServerDelegateObject;
    jmethodID mOnCommissioningSessionEstablishmentStartedMethod = nullptr;
    jmethodID mOnCommissioningSessionStartedMethod              = nullptr;
    jmethodID mOnCommissioningSessionEstablishmentErrorMethod   = nullptr;
    jmethodID mOnCommissioningSessionStoppedMethod              = nullptr;
    jmethodID mOnCommissioningWindowOpenedMethod                = nullptr;
    jmethodID mOnCommissioningWindowClosedMethod                = nullptr;
};

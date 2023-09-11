/*
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <app/server/AppDelegate.h>
#include <jni.h>
#include <lib/core/CHIPError.h>

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
    jobject mChipAppServerDelegateObject                        = nullptr;
    jmethodID mOnCommissioningSessionEstablishmentStartedMethod = nullptr;
    jmethodID mOnCommissioningSessionStartedMethod              = nullptr;
    jmethodID mOnCommissioningSessionEstablishmentErrorMethod   = nullptr;
    jmethodID mOnCommissioningSessionStoppedMethod              = nullptr;
    jmethodID mOnCommissioningWindowOpenedMethod                = nullptr;
    jmethodID mOnCommissioningWindowClosedMethod                = nullptr;
};

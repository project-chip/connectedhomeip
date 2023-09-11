/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "AppEventCommon.h"

class ContactSensorManager
{
public:
    enum class Action : uint8_t
    {
        kSignalDetected = 0,
        kSignalLost,
        kInvalid
    };

    enum class State : uint8_t
    {
        kContactClosed = 0,
        kContactOpened,
        kInvalid
    };

    int Init();
    bool IsContactClosed();
    void InitiateAction(Action aAction);

    typedef void (*CallbackStateChanged)(State aState);
    void SetCallback(CallbackStateChanged aCallbackStateChanged);

    static void HandleAction(AppEvent * aEvent);

private:
    friend ContactSensorManager & ContactSensorMgr(void);
    State mState;
    CallbackStateChanged mCallbackStateChanged;
    static ContactSensorManager sContactSensor;
};

inline ContactSensorManager & ContactSensorMgr(void)
{
    return ContactSensorManager::sContactSensor;
}

/*
 *
 * SPDX-FileCopyrightText: 2022 Google LLC.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "AppEvent.h"

#include "FreeRTOS.h"
#include "timers.h" // provides FreeRTOS timer support

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

    static void HandleAction(void * aGenericEvent);

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

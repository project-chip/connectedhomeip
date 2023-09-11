/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "esp_system.h"

struct AppEvent;
typedef void (*EventHandler)(AppEvent *);

struct AppEvent
{
    enum AppEventTypes
    {
        kEventType_Button = 0,
        kEventType_Timer,
        kEventType_Light,
        kEventType_Install,
    };

    uint16_t mType;

    union
    {
        struct
        {
            uint8_t mPinNo;
            uint8_t mAction;
        } mButtonEvent;
        struct
        {
            void * mContext;
        } mTimerEvent;
    };

    EventHandler mHandler;
};

/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef APP_EVENT_H
#define APP_EVENT_H

struct AppEvent;
typedef void (*EventHandler)(AppEvent *);

struct AppEvent
{
    enum AppEventType
    {
        kEventType_None = 0,
        kEventType_ButtonLeft,
        kEventType_ButtonRight,
        kEventType_AppEvent,
        kEventType_IdentifyStart,
        kEventType_IdentifyStop,
        kEventType_Light,
        kEventType_Timer,
    };

    enum AppEventButtonType
    {
        kAppEventButtonType_None = 0,
        kAppEventButtonType_Clicked,
        kAppEventButtonType_LongClicked,
    };

    enum AppEventIdentifyType
    {
        kAppEventIdentifyType_Blink = 0,
        kAppEventIdentifyType_Breathe,
        kAppEventIdentifyType_Okay,
        kAppEventIdentifyType_Finish,
        kAppEventIdentifyType_Stop,
    };

    enum AppEventType Type;

    union
    {
        struct
        {
            enum AppEventButtonType Type;
        } ButtonEvent;

        struct
        {
            void * Context;
        } TimerEvent;

        struct
        {
            uint8_t Action;
            int32_t Actor;
            void * Context;
        } LightEvent;

        struct
        {
            enum AppEventIdentifyType Type;
        } IdentifyEvent;
    };

    EventHandler Handler;
};

#endif // APP_EVENT_H

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
    };

    enum AppEventButtonType
    {
        kAppEventButtonType_None = 0,
        kAppEventButtonType_Clicked,
        kAppEventButtonType_LongClicked,
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
        } BoltLockEvent;
    };

    EventHandler Handler;
};

#endif // APP_EVENT_H

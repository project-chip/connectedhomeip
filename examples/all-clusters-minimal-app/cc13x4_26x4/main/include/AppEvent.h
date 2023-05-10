/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

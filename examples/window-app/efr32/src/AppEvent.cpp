/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include <AppEvent.h>

AppEvent::AppEvent(EventType type, void * context) : mType(type), mContext(context) {}

const char * AppEvent::TypeString(EventType type)
{
    switch (type)
    {
    case EventType::None:
        return "None";

    case EventType::ButtonPressed:
        return "Button Pressed";
    case EventType::ButtonReleased:
        return "Button Released";

    case EventType::CoverTypeChange:
        return "Cover Type Changed";
    case EventType::CoverTiltModeChange:
        return "Tilt Mode Changed";
    case EventType::CoverLiftUp:
        return "Lift Up";
    case EventType::CoverLiftDown:
        return "Lift Down";
    case EventType::CoverTiltUp:
        return "Tilt Up";
    case EventType::CoverTiltDown:
        return "Tilt Down";
    case EventType::CoverOpen:
        return "Open";
    case EventType::CoverClosed:
        return "Close";
    case EventType::CoverStart:
        return "Start";
    case EventType::CoverStop:
        return "Stop";

    default:
        return "?";
    }
}

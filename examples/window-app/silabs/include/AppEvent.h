/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include "BaseAppEvent.h"
#include <app/clusters/window-covering-server/window-covering-server.h>
#include <lib/core/CHIPError.h>

using namespace chip::app::Clusters::WindowCovering;

struct AppEvent : public BaseAppEvent
{
    enum AppEventTypes
    {
        kEventType_ResetWarning = BaseAppEvent::kEventType_Max + 1,
        kEventType_ResetCanceled,
        // Button events
        kEventType_UpPressed,
        kEventType_UpReleased,
        kEventType_DownPressed,
        kEventType_DownReleased,
        // Cover events
        kEventType_CoverChange,
        kEventType_CoverTypeChange,
        kEventType_TiltModeChange,

        // Cover Attribute update events
        kEventType_AttributeChange,
    };

    chip::EndpointId mEndpoint = 0;
    chip::AttributeId mAttributeId;

    union
    {
        struct
        {
            void * Context;
        } WindowEvent;
    };
};

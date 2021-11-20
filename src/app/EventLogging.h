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

#pragma once

#include <app/ConcreteEventPath.h>
#include <app/EventLoggingDelegate.h>
#include <app/data-model/Decode.h>
#include <app/data-model/List.h> // So we can encode lists

namespace chip {
namespace app {

template <typename T>
class EventLogger : EventLoggingDelegate
{
public:
    EventLogger(const T & aEventData) : mEventData(aEventData){};
    CHIP_ERROR WriteEvent(chip::TLV::TLVWriter & aWriter) final override { return mEventData.Encode(aWriter, TLV::AnonymousTag); }

private:
    const T & mEventData;
};

template <typename T>
CHIP_ERROR LogEvent(const T & aEventData, EndpointId aEndpoint, EventOptions aEventOptions, EventNumber & aEventNumber)
{
    EventLogger<T> eventData(aEventData);
    ConcreteEventPath path(aEndpoint, aEventData.GetClusterId(), aEventData.GetEventId());
    // log the actual event
    aEventNumber = 0;
    return CHIP_NO_ERROR;
}

} // namespace app
} // namespace chip

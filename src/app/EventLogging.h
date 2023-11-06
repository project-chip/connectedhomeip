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
#include <app/EventManagement.h>
#include <app/data-model/Encode.h>
#include <app/data-model/FabricScoped.h>
#include <app/data-model/List.h> // So we can encode lists

namespace chip {
namespace app {

template <typename T>
class EventLogger : public EventLoggingDelegate
{
public:
    EventLogger(const T & aEventData) : mEventData(aEventData){};
    CHIP_ERROR WriteEvent(chip::TLV::TLVWriter & aWriter) final override
    {
        return DataModel::Encode(aWriter, TLV::ContextTag(EventDataIB::Tag::kData), mEventData);
    }

private:
    const T & mEventData;
};

/**
 * @brief
 *   Log an event via a EventLoggingDelegate, with options.
 *
 * The EventLoggingDelegate writes the event metadata and calls the `apDelegate`
 * with an TLV::TLVWriter reference so that the user code can emit
 * the event data directly into the event log.  This form of event
 * logging minimizes memory consumption, as event data is serialized
 * directly into the target buffer.  The event data MUST contain
 * context tags to be interpreted within the schema identified by
 * `ClusterID` and `EventId`.
 *
 * LogEvent has 2 variant, one for fabric-scoped events and one for non-fabric-scoped events.
 * @param[in] aEventData  The event cluster object
 * @param[in] aEndpoint    The current cluster's Endpoint Id
 * @param[out] aEventNumber The event Number if the event was written to the
 *                         log, 0 otherwise. The Event number is expected to monotonically increase.
 *
 * @return CHIP_ERROR  CHIP Error Code
 */
template <typename T, std::enable_if_t<DataModel::IsFabricScoped<T>::value, bool> = true>
CHIP_ERROR LogEvent(const T & aEventData, EndpointId aEndpoint, EventNumber & aEventNumber)
{
    EventLogger<T> eventData(aEventData);
    ConcreteEventPath path(aEndpoint, aEventData.GetClusterId(), aEventData.GetEventId());
    EventManagement & logMgmt = chip::app::EventManagement::GetInstance();
    EventOptions eventOptions;
    eventOptions.mPath        = path;
    eventOptions.mPriority    = aEventData.GetPriorityLevel();
    eventOptions.mFabricIndex = aEventData.GetFabricIndex();
    // this skips logging the event if it's fabric-scoped but no fabric association exists yet.
    VerifyOrReturnError(eventOptions.mFabricIndex != kUndefinedFabricIndex, CHIP_ERROR_INVALID_FABRIC_INDEX);

    //
    // Unlike attributes which have a different 'EncodeForRead' for fabric-scoped structs,
    // fabric-sensitive events don't require that since the actual omission of the event in its entirety
    // happens within the event management framework itself at the time of access.
    //
    // The 'mFabricIndex' field in the event options above is encoded out-of-band alongside the event payload
    // and used to match against the accessing fabric.
    //
    return logMgmt.LogEvent(&eventData, eventOptions, aEventNumber);
}

template <typename T, std::enable_if_t<!DataModel::IsFabricScoped<T>::value, bool> = true>
CHIP_ERROR LogEvent(const T & aEventData, EndpointId aEndpoint, EventNumber & aEventNumber)
{
    EventLogger<T> eventData(aEventData);
    ConcreteEventPath path(aEndpoint, aEventData.GetClusterId(), aEventData.GetEventId());
    EventManagement & logMgmt = chip::app::EventManagement::GetInstance();
    EventOptions eventOptions;
    eventOptions.mPath     = path;
    eventOptions.mPriority = aEventData.GetPriorityLevel();
    return logMgmt.LogEvent(&eventData, eventOptions, aEventNumber);
}

} // namespace app
} // namespace chip

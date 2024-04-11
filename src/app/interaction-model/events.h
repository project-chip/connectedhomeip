/*
 *    Copyright (c) 2024 Project CHIP Authors
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

#include <app/EventLoggingDelegate.h>
#include <app/EventLoggingTypes.h>
#include <app/MessageDef/EventDataIB.h>
#include <app/data-model/Encode.h>
#include <app/data-model/FabricScoped.h>
#include <lib/core/CHIPError.h>
#include <lib/support/logging/CHIPLogging.h>

#include <type_traits>

namespace chip {
namespace app {
namespace InteractionModel {

namespace internal {
template <typename T>
class SimpleEventLoggingDelegate : public EventLoggingDelegate
{
public:
    SimpleEventLoggingDelegate(const T & aEventData) : mEventData(aEventData){};
    CHIP_ERROR WriteEvent(chip::TLV::TLVWriter & aWriter) final override
    {
        return DataModel::Encode(aWriter, TLV::ContextTag(EventDataIB::Tag::kData), mEventData);
    }

private:
    const T & mEventData;
};

template <typename E, typename T, std::enable_if_t<DataModel::IsFabricScoped<T>::value, bool> = true>
EventNumber GenerateEvent(E & emittor, const T & aEventData, EndpointId aEndpoint)
{
    internal::SimpleEventLoggingDelegate<T> eventData(aEventData);
    ConcreteEventPath path(aEndpoint, aEventData.GetClusterId(), aEventData.GetEventId());
    EventOptions eventOptions;
    eventOptions.mPath        = path;
    eventOptions.mPriority    = aEventData.GetPriorityLevel();
    eventOptions.mFabricIndex = aEventData.GetFabricIndex();

    // this skips logging the event if it's fabric-scoped but no fabric association exists yet.

    if (eventOptions.mFabricIndex == kUndefinedFabricIndex)
    {
        ChipLogError(EventLogging, "Event encode failure: no fabric index for fabric scoped event");
        return kInvalidEventId;
    }

    //
    // Unlike attributes which have a different 'EncodeForRead' for fabric-scoped structs,
    // fabric-sensitive events don't require that since the actual omission of the event in its entirety
    // happens within the event management framework itself at the time of access.
    //
    // The 'mFabricIndex' field in the event options above is encoded out-of-band alongside the event payload
    // and used to match against the accessing fabric.
    //
    EventNumber eventNumber;
    CHIP_ERROR err = emittor.GenerateEvent(&eventData, eventOptions, eventNumber);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(EventLogging, "Failed to log event: %" CHIP_ERROR_FORMAT, err.Format());
        return kInvalidEventId;
    }

    return eventNumber;
}

template <typename E, typename T, std::enable_if_t<!DataModel::IsFabricScoped<T>::value, bool> = true>
EventNumber GenerateEvent(E & emittor, const T & aEventData, EndpointId aEndpoint)
{
    internal::SimpleEventLoggingDelegate<T> eventData(aEventData);
    ConcreteEventPath path(aEndpoint, aEventData.GetClusterId(), aEventData.GetEventId());
    EventOptions eventOptions;
    eventOptions.mPath     = path;
    eventOptions.mPriority = aEventData.GetPriorityLevel();
    EventNumber eventNumber;
    CHIP_ERROR err = emittor.GenerateEvent(&eventData, eventOptions, eventNumber);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(EventLogging, "Failed to log event: %" CHIP_ERROR_FORMAT, err.Format());
        return kInvalidEventId;
    }

    return eventNumber;
}

} // namespace internal

class Events
{
public:
    virtual ~Events() = default;

    /// Emits the given event
    virtual CHIP_ERROR GenerateEvent(EventLoggingDelegate * eventContentWriter, const EventOptions & options,
                                 EventNumber & generatedEventNumber) = 0;

    // Convenience methods for event logging using cluster-object structures
    // On error, these log and return kInvalidEventId
    template <typename T>
    EventNumber GenerateEvent(const T & aEventData, EndpointId aEndpoint)
    {
        return internal::GenerateEvent(*this, aEventData, aEndpoint);
    }
};

} // namespace InteractionModel
} // namespace app
} // namespace chip

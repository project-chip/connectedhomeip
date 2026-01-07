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

#include <app/ConcreteEventPath.h>
#include <app/EventLoggingDelegate.h>
#include <app/EventLoggingTypes.h>
#include <app/MessageDef/EventDataIB.h>
#include <app/data-model/Encode.h>
#include <app/data-model/FabricScoped.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/logging/CHIPLogging.h>

#include <optional>

namespace chip {
namespace app {
namespace DataModel {

class EventsGenerator;

namespace internal {
template <typename T>
class SimpleEventPayloadWriter : public EventLoggingDelegate
{
public:
    SimpleEventPayloadWriter(const T & aEventData) : mEventData(aEventData){};
    CHIP_ERROR WriteEvent(chip::TLV::TLVWriter & aWriter) final override
    {
        return DataModel::Encode(aWriter, TLV::ContextTag(EventDataIB::Tag::kData), mEventData);
    }

private:
    const T & mEventData;
};

std::optional<EventNumber> GenerateEvent(const EventOptions & eventOptions, EventsGenerator & generator,
                                         EventLoggingDelegate & delegate, bool isFabricSensitiveEvent);

} // namespace internal

/// Exposes event generation capabilities.
///
/// Allows callers to "generate events" which effectively notifies of an event having
/// ocurred.
class EventsGenerator
{
public:
    virtual ~EventsGenerator() = default;

    /// Scnedule event delivery to happen immediately (and synchronously).
    ///
    /// Use when it is imperative that some events are to be delivered because
    /// they will not be delivered soon after (e.g. device shutdown events or
    /// fabric removal events).
    ///
    /// This can be done either for a specific fabric, identified by the provided
    /// FabricIndex, or across all fabrics if no FabricIndex is provided.
    virtual void ScheduleUrgentEventDeliverySync(std::optional<FabricIndex> fabricIndex = std::nullopt) = 0;

    /// Generates the given event.
    ///
    /// Events are generally expected to be sent to subscribed clients and also
    /// be available for read later until they get overwritten by new events
    /// that are being generated.
    virtual CHIP_ERROR GenerateEvent(EventLoggingDelegate * eventPayloadWriter, const EventOptions & options,
                                     EventNumber & generatedEventNumber) = 0;

    // Convenience methods for event logging using cluster-object structures
    //
    // On error, these log and return nullopt.
    template <typename T>
    std::optional<EventNumber> GenerateEvent(const T & eventData, EndpointId endpointId)
    {
        internal::SimpleEventPayloadWriter<T> eventPayloadWriter(eventData);
        constexpr bool isFabricSensitiveEvent = DataModel::IsFabricScoped<T>::value;
        return internal::GenerateEvent({ endpointId, eventData }, *this, eventPayloadWriter, isFabricSensitiveEvent);
    }
};

} // namespace DataModel
} // namespace app
} // namespace chip

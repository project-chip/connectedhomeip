/*
 *    Copyright (c) 2025 Project CHIP Authors
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
#include <app/data-model-provider/EventsGenerator.h>
#include <app/data-model/Decode.h>
#include <deque>

namespace chip {
namespace Testing {

/// Keeps a queue of generated events that can be acquired later for testing purposes
class LogOnlyEvents : public app::DataModel::EventsGenerator
{
public:
    // struct to hold information about a generated event
    struct EventInformation
    {
        EventNumber eventNumber;
        app::EventOptions eventOptions;
        bool wasDeliveredUrgently{ false };
        ByteSpan GetEncodeByteSpan() const { return ByteSpan(mEventEncodeBuffer, mEncodedLength); }

        // This relies on the default encoding of events which uses
        // DataModel::Encode on a EventDataIB::Tag::kData
        // The caller MUST ensure that T is the correct type for the event
        // Use app::Clusters::<ClusterName>::Events::<EventName>::DecodableType to be spec compliant
        template <typename T>
        CHIP_ERROR GetEventData(T & dest)
        {
            // attempt to decode the last encoded event
            TLV::TLVReader reader;
            TLV::TLVType outerType;

            reader.Init(GetEncodeByteSpan());

            ReturnErrorOnFailure(reader.Next());
            ReturnErrorOnFailure(reader.EnterContainer(outerType));

            ReturnErrorOnFailure(reader.Next()); // MUST be positioned on the first element
            ReturnErrorOnFailure(app::DataModel::Decode(reader, dest));

            ReturnErrorOnFailure(reader.ExitContainer(outerType));

            return CHIP_NO_ERROR;
        }

    private:
        uint8_t mEventEncodeBuffer[128];
        uint32_t mEncodedLength;

        friend class LogOnlyEvents;
    };

    // Marks the last event (that has the given fabric index if given any) as delivered urgently.
    void ScheduleUrgentEventDeliverySync(std::optional<FabricIndex> fabricIndex = std::nullopt) override
    {
        for (auto it = mEventQueue.rbegin(); it != mEventQueue.rend(); ++it)
        {
            if (!fabricIndex.has_value() || it->eventOptions.mFabricIndex == fabricIndex.value())
            {
                it->wasDeliveredUrgently = true;
                break;
            }
        }
    }

    CHIP_ERROR GenerateEvent(app::EventLoggingDelegate * eventContentWriter, const app::EventOptions & options,
                             EventNumber & generatedEventNumber) override
    {
        TLV::TLVWriter writer;
        TLV::TLVType outerType;

        EventInformation eventInfo;
        writer.Init(eventInfo.mEventEncodeBuffer);

        ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, outerType));
        ReturnErrorOnFailure(eventContentWriter->WriteEvent(writer));
        ReturnErrorOnFailure(writer.EndContainer(outerType));
        ReturnErrorOnFailure(writer.Finalize());
        eventInfo.mEncodedLength = writer.GetLengthWritten();
        eventInfo.eventOptions   = options;
        eventInfo.eventNumber = generatedEventNumber = ++mCurrentEventNumber;

        mEventQueue.push_back(eventInfo);

        return CHIP_NO_ERROR;
    }

    // Returns next event in the event queue, removing it from the queue.
    // Returns `std::nullopt` if no event is in the queue (i.e. no event was generated after consuming last generated one).
    [[nodiscard]] std::optional<EventInformation> GetNextEvent()
    {
        if (mEventQueue.empty())
        {
            return std::nullopt;
        }
        std::optional<EventInformation> info{ std::move(mEventQueue.front()) };
        mEventQueue.pop_front();
        return info;
    }

private:
    std::deque<EventInformation> mEventQueue;
    EventNumber mCurrentEventNumber = 0;
};

} // namespace Test
} // namespace chip

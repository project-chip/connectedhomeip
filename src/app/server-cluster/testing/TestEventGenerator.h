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

namespace chip {
namespace Test {

/// Keeps the "last event" in-memory to allow tests to validate
/// that event writing and encoding worked.
///
/// Provides the ability to get last event information as well as
/// to decode the last event contents for inspection.
class LogOnlyEvents : public app::DataModel::EventsGenerator
{
public:
    CHIP_ERROR GenerateEvent(app::EventLoggingDelegate * eventContentWriter, const app::EventOptions & options,
                             EventNumber & generatedEventNumber) override
    {
        TLV::TLVWriter writer;
        TLV::TLVType outerType;
        writer.Init(mLastEventEncodeBuffer);

        ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, outerType));
        ReturnErrorOnFailure(eventContentWriter->WriteEvent(writer));
        ReturnErrorOnFailure(writer.EndContainer(outerType));
        ReturnErrorOnFailure(writer.Finalize());
        mLastEncodedSpan = ByteSpan(mLastEventEncodeBuffer, writer.GetLengthWritten());

        mLastOptions         = options;
        generatedEventNumber = ++mCurrentEventNumber;

        return CHIP_NO_ERROR;
    }

    [[nodiscard]] EventNumber CurrentEventNumber() const { return mCurrentEventNumber; }
    [[nodiscard]] const app::EventOptions & LastOptions() const { return mLastOptions; }
    [[nodiscard]] ByteSpan LastWrittenEvent() const { return mLastEncodedSpan; }

    // This relies on the default encoding of events which uses
    // DataModel::Encode on a EventDataIB::Tag::kData
    template <typename T>
    CHIP_ERROR DecodeLastEvent(T & dest)
    {
        // attempt to decode the last encoded event
        TLV::TLVReader reader;
        TLV::TLVType outerType;

        reader.Init(LastWrittenEvent());

        ReturnErrorOnFailure(reader.Next());
        ReturnErrorOnFailure(reader.EnterContainer(outerType));

        ReturnErrorOnFailure(reader.Next()); // MUST be positioned on the first element
        ReturnErrorOnFailure(app::DataModel::Decode(reader, dest));

        ReturnErrorOnFailure(reader.ExitContainer(outerType));

        return CHIP_NO_ERROR;
    }

private:
    EventNumber mCurrentEventNumber = 0;
    app::EventOptions mLastOptions;
    uint8_t mLastEventEncodeBuffer[128];
    ByteSpan mLastEncodedSpan;
};

} // namespace Test
} // namespace chip

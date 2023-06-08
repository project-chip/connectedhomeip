/**
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include "EventReportIB.h"
#include "EventDataIB.h"
#include "MessageDefHelper.h"
#include "StructBuilder.h"
#include "StructParser.h"

#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>

#include <app/AppConfig.h>

namespace chip {
namespace app {
#if CHIP_CONFIG_IM_PRETTY_PRINT
CHIP_ERROR EventReportIB::Parser::PrettyPrint() const
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    TLV::TLVReader reader;

    PRETTY_PRINT("EventReportIB =");
    PRETTY_PRINT("{");

    // make a copy of the reader
    reader.Init(mReader);

    while (CHIP_NO_ERROR == (err = reader.Next()))
    {
        if (!TLV::IsContextTag(reader.GetTag()))
        {
            continue;
        }
        uint32_t tagNum = TLV::TagNumFromTag(reader.GetTag());
        switch (tagNum)
        {
        case to_underlying(Tag::kEventStatus): {
            EventStatusIB::Parser eventStatus;
            ReturnErrorOnFailure(eventStatus.Init(reader));

            PRETTY_PRINT_INCDEPTH();
            ReturnErrorOnFailure(eventStatus.PrettyPrint());
            PRETTY_PRINT_DECDEPTH();
        }
        break;
        case to_underlying(Tag::kEventData): {
            EventDataIB::Parser eventData;
            ReturnErrorOnFailure(eventData.Init(reader));

            PRETTY_PRINT_INCDEPTH();
            ReturnErrorOnFailure(eventData.PrettyPrint());
            PRETTY_PRINT_DECDEPTH();
        }
        break;
        default:
            PRETTY_PRINT("Unknown tag num %" PRIu32, tagNum);
            break;
        }
    }

    PRETTY_PRINT("},");
    PRETTY_PRINT_BLANK_LINE();

    if (CHIP_END_OF_TLV == err)
    {
        err = CHIP_NO_ERROR;
    }

    ReturnErrorOnFailure(err);
    return reader.ExitContainer(mOuterContainerType);
}
#endif // CHIP_CONFIG_IM_PRETTY_PRINT

CHIP_ERROR EventReportIB::Parser::GetEventStatus(EventStatusIB::Parser * const apEventStatus) const
{
    TLV::TLVReader reader;
    ReturnErrorOnFailure(mReader.FindElementWithTag(TLV::ContextTag(Tag::kEventStatus), reader));
    return apEventStatus->Init(reader);
}

CHIP_ERROR EventReportIB::Parser::GetEventData(EventDataIB::Parser * const apEventData) const
{
    TLV::TLVReader reader;
    ReturnErrorOnFailure(mReader.FindElementWithTag(TLV::ContextTag(Tag::kEventData), reader));
    return apEventData->Init(reader);
}

EventStatusIB::Builder & EventReportIB::Builder::CreateEventStatus()
{
    if (mError == CHIP_NO_ERROR)
    {
        mError = mEventStatus.Init(mpWriter, to_underlying(Tag::kEventStatus));
    }
    return mEventStatus;
}

EventDataIB::Builder & EventReportIB::Builder::CreateEventData()
{
    if (mError == CHIP_NO_ERROR)
    {
        mError = mEventData.Init(mpWriter, to_underlying(Tag::kEventData));
    }
    return mEventData;
}

CHIP_ERROR EventReportIB::Builder::EndOfEventReportIB()
{
    EndOfContainer();
    return GetError();
}

CHIP_ERROR EventReportIB::ConstructEventStatusIB(TLV::TLVWriter & aWriter, const ConcreteEventPath & aEvent, StatusIB aStatus)
{
    Builder eventReportIBBuilder;
    ReturnErrorOnFailure(eventReportIBBuilder.Init(&aWriter));
    EventStatusIB::Builder & eventStatusIBBuilder = eventReportIBBuilder.CreateEventStatus();
    ReturnErrorOnFailure(eventReportIBBuilder.GetError());
    EventPathIB::Builder & eventPathIBBuilder = eventStatusIBBuilder.CreatePath();
    ReturnErrorOnFailure(eventStatusIBBuilder.GetError());
    ReturnErrorOnFailure(
        eventPathIBBuilder.Endpoint(aEvent.mEndpointId).Cluster(aEvent.mClusterId).Event(aEvent.mEventId).EndOfEventPathIB());

    ReturnErrorOnFailure(eventStatusIBBuilder.CreateErrorStatus().EncodeStatusIB(aStatus).GetError());

    ReturnErrorOnFailure(eventStatusIBBuilder.EndOfEventStatusIB());
    ReturnErrorOnFailure(eventReportIBBuilder.EndOfEventReportIB());
    ReturnErrorOnFailure(aWriter.Finalize());
    return CHIP_NO_ERROR;
}
} // namespace app
} // namespace chip

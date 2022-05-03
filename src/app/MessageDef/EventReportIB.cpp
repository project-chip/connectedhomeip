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

#include <app/AppBuildConfig.h>

namespace chip {
namespace app {
#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
CHIP_ERROR EventReportIB::Parser::CheckSchemaValidity() const
{
    CHIP_ERROR err      = CHIP_NO_ERROR;
    int TagPresenceMask = 0;
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
        case to_underlying(Tag::kEventStatus):
            // check if this tag has appeared before
            VerifyOrReturnError(!(TagPresenceMask & (1 << to_underlying(Tag::kEventStatus))), CHIP_ERROR_INVALID_TLV_TAG);
            TagPresenceMask |= (1 << to_underlying(Tag::kEventStatus));
            {
                EventStatusIB::Parser eventStatus;
                ReturnErrorOnFailure(eventStatus.Init(reader));

                PRETTY_PRINT_INCDEPTH();
                ReturnErrorOnFailure(eventStatus.CheckSchemaValidity());
                PRETTY_PRINT_DECDEPTH();
            }
            break;
        case to_underlying(Tag::kEventData):
            // check if this tag has appeared before
            VerifyOrReturnError(!(TagPresenceMask & (1 << to_underlying(Tag::kEventData))), CHIP_ERROR_INVALID_TLV_TAG);
            TagPresenceMask |= (1 << to_underlying(Tag::kEventData));
            {
                EventDataIB::Parser eventData;
                ReturnErrorOnFailure(eventData.Init(reader));

                PRETTY_PRINT_INCDEPTH();
                ReturnErrorOnFailure(eventData.CheckSchemaValidity());
                PRETTY_PRINT_DECDEPTH();
            }
            break;
        default:
            PRETTY_PRINT("Unknown tag num %" PRIu32, tagNum);
            break;
        }
    }

    PRETTY_PRINT("},");
    PRETTY_PRINT("");

    if (CHIP_END_OF_TLV == err)
    {
        // check for at most field:
        const int CheckDataField   = 1 << to_underlying(Tag::kEventData);
        const int CheckStatusField = (1 << to_underlying(Tag::kEventStatus));

        if ((TagPresenceMask & CheckDataField) == CheckDataField && (TagPresenceMask & CheckStatusField) == CheckStatusField)
        {
            // kEventData and kEventStatus both exist
            err = CHIP_ERROR_IM_MALFORMED_EVENT_REPORT_IB;
        }
        else if ((TagPresenceMask & CheckDataField) != CheckDataField && (TagPresenceMask & CheckStatusField) != CheckStatusField)
        {
            // kEventData and kErrorStatus not exist
            err = CHIP_ERROR_IM_MALFORMED_EVENT_REPORT_IB;
        }
        else
        {
            err = CHIP_NO_ERROR;
        }
    }

    ReturnErrorOnFailure(err);
    ReturnErrorOnFailure(reader.ExitContainer(mOuterContainerType));
    return CHIP_NO_ERROR;
}
#endif // CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK

CHIP_ERROR EventReportIB::Parser::GetEventStatus(EventStatusIB::Parser * const apEventStatus) const
{
    TLV::TLVReader reader;
    ReturnErrorOnFailure(mReader.FindElementWithTag(TLV::ContextTag(to_underlying(Tag::kEventStatus)), reader));
    ReturnErrorOnFailure(apEventStatus->Init(reader));
    return CHIP_NO_ERROR;
}

CHIP_ERROR EventReportIB::Parser::GetEventData(EventDataIB::Parser * const apEventData) const
{
    TLV::TLVReader reader;
    ReturnErrorOnFailure(mReader.FindElementWithTag(TLV::ContextTag(to_underlying(Tag::kEventData)), reader));
    ReturnErrorOnFailure(apEventData->Init(reader));
    return CHIP_NO_ERROR;
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

EventReportIB::Builder & EventReportIB::Builder::EndOfEventReportIB()
{
    EndOfContainer();
    return *this;
}
} // namespace app
} // namespace chip

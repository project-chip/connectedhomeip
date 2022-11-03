/*
 * SPDX-FileCopyrightText: (c) 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "EventFilterIBs.h"

#include "MessageDefHelper.h"

#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>

#include <app/AppConfig.h>

namespace chip {
namespace app {
#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
CHIP_ERROR EventFilterIBs::Parser::CheckSchemaValidity() const
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    TLV::TLVReader reader;

    PRETTY_PRINT("EventFilterIBs =");
    PRETTY_PRINT("[");

    // make a copy of the reader
    reader.Init(mReader);

    while (CHIP_NO_ERROR == (err = reader.Next()))
    {
        VerifyOrReturnError(TLV::AnonymousTag() == reader.GetTag(), CHIP_ERROR_INVALID_TLV_TAG);
        {
            EventFilterIB::Parser eventFilter;
            ReturnErrorOnFailure(eventFilter.Init(reader));
            PRETTY_PRINT_INCDEPTH();
            ReturnErrorOnFailure(eventFilter.CheckSchemaValidity());
            PRETTY_PRINT_DECDEPTH();
        }
    }

    PRETTY_PRINT("],");
    PRETTY_PRINT_BLANK_LINE();

    // if we have exhausted this container
    if (CHIP_END_OF_TLV == err)
    {
        err = CHIP_NO_ERROR;
    }
    ReturnErrorOnFailure(err);
    return reader.ExitContainer(mOuterContainerType);
}
#endif // CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK

EventFilterIB::Builder & EventFilterIBs::Builder::CreateEventFilter()
{
    mError = mEventFilter.Init(mpWriter);
    return mEventFilter;
}

EventFilterIBs::Builder & EventFilterIBs::Builder::EndOfEventFilters()
{
    EndOfContainer();
    return *this;
}

CHIP_ERROR EventFilterIBs::Builder::GenerateEventFilter(EventNumber aEventNumber)
{
    EventFilterIB::Builder & eventFilter = CreateEventFilter();
    ReturnErrorOnFailure(GetError());
    ReturnErrorOnFailure(eventFilter.EventMin(aEventNumber).EndOfEventFilterIB().GetError());
    ReturnErrorOnFailure(EndOfEventFilters().GetError());
    return CHIP_NO_ERROR;
}

}; // namespace app
}; // namespace chip

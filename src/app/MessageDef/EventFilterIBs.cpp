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

#include "EventFilterIBs.h"

#include "MessageDefHelper.h"

#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>

#include <app/AppBuildConfig.h>

namespace chip {
namespace app {
#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
CHIP_ERROR EventFilterIBs::Parser::CheckSchemaValidity() const
{
    CHIP_ERROR err         = CHIP_NO_ERROR;
    size_t numEventFilters = 0;
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

        ++numEventFilters;
    }

    PRETTY_PRINT("],");
    PRETTY_PRINT("");

    // if we have exhausted this container
    if (CHIP_END_OF_TLV == err)
    {
        // if we have at least one event filter
        if (numEventFilters > 0)
        {
            err = CHIP_NO_ERROR;
        }
    }
    ReturnErrorOnFailure(err);
    ReturnErrorOnFailure(reader.ExitContainer(mOuterContainerType));
    return CHIP_NO_ERROR;
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
}; // namespace app
}; // namespace chip

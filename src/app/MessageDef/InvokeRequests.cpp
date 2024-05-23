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

#include "InvokeRequests.h"

#include "MessageDefHelper.h"

#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>

#include <app/AppConfig.h>

namespace chip {
namespace app {
#if CHIP_CONFIG_IM_PRETTY_PRINT
CHIP_ERROR InvokeRequests::Parser::PrettyPrint() const
{
    CHIP_ERROR err         = CHIP_NO_ERROR;
    size_t numCommandDatas = 0;
    TLV::TLVReader reader;

    PRETTY_PRINT("InvokeRequests =");
    PRETTY_PRINT("[");

    // make a copy of the reader
    reader.Init(mReader);

    while (CHIP_NO_ERROR == (err = reader.Next()))
    {
        VerifyOrReturnError(TLV::AnonymousTag() == reader.GetTag(), CHIP_ERROR_INVALID_TLV_TAG);
        {
            CommandDataIB::Parser commandData;
            ReturnErrorOnFailure(commandData.Init(reader));
            PRETTY_PRINT_INCDEPTH();
            ReturnErrorOnFailure(commandData.PrettyPrint());
            PRETTY_PRINT_DECDEPTH();
        }

        ++numCommandDatas;
    }

    PRETTY_PRINT("],");
    PRETTY_PRINT_BLANK_LINE();

    // if we have exhausted this container
    if (CHIP_END_OF_TLV == err)
    {
        // if we have at least one data element
        if (numCommandDatas > 0)
        {
            err = CHIP_NO_ERROR;
        }
    }
    ReturnErrorOnFailure(err);
    return reader.ExitContainer(mOuterContainerType);
}
#endif // CHIP_CONFIG_IM_PRETTY_PRINT

CHIP_ERROR InvokeRequests::Builder::InitWithEndBufferReserved(TLV::TLVWriter * const apWriter, const uint8_t aContextTagToUse)
{
    ReturnErrorOnFailure(Init(apWriter, aContextTagToUse));
    ReturnErrorOnFailure(GetWriter()->ReserveBuffer(GetSizeToEndInvokeRequests()));
    mIsEndBufferReserved = true;
    return CHIP_NO_ERROR;
}

CommandDataIB::Builder & InvokeRequests::Builder::CreateCommandData()
{
    if (mError == CHIP_NO_ERROR)
    {
        mError = mCommandData.Init(mpWriter);
    }
    return mCommandData;
}

CHIP_ERROR InvokeRequests::Builder::EndOfInvokeRequests()
{
    // If any changes are made to how we end the invoke requests that involves how many bytes are
    // needed, a corresponding change to GetSizeToEndInvokeRequests indicating the new size that
    // will be required.
    if (mIsEndBufferReserved)
    {
        ReturnErrorOnFailure(GetWriter()->UnreserveBuffer(GetSizeToEndInvokeRequests()));
        mIsEndBufferReserved = false;
    }
    EndOfContainer();
    return GetError();
}

uint32_t InvokeRequests::Builder::GetSizeToEndInvokeRequests()
{
    uint32_t kEndOfContainerSize = 1;
    return kEndOfContainerSize;
}
} // namespace app
} // namespace chip

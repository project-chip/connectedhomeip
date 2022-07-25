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
#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
CHIP_ERROR InvokeRequests::Parser::CheckSchemaValidity() const
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
            ReturnErrorOnFailure(commandData.CheckSchemaValidity());
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
#endif // CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK

CommandDataIB::Builder & InvokeRequests::Builder::CreateCommandData()
{
    if (mError == CHIP_NO_ERROR)
    {
        mError = mCommandData.Init(mpWriter);
    }
    return mCommandData;
}

InvokeRequests::Builder & InvokeRequests::Builder::EndOfInvokeRequests()
{
    EndOfContainer();
    return *this;
}
} // namespace app
} // namespace chip

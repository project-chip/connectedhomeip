/**
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2018 Google LLC.
 *    Copyright (c) 2016-2017 Nest Labs, Inc.
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
/**
 *    @file
 *      This file defines InvokeCommand parser and builder in CHIP interaction model
 *
 */

#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>

#include "InvokeCommand.h"
#include "MessageDefHelper.h"

using namespace chip;
using namespace chip::TLV;

namespace chip {
namespace app {
CHIP_ERROR InvokeCommand::Parser::Init(const chip::TLV::TLVReader & aReader)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    // make a copy of the reader here
    mReader.Init(aReader);

    VerifyOrExit(chip::TLV::kTLVType_Structure == mReader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);

    err = mReader.EnterContainer(mOuterContainerType);

exit:
    ChipLogFunctError(err);

    return err;
}

#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
CHIP_ERROR InvokeCommand::Parser::CheckSchemaValidity() const
{
    CHIP_ERROR err           = CHIP_NO_ERROR;
    uint16_t TagPresenceMask = 0;
    chip::TLV::TLVReader reader;
    CommandList::Parser commandList;

    PRETTY_PRINT("InvokeCommand =");
    PRETTY_PRINT("{");

    // make a copy of the reader
    reader.Init(mReader);

    while (CHIP_NO_ERROR == (err = reader.Next()))
    {
        const uint64_t tag = reader.GetTag();

        if (chip::TLV::ContextTag(kCsTag_CommandList) == tag)
        {
            VerifyOrExit(!(TagPresenceMask & (1 << kCsTag_CommandList)), err = CHIP_ERROR_INVALID_TLV_TAG);
            TagPresenceMask |= (1 << kCsTag_CommandList);
            VerifyOrExit(chip::TLV::kTLVType_Array == reader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);

            commandList.Init(reader);

            PRETTY_PRINT_INCDEPTH();

            err = commandList.CheckSchemaValidity();
            SuccessOrExit(err);

            PRETTY_PRINT_DECDEPTH();
        }
        else
        {
            PRETTY_PRINT("\tUnknown tag 0x%" PRIx64, tag);
        }
    }

    PRETTY_PRINT("}");
    PRETTY_PRINT("");

    // if we have exhausted this container
    if (CHIP_END_OF_TLV == err)
    {
        // if we have at least the DataList or EventList field
        if ((TagPresenceMask & (1 << kCsTag_CommandList)))
        {
            err = CHIP_NO_ERROR;
        }
    }
    SuccessOrExit(err);
    err = reader.ExitContainer(mOuterContainerType);

exit:
    ChipLogFunctError(err);

    return err;
}
#endif // CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK

CHIP_ERROR InvokeCommand::Parser::GetCommandList(CommandList::Parser * const apCommandList) const
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::TLV::TLVReader reader;

    err = mReader.FindElementWithTag(chip::TLV::ContextTag(kCsTag_CommandList), reader);
    SuccessOrExit(err);

    VerifyOrExit(chip::TLV::kTLVType_Array == reader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);

    err = apCommandList->Init(reader);
    SuccessOrExit(err);

exit:
    ChipLogIfFalse((CHIP_NO_ERROR == err) || (CHIP_END_OF_TLV == err));

    return err;
}

CHIP_ERROR InvokeCommand::Builder::Init(chip::TLV::TLVWriter * const apWriter)
{
    return InitAnonymousStructure(apWriter);
}

CommandList::Builder & InvokeCommand::Builder::CreateCommandListBuilder()
{
    // skip if error has already been set
    VerifyOrExit(CHIP_NO_ERROR == mError, mCommandListBuilder.ResetError(mError));

    mError = mCommandListBuilder.Init(mpWriter, kCsTag_CommandList);
    ChipLogFunctError(mError);

exit:
    // on error, mCommandListBuilder would be un-/partial initialized and cannot be used to write anything
    return mCommandListBuilder;
}

CommandList::Builder & InvokeCommand::Builder::GetCommandListBuilder()
{
    return mCommandListBuilder;
}

InvokeCommand::Builder & InvokeCommand::Builder::EndOfInvokeCommand()
{
    EndOfContainer();
    return *this;
}
}; // namespace app
}; // namespace chip

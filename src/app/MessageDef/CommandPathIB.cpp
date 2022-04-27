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

#include "CommandPathIB.h"

#include "MessageDefHelper.h"

#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>

#include <app/AppBuildConfig.h>

using namespace chip;
using namespace chip::TLV;

namespace chip {
namespace app {
#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
CHIP_ERROR CommandPathIB::Parser::CheckSchemaValidity() const
{
    CHIP_ERROR err      = CHIP_NO_ERROR;
    int TagPresenceMask = 0;
    chip::TLV::TLVReader reader;
    PRETTY_PRINT("CommandPathIB =");
    PRETTY_PRINT("{");

    // make a copy of the Path reader
    reader.Init(mReader);

    while (CHIP_NO_ERROR == (err = reader.Next()))
    {
        if (!TLV::IsContextTag(reader.GetTag()))
        {
            continue;
        }
        switch (chip::TLV::TagNumFromTag(reader.GetTag()))
        {
        case kCsTag_EndpointId:
            // check if this tag has appeared before
            VerifyOrExit(!(TagPresenceMask & (1 << kCsTag_EndpointId)), err = CHIP_ERROR_INVALID_TLV_TAG);
            TagPresenceMask |= (1 << kCsTag_EndpointId);
            VerifyOrExit(chip::TLV::kTLVType_UnsignedInteger == reader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);

#if CHIP_DETAIL_LOGGING
            {
                uint16_t endpointId;
                reader.Get(endpointId);
                PRETTY_PRINT("\tEndpointId = 0x%x,", endpointId);
            }
#endif // CHIP_DETAIL_LOGGING
            break;
        case kCsTag_ClusterId:
            // check if this tag has appeared before
            VerifyOrExit(!(TagPresenceMask & (1 << kCsTag_ClusterId)), err = CHIP_ERROR_INVALID_TLV_TAG);
            TagPresenceMask |= (1 << kCsTag_ClusterId);
            VerifyOrExit(chip::TLV::kTLVType_UnsignedInteger == reader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);
#if CHIP_DETAIL_LOGGING
            {
                chip::ClusterId clusterId;
                reader.Get(clusterId);
                PRETTY_PRINT("\tClusterId = 0x%" PRIx32 ",", clusterId);
            }
#endif // CHIP_DETAIL_LOGGING
            break;
        case kCsTag_CommandId:
            // check if this tag has appeared before
            VerifyOrExit(!(TagPresenceMask & (1 << kCsTag_CommandId)), err = CHIP_ERROR_INVALID_TLV_TAG);
            TagPresenceMask |= (1 << kCsTag_CommandId);
            VerifyOrExit(chip::TLV::kTLVType_UnsignedInteger == reader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);
#if CHIP_DETAIL_LOGGING
            {
                chip::CommandId commandId;
                reader.Get(commandId);
                PRETTY_PRINT("\tCommandId = 0x%x,", commandId);
            }
#endif // CHIP_DETAIL_LOGGING
            break;
        default:
            ExitNow(err = CHIP_ERROR_INVALID_TLV_TAG);
        }
    }
    PRETTY_PRINT("},");
    PRETTY_PRINT("");
    // if we have exhausted this container
    if (CHIP_END_OF_TLV == err)
    {
        // check for required fields:
        const uint16_t RequiredFields = (1 << kCsTag_CommandId) | (1 << kCsTag_ClusterId);

        if ((TagPresenceMask & RequiredFields) == RequiredFields)
        {
            err = CHIP_NO_ERROR;
        }
        else
        {
            err = CHIP_ERROR_IM_MALFORMED_COMMAND_PATH;
        }
    }
    SuccessOrExit(err);
    err = reader.ExitContainer(mOuterContainerType);

exit:

    return err;
}
#endif // CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK

CHIP_ERROR CommandPathIB::Parser::GetEndpointId(chip::EndpointId * const apEndpointID) const
{
    return GetUnsignedInteger(kCsTag_EndpointId, apEndpointID);
}

CHIP_ERROR CommandPathIB::Parser::GetClusterId(chip::ClusterId * const apClusterId) const
{
    return GetUnsignedInteger(kCsTag_ClusterId, apClusterId);
}

CHIP_ERROR CommandPathIB::Parser::GetCommandId(chip::CommandId * const apCommandId) const
{
    return GetUnsignedInteger(kCsTag_CommandId, apCommandId);
}

CommandPathIB::Builder & CommandPathIB::Builder::EndpointId(const chip::EndpointId aEndpointId)
{
    // skip if error has already been set
    if (mError == CHIP_NO_ERROR)
    {
        mError = mpWriter->Put(chip::TLV::ContextTag(kCsTag_EndpointId), aEndpointId);
    }
    return *this;
}

CommandPathIB::Builder & CommandPathIB::Builder::ClusterId(const chip::ClusterId aClusterId)
{
    // skip if error has already been set
    if (mError == CHIP_NO_ERROR)
    {
        mError = mpWriter->Put(chip::TLV::ContextTag(kCsTag_ClusterId), aClusterId);
    }
    return *this;
}

CommandPathIB::Builder & CommandPathIB::Builder::CommandId(const chip::CommandId aCommandId)
{
    // skip if error has already been set
    if (mError == CHIP_NO_ERROR)
    {
        mError = mpWriter->Put(chip::TLV::ContextTag(kCsTag_CommandId), aCommandId);
    }
    return *this;
}

CommandPathIB::Builder & CommandPathIB::Builder::EndOfCommandPathIB()
{
    EndOfContainer();
    return *this;
}

CHIP_ERROR CommandPathIB::Builder::Encode(const CommandPathParams & aCommandPathParams)
{
    if (aCommandPathParams.mFlags.Has(CommandPathFlags::kEndpointIdValid))
    {
        EndpointId(aCommandPathParams.mEndpointId);
    }

    ClusterId(aCommandPathParams.mClusterId).CommandId(aCommandPathParams.mCommandId).EndOfCommandPathIB();
    return GetError();
}

CHIP_ERROR CommandPathIB::Builder::Encode(const ConcreteCommandPath & aConcreteCommandPath)
{
    EndpointId(aConcreteCommandPath.mEndpointId)
        .ClusterId(aConcreteCommandPath.mClusterId)
        .CommandId(aConcreteCommandPath.mCommandId)
        .EndOfCommandPathIB();
    return GetError();
}

}; // namespace app
}; // namespace chip

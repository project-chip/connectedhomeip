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

#include <app/AppConfig.h>

using namespace chip;
using namespace chip::TLV;

namespace chip {
namespace app {
#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
CHIP_ERROR CommandPathIB::Parser::CheckSchemaValidity() const
{
    CHIP_ERROR err      = CHIP_NO_ERROR;
    int tagPresenceMask = 0;
    TLV::TLVReader reader;
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
        uint32_t tagNum = TLV::TagNumFromTag(reader.GetTag());
        switch (tagNum)
        {
        case to_underlying(Tag::kEndpointId):
            // check if this tag has appeared before
            VerifyOrReturnError(!(tagPresenceMask & (1 << to_underlying(Tag::kEndpointId))), CHIP_ERROR_INVALID_TLV_TAG);
            tagPresenceMask |= (1 << to_underlying(Tag::kEndpointId));
            VerifyOrReturnError(TLV::kTLVType_UnsignedInteger == reader.GetType(), CHIP_ERROR_WRONG_TLV_TYPE);

#if CHIP_DETAIL_LOGGING
            {
                uint16_t endpointId;
                ReturnErrorOnFailure(reader.Get(endpointId));
                PRETTY_PRINT("\tEndpointId = 0x%x,", endpointId);
            }
#endif // CHIP_DETAIL_LOGGING
            break;
        case to_underlying(Tag::kClusterId):
            // check if this tag has appeared before
            VerifyOrReturnError(!(tagPresenceMask & (1 << to_underlying(Tag::kClusterId))), CHIP_ERROR_INVALID_TLV_TAG);
            tagPresenceMask |= (1 << to_underlying(Tag::kClusterId));
            VerifyOrReturnError(TLV::kTLVType_UnsignedInteger == reader.GetType(), CHIP_ERROR_WRONG_TLV_TYPE);
#if CHIP_DETAIL_LOGGING
            {
                chip::ClusterId clusterId;
                ReturnErrorOnFailure(reader.Get(clusterId));
                PRETTY_PRINT("\tClusterId = 0x%" PRIx32 ",", clusterId);
            }
#endif // CHIP_DETAIL_LOGGING
            break;
        case to_underlying(Tag::kCommandId):
            // check if this tag has appeared before
            VerifyOrReturnError(!(tagPresenceMask & (1 << to_underlying(Tag::kCommandId))), CHIP_ERROR_INVALID_TLV_TAG);
            tagPresenceMask |= (1 << to_underlying(Tag::kCommandId));
            VerifyOrReturnError(chip::TLV::kTLVType_UnsignedInteger == reader.GetType(), CHIP_ERROR_WRONG_TLV_TYPE);
#if CHIP_DETAIL_LOGGING
            {
                chip::CommandId commandId;
                ReturnErrorOnFailure(reader.Get(commandId));
                PRETTY_PRINT("\tCommandId = 0x%x,", commandId);
            }
#endif // CHIP_DETAIL_LOGGING
            break;
        default:
            PRETTY_PRINT("Unknown tag num %" PRIu32, tagNum);
            break;
        }
    }
    PRETTY_PRINT("},");
    PRETTY_PRINT_BLANK_LINE();
    // if we have exhausted this container
    if (CHIP_END_OF_TLV == err)
    {
        // check for required fields:
        const uint16_t requiredFields = (1 << to_underlying(Tag::kCommandId)) | (1 << to_underlying(Tag::kClusterId));

        err = (tagPresenceMask & requiredFields) == requiredFields ? CHIP_NO_ERROR : CHIP_ERROR_IM_MALFORMED_COMMAND_PATH_IB;
    }
    ReturnErrorOnFailure(err);
    return reader.ExitContainer(mOuterContainerType);
}
#endif // CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK

CHIP_ERROR CommandPathIB::Parser::GetEndpointId(chip::EndpointId * const apEndpointID) const
{
    return GetUnsignedInteger(to_underlying(Tag::kEndpointId), apEndpointID);
}

CHIP_ERROR CommandPathIB::Parser::GetClusterId(chip::ClusterId * const apClusterId) const
{
    return GetUnsignedInteger(to_underlying(Tag::kClusterId), apClusterId);
}

CHIP_ERROR CommandPathIB::Parser::GetCommandId(chip::CommandId * const apCommandId) const
{
    return GetUnsignedInteger(to_underlying(Tag::kCommandId), apCommandId);
}

CommandPathIB::Builder & CommandPathIB::Builder::EndpointId(const chip::EndpointId aEndpointId)
{
    // skip if error has already been set
    if (mError == CHIP_NO_ERROR)
    {
        mError = mpWriter->Put(TLV::ContextTag(to_underlying(Tag::kEndpointId)), aEndpointId);
    }
    return *this;
}

CommandPathIB::Builder & CommandPathIB::Builder::ClusterId(const chip::ClusterId aClusterId)
{
    // skip if error has already been set
    if (mError == CHIP_NO_ERROR)
    {
        mError = mpWriter->Put(TLV::ContextTag(to_underlying(Tag::kClusterId)), aClusterId);
    }
    return *this;
}

CommandPathIB::Builder & CommandPathIB::Builder::CommandId(const chip::CommandId aCommandId)
{
    // skip if error has already been set
    if (mError == CHIP_NO_ERROR)
    {
        mError = mpWriter->Put(TLV::ContextTag(to_underlying(Tag::kCommandId)), aCommandId);
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

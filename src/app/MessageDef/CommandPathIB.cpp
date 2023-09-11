/**
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 * SPDX-FileCopyrightText: 2018 Google LLC.
 * SPDX-FileCopyrightText: 2016-2017 Nest Labs, Inc.
 * SPDX-License-Identifier: Apache-2.0
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
#if CHIP_CONFIG_IM_PRETTY_PRINT
CHIP_ERROR CommandPathIB::Parser::PrettyPrint() const
{
    CHIP_ERROR err = CHIP_NO_ERROR;
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
            VerifyOrReturnError(chip::TLV::kTLVType_UnsignedInteger == reader.GetType(), CHIP_ERROR_WRONG_TLV_TYPE);
#if CHIP_DETAIL_LOGGING
            {
                chip::CommandId commandId;
                ReturnErrorOnFailure(reader.Get(commandId));
                PRETTY_PRINT("\tCommandId = 0x%" PRIx32 ",", commandId);
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
        err = CHIP_NO_ERROR;
    }
    ReturnErrorOnFailure(err);
    return reader.ExitContainer(mOuterContainerType);
}
#endif // CHIP_CONFIG_IM_PRETTY_PRINT

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
        mError = mpWriter->Put(TLV::ContextTag(Tag::kEndpointId), aEndpointId);
    }
    return *this;
}

CommandPathIB::Builder & CommandPathIB::Builder::ClusterId(const chip::ClusterId aClusterId)
{
    // skip if error has already been set
    if (mError == CHIP_NO_ERROR)
    {
        mError = mpWriter->Put(TLV::ContextTag(Tag::kClusterId), aClusterId);
    }
    return *this;
}

CommandPathIB::Builder & CommandPathIB::Builder::CommandId(const chip::CommandId aCommandId)
{
    // skip if error has already been set
    if (mError == CHIP_NO_ERROR)
    {
        mError = mpWriter->Put(TLV::ContextTag(Tag::kCommandId), aCommandId);
    }
    return *this;
}

CHIP_ERROR CommandPathIB::Builder::EndOfCommandPathIB()
{
    EndOfContainer();
    return GetError();
}

CHIP_ERROR CommandPathIB::Builder::Encode(const CommandPathParams & aCommandPathParams)
{
    if (aCommandPathParams.mFlags.Has(CommandPathFlags::kEndpointIdValid))
    {
        EndpointId(aCommandPathParams.mEndpointId);
    }

    return ClusterId(aCommandPathParams.mClusterId).CommandId(aCommandPathParams.mCommandId).EndOfCommandPathIB();
}

CHIP_ERROR CommandPathIB::Builder::Encode(const ConcreteCommandPath & aConcreteCommandPath)
{
    return EndpointId(aConcreteCommandPath.mEndpointId)
        .ClusterId(aConcreteCommandPath.mClusterId)
        .CommandId(aConcreteCommandPath.mCommandId)
        .EndOfCommandPathIB();
}

}; // namespace app
}; // namespace chip

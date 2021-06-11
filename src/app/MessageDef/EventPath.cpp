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
 *      This file defines EventPath parser and builder in CHIP interaction model
 *
 */

#include "EventPath.h"

#include "MessageDefHelper.h"

#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>

using namespace chip;
using namespace chip::TLV;

namespace chip {
namespace app {
CHIP_ERROR EventPath::Parser::Init(const chip::TLV::TLVReader & aReader)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    // make a copy of the reader here
    mReader.Init(aReader);

    VerifyOrExit(chip::TLV::kTLVType_List == mReader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);

    err = mReader.EnterContainer(mOuterContainerType);

exit:
    ChipLogFunctError(err);

    return err;
}

#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
CHIP_ERROR EventPath::Parser::CheckSchemaValidity() const
{
    CHIP_ERROR err           = CHIP_NO_ERROR;
    uint16_t TagPresenceMask = 0;
    chip::TLV::TLVReader reader;

    PRETTY_PRINT("EventPath =");
    PRETTY_PRINT("{");

    // make a copy of the Path reader
    reader.Init(mReader);

    while (CHIP_NO_ERROR == (err = reader.Next()))
    {
        VerifyOrExit(chip::TLV::IsContextTag(reader.GetTag()), err = CHIP_ERROR_INVALID_TLV_TAG);
        switch (chip::TLV::TagNumFromTag(reader.GetTag()))
        {
        case kCsTag_NodeId:
            // check if this tag has appeared before
            VerifyOrExit(!(TagPresenceMask & (1 << kCsTag_NodeId)), err = CHIP_ERROR_INVALID_TLV_TAG);
            TagPresenceMask |= (1 << kCsTag_NodeId);
            VerifyOrExit(chip::TLV::kTLVType_UnsignedInteger == reader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);
#if CHIP_DETAIL_LOGGING
            {
                uint64_t nodeId;
                reader.Get(nodeId);
                PRETTY_PRINT("\tNodeId = 0x%" PRIx64 ",", nodeId);
            }
#endif // CHIP_DETAIL_LOGGING
            break;
        case kCsTag_EndpointId:
            // check if this tag has appeared before
            VerifyOrExit(!(TagPresenceMask & (1 << kCsTag_EndpointId)), err = CHIP_ERROR_INVALID_TLV_TAG);
            TagPresenceMask |= (1 << kCsTag_EndpointId);
            VerifyOrExit(chip::TLV::kTLVType_UnsignedInteger == reader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);
#if CHIP_DETAIL_LOGGING
            {
                uint16_t endpointId;
                reader.Get(endpointId);
                PRETTY_PRINT("\tEndpointId = 0x%" PRIx16 ",", endpointId);
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
        case chip::app::EventPath::kCsTag_EventId:
            // check if this tag has appeared before
            VerifyOrExit(!(TagPresenceMask & (1 << chip::app::EventPath::kCsTag_EventId)), err = CHIP_ERROR_INVALID_TLV_TAG);
            TagPresenceMask |= (1 << chip::app::EventPath::kCsTag_EventId);
            VerifyOrExit(chip::TLV::kTLVType_UnsignedInteger == reader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);

#if CHIP_DETAIL_LOGGING
            {
                chip::EventId eventId;
                reader.Get(eventId);
                PRETTY_PRINT("\tEventId = 0x%" PRIx16 ",", eventId);
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
        const uint16_t RequiredFields = (1 << kCsTag_EndpointId) | (1 << kCsTag_ClusterId);

        if ((TagPresenceMask & RequiredFields) == RequiredFields)
        {
            err = CHIP_NO_ERROR;
        }
        else
        {
            err = CHIP_ERROR_IM_MALFORMED_EVENT_PATH;
        }
    }
    SuccessOrExit(err);
    err = reader.ExitContainer(mOuterContainerType);

exit:
    ChipLogFunctError(err);

    return err;
}
#endif // CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK

CHIP_ERROR EventPath::Parser::GetNodeId(chip::NodeId * const apNodeId) const
{
    return GetUnsignedInteger(kCsTag_NodeId, apNodeId);
}

CHIP_ERROR EventPath::Parser::GetEndpointId(chip::EndpointId * const apEndpointID) const
{
    return GetUnsignedInteger(kCsTag_EndpointId, apEndpointID);
}

CHIP_ERROR EventPath::Parser::GetClusterId(chip::ClusterId * const apClusterId) const
{
    return GetUnsignedInteger(kCsTag_ClusterId, apClusterId);
}

CHIP_ERROR EventPath::Parser::GetEventId(chip::EventId * const apEventId) const
{
    return GetUnsignedInteger(kCsTag_EventId, apEventId);
}

CHIP_ERROR EventPath::Builder::_Init(chip::TLV::TLVWriter * const apWriter, const uint64_t aTag)
{
    mpWriter = apWriter;
    mError   = mpWriter->StartContainer(aTag, chip::TLV::kTLVType_List, mOuterContainerType);
    SuccessOrExit(mError);

exit:
    ChipLogFunctError(mError);
    return mError;
}

CHIP_ERROR EventPath::Builder::Init(chip::TLV::TLVWriter * const apWriter)
{
    return _Init(apWriter, chip::TLV::AnonymousTag);
}

CHIP_ERROR EventPath::Builder::Init(chip::TLV::TLVWriter * const apWriter, const uint8_t aContextTagToUse)
{
    return _Init(apWriter, chip::TLV::ContextTag(aContextTagToUse));
}

EventPath::Builder & EventPath::Builder::NodeId(const uint64_t aNodeId)
{
    // skip if error has already been set
    SuccessOrExit(mError);

    mError = mpWriter->Put(chip::TLV::ContextTag(kCsTag_NodeId), aNodeId);
    ChipLogFunctError(mError);

exit:
    return *this;
}

EventPath::Builder & EventPath::Builder::EndpointId(const chip::EndpointId aEndpointId)
{
    // skip if error has already been set
    SuccessOrExit(mError);

    mError = mpWriter->Put(chip::TLV::ContextTag(kCsTag_EndpointId), aEndpointId);
    ChipLogFunctError(mError);

exit:
    return *this;
}

EventPath::Builder & EventPath::Builder::ClusterId(const chip::ClusterId aClusterId)
{
    // skip if error has already been set
    SuccessOrExit(mError);

    mError = mpWriter->Put(chip::TLV::ContextTag(kCsTag_ClusterId), aClusterId);
    ChipLogFunctError(mError);

exit:
    return *this;
}

EventPath::Builder & EventPath::Builder::EventId(const chip::EventId aEventId)
{
    // skip if error has already been set
    SuccessOrExit(mError);

    mError = mpWriter->Put(chip::TLV::ContextTag(kCsTag_EventId), aEventId);
    ChipLogFunctError(mError);

exit:
    return *this;
}

EventPath::Builder & EventPath::Builder::EndOfEventPath()
{
    EndOfContainer();
    return *this;
}

}; // namespace app
}; // namespace chip

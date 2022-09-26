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

#include "EventPathIB.h"

#include "MessageDefHelper.h"

#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>

#include <app/AppConfig.h>

#include <protocols/interaction_model/Constants.h>

namespace chip {
namespace app {
#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
CHIP_ERROR EventPathIB::Parser::CheckSchemaValidity() const
{
    CHIP_ERROR err      = CHIP_NO_ERROR;
    int tagPresenceMask = 0;
    TLV::TLVReader reader;

    PRETTY_PRINT("EventPath =");
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
        case to_underlying(Tag::kNode):
            // check if this tag has appeared before
            VerifyOrReturnError(!(tagPresenceMask & (1 << to_underlying(Tag::kNode))), CHIP_ERROR_INVALID_TLV_TAG);
            tagPresenceMask |= (1 << to_underlying(Tag::kNode));
            VerifyOrReturnError(TLV::kTLVType_UnsignedInteger == reader.GetType(), CHIP_ERROR_WRONG_TLV_TYPE);
#if CHIP_DETAIL_LOGGING
            {
                NodeId node;
                reader.Get(node);
                PRETTY_PRINT("\tNode = 0x%" PRIx64 ",", node);
            }
#endif // CHIP_DETAIL_LOGGING
            break;
        case to_underlying(Tag::kEndpoint):
            // check if this tag has appeared before
            VerifyOrReturnError(!(tagPresenceMask & (1 << to_underlying(Tag::kEndpoint))), CHIP_ERROR_INVALID_TLV_TAG);
            tagPresenceMask |= (1 << to_underlying(Tag::kEndpoint));
            VerifyOrReturnError(TLV::kTLVType_UnsignedInteger == reader.GetType(), CHIP_ERROR_WRONG_TLV_TYPE);
#if CHIP_DETAIL_LOGGING
            {
                EndpointId endpoint;
                reader.Get(endpoint);
                PRETTY_PRINT("\tEndpoint = 0x%x,", endpoint);
            }
#endif // CHIP_DETAIL_LOGGING
            break;
        case to_underlying(Tag::kCluster):
            // check if this tag has appeared before
            VerifyOrReturnError(!(tagPresenceMask & (1 << to_underlying(Tag::kCluster))), CHIP_ERROR_INVALID_TLV_TAG);
            tagPresenceMask |= (1 << to_underlying(Tag::kCluster));
            VerifyOrReturnError(TLV::kTLVType_UnsignedInteger == reader.GetType(), CHIP_ERROR_WRONG_TLV_TYPE);

#if CHIP_DETAIL_LOGGING
            {
                ClusterId cluster;
                reader.Get(cluster);
                PRETTY_PRINT("\tCluster = 0x%" PRIx32 ",", cluster);
            }
#endif // CHIP_DETAIL_LOGGING
            break;
        case to_underlying(Tag::kEvent):
            // check if this tag has appeared before
            VerifyOrReturnError(!(tagPresenceMask & (1 << to_underlying(Tag::kEvent))), CHIP_ERROR_INVALID_TLV_TAG);
            tagPresenceMask |= (1 << to_underlying(Tag::kEvent));
            VerifyOrReturnError(TLV::kTLVType_UnsignedInteger == reader.GetType(), CHIP_ERROR_WRONG_TLV_TYPE);

#if CHIP_DETAIL_LOGGING
            {
                EventId event;
                reader.Get(event);
                PRETTY_PRINT("\tEvent = 0x%x,", event);
            }
#endif // CHIP_DETAIL_LOGGING
            break;
        case to_underlying(Tag::kIsUrgent):
            // check if this tag has appeared before
            VerifyOrReturnError(!(tagPresenceMask & (1 << to_underlying(Tag::kIsUrgent))), CHIP_ERROR_INVALID_TLV_TAG);
            tagPresenceMask |= (1 << to_underlying(Tag::kIsUrgent));
            VerifyOrReturnError(TLV::kTLVType_Boolean == reader.GetType(), CHIP_ERROR_WRONG_TLV_TYPE);

#if CHIP_DETAIL_LOGGING
            {
                bool isUrgent;
                ReturnErrorOnFailure(reader.Get(isUrgent));
                PRETTY_PRINT("\tisUrgent = %s, ", isUrgent ? "true" : "false");
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
#endif // CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK

CHIP_ERROR EventPathIB::Parser::GetNode(NodeId * const apNode) const
{
    return GetUnsignedInteger(to_underlying(Tag::kNode), apNode);
}

CHIP_ERROR EventPathIB::Parser::GetEndpoint(EndpointId * const apEndpoint) const
{
    return GetUnsignedInteger(to_underlying(Tag::kEndpoint), apEndpoint);
}

CHIP_ERROR EventPathIB::Parser::GetCluster(ClusterId * const apCluster) const
{
    return GetUnsignedInteger(to_underlying(Tag::kCluster), apCluster);
}

CHIP_ERROR EventPathIB::Parser::GetEvent(EventId * const apEvent) const
{
    return GetUnsignedInteger(to_underlying(Tag::kEvent), apEvent);
}

CHIP_ERROR EventPathIB::Parser::GetEventPath(ConcreteEventPath * const apPath) const
{
    VerifyOrReturnError(GetEndpoint(&(apPath->mEndpointId)) == CHIP_NO_ERROR, CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB);
    VerifyOrReturnError(GetCluster(&(apPath->mClusterId)) == CHIP_NO_ERROR, CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB);
    VerifyOrReturnError(GetEvent(&(apPath->mEventId)) == CHIP_NO_ERROR, CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB);
    return CHIP_NO_ERROR;
}

CHIP_ERROR EventPathIB::Parser::GetIsUrgent(bool * const apIsUrgent) const
{
    return GetSimpleValue(to_underlying(Tag::kIsUrgent), TLV::kTLVType_Boolean, apIsUrgent);
}

CHIP_ERROR EventPathIB::Parser::ParsePath(EventPathParams & aEvent) const
{
    CHIP_ERROR err = GetEndpoint(&(aEvent.mEndpointId));
    if (err == CHIP_NO_ERROR)
    {
        VerifyOrReturnError(!aEvent.HasWildcardEndpointId(), CHIP_IM_GLOBAL_STATUS(InvalidAction));
    }
    else if (err == CHIP_END_OF_TLV)
    {
        err = CHIP_NO_ERROR;
    }
    VerifyOrReturnError(err == CHIP_NO_ERROR, CHIP_IM_GLOBAL_STATUS(InvalidAction));

    err = GetCluster(&(aEvent.mClusterId));
    if (err == CHIP_NO_ERROR)
    {
        VerifyOrReturnError(!aEvent.HasWildcardClusterId(), CHIP_IM_GLOBAL_STATUS(InvalidAction));
    }
    else if (err == CHIP_END_OF_TLV)
    {
        err = CHIP_NO_ERROR;
    }
    VerifyOrReturnError(err == CHIP_NO_ERROR, CHIP_IM_GLOBAL_STATUS(InvalidAction));

    err = GetEvent(&(aEvent.mEventId));
    if (CHIP_END_OF_TLV == err)
    {
        err = CHIP_NO_ERROR;
    }
    else if (err == CHIP_NO_ERROR)
    {
        VerifyOrReturnError(!aEvent.HasWildcardEventId(), CHIP_IM_GLOBAL_STATUS(InvalidAction));
    }
    VerifyOrReturnError(err == CHIP_NO_ERROR, CHIP_IM_GLOBAL_STATUS(InvalidAction));

    err = GetIsUrgent(&(aEvent.mIsUrgentEvent));
    if (CHIP_END_OF_TLV == err)
    {
        err = CHIP_NO_ERROR;
    }
    VerifyOrReturnError(err == CHIP_NO_ERROR, CHIP_IM_GLOBAL_STATUS(InvalidAction));
    return CHIP_NO_ERROR;
}

EventPathIB::Builder & EventPathIB::Builder::Node(const NodeId aNode)
{
    // skip if error has already been set
    if (mError == CHIP_NO_ERROR)
    {
        mError = mpWriter->Put(TLV::ContextTag(to_underlying(Tag::kNode)), aNode);
    }
    return *this;
}

EventPathIB::Builder & EventPathIB::Builder::Endpoint(const EndpointId aEndpoint)
{
    // skip if error has already been set
    if (mError == CHIP_NO_ERROR)
    {
        mError = mpWriter->Put(TLV::ContextTag(to_underlying(Tag::kEndpoint)), aEndpoint);
    }
    return *this;
}

EventPathIB::Builder & EventPathIB::Builder::Cluster(const ClusterId aCluster)
{
    // skip if error has already been set
    if (mError == CHIP_NO_ERROR)
    {
        mError = mpWriter->Put(TLV::ContextTag(to_underlying(Tag::kCluster)), aCluster);
    }
    return *this;
}

EventPathIB::Builder & EventPathIB::Builder::Event(const EventId aEvent)
{
    // skip if error has already been set
    if (mError == CHIP_NO_ERROR)
    {
        mError = mpWriter->Put(TLV::ContextTag(to_underlying(Tag::kEvent)), aEvent);
    }
    return *this;
}

EventPathIB::Builder & EventPathIB::Builder::IsUrgent(const bool aIsUrgent)
{
    // skip if error has already been set
    if (mError == CHIP_NO_ERROR)
    {
        mError = mpWriter->PutBoolean(TLV::ContextTag(to_underlying(Tag::kIsUrgent)), aIsUrgent);
    }
    return *this;
}

EventPathIB::Builder & EventPathIB::Builder::EndOfEventPathIB()
{
    EndOfContainer();
    return *this;
}

CHIP_ERROR EventPathIB::Builder::Encode(const EventPathParams & aEventPathParams)
{
    if (!(aEventPathParams.HasWildcardEndpointId()))
    {
        Endpoint(aEventPathParams.mEndpointId);
    }

    if (!(aEventPathParams.HasWildcardClusterId()))
    {
        Cluster(aEventPathParams.mClusterId);
    }

    if (!(aEventPathParams.HasWildcardEventId()))
    {
        Event(aEventPathParams.mEventId);
    }

    if (aEventPathParams.mIsUrgentEvent)
    {
        IsUrgent(aEventPathParams.mIsUrgentEvent);
    }
    EndOfEventPathIB();
    return GetError();
}
} // namespace app
} // namespace chip

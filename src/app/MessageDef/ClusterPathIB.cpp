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

#include "ClusterPathIB.h"
#include "MessageDefHelper.h"

#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>

#include <app/AppConfig.h>

namespace chip {
namespace app {
#if CHIP_CONFIG_IM_PRETTY_PRINT
CHIP_ERROR ClusterPathIB::Parser::PrettyPrint() const
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    TLV::TLVReader reader;

    PRETTY_PRINT("ClusterPathIB =");
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
            VerifyOrReturnError(TLV::kTLVType_UnsignedInteger == reader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);

#if CHIP_DETAIL_LOGGING
            {
                NodeId node;
                reader.Get(node);
                PRETTY_PRINT("\tNode = 0x" ChipLogFormatX64 ",", ChipLogValueX64(node));
            }
#endif // CHIP_DETAIL_LOGGING
            break;
        case to_underlying(Tag::kEndpoint):
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
            VerifyOrReturnError(TLV::kTLVType_UnsignedInteger == reader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);

#if CHIP_DETAIL_LOGGING
            {
                ClusterId cluster;
                ReturnErrorOnFailure(reader.Get(cluster));
                PRETTY_PRINT("\tCluster = 0x%" PRIx32 ",", cluster);
            }
#endif // CHIP_DETAIL_LOGGING
            break;
        default:
            PRETTY_PRINT("Unknown tag num %" PRIu32, tagNum);
            break;
        }
    }

    PRETTY_PRINT("}");
    PRETTY_PRINT("\t");
    // if we have exhausted this container
    if (CHIP_END_OF_TLV == err)
    {
        err = CHIP_NO_ERROR;
    }

    ReturnErrorOnFailure(err);
    return reader.ExitContainer(mOuterContainerType);
}
#endif // CHIP_CONFIG_IM_PRETTY_PRINT

CHIP_ERROR ClusterPathIB::Parser::GetNode(NodeId * const apNode) const
{
    return GetUnsignedInteger(to_underlying(Tag::kNode), apNode);
}

CHIP_ERROR ClusterPathIB::Parser::GetEndpoint(EndpointId * const apEndpoint) const
{
    return GetUnsignedInteger(to_underlying(Tag::kEndpoint), apEndpoint);
}

CHIP_ERROR ClusterPathIB::Parser::GetCluster(ClusterId * const apCluster) const
{
    return GetUnsignedInteger(to_underlying(Tag::kCluster), apCluster);
}

ClusterPathIB::Builder & ClusterPathIB::Builder::Node(const NodeId aNode)
{
    // skip if error has already been set
    if (mError == CHIP_NO_ERROR)
    {
        mError = mpWriter->Put(TLV::ContextTag(Tag::kNode), aNode);
    }
    return *this;
}

ClusterPathIB::Builder & ClusterPathIB::Builder::Endpoint(const EndpointId aEndpoint)
{
    // skip if error has already been set
    if (mError == CHIP_NO_ERROR)
    {
        mError = mpWriter->Put(TLV::ContextTag(Tag::kEndpoint), aEndpoint);
    }
    return *this;
}

ClusterPathIB::Builder & ClusterPathIB::Builder::Cluster(const ClusterId aCluster)
{
    // skip if error has already been set
    if (mError == CHIP_NO_ERROR)
    {
        mError = mpWriter->Put(TLV::ContextTag(Tag::kCluster), aCluster);
    }
    return *this;
}

CHIP_ERROR ClusterPathIB::Builder::EndOfClusterPathIB()
{
    EndOfContainer();
    return GetError();
}
} // namespace app
} // namespace chip

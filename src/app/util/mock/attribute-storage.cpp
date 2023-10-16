/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
 *    All rights reserved.
 *
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
 *     This file contains the mock implementation for the generated attribute-storage.cpp
 *     - It contains three endpoints, 0xFFFE, 0xFFFD, 0xFFFC
 *     - It contains four clusters: 0xFFF1'0001 to 0xFFF1'0004
 *     - All cluster has two global attribute (0x0000'FFFC, 0x0000'FFFD)
 *     - Some clusters has some cluster-specific attributes, with 0xFFF1 prefix.
 *
 *    Note: The ember's attribute-storage.cpp will include some app specific generated files. So we cannot use it directly. This
 *    might be fixed with a mock endpoint-config.h
 */

#include <app-common/zap-generated/attribute-type.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app/MessageDef/AttributeDataIB.h>
#include <app/MessageDef/AttributeReportIB.h>
#include <app/MessageDef/AttributeStatusIB.h>
#include <app/att-storage.h>
#include <app/util/af.h>
#include <app/util/mock/Constants.h>

#include <app/AttributeAccessInterface.h>
#include <app/ConcreteAttributePath.h>
#include <app/EventManagement.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/CHIPEncoding.h>
#include <lib/core/TLVDebug.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/DLLUtil.h>
#include <lib/support/UnitTestRegistration.h>
#include <lib/support/logging/CHIPLogging.h>

#include <app/util/af-types.h>
#include <app/util/attribute-metadata.h>

typedef uint8_t EmberAfClusterMask;

using namespace chip;
using namespace chip::Test;
using namespace chip::app;

namespace {
DataVersion dataVersion   = 0;
EndpointId endpoints[]    = { kMockEndpoint1, kMockEndpoint2, kMockEndpoint3 };
uint16_t clusterIndex[]   = { 0, 2, 5 };
uint8_t clusterCount[]    = { 2, 3, 4 };
ClusterId clusters[]      = { MockClusterId(1), MockClusterId(2), MockClusterId(1), MockClusterId(2), MockClusterId(3),
                              MockClusterId(1), MockClusterId(2), MockClusterId(3), MockClusterId(4) };
uint16_t attributeIndex[] = { 0, 2, 5, 7, 11, 16, 19, 25, 27 };
uint16_t attributeCount[] = { 2, 3, 2, 4, 5, 3, 6, 2, 2 };
uint16_t eventIndex[]     = { 0, 2, 2, 2, 2, 2, 2, 2, 2 };
uint16_t eventCount[]     = { 2, 0, 0, 0, 0, 0, 0, 0, 0 };
AttributeId attributes[]  = {
    // clang-format off
    Clusters::Globals::Attributes::ClusterRevision::Id, Clusters::Globals::Attributes::FeatureMap::Id,
    Clusters::Globals::Attributes::ClusterRevision::Id, Clusters::Globals::Attributes::FeatureMap::Id, MockAttributeId(1),
    Clusters::Globals::Attributes::ClusterRevision::Id, Clusters::Globals::Attributes::FeatureMap::Id,
    Clusters::Globals::Attributes::ClusterRevision::Id, Clusters::Globals::Attributes::FeatureMap::Id, MockAttributeId(1), MockAttributeId(2),
    Clusters::Globals::Attributes::ClusterRevision::Id, Clusters::Globals::Attributes::FeatureMap::Id, MockAttributeId(1), MockAttributeId(2), MockAttributeId(3),
    Clusters::Globals::Attributes::ClusterRevision::Id, Clusters::Globals::Attributes::FeatureMap::Id, MockAttributeId(1),
    Clusters::Globals::Attributes::ClusterRevision::Id, Clusters::Globals::Attributes::FeatureMap::Id, MockAttributeId(1), MockAttributeId(2), MockAttributeId(3), MockAttributeId(4),
    Clusters::Globals::Attributes::ClusterRevision::Id, Clusters::Globals::Attributes::FeatureMap::Id,
    Clusters::Globals::Attributes::ClusterRevision::Id, Clusters::Globals::Attributes::FeatureMap::Id
    // clang-format on
};
EventId events[] = {
    MockEventId(1),
    MockEventId(2),
};

uint16_t mockClusterRevision = 1;
uint32_t mockFeatureMap      = 0x1234;
bool mockAttribute1          = true;
int16_t mockAttribute2       = 42;
uint64_t mockAttribute3      = 0xdeadbeef0000cafe;
uint8_t mockAttribute4[256]  = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf,
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf,
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf,
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf,
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf,
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf,
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf,
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf,
};

#define MOCK_CLUSTER_DECL(idx)                                                                                                     \
    {                                                                                                                              \
        .clusterId = clusters[idx], .attributes = nullptr,                                     /* Not used for now. */             \
            .attributeCount = attributeCount[idx], .clusterSize = 0,                           /* Not used for now. */             \
            .mask = CLUSTER_MASK_SERVER, .functions = nullptr, .acceptedCommandList = nullptr, /* Not used for now */              \
            .generatedCommandList = nullptr,                                                   /* Not used for now */              \
            .eventList = &events[eventIndex[idx]], .eventCount = eventCount[idx],                                                  \
    }

EmberAfCluster clusterStructs[] = {
    MOCK_CLUSTER_DECL(0), MOCK_CLUSTER_DECL(1), MOCK_CLUSTER_DECL(2), MOCK_CLUSTER_DECL(3), MOCK_CLUSTER_DECL(4),
    MOCK_CLUSTER_DECL(5), MOCK_CLUSTER_DECL(6), MOCK_CLUSTER_DECL(7), MOCK_CLUSTER_DECL(8),
};

#define MOCK_ENDPOINT_DECL(idx)                                                                                                    \
    {                                                                                                                              \
        .cluster = &clusterStructs[clusterIndex[idx]], .clusterCount = clusterCount[idx],                                          \
        .endpointSize = 0, /* Not used for now */                                                                                  \
    }

EmberAfEndpointType endpointStructs[] = {
    MOCK_ENDPOINT_DECL(0),
    MOCK_ENDPOINT_DECL(1),
    MOCK_ENDPOINT_DECL(2),
};

} // namespace

uint16_t emberAfEndpointCount()
{
    return ArraySize(endpoints);
}

uint16_t emberAfIndexFromEndpoint(chip::EndpointId endpoint)
{
    static_assert(ArraySize(endpoints) < UINT16_MAX, "Need to be able to return endpoint index as a 16-bit value.");

    for (size_t i = 0; i < ArraySize(endpoints); i++)
    {
        if (endpoints[i] == endpoint)
        {
            return static_cast<uint16_t>(i);
        }
    }
    return UINT16_MAX;
}

uint8_t emberAfGetClusterCountForEndpoint(chip::EndpointId endpoint)
{
    for (size_t i = 0; i < ArraySize(endpoints); i++)
    {
        if (endpoints[i] == endpoint)
        {
            return clusterCount[i];
        }
    }
    return 0;
}

uint8_t emberAfClusterCount(chip::EndpointId endpoint, bool server)
{
    return emberAfGetClusterCountForEndpoint(endpoint);
}

uint16_t emberAfGetServerAttributeCount(chip::EndpointId endpoint, chip::ClusterId cluster)
{
    uint16_t endpointIndex         = emberAfIndexFromEndpoint(endpoint);
    uint8_t clusterCountOnEndpoint = emberAfClusterCount(endpoint, true);
    for (uint8_t i = 0; i < clusterCountOnEndpoint; i++)
    {
        if (clusters[i + clusterIndex[endpointIndex]] == cluster)
        {
            return attributeCount[i + clusterIndex[endpointIndex]];
        }
    }
    return 0;
}

uint16_t emberAfGetServerAttributeIndexByAttributeId(chip::EndpointId endpoint, chip::ClusterId cluster,
                                                     chip::AttributeId attributeId)
{
    uint16_t endpointIndex         = emberAfIndexFromEndpoint(endpoint);
    uint8_t clusterCountOnEndpoint = emberAfClusterCount(endpoint, true);
    for (uint8_t i = 0; i < clusterCountOnEndpoint; i++)
    {
        if (clusters[i + clusterIndex[endpointIndex]] == cluster)
        {
            uint16_t clusterAttributeOffset = attributeIndex[i + clusterIndex[endpointIndex]];
            for (uint16_t j = 0; j < emberAfGetServerAttributeCount(endpoint, cluster); j++)
            {
                if (attributes[clusterAttributeOffset + j] == attributeId)
                {
                    return j;
                }
            }
            break;
        }
    }
    return UINT16_MAX;
}

bool emberAfContainsAttribute(chip::EndpointId endpoint, chip::ClusterId clusterId, chip::AttributeId attributeId)
{
    return !(emberAfGetServerAttributeIndexByAttributeId(endpoint, clusterId, attributeId) == UINT16_MAX);
}

chip::EndpointId emberAfEndpointFromIndex(uint16_t index)
{
    VerifyOrDie(index < ArraySize(endpoints));
    return endpoints[index];
}

chip::Optional<chip::ClusterId> emberAfGetNthClusterId(chip::EndpointId endpoint, uint8_t n, bool server)
{
    if (n >= emberAfClusterCount(endpoint, server))
    {
        return chip::Optional<chip::ClusterId>::Missing();
    }
    return chip::Optional<chip::ClusterId>(clusters[clusterIndex[emberAfIndexFromEndpoint(endpoint)] + n]);
}

// Returns number of clusters put into the passed cluster list
// for the given endpoint and client/server polarity
uint8_t emberAfGetClustersFromEndpoint(EndpointId endpoint, ClusterId * clusterList, uint8_t listLen, bool server)
{
    uint8_t cluster_count = emberAfClusterCount(endpoint, server);
    uint8_t i;

    if (cluster_count > listLen)
    {
        cluster_count = listLen;
    }
    for (i = 0; i < cluster_count; i++)
    {
        clusterList[i] = emberAfGetNthClusterId(endpoint, i, server).Value();
    }
    return cluster_count;
}

chip::Optional<chip::AttributeId> emberAfGetServerAttributeIdByIndex(chip::EndpointId endpoint, chip::ClusterId cluster,
                                                                     uint16_t index)
{
    uint16_t endpointIndex         = emberAfIndexFromEndpoint(endpoint);
    uint8_t clusterCountOnEndpoint = emberAfClusterCount(endpoint, true);
    for (uint8_t i = 0; i < clusterCountOnEndpoint; i++)
    {
        if (clusters[i + clusterIndex[endpointIndex]] == cluster)
        {
            uint16_t clusterAttributeOffset = attributeIndex[i + clusterIndex[endpointIndex]];
            if (index < emberAfGetServerAttributeCount(endpoint, cluster))
            {
                return Optional<AttributeId>(attributes[clusterAttributeOffset + index]);
            }
            break;
        }
    }
    return Optional<AttributeId>::Missing();
}

uint8_t emberAfClusterIndex(chip::EndpointId endpoint, chip::ClusterId cluster, EmberAfClusterMask mask)
{
    uint16_t endpointIndex         = emberAfIndexFromEndpoint(endpoint);
    uint8_t clusterCountOnEndpoint = emberAfClusterCount(endpoint, true);
    for (uint8_t i = 0; i < clusterCountOnEndpoint; i++)
    {
        if (clusters[i + clusterIndex[endpointIndex]] == cluster)
        {
            return i;
        }
    }
    return UINT8_MAX;
}

bool emberAfEndpointIndexIsEnabled(uint16_t index)
{
    return index < ArraySize(endpoints);
}

// This duplication of basic utilities is really unfortunate, but we can't link
// to the normal attribute-storage.cpp because we redefine some of its symbols
// above.
bool emberAfIsStringAttributeType(EmberAfAttributeType attributeType)
{
    return (attributeType == ZCL_OCTET_STRING_ATTRIBUTE_TYPE || attributeType == ZCL_CHAR_STRING_ATTRIBUTE_TYPE);
}

bool emberAfIsLongStringAttributeType(EmberAfAttributeType attributeType)
{
    return (attributeType == ZCL_LONG_OCTET_STRING_ATTRIBUTE_TYPE || attributeType == ZCL_LONG_CHAR_STRING_ATTRIBUTE_TYPE);
}

// And we don't have a good way to link to message.cpp either.
uint8_t emberAfStringLength(const uint8_t * buffer)
{
    // The first byte specifies the length of the string.  A length of 0xFF means
    // the string is invalid and there is no character data.
    return (buffer[0] == 0xFF ? 0 : buffer[0]);
}

uint16_t emberAfLongStringLength(const uint8_t * buffer)
{
    // The first two bytes specify the length of the long string.  A length of
    // 0xFFFF means the string is invalid and there is no character data.
    uint16_t length = Encoding::LittleEndian::Get16(buffer);
    return (length == 0xFFFF ? 0 : length);
}

// This will find the first server that has the clusterId given from the index of endpoint.
bool emberAfContainsServerFromIndex(uint16_t index, ClusterId clusterId)
{
    if (index == kEmberInvalidEndpointIndex)
    {
        return false;
    }

    return clusterId; // Mock version return true as long as the endpoint is
                      // valid
}

const EmberAfEndpointType * emberAfFindEndpointType(EndpointId endpointId)
{
    uint16_t ep = emberAfIndexFromEndpoint(endpointId);
    if (ep == UINT16_MAX)
    {
        return nullptr;
    }
    return &endpointStructs[ep];
}

const EmberAfCluster * emberAfFindServerCluster(EndpointId endpoint, ClusterId clusterId)
{
    auto * endpointType = emberAfFindEndpointType(endpoint);
    if (endpointType == nullptr)
    {
        return nullptr;
    }

    for (decltype(endpointType->clusterCount) idx = 0; idx < endpointType->clusterCount; ++idx)
    {
        auto * cluster = &endpointType->cluster[idx];
        if (cluster->clusterId == clusterId && (cluster->mask & CLUSTER_MASK_SERVER))
        {
            return cluster;
        }
    }
    return nullptr;
}

namespace chip {
namespace app {

AttributeAccessInterface * GetAttributeAccessOverride(EndpointId aEndpointId, ClusterId aClusterId)
{
    return nullptr;
}

EnabledEndpointsWithServerCluster::EnabledEndpointsWithServerCluster(ClusterId clusterId) : mClusterId(clusterId)
{
    EnsureMatchingEndpoint();
}
EnabledEndpointsWithServerCluster & EnabledEndpointsWithServerCluster::operator++()
{
    ++mEndpointIndex;
    EnsureMatchingEndpoint();
    return *this;
}

void EnabledEndpointsWithServerCluster::EnsureMatchingEndpoint()
{
    for (; mEndpointIndex < mEndpointCount; ++mEndpointIndex)
    {
        if (!emberAfEndpointIndexIsEnabled(mEndpointIndex))
        {
            continue;
        }

        if (emberAfContainsServerFromIndex(mEndpointIndex, mClusterId))
        {
            break;
        }
    }
}

} // namespace app
namespace Test {

void BumpVersion()
{
    dataVersion++;
}

DataVersion GetVersion()
{
    return dataVersion;
}

CHIP_ERROR ReadSingleMockClusterData(FabricIndex aAccessingFabricIndex, const ConcreteAttributePath & aPath,
                                     AttributeReportIBs::Builder & aAttributeReports,
                                     AttributeValueEncoder::AttributeEncodeState * apEncoderState)
{
    bool dataExists =
        (emberAfGetServerAttributeIndexByAttributeId(aPath.mEndpointId, aPath.mClusterId, aPath.mAttributeId) != UINT16_MAX);

    ChipLogDetail(DataManagement, "Reading Mock Endpoint %x Mock Cluster %" PRIx32 ", Field %" PRIx32 " is dirty",
                  aPath.mEndpointId, aPath.mClusterId, aPath.mAttributeId);

    if (!dataExists)
    {
        AttributeReportIB::Builder & attributeReport = aAttributeReports.CreateAttributeReport();
        ReturnErrorOnFailure(aAttributeReports.GetError());
        AttributeStatusIB::Builder & attributeStatus = attributeReport.CreateAttributeStatus();
        ReturnErrorOnFailure(attributeReport.GetError());
        AttributePathIB::Builder & attributePath = attributeStatus.CreatePath();
        ReturnErrorOnFailure(attributeStatus.GetError());
        attributePath.Endpoint(aPath.mEndpointId).Cluster(aPath.mClusterId).Attribute(aPath.mAttributeId).EndOfAttributePathIB();
        ReturnErrorOnFailure(attributePath.GetError());
        StatusIB::Builder & errorStatus = attributeStatus.CreateErrorStatus();
        ReturnErrorOnFailure(attributeStatus.GetError());
        errorStatus.EncodeStatusIB(StatusIB(Protocols::InteractionModel::Status::UnsupportedAttribute));
        ReturnErrorOnFailure(errorStatus.GetError());
        ReturnErrorOnFailure(attributeStatus.EndOfAttributeStatusIB());
        return attributeReport.EndOfAttributeReportIB();
    }

    // Attribute 4 acts as a large attribute to trigger chunking.
    if (aPath.mAttributeId == MockAttributeId(4))
    {
        AttributeValueEncoder::AttributeEncodeState state =
            (apEncoderState == nullptr ? AttributeValueEncoder::AttributeEncodeState() : *apEncoderState);
        AttributeValueEncoder valueEncoder(aAttributeReports, aAccessingFabricIndex, aPath, dataVersion, false, state);

        CHIP_ERROR err = valueEncoder.EncodeList([](const auto & encoder) -> CHIP_ERROR {
            for (int i = 0; i < 6; i++)
            {
                ReturnErrorOnFailure(encoder.Encode(chip::ByteSpan(mockAttribute4, sizeof(mockAttribute4))));
            }
            return CHIP_NO_ERROR;
        });

        if (apEncoderState != nullptr)
        {
            *apEncoderState = valueEncoder.GetState();
        }
        return err;
    }

    AttributeReportIB::Builder & attributeReport = aAttributeReports.CreateAttributeReport();
    ReturnErrorOnFailure(aAttributeReports.GetError());
    AttributeDataIB::Builder & attributeData = attributeReport.CreateAttributeData();
    ReturnErrorOnFailure(attributeReport.GetError());
    attributeData.DataVersion(dataVersion);
    AttributePathIB::Builder & attributePath = attributeData.CreatePath();
    ReturnErrorOnFailure(attributeData.GetError());
    attributePath.Endpoint(aPath.mEndpointId).Cluster(aPath.mClusterId).Attribute(aPath.mAttributeId).EndOfAttributePathIB();
    ReturnErrorOnFailure(attributePath.GetError());

    TLV::TLVWriter * writer = attributeData.GetWriter();

    switch (aPath.mAttributeId)
    {
    case Clusters::Globals::Attributes::ClusterRevision::Id:
        ReturnErrorOnFailure(writer->Put(TLV::ContextTag(AttributeDataIB::Tag::kData), mockClusterRevision));
        break;
    case Clusters::Globals::Attributes::FeatureMap::Id:
        ReturnErrorOnFailure(writer->Put(TLV::ContextTag(AttributeDataIB::Tag::kData), mockFeatureMap));
        break;
    case MockAttributeId(1):
        ReturnErrorOnFailure(writer->Put(TLV::ContextTag(AttributeDataIB::Tag::kData), mockAttribute1));
        break;
    case MockAttributeId(2):
        ReturnErrorOnFailure(writer->Put(TLV::ContextTag(AttributeDataIB::Tag::kData), mockAttribute2));
        break;
    case MockAttributeId(3):
        ReturnErrorOnFailure(writer->Put(TLV::ContextTag(AttributeDataIB::Tag::kData), mockAttribute3));
        break;
    default:
        // The key should found since we have checked above.
        return CHIP_ERROR_KEY_NOT_FOUND;
    }

    ReturnErrorOnFailure(attributeData.EndOfAttributeDataIB());
    return attributeReport.EndOfAttributeReportIB();
}

} // namespace Test
} // namespace chip

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

#include <app-common/zap-generated/att-storage.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app/MessageDef/AttributeDataIB.h>
#include <app/MessageDef/AttributeReportIB.h>
#include <app/MessageDef/AttributeStatusIB.h>
#include <app/util/mock/Constants.h>

#include <app/AttributeAccessInterface.h>
#include <app/ClusterInfo.h>
#include <app/ConcreteAttributePath.h>
#include <app/EventManagement.h>
#include <app/InteractionModelDelegate.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/CHIPTLVDebug.hpp>
#include <lib/support/CodeUtils.h>
#include <lib/support/DLLUtil.h>
#include <lib/support/UnitTestRegistration.h>
#include <lib/support/logging/CHIPLogging.h>

typedef uint8_t EmberAfClusterMask;

using namespace chip;
using namespace chip::Test;
using namespace chip::app;

namespace {

EndpointId endpoints[]    = { kMockEndpoint1, kMockEndpoint2, kMockEndpoint3 };
uint16_t clusterIndex[]   = { 0, 2, 5 };
uint8_t clusterCount[]    = { 2, 3, 4 };
ClusterId clusters[]      = { MockClusterId(1), MockClusterId(2), MockClusterId(1), MockClusterId(2), MockClusterId(3),
                         MockClusterId(1), MockClusterId(2), MockClusterId(3), MockClusterId(4) };
uint16_t attributeIndex[] = { 0, 2, 5, 7, 11, 16, 19, 25, 27 };
uint16_t attributeCount[] = { 2, 3, 2, 4, 5, 3, 6, 2, 2 };
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

} // namespace

uint16_t emberAfEndpointCount(void)
{
    return ArraySize(endpoints);
}

uint16_t emberAfIndexFromEndpoint(chip::EndpointId endpoint)
{
    for (uint16_t i = 0; i < ArraySize(endpoints); i++)
    {
        if (endpoints[i] == endpoint)
        {
            return i;
        }
    }
    return UINT16_MAX;
}

uint8_t emberAfClusterCount(chip::EndpointId endpoint, bool server)
{
    for (uint16_t i = 0; i < ArraySize(endpoints); i++)
    {
        if (endpoints[i] == endpoint)
        {
            return clusterCount[i];
        }
    }
    return 0;
}

uint16_t emberAfGetServerAttributeCount(chip::EndpointId endpoint, chip::ClusterId cluster)
{
    uint16_t endpointIndex          = emberAfIndexFromEndpoint(endpoint);
    uint16_t clusterCountOnEndpoint = emberAfClusterCount(endpoint, true);
    for (uint16_t i = 0; i < clusterCountOnEndpoint; i++)
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
    uint16_t endpointIndex          = emberAfIndexFromEndpoint(endpoint);
    uint16_t clusterCountOnEndpoint = emberAfClusterCount(endpoint, true);
    for (uint16_t i = 0; i < clusterCountOnEndpoint; i++)
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

chip::Optional<chip::AttributeId> emberAfGetServerAttributeIdByIndex(chip::EndpointId endpoint, chip::ClusterId cluster,
                                                                     uint16_t index)
{
    uint16_t endpointIndex          = emberAfIndexFromEndpoint(endpoint);
    uint16_t clusterCountOnEndpoint = emberAfClusterCount(endpoint, true);
    for (uint16_t i = 0; i < clusterCountOnEndpoint; i++)
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
    uint16_t endpointIndex          = emberAfIndexFromEndpoint(endpoint);
    uint16_t clusterCountOnEndpoint = emberAfClusterCount(endpoint, true);
    for (uint8_t i = 0; i < clusterCountOnEndpoint; i++)
    {
        if (clusters[i + clusterIndex[endpointIndex]] == cluster)
        {
            return i;
        }
    }
    return UINT8_MAX;
}

namespace chip {
namespace Test {

CHIP_ERROR ReadSingleMockClusterData(FabricIndex aAccessingFabricIndex, const ConcreteAttributePath & aPath,
                                     AttributeReportIB::Builder & aAttributeReport)
{
    bool dataExists =
        (emberAfGetServerAttributeIndexByAttributeId(aPath.mEndpointId, aPath.mClusterId, aPath.mAttributeId) != UINT16_MAX);

    ChipLogDetail(DataManagement, "Reading Mock Cluster %" PRIx32 ", Field %" PRIx32 " is dirty", aPath.mClusterId,
                  aPath.mAttributeId);

    AttributeDataIB::Builder attributeData;
    AttributePathIB::Builder attributePath;

    if (!dataExists)
    {
        AttributeStatusIB::Builder attributeStatus;
        attributeStatus = aAttributeReport.CreateAttributeStatus();
        attributePath   = attributeStatus.CreatePath();
        attributePath.Endpoint(aPath.mEndpointId).Cluster(aPath.mClusterId).Attribute(aPath.mAttributeId).EndOfAttributePathIB();
        ReturnErrorOnFailure(attributePath.GetError());
        StatusIB::Builder errorStatus = attributeStatus.CreateErrorStatus();
        errorStatus.EncodeStatusIB(StatusIB(Protocols::InteractionModel::Status::UnsupportedAttribute));
        attributeStatus.EndOfAttributeStatusIB();
        ReturnErrorOnFailure(attributeStatus.GetError());
        return CHIP_NO_ERROR;
    }

    attributeData = aAttributeReport.CreateAttributeData();
    attributePath = attributeData.CreatePath();
    attributePath.Endpoint(aPath.mEndpointId).Cluster(aPath.mClusterId).Attribute(aPath.mAttributeId).EndOfAttributePathIB();
    ReturnErrorOnFailure(attributePath.GetError());

    TLV::TLVWriter * writer = attributeData.GetWriter();

    switch (aPath.mAttributeId)
    {
    case Clusters::Globals::Attributes::ClusterRevision::Id:
        ReturnErrorOnFailure(writer->Put(TLV::ContextTag(to_underlying(AttributeDataIB::Tag::kData)), mockClusterRevision));
        break;
    case Clusters::Globals::Attributes::FeatureMap::Id:
        ReturnErrorOnFailure(writer->Put(TLV::ContextTag(to_underlying(AttributeDataIB::Tag::kData)), mockFeatureMap));
        break;
    case MockAttributeId(1):
        ReturnErrorOnFailure(writer->Put(TLV::ContextTag(to_underlying(AttributeDataIB::Tag::kData)), mockAttribute1));
        break;
    case MockAttributeId(2):
        ReturnErrorOnFailure(writer->Put(TLV::ContextTag(to_underlying(AttributeDataIB::Tag::kData)), mockAttribute2));
        break;
    case MockAttributeId(3):
        ReturnErrorOnFailure(writer->Put(TLV::ContextTag(to_underlying(AttributeDataIB::Tag::kData)), mockAttribute3));
        break;
    case MockAttributeId(4):
        ReturnErrorOnFailure(writer->Put(TLV::ContextTag(to_underlying(AttributeDataIB::Tag::kData)),
                                         chip::ByteSpan(mockAttribute4, sizeof(mockAttribute4))));
        break;
    default:
        // The key should found since we have checked above.
        return CHIP_ERROR_KEY_NOT_FOUND;
    }

    attributeData.DataVersion(0).EndOfAttributeDataIB();
    return attributeData.GetError();
}

} // namespace Test
} // namespace chip

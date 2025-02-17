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
#include <app/util/attribute-storage.h>
#include <app/util/endpoint-config-api.h>
#include <app/util/mock/Constants.h>
#include <app/util/mock/MockNodeConfig.h>

#include <app/AttributeValueEncoder.h>
#include <app/ConcreteAttributePath.h>
#include <app/EventManagement.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/CHIPEncoding.h>
#include <lib/core/TLVDebug.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/DLLUtil.h>
#include <lib/support/logging/CHIPLogging.h>

#include <app/util/af-types.h>
#include <app/util/attribute-metadata.h>

typedef uint8_t EmberAfClusterMask;

using namespace chip;
using namespace chip::Test;
using namespace chip::app;
using namespace Clusters::Globals::Attributes;

namespace {

unsigned metadataStructureGeneration = 0;
DataVersion dataVersion              = 0;
const MockNodeConfig * mockConfig    = nullptr;

const MockNodeConfig & DefaultMockNodeConfig()
{
    // clang-format off
    static const MockNodeConfig config({
        MockEndpointConfig(kMockEndpoint1, {
            MockClusterConfig(MockClusterId(1), {
                ClusterRevision::Id, FeatureMap::Id,
            }, {
                MockEventId(1), MockEventId(2),
            }),
            MockClusterConfig(MockClusterId(2), {
                ClusterRevision::Id, FeatureMap::Id, MockAttributeId(1),
            }),
        }),
        MockEndpointConfig(kMockEndpoint2, {
            MockClusterConfig(MockClusterId(1), {
                ClusterRevision::Id, FeatureMap::Id,
            }),
            MockClusterConfig(MockClusterId(2), {
                ClusterRevision::Id, FeatureMap::Id, MockAttributeId(1), MockAttributeId(2),
            }),
            MockClusterConfig(MockClusterId(3), {
                ClusterRevision::Id, FeatureMap::Id, MockAttributeId(1), MockAttributeId(2), MockAttributeId(3),
            }),
        }),
        MockEndpointConfig(kMockEndpoint3, {
            MockClusterConfig(MockClusterId(1), {
                ClusterRevision::Id, FeatureMap::Id, MockAttributeId(1),
            }),
            MockClusterConfig(MockClusterId(2), {
                ClusterRevision::Id, FeatureMap::Id, MockAttributeId(1), MockAttributeId(2), MockAttributeId(3), MockAttributeId(4),
            }),
            MockClusterConfig(MockClusterId(3), {
                ClusterRevision::Id, FeatureMap::Id,
            }),
            MockClusterConfig(MockClusterId(4), {
                ClusterRevision::Id, FeatureMap::Id,
            }),
        }),
    });
    // clang-format on
    return config;
}

const MockNodeConfig & GetMockNodeConfig()
{
    return (mockConfig != nullptr) ? *mockConfig : DefaultMockNodeConfig();
}

} // namespace

namespace chip {
namespace Test {

const uint16_t mockClusterRevision = 1;
const uint32_t mockFeatureMap      = 0x1234;
const bool mockAttribute1          = true;
const int16_t mockAttribute2       = 42;
const uint64_t mockAttribute3      = 0xdeadbeef0000cafe;
const uint8_t mockAttribute4[256]  = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf,
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf,
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf,
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf,
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf,
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf,
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf,
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf,
};

} // namespace Test
} // namespace chip

uint16_t emberAfEndpointCount()
{
    return static_cast<uint16_t>(GetMockNodeConfig().endpoints.size());
}

uint16_t emberAfIndexFromEndpoint(EndpointId endpointId)
{
    ptrdiff_t index;
    auto endpoint = GetMockNodeConfig().endpointById(endpointId, &index);
    VerifyOrReturnValue(endpoint != nullptr, kEmberInvalidEndpointIndex);
    return static_cast<uint16_t>(index);
}

uint8_t emberAfGetClusterCountForEndpoint(EndpointId endpointId)
{
    auto endpoint = GetMockNodeConfig().endpointById(endpointId);
    VerifyOrReturnValue(endpoint != nullptr, 0);
    return static_cast<uint8_t>(endpoint->clusters.size());
}

const EmberAfAttributeMetadata * emberAfLocateAttributeMetadata(EndpointId endpointId, ClusterId clusterId, AttributeId attributeId)
{
    auto ep = GetMockNodeConfig().endpointById(endpointId);
    VerifyOrReturnValue(ep != nullptr, nullptr);

    auto cluster = ep->clusterById(clusterId);
    VerifyOrReturnValue(cluster != nullptr, nullptr);

    auto attr = cluster->attributeById(attributeId);
    VerifyOrReturnValue(attr != nullptr, nullptr);

    return &attr->attributeMetaData;
}

const EmberAfCluster * emberAfFindClusterInType(const EmberAfEndpointType * endpointType, ClusterId clusterId,
                                                EmberAfClusterMask mask, uint8_t * index)
{
    // This is a copy & paste implementation from ember attribute storage
    // TODO: this hard-codes ember logic and is duplicated code.
    uint8_t scopedIndex = 0;

    for (uint8_t i = 0; i < endpointType->clusterCount; i++)
    {
        const EmberAfCluster * cluster = &(endpointType->cluster[i]);

        if (mask == 0 || ((cluster->mask & mask) != 0))
        {
            if (cluster->clusterId == clusterId)
            {
                if (index)
                {
                    *index = scopedIndex;
                }

                return cluster;
            }

            scopedIndex++;
        }
    }

    return nullptr;
}

uint8_t emberAfClusterCount(chip::EndpointId endpoint, bool server)
{
    return (server) ? emberAfGetClusterCountForEndpoint(endpoint) : 0;
}

uint8_t emberAfClusterCountForEndpointType(const EmberAfEndpointType * type, bool server)
{
    const EmberAfClusterMask cluster_mask = server ? MATTER_CLUSTER_FLAG_SERVER : MATTER_CLUSTER_FLAG_CLIENT;

    return static_cast<uint8_t>(std::count_if(type->cluster, type->cluster + type->clusterCount,
                                              [=](const EmberAfCluster & cluster) { return (cluster.mask & cluster_mask) != 0; }));
}

uint16_t emberAfGetServerAttributeCount(chip::EndpointId endpointId, chip::ClusterId clusterId)
{
    auto cluster = GetMockNodeConfig().clusterByIds(endpointId, clusterId);
    VerifyOrReturnValue(cluster != nullptr, 0);
    return static_cast<uint16_t>(cluster->attributes.size());
}

uint16_t emberAfGetServerAttributeIndexByAttributeId(chip::EndpointId endpointId, chip::ClusterId clusterId,
                                                     chip::AttributeId attributeId)
{
    auto cluster = GetMockNodeConfig().clusterByIds(endpointId, clusterId);
    VerifyOrReturnValue(cluster != nullptr, kEmberInvalidEndpointIndex);
    ptrdiff_t index;
    auto attribute = cluster->attributeById(attributeId, &index);
    VerifyOrReturnValue(attribute != nullptr, kEmberInvalidEndpointIndex);
    return static_cast<uint16_t>(index);
}

bool emberAfContainsAttribute(chip::EndpointId endpoint, chip::ClusterId clusterId, chip::AttributeId attributeId)
{
    return emberAfGetServerAttributeIndexByAttributeId(endpoint, clusterId, attributeId) != kEmberInvalidEndpointIndex;
}

chip::EndpointId emberAfEndpointFromIndex(uint16_t index)
{
    auto & config = GetMockNodeConfig();
    VerifyOrDie(index < config.endpoints.size());
    return config.endpoints[index].id;
}

namespace chip {
namespace app {

EndpointComposition GetCompositionForEndpointIndex(uint16_t endpointIndex)
{
    return GetMockNodeConfig().endpoints[endpointIndex].composition;
}

} // namespace app
} // namespace chip

EndpointId emberAfParentEndpointFromIndex(uint16_t index)
{
    return kInvalidEndpointId;
}

CHIP_ERROR GetSemanticTagForEndpointAtIndex(EndpointId endpoint, size_t index,
                                            Clusters::Descriptor::Structs::SemanticTagStruct::Type & tag)
{
    auto ep = GetMockNodeConfig().endpointById(endpoint);

    if (ep)
    {
        auto semanticTags = ep->semanticTags();
        if (index < semanticTags.size())
        {
            tag = semanticTags[index];
            return CHIP_NO_ERROR;
        }
    }
    return CHIP_ERROR_NOT_FOUND;
}

chip::Optional<chip::ClusterId> emberAfGetNthClusterId(chip::EndpointId endpointId, uint8_t n, bool server)
{
    VerifyOrReturnValue(server, NullOptional); // only server clusters supported
    auto endpoint = GetMockNodeConfig().endpointById(endpointId);
    VerifyOrReturnValue(endpoint != nullptr && n < endpoint->clusters.size(), NullOptional);
    return MakeOptional(endpoint->clusters[n].id);
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

chip::Optional<chip::AttributeId> emberAfGetServerAttributeIdByIndex(chip::EndpointId endpointId, chip::ClusterId clusterId,
                                                                     uint16_t index)
{
    auto cluster = GetMockNodeConfig().clusterByIds(endpointId, clusterId);
    VerifyOrReturnValue(cluster != nullptr && index < cluster->attributes.size(), NullOptional);
    return MakeOptional(cluster->attributes[index].id);
}

uint8_t emberAfClusterIndex(chip::EndpointId endpointId, chip::ClusterId clusterId, EmberAfClusterMask mask)
{
    VerifyOrReturnValue(mask == 0 || (mask & MATTER_CLUSTER_FLAG_SERVER) != 0, UINT8_MAX); // only server clusters supported
    ptrdiff_t index;
    auto cluster = GetMockNodeConfig().clusterByIds(endpointId, clusterId, &index);
    VerifyOrReturnValue(cluster != nullptr, UINT8_MAX);
    return static_cast<uint8_t>(index);
}

bool emberAfEndpointIndexIsEnabled(uint16_t index)
{
    return index < GetMockNodeConfig().endpoints.size();
}

// This will find the first server that has the clusterId given from the index of endpoint.
bool emberAfContainsServerFromIndex(uint16_t index, ClusterId clusterId)
{
    auto config = GetMockNodeConfig();
    VerifyOrReturnValue(index < config.endpoints.size(), false);
    return true; // TODO: TestSceneTable relies on returning true here: https://github.com/project-chip/connectedhomeip/issues/30696
    // return config.endpoints[index].clusterById(clusterId) != nullptr;
}

const EmberAfEndpointType * emberAfFindEndpointType(EndpointId endpointId)
{
    auto endpoint = GetMockNodeConfig().endpointById(endpointId);
    VerifyOrReturnValue(endpoint != nullptr, nullptr);
    return endpoint->emberEndpoint();
}

const EmberAfCluster * emberAfFindServerCluster(EndpointId endpointId, ClusterId clusterId)
{
    auto cluster = GetMockNodeConfig().clusterByIds(endpointId, clusterId);
    VerifyOrReturnValue(cluster != nullptr, nullptr);
    return cluster->emberCluster();
}

DataVersion * emberAfDataVersionStorage(const chip::app::ConcreteClusterPath & aConcreteClusterPath)
{
    // shared data version storage
    return &dataVersion;
}

chip::Span<const EmberAfDeviceType> emberAfDeviceTypeListFromEndpoint(chip::EndpointId endpointId, CHIP_ERROR & err)
{
    auto endpoint = GetMockNodeConfig().endpointById(endpointId);

    if (endpoint == nullptr)
    {
        return chip::Span<const EmberAfDeviceType>();
    }

    return endpoint->deviceTypes();
}

chip::Span<const EmberAfDeviceType> emberAfDeviceTypeListFromEndpointIndex(unsigned index, CHIP_ERROR & err)
{
    if (index >= GetMockNodeConfig().endpoints.size())
    {
        return chip::Span<const EmberAfDeviceType>();
    }

    return GetMockNodeConfig().endpoints[index].deviceTypes();
}

void emberAfAttributeChanged(EndpointId endpoint, ClusterId clusterId, AttributeId attributeId,
                             AttributesChangedListener * listener)
{
    dataVersion++;
    listener->MarkDirty(AttributePathParams(endpoint, clusterId, attributeId));
}

void emberAfEndpointChanged(EndpointId endpoint, AttributesChangedListener * listener)
{
    listener->MarkDirty(AttributePathParams(endpoint));
}

unsigned emberAfMetadataStructureGeneration()
{
    return metadataStructureGeneration;
}

namespace chip {
namespace app {

EndpointId EnabledEndpointsWithServerCluster::operator*() const
{
    return emberAfEndpointFromIndex(mEndpointIndex);
}

EnabledEndpointsWithServerCluster::EnabledEndpointsWithServerCluster(ClusterId clusterId) :
    mEndpointCount(emberAfEndpointCount()), mClusterId(clusterId)
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

void ResetVersion()
{
    dataVersion = 0;
}

void BumpVersion()
{
    dataVersion++;
}

DataVersion GetVersion()
{
    return dataVersion;
}

void SetVersionTo(DataVersion version)
{
    dataVersion = version;
}

CHIP_ERROR ReadSingleMockClusterData(FabricIndex aAccessingFabricIndex, const ConcreteAttributePath & aPath,
                                     AttributeReportIBs::Builder & aAttributeReports, AttributeEncodeState * apEncoderState)
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
        AttributeEncodeState state(apEncoderState);
        Access::SubjectDescriptor subject;
        subject.fabricIndex = aAccessingFabricIndex;

        AttributeValueEncoder valueEncoder(aAttributeReports, subject, aPath, dataVersion, /* aIsFabricFiltered = */ false, state);

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

void SetMockNodeConfig(const MockNodeConfig & config)
{
    metadataStructureGeneration++;
    mockConfig = &config;
}

/// Resets the mock attribute storage to the default configuration.
void ResetMockNodeConfig()
{
    metadataStructureGeneration++;
    mockConfig = nullptr;
}

} // namespace Test
} // namespace chip

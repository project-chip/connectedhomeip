/*
 *    Copyright (c) 2026 Project CHIP Authors
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
#pragma once

#include <app-common/zap-generated/cluster-objects.h>
#include "AmbientSensingUnionDelegate.h"
#include <app/server-cluster/DefaultServerCluster.h>
#include <app/server-cluster/ServerClusterContext.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/CHIPMemString.h>

#include <algorithm>
#include <cstdint>
#include <cstring>

namespace chip {
namespace app {
namespace Clusters {

/**
 * @brief Server-side implementation of the Ambient Sensing Union cluster.
 *
 * This cluster provides data modeling for multi-node ambient sensing systems,
 * allowing a union of sensors to be managed as a cohesive group.
 *
 */
class AmbientSensingUnionCluster : public DefaultServerCluster
{
public:
    static constexpr size_t kMaxUnionNameLength       = 128;
    static constexpr size_t kMaxContributorNameLength = 128;
    static constexpr size_t kMaxContributors          = 128;
    static constexpr size_t kMinContributors          = 1;

    /**
     * @brief Unified contributor entry supporting both Matter and non-Matter contributors.
     */
    struct ContributorEntry
    {
        NodeId nodeId         = kUndefinedNodeId;
        EndpointId endpointId = kInvalidEndpointId;
        AmbientSensingUnion::UnionContributorStatusEnum status =
            AmbientSensingUnion::UnionContributorStatusEnum::kUnionContributorOffline;
        bool active = false;

        char name[kMaxContributorNameLength + 1] = { 0 };
        size_t nameLength                        = 0;

        bool IsMatter() const { return nodeId != kUndefinedNodeId; }

        void SetName(const CharSpan & contributorName)
        {
            chip::Platform::CopyString(name, contributorName);
            nameLength = strlen(name);
        }

        CharSpan GetName() const { return CharSpan(name, nameLength); }

        void Clear()
        {
            nodeId     = kUndefinedNodeId;
            endpointId = kInvalidEndpointId;
            status     = AmbientSensingUnion::UnionContributorStatusEnum::kUnionContributorOffline;
            active     = false;
            nameLength = 0;
            name[0]    = '\0';
        }

        void CopyTo(AmbientSensingUnion::Structs::UnionContributorStruct::Type & dest) const
        {
            if (IsMatter())
            {
                dest.contributorNodeID.SetNonNull(nodeId);
                dest.contributorEndpointID.SetNonNull(endpointId);
            }
            else
            {
                dest.contributorNodeID.SetNull();
                dest.contributorEndpointID.SetNull();
                dest.contributorName.SetValue(GetName());
            }
            dest.contributorHealth = status;
        }
    };

    /**
     * @brief Configuration structure for the Ambient Sensing Union cluster.
     *
     */
    struct Config
    {
        Config(EndpointId endpointId) : mEndpointId(endpointId) {}

        Config & WithUnionName(const CharSpan & unionName)
        {
            mUnionName = unionName;
            return *this;
        }

        Config & WithDelegate(AmbientSensingUnionDelegate * delegate)
        {
            mDelegate = delegate;
            return *this;
        }

        EndpointId mEndpointId;
        CharSpan mUnionName                     = CharSpan();
        AmbientSensingUnionDelegate * mDelegate = nullptr;
    };

    explicit AmbientSensingUnionCluster(const Config & config);
    ~AmbientSensingUnionCluster() override = default;

    AmbientSensingUnionCluster(const AmbientSensingUnionCluster &)             = delete;
    AmbientSensingUnionCluster & operator=(const AmbientSensingUnionCluster &) = delete;
    AmbientSensingUnionCluster(AmbientSensingUnionCluster &&)                  = delete;
    AmbientSensingUnionCluster & operator=(AmbientSensingUnionCluster &&)      = delete;

    // DefaultServerCluster overrides
    CHIP_ERROR Startup(ServerClusterContext & context) override;
    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override;
    DataModel::ActionReturnStatus WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                 AttributeValueDecoder & decoder) override;
    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) override;

    void SetDelegate(AmbientSensingUnionDelegate * delegate) { mDelegate = delegate; }

    CHIP_ERROR SetUnionName(const CharSpan & unionName);

    CharSpan GetUnionName() const;
    AmbientSensingUnion::UnionHealthEnum GetUnionHealth() const { return mUnionHealth; }
    size_t GetContributorCount() const { return mContributorCount; }

    const ContributorEntry * GetContributors() const { return mContributors; }

    CHIP_ERROR AddMatterContributor(NodeId nodeId, EndpointId endpointId,
                                    AmbientSensingUnion::UnionContributorStatusEnum status =
                                        AmbientSensingUnion::UnionContributorStatusEnum::kUnionContributorOnline);
    CHIP_ERROR RemoveMatterContributor(NodeId nodeId, EndpointId endpointId);
    CHIP_ERROR UpdateMatterContributorStatus(NodeId nodeId, EndpointId endpointId,
                                             AmbientSensingUnion::UnionContributorStatusEnum status);

    CHIP_ERROR AddNonMatterContributor(const CharSpan & name,
                                       AmbientSensingUnion::UnionContributorStatusEnum status =
                                           AmbientSensingUnion::UnionContributorStatusEnum::kUnionContributorOnline);
    CHIP_ERROR RemoveNonMatterContributor(const CharSpan & name);
    CHIP_ERROR UpdateNonMatterContributorStatus(const CharSpan & name, AmbientSensingUnion::UnionContributorStatusEnum status);

private:
    ContributorEntry * FindMatterContributor(NodeId nodeId, EndpointId endpointId);
    ContributorEntry * FindNonMatterContributor(const CharSpan & name);
    ContributorEntry * FindFreeSlot();

    void EmitContributorAddedEvent(const ContributorEntry & entry);
    void EmitContributorRemovedEvent(const ContributorEntry & entry);
    void EmitContributorStatusChangedEvent(const ContributorEntry & entry);

    CHIP_ERROR EncodeContributorList(AttributeValueEncoder & encoder);

    void RecalculateUnionHealth();

    void PersistUnionName();

    AmbientSensingUnionDelegate * mDelegate;

    char mUnionNameBuffer[kMaxUnionNameLength + 1];
    size_t mUnionNameLength;
    AmbientSensingUnion::UnionHealthEnum mUnionHealth;

    ContributorEntry mContributors[kMaxContributors];
    size_t mContributorCount;
};

} // namespace Clusters
} // namespace app
} // namespace chip

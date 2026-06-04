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
#include <app/server-cluster/DefaultServerCluster.h>
#include <app/server-cluster/ServerClusterContext.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <lib/core/Optional.h>

#include <algorithm>
#include <cstdint>
#include <cstring>

namespace chip {
namespace app {
namespace Clusters {

/**
 * @brief Delegate interface for the Ambient Sensing Union cluster.
 *
 * Applications implement this to receive notifications about union state changes.
 */
class AmbientSensingUnionDelegate
{
public:
    virtual ~AmbientSensingUnionDelegate() = default;

    /**
     * @brief Called when the union name changes.
     * @param unionName The new union name.
     */
    virtual void OnUnionNameChanged(const CharSpan & unionName) = 0;

    /**
     * @brief Called when the union health changes.
     * @param unionHealth The new union health state.
     */
    virtual void OnUnionHealthChanged(AmbientSensingUnion::UnionHealthEnum unionHealth) = 0;
};

/**
 * @brief Persistence delegate for non-volatile attributes.
 *
 * Per spec, UnionName has Quality "N" (non-volatile) and must survive reboots.
 */
class AmbientSensingUnionPersistenceDelegate
{
public:
    virtual ~AmbientSensingUnionPersistenceDelegate() = default;

    /**
     * @brief Load the persisted union name.
     * @param buffer Buffer to store the name.
     * @param bufferSize Size of the buffer.
     * @param outLength Output: actual length of the name.
     * @return CHIP_NO_ERROR on success, CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND if not found.
     */
    virtual CHIP_ERROR LoadUnionName(char * buffer, size_t bufferSize, size_t & outLength) = 0;

    /**
     * @brief Persist the union name.
     * @param unionName The name to persist.
     * @return CHIP_NO_ERROR on success.
     */
    virtual CHIP_ERROR SaveUnionName(const CharSpan & unionName) = 0;
};

/**
 * @brief Server-side implementation of the Ambient Sensing Union cluster.
 *
 * This cluster provides data modeling for multi-node ambient sensing systems,
 * allowing a union of sensors to be managed as a cohesive group.
 *
 * Per spec:
 * - UnionName: max 128 characters, writable, non-volatile
 * - UnionHealth: read-only, derived from contributor health states
 * - UnionContributorList: max 128 entries, read-only
 * - ContributorName: max 128 characters (for non-Matter contributors)
 */
class AmbientSensingUnionCluster : public DefaultServerCluster
{
public:

    static constexpr size_t kMaxUnionNameLength       = 128;  // UnionName max length
    static constexpr size_t kMaxContributorNameLength = 128;  // ContributorName max length
    static constexpr size_t kMaxContributors          = 128;  // UnionContributorList max entries
    static constexpr size_t kMinContributors          = 1;    // UnionContributorList min entries

    /**
     * @brief Unified contributor entry supporting both Matter and non-Matter contributors.
     *
     */
    struct ContributorEntry
    {
        NodeId nodeId       = kUndefinedNodeId;
        EndpointId endpointId = kInvalidEndpointId;
        AmbientSensingUnion::UnionContributorStatusEnum status =
            AmbientSensingUnion::UnionContributorStatusEnum::kUnionContributorOffline;
        bool active = false;

        char name[kMaxContributorNameLength + 1] = {0};
        size_t nameLength = 0;

        bool IsMatter() const { return nodeId != kUndefinedNodeId; }

        void SetName(const CharSpan & contributorName)
        {
            nameLength = std::min(contributorName.size(), kMaxContributorNameLength);
            if (nameLength > 0 && contributorName.data() != nullptr)
            {
                memcpy(name, contributorName.data(), nameLength);
            }
            name[nameLength] = '\0';
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
                // ContributorName is optional for Matter contributors
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

        Config & WithPersistence(AmbientSensingUnionPersistenceDelegate * persistence)
        {
            mPersistence = persistence;
            return *this;
        }

        EndpointId mEndpointId;
        CharSpan mUnionName                                   = CharSpan();
        AmbientSensingUnionDelegate * mDelegate               = nullptr;
        AmbientSensingUnionPersistenceDelegate * mPersistence = nullptr;
    };


    /**
     * @brief Constructs an AmbientSensingUnionCluster with the given configuration.
     * @param config The configuration for the cluster.
     */
    explicit AmbientSensingUnionCluster(const Config & config);
    ~AmbientSensingUnionCluster() override = default;

    // Disallow copy and move
    AmbientSensingUnionCluster(const AmbientSensingUnionCluster &)             = delete;
    AmbientSensingUnionCluster & operator=(const AmbientSensingUnionCluster &) = delete;
    AmbientSensingUnionCluster(AmbientSensingUnionCluster &&)                  = delete;
    AmbientSensingUnionCluster & operator=(AmbientSensingUnionCluster &&)      = delete;

    // DefaultServerCluster overrides
    CHIP_ERROR Startup(ServerClusterContext & context) override;
    void Shutdown(ClusterShutdownType shutdownType) override;
    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override;
    DataModel::ActionReturnStatus WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                 AttributeValueDecoder & decoder) override;
    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) override;

    // Delegate management
    void SetDelegate(AmbientSensingUnionDelegate * delegate) { mDelegate = delegate; }

    // Attribute setters
    CHIP_ERROR SetUnionName(const CharSpan & unionName);

    // Attribute getters
    CharSpan GetUnionName() const;
    AmbientSensingUnion::UnionHealthEnum GetUnionHealth() const { return mUnionHealth; }
    size_t GetContributorCount() const { return mContributorCount; }

    // Direct access to contributor array (for iteration)
    const ContributorEntry * GetContributors() const { return mContributors; }

    // Matter contributor management (NodeID is not null)
    CHIP_ERROR AddMatterContributor(NodeId nodeId, EndpointId endpointId,
                                    AmbientSensingUnion::UnionContributorStatusEnum status =
                                        AmbientSensingUnion::UnionContributorStatusEnum::kUnionContributorOnline);
    CHIP_ERROR RemoveMatterContributor(NodeId nodeId, EndpointId endpointId);
    CHIP_ERROR UpdateMatterContributorStatus(NodeId nodeId, EndpointId endpointId,
                                             AmbientSensingUnion::UnionContributorStatusEnum status);

    // Non-Matter contributor management (NodeID is null, name is mandatory)
    CHIP_ERROR AddNonMatterContributor(const CharSpan & name,
                                       AmbientSensingUnion::UnionContributorStatusEnum status =
                                           AmbientSensingUnion::UnionContributorStatusEnum::kUnionContributorOnline);
    CHIP_ERROR RemoveNonMatterContributor(const CharSpan & name);
    CHIP_ERROR UpdateNonMatterContributorStatus(const CharSpan & name,
                                                AmbientSensingUnion::UnionContributorStatusEnum status);

private:
    // Find contributor by criteria
    ContributorEntry * FindMatterContributor(NodeId nodeId, EndpointId endpointId);
    ContributorEntry * FindNonMatterContributor(const CharSpan & name);
    ContributorEntry * FindFreeSlot();

    // Event emission helpers
    void EmitContributorAddedEvent(const ContributorEntry & entry);
    void EmitContributorRemovedEvent(const ContributorEntry & entry);
    void EmitContributorStatusChangedEvent(const ContributorEntry & entry);
    // Bulk event emission helper (used by ClearAllContributors)
    void EmitContributorsBulkRemovedEvent(
        const DataModel::List<const AmbientSensingUnion::Structs::UnionContributorStruct::Type> & contributors);

    // Attribute encoding
    CHIP_ERROR EncodeContributorList(AttributeValueEncoder & encoder);

    // Health recalculation
    void RecalculateUnionHealth();

    // Persistence
    CHIP_ERROR LoadPersistedAttributes();
    CHIP_ERROR PersistUnionName();

    // Delegates
    AmbientSensingUnionDelegate * mDelegate;
    AmbientSensingUnionPersistenceDelegate * mPersistence;

    // Attribute storage
    char mUnionNameBuffer[kMaxUnionNameLength + 1];
    size_t mUnionNameLength;
    AmbientSensingUnion::UnionHealthEnum mUnionHealth;

    // Contributor storage (inline, fixed capacity per spec)
    ContributorEntry mContributors[kMaxContributors];
    size_t mContributorCount;
};

} // namespace Clusters
} // namespace app
} // namespace chip


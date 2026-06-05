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

#include <app/clusters/ambient-sensing-union-server/AmbientSensingUnionCluster.h>

#include <algorithm>
#include <app/server-cluster/AttributeListBuilder.h>
#include <clusters/AmbientSensingUnion/Attributes.h>
#include <clusters/AmbientSensingUnion/Events.h>
#include <clusters/AmbientSensingUnion/Metadata.h>
#include <lib/support/CodeUtils.h>

namespace chip {
namespace app {
namespace Clusters {

using namespace AmbientSensingUnion;
using namespace AmbientSensingUnion::Attributes;

AmbientSensingUnionCluster::AmbientSensingUnionCluster(const Config & config) :
    DefaultServerCluster({ config.mEndpointId, AmbientSensingUnion::Id }), mDelegate(config.mDelegate),
    mPersistence(config.mPersistence), mUnionNameLength(0), mUnionHealth(UnionHealthEnum::kNonFunctional), mContributorCount(0)
{
    mUnionNameBuffer[0] = '\0';

    for (size_t i = 0; i < kMaxContributors; i++)
    {
        mContributors[i].Clear();
    }

    if (config.mUnionName.data() != nullptr && !config.mUnionName.empty())
    {
        size_t len = std::min(config.mUnionName.size(), kMaxUnionNameLength);
        memcpy(mUnionNameBuffer, config.mUnionName.data(), len);
        mUnionNameBuffer[len] = '\0';
        mUnionNameLength      = len;
    }
}

CHIP_ERROR AmbientSensingUnionCluster::Startup(ServerClusterContext & context)
{
    ReturnErrorOnFailure(DefaultServerCluster::Startup(context));

    // Load persisted attributes if persistence delegate is provided
    if (mPersistence != nullptr)
    {
        CHIP_ERROR err = LoadPersistedAttributes();
        if (err != CHIP_NO_ERROR && err != CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
        {
            ChipLogError(Zcl, "AmbientSensingUnion: Failed to load persisted attributes: %" CHIP_ERROR_FORMAT, err.Format());
        }
    }

    // Recalculate union health based on current contributor state
    RecalculateUnionHealth();

    return CHIP_NO_ERROR;
}

void AmbientSensingUnionCluster::Shutdown(ClusterShutdownType shutdownType)
{
    DefaultServerCluster::Shutdown(shutdownType);
}

DataModel::ActionReturnStatus AmbientSensingUnionCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                                        AttributeValueEncoder & encoder)
{
    switch (request.path.mAttributeId)
    {
    case Attributes::FeatureMap::Id:
        return encoder.Encode(static_cast<uint32_t>(0));

    case Attributes::ClusterRevision::Id:
        return encoder.Encode(AmbientSensingUnion::kRevision);

    case Attributes::UnionName::Id:
        return encoder.Encode(GetUnionName());

    case Attributes::UnionHealth::Id:
        return encoder.Encode(mUnionHealth);

    case Attributes::UnionContributorList::Id:
        return EncodeContributorList(encoder);

    default:
        return Protocols::InteractionModel::Status::UnsupportedAttribute;
    }
}

DataModel::ActionReturnStatus AmbientSensingUnionCluster::WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                                         AttributeValueDecoder & decoder)
{
    switch (request.path.mAttributeId)
    {
    case UnionName::Id: {
        CharSpan newName;
        ReturnErrorOnFailure(decoder.Decode(newName));

        if (newName.size() > kMaxUnionNameLength)
        {
            return Protocols::InteractionModel::Status::ConstraintError;
        }

        CHIP_ERROR err = SetUnionName(newName);
        if (err != CHIP_NO_ERROR)
        {
            return Protocols::InteractionModel::Status::Failure;
        }
        return Protocols::InteractionModel::Status::Success;
    }

    default:
        return Protocols::InteractionModel::Status::UnsupportedWrite;
    }
}

CHIP_ERROR AmbientSensingUnionCluster::Attributes(const ConcreteClusterPath & path,
                                                  ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    AttributeListBuilder listBuilder(builder);
    return listBuilder.Append(Span(AmbientSensingUnion::Attributes::kMandatoryMetadata),
                              Span<const AttributeListBuilder::OptionalAttributeEntry>{});
}

CHIP_ERROR AmbientSensingUnionCluster::SetUnionName(const CharSpan & unionName)
{
    VerifyOrReturnError(unionName.size() <= kMaxUnionNameLength, CHIP_ERROR_INVALID_ARGUMENT);

    CharSpan currentName = GetUnionName();
    if (currentName.data_equal(unionName))
    {
        return CHIP_NO_ERROR;
    }

    if (unionName.data() != nullptr && unionName.size() > 0)
    {
        mUnionNameLength = unionName.size();
        memcpy(mUnionNameBuffer, unionName.data(), mUnionNameLength);
    }
    else
    {
        mUnionNameLength = 0;
    }

    mUnionNameBuffer[mUnionNameLength] = '\0';

    NotifyAttributeChanged(UnionName::Id);

    if (mPersistence != nullptr)
    {
        CHIP_ERROR err = PersistUnionName();
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(Zcl, "AmbientSensingUnion: Failed to persist UnionName: %" CHIP_ERROR_FORMAT, err.Format());
        }
    }

    if (mDelegate != nullptr)
    {
        mDelegate->OnUnionNameChanged(GetUnionName());
    }

    return CHIP_NO_ERROR;
}

CharSpan AmbientSensingUnionCluster::GetUnionName() const
{
    return CharSpan(mUnionNameBuffer, mUnionNameLength);
}

// =============================================================================
// Contributor Lookup Methods
// =============================================================================

AmbientSensingUnionCluster::ContributorEntry * AmbientSensingUnionCluster::FindMatterContributor(NodeId nodeId,
                                                                                                 EndpointId endpointId)
{
    for (size_t i = 0; i < kMaxContributors; i++)
    {
        if (mContributors[i].active && mContributors[i].IsMatter() && mContributors[i].nodeId == nodeId &&
            mContributors[i].endpointId == endpointId)
        {
            return &mContributors[i];
        }
    }
    return nullptr;
}

AmbientSensingUnionCluster::ContributorEntry * AmbientSensingUnionCluster::FindNonMatterContributor(const CharSpan & name)
{
    for (size_t i = 0; i < kMaxContributors; i++)
    {
        if (mContributors[i].active && !mContributors[i].IsMatter() && mContributors[i].GetName().data_equal(name))
        {
            return &mContributors[i];
        }
    }
    return nullptr;
}

AmbientSensingUnionCluster::ContributorEntry * AmbientSensingUnionCluster::FindFreeSlot()
{
    for (size_t i = 0; i < kMaxContributors; i++)
    {
        if (!mContributors[i].active)
        {
            return &mContributors[i];
        }
    }
    return nullptr;
}

// =============================================================================
// Matter Contributor Management
// =============================================================================

static bool IsValidContributorStatus(UnionContributorStatusEnum status)
{
    return status == UnionContributorStatusEnum::kUnionContributorOnline ||
        status == UnionContributorStatusEnum::kUnionContributorOffline || status == UnionContributorStatusEnum::kUnknownEnumValue;
}

CHIP_ERROR AmbientSensingUnionCluster::AddMatterContributor(NodeId nodeId, EndpointId endpointId,
                                                            AmbientSensingUnion::UnionContributorStatusEnum status)
{
    VerifyOrReturnError(IsValidContributorStatus(status), CHIP_ERROR_INVALID_ARGUMENT);

    // Check for duplicate
    if (FindMatterContributor(nodeId, endpointId) != nullptr)
    {
        return CHIP_ERROR_DUPLICATE_KEY_ID;
    }

    // Find free slot
    ContributorEntry * entry = FindFreeSlot();
    if (entry == nullptr)
    {
        return CHIP_ERROR_NO_MEMORY;
    }

    // Initialize entry
    entry->Clear();
    entry->nodeId     = nodeId;
    entry->endpointId = endpointId;
    entry->status     = status;
    entry->active     = true;
    mContributorCount++;

    NotifyAttributeChanged(UnionContributorList::Id);
    EmitContributorAddedEvent(*entry);
    RecalculateUnionHealth();

    return CHIP_NO_ERROR;
}

CHIP_ERROR AmbientSensingUnionCluster::RemoveMatterContributor(NodeId nodeId, EndpointId endpointId)
{
    ContributorEntry * entry = FindMatterContributor(nodeId, endpointId);
    VerifyOrReturnError(entry != nullptr, CHIP_ERROR_NOT_FOUND);

    EmitContributorRemovedEvent(*entry);

    entry->Clear();
    mContributorCount--;

    NotifyAttributeChanged(UnionContributorList::Id);
    RecalculateUnionHealth();

    return CHIP_NO_ERROR;
}

CHIP_ERROR AmbientSensingUnionCluster::UpdateMatterContributorStatus(NodeId nodeId, EndpointId endpointId,
                                                                     AmbientSensingUnion::UnionContributorStatusEnum status)
{
    ContributorEntry * entry = FindMatterContributor(nodeId, endpointId);
    VerifyOrReturnError(entry != nullptr, CHIP_ERROR_NOT_FOUND);

    if (entry->status == status)
    {
        return CHIP_NO_ERROR;
    }

    entry->status = status;

    NotifyAttributeChanged(UnionContributorList::Id);
    EmitContributorStatusChangedEvent(*entry);
    RecalculateUnionHealth();

    return CHIP_NO_ERROR;
}

// =============================================================================
// Non-Matter Contributor Management
// =============================================================================

CHIP_ERROR AmbientSensingUnionCluster::AddNonMatterContributor(const CharSpan & name,
                                                               AmbientSensingUnion::UnionContributorStatusEnum status)
{
    // ContributorName is mandatory when NodeID is NULL
    VerifyOrReturnError(!name.empty(), CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(name.size() <= kMaxContributorNameLength, CHIP_ERROR_INVALID_ARGUMENT);

    // Check for duplicate
    if (FindNonMatterContributor(name) != nullptr)
    {
        return CHIP_ERROR_DUPLICATE_KEY_ID;
    }

    // Find free slot
    ContributorEntry * entry = FindFreeSlot();
    if (entry == nullptr)
    {
        return CHIP_ERROR_NO_MEMORY;
    }

    // Initialize entry
    entry->Clear();
    entry->SetName(name);
    entry->status = status;
    entry->active = true;
    mContributorCount++;

    NotifyAttributeChanged(UnionContributorList::Id);
    EmitContributorAddedEvent(*entry);
    RecalculateUnionHealth();

    return CHIP_NO_ERROR;
}

CHIP_ERROR AmbientSensingUnionCluster::RemoveNonMatterContributor(const CharSpan & name)
{
    ContributorEntry * entry = FindNonMatterContributor(name);
    VerifyOrReturnError(entry != nullptr, CHIP_ERROR_NOT_FOUND);

    EmitContributorRemovedEvent(*entry);

    entry->Clear();
    mContributorCount--;

    NotifyAttributeChanged(UnionContributorList::Id);
    RecalculateUnionHealth();

    return CHIP_NO_ERROR;
}

CHIP_ERROR AmbientSensingUnionCluster::UpdateNonMatterContributorStatus(const CharSpan & name,
                                                                        AmbientSensingUnion::UnionContributorStatusEnum status)
{
    ContributorEntry * entry = FindNonMatterContributor(name);
    VerifyOrReturnError(entry != nullptr, CHIP_ERROR_NOT_FOUND);

    if (entry->status == status)
    {
        return CHIP_NO_ERROR;
    }

    entry->status = status;

    NotifyAttributeChanged(UnionContributorList::Id);
    EmitContributorStatusChangedEvent(*entry);
    RecalculateUnionHealth();

    return CHIP_NO_ERROR;
}

// =============================================================================
// Event Emission
// =============================================================================

void AmbientSensingUnionCluster::EmitContributorAddedEvent(const ContributorEntry & entry)
{
    if (mContext == nullptr)
    {
        return;
    }

    AmbientSensingUnion::Structs::UnionContributorStruct::Type contributor;
    entry.CopyTo(contributor);

    Events::UnionContributorAdded::Type event;
    event.addedContributor = DataModel::List<const AmbientSensingUnion::Structs::UnionContributorStruct::Type>(&contributor, 1);

    mContext->interactionContext.eventsGenerator.GenerateEvent(event, mPath.mEndpointId);
}

void AmbientSensingUnionCluster::EmitContributorRemovedEvent(const ContributorEntry & entry)
{
    if (mContext == nullptr)
    {
        return;
    }

    AmbientSensingUnion::Structs::UnionContributorStruct::Type contributor;
    entry.CopyTo(contributor);

    Events::UnionContributorRemoved::Type event;
    event.removedContributor = DataModel::List<const AmbientSensingUnion::Structs::UnionContributorStruct::Type>(&contributor, 1);

    mContext->interactionContext.eventsGenerator.GenerateEvent(event, mPath.mEndpointId);
}

void AmbientSensingUnionCluster::EmitContributorStatusChangedEvent(const ContributorEntry & entry)
{
    if (mContext == nullptr)
    {
        return;
    }

    AmbientSensingUnion::Structs::UnionContributorStruct::Type contributor;
    entry.CopyTo(contributor);

    Events::UnionContributorStatusChanged::Type event;
    event.statusChangedContributor =
        DataModel::List<const AmbientSensingUnion::Structs::UnionContributorStruct::Type>(&contributor, 1);

    mContext->interactionContext.eventsGenerator.GenerateEvent(event, mPath.mEndpointId);
}

// =============================================================================
// Attribute Encoding
// =============================================================================

CHIP_ERROR AmbientSensingUnionCluster::EncodeContributorList(AttributeValueEncoder & encoder)
{
    return encoder.EncodeList([this](const auto & listEncoder) {
        for (size_t i = 0; i < kMaxContributors; i++)
        {
            if (mContributors[i].active)
            {
                AmbientSensingUnion::Structs::UnionContributorStruct::Type contributor;
                mContributors[i].CopyTo(contributor);

                CHIP_ERROR err = listEncoder.Encode(contributor);
                ReturnErrorOnFailure(err);
            }
        }
        return CHIP_NO_ERROR;
    });
}

// =============================================================================
// Health Recalculation
// =============================================================================

void AmbientSensingUnionCluster::RecalculateUnionHealth()
{
    size_t onlineCount = 0;

    for (size_t i = 0; i < kMaxContributors; i++)
    {
        if (mContributors[i].active && mContributors[i].status == UnionContributorStatusEnum::kUnionContributorOnline)
        {
            onlineCount++;
        }
    }

    UnionHealthEnum newHealth;
    if (mContributorCount == 0)
    {
        newHealth = UnionHealthEnum::kNonFunctional;
    }
    else if (onlineCount == mContributorCount)
    {
        newHealth = UnionHealthEnum::kFullyFunctional;
    }
    else if (onlineCount == 0)
    {
        newHealth = UnionHealthEnum::kNonFunctional;
    }
    else
    {
        newHealth = UnionHealthEnum::kLimitedDegraded;
    }

    if (mUnionHealth != newHealth)
    {
        mUnionHealth = newHealth;
        NotifyAttributeChanged(UnionHealth::Id);

        if (mDelegate != nullptr)
        {
            mDelegate->OnUnionHealthChanged(mUnionHealth);
        }
    }
}

// =============================================================================
// Persistence
// =============================================================================

CHIP_ERROR AmbientSensingUnionCluster::LoadPersistedAttributes()
{
    if (mPersistence == nullptr)
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }

    size_t loadedLength = 0;
    CHIP_ERROR err      = mPersistence->LoadUnionName(mUnionNameBuffer, kMaxUnionNameLength, loadedLength);

    if (err == CHIP_NO_ERROR)
    {
        mUnionNameLength                   = std::min(loadedLength, kMaxUnionNameLength);
        mUnionNameBuffer[mUnionNameLength] = '\0';
        ChipLogProgress(Zcl, "AmbientSensingUnion: Loaded persisted UnionName: %.*s", static_cast<int>(mUnionNameLength),
                        mUnionNameBuffer);
    }
    else if (err == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
    {
        ChipLogProgress(Zcl, "AmbientSensingUnion: No persisted UnionName found");
    }

    return err;
}

CHIP_ERROR AmbientSensingUnionCluster::PersistUnionName()
{
    if (mPersistence == nullptr)
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }

    return mPersistence->SaveUnionName(GetUnionName());
}

} // namespace Clusters
} // namespace app
} // namespace chip

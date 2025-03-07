/*
 *    Copyright (c) 2025 Project CHIP Authors
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
#include <app/server-cluster/DefaultServerCluster.h>

#include <access/Privilege.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app/ConcreteClusterPath.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <crypto/RandUtils.h>
#include <lib/support/BitFlags.h>
#include <optional>
#include <protocols/interaction_model/StatusCode.h>

namespace chip {
namespace app {
namespace {

using namespace chip::app::Clusters;
using namespace chip::app::DataModel;

constexpr std::array<AttributeEntry, 5> kGlobalAttributeEntries{ {
    {
        Globals::Attributes::ClusterRevision::Id,
        BitFlags<AttributeQualityFlags>(),
        Access::Privilege::kView,
        std::nullopt,
    },
    {
        Globals::Attributes::FeatureMap::Id,
        BitFlags<AttributeQualityFlags>(),
        Access::Privilege::kView,
        std::nullopt,
    },
    {
        Globals::Attributes::AttributeList::Id,
        BitFlags<AttributeQualityFlags>(AttributeQualityFlags::kListAttribute),
        Access::Privilege::kView,
        std::nullopt,
    },
    {
        Globals::Attributes::AcceptedCommandList::Id,
        BitFlags<AttributeQualityFlags>(AttributeQualityFlags::kListAttribute),
        Access::Privilege::kView,
        std::nullopt,
    },
    {
        Globals::Attributes::GeneratedCommandList::Id,
        BitFlags<AttributeQualityFlags>(AttributeQualityFlags::kListAttribute),
        Access::Privilege::kView,
        std::nullopt,
    },
} };

} // namespace

Span<const DataModel::AttributeEntry> DefaultServerCluster::GlobalAttributes()
{
    return { kGlobalAttributeEntries.data(), kGlobalAttributeEntries.size() };
}

DefaultServerCluster::DefaultServerCluster()
{
    // SPEC - 7.10.3. Cluster Data Version
    //   A cluster data version SHALL be initialized randomly when it is first published.
    mDataVersion = Crypto::GetRandU32();
}

CHIP_ERROR DefaultServerCluster::Attributes(const ConcreteClusterPath & path, DataModel::ListBuilder<AttributeEntry> & builder)
{

    return builder.ReferenceExisting(GlobalAttributes());
}

CHIP_ERROR DefaultServerCluster::Startup(ServerClusterContext * context)
{
    VerifyOrReturnError(mContext == nullptr, CHIP_ERROR_ALREADY_INITIALIZED);
    mContext = context;
    return CHIP_NO_ERROR;
}

void DefaultServerCluster::Shutdown()
{
    mContext = nullptr;
}

void DefaultServerCluster::NotifyAttributeChanged(AttributeId attributeId)
{
    IncreaseDataVersion();

    VerifyOrReturn(mContext != nullptr);
    const ConcreteClusterPath path = GetPath();
    mContext->interactionContext->dataModelChangeListener->MarkDirty({ path.mEndpointId, path.mClusterId, attributeId });
}

BitFlags<ClusterQualityFlags> DefaultServerCluster::GetClusterFlags() const
{
    return {};
}

ActionReturnStatus DefaultServerCluster::WriteAttribute(const WriteAttributeRequest & request, AttributeValueDecoder & decoder)
{
    return Protocols::InteractionModel::Status::UnsupportedWrite;
}

std::optional<ActionReturnStatus>
DefaultServerCluster::InvokeCommand(const InvokeRequest & request, chip::TLV::TLVReader & input_arguments, CommandHandler * handler)
{
    return Protocols::InteractionModel::Status::UnsupportedCommand;
}

CHIP_ERROR DefaultServerCluster::AcceptedCommands(const ConcreteClusterPath & path,
                                                  DataModel::ListBuilder<AcceptedCommandEntry> & builder)
{
    return CHIP_NO_ERROR;
}

CHIP_ERROR DefaultServerCluster::GeneratedCommands(const ConcreteClusterPath & path, DataModel::ListBuilder<CommandId> & builder)
{
    return CHIP_NO_ERROR;
}

} // namespace app
} // namespace chip

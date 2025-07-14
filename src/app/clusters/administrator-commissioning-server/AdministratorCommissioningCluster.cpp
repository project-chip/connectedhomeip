/*
 *    Copyright (c) 2025 Project CHIP Authors
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
#include "AdministratorCommissioningCluster.h"

#include <app/data-model-provider/MetadataTypes.h>
#include <clusters/AdministratorCommissioning/Commands.h>
#include <lib/support/CodeUtils.h>

#include <clusters/AdministratorCommissioning/AttributeIds.h>
#include <clusters/AdministratorCommissioning/CommandIds.h>
#include <clusters/AdministratorCommissioning/Metadata.h>

namespace chip {
namespace app {
namespace Clusters {

namespace {

constexpr DataModel::AcceptedCommandEntry kAcceptedCommands[] = {
    AdministratorCommissioning::Commands::OpenCommissioningWindow::kMetadataEntry,
    AdministratorCommissioning::Commands::RevokeCommissioning::kMetadataEntry,
};

constexpr DataModel::AcceptedCommandEntry kAcceptedCommandsWithBasicCommissioning[] = {
    AdministratorCommissioning::Commands::OpenCommissioningWindow::kMetadataEntry,
    AdministratorCommissioning::Commands::RevokeCommissioning::kMetadataEntry,
    AdministratorCommissioning::Commands::OpenBasicCommissioningWindow::kMetadataEntry,
};

constexpr DataModel::AttributeEntry kAttributes[] = {
    AdministratorCommissioning::Attributes::WindowStatus::kMetadataEntry,
    AdministratorCommissioning::Attributes::AdminFabricIndex::kMetadataEntry,
    AdministratorCommissioning::Attributes::AdminVendorId::kMetadataEntry,
};

}; // namespace

DataModel::ActionReturnStatus AdministratorCommissioningCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                                               AttributeValueEncoder & encoder)
{
    using namespace AdministratorCommissioning::Attributes;

    switch (request.path.mAttributeId)
    {
    case FeatureMap::Id:
        return encoder.Encode(BitFlags<AdministratorCommissioning::Feature>{ 0 });
    case ClusterRevision::Id:
        return encoder.Encode(AdministratorCommissioning::kRevision);
    case WindowStatus::Id:
        return encoder.Encode(mLogic.GetWindowStatus());
    case AdminFabricIndex::Id:
        return encoder.Encode(mLogic.GetOpenerFabricIndex());
    case AdminVendorId::Id:
        return encoder.Encode(mLogic.GetAdminVendorId());
    default:
        return Protocols::InteractionModel::Status::UnsupportedAttribute;
    }
}

std::optional<DataModel::ActionReturnStatus>
AdministratorCommissioningCluster::InvokeCommand(const DataModel::InvokeRequest & request, TLV::TLVReader & input_arguments,
                                                 CommandHandler * handler)
{
    using namespace AdministratorCommissioning::Commands;

    switch (request.path.mCommandId)
    {
    case OpenCommissioningWindow::Id: {
        OpenCommissioningWindow::DecodableType request_data;
        ReturnErrorOnFailure(request_data.Decode(input_arguments));
        return mLogic.OpenCommissioningWindow(handler->GetAccessingFabricIndex(), request_data);
    }
    case RevokeCommissioning::Id: {
        RevokeCommissioning::DecodableType request_data;
        ReturnErrorOnFailure(request_data.Decode(input_arguments));
        return mLogic.RevokeCommissioning(request_data);
    }
    default:
        return Protocols::InteractionModel::Status::UnsupportedCommand;
    }
}

CHIP_ERROR AdministratorCommissioningCluster::AcceptedCommands(const ConcreteClusterPath & path,
                                                               ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder)
{
    return builder.ReferenceExisting(kAcceptedCommands);
}

CHIP_ERROR AdministratorCommissioningCluster::Attributes(const ConcreteClusterPath & path,
                                                         ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    ReturnErrorOnFailure(builder.ReferenceExisting(kAttributes));
    return builder.AppendElements(DefaultServerCluster::GlobalAttributes());
}

std::optional<DataModel::ActionReturnStatus> AdministratorCommissioningWithBasicCommissioningWindowCluster::InvokeCommand(
    const DataModel::InvokeRequest & request, TLV::TLVReader & input_arguments, CommandHandler * handler)
{
    using namespace AdministratorCommissioning::Commands;

    if (mFeatures.Has(AdministratorCommissioning::Feature::kBasic))
    {

        if (request.path.mCommandId == OpenBasicCommissioningWindow::Id)
        {
            OpenBasicCommissioningWindow::DecodableType request_data;
            ReturnErrorOnFailure(request_data.Decode(input_arguments));
            return mLogic.OpenBasicCommissioningWindow(handler->GetAccessingFabricIndex(), request_data);
        }
    }

    return AdministratorCommissioningCluster::InvokeCommand(request, input_arguments, handler);
}

CHIP_ERROR AdministratorCommissioningWithBasicCommissioningWindowCluster::AcceptedCommands(
    const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder)
{
    if (mFeatures.Has(AdministratorCommissioning::Feature::kBasic))
    {
        return builder.ReferenceExisting(kAcceptedCommandsWithBasicCommissioning);
    }

    return builder.ReferenceExisting(kAcceptedCommands);
}

DataModel::ActionReturnStatus
AdministratorCommissioningWithBasicCommissioningWindowCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                                             AttributeValueEncoder & encoder)
{
    using namespace AdministratorCommissioning::Attributes;

    if (request.path.mAttributeId == FeatureMap::Id)
    {
        return encoder.Encode(mFeatures);
    }

    return AdministratorCommissioningCluster::ReadAttribute(request, encoder);
}

} // namespace Clusters
} // namespace app
} // namespace chip

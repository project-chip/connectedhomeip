/**
 *
 *    Copyright (c) 2021-25 Project CHIP Authors
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

#include <app/clusters/general-commissioning-server/general-commissioning-cluster.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <clusters/GeneralCommissioning/Commands.h>
#include <lib/support/CodeUtils.h>

#include <clusters/GeneralCommissioning/Ids.h>
#include <clusters/GeneralCommissioning/Metadata.h>

using namespace chip::app::Clusters::GeneralCommissioning;
using namespace chip::app::Clusters::GeneralCommissioning::Attributes;


namespace chip {
namespace app {
namespace Clusters {

namespace {

constexpr DataModel::AcceptedCommandEntry kAcceptedCommands[] = {
    GeneralCommissioning::Commands::ArmFailSafe::kMetadataEntry,
    GeneralCommissioning::Commands::SetRegulatoryConfig::kMetadataEntry,
    GeneralCommissioning::Commands::CommissioningComplete::kMetadataEntry,
#if CHIP_CONFIG_TERMS_AND_CONDITIONS_REQUIRED
    GeneralCommissioning::Commands::SetTCAcknowledgements::kMetadataEntry,
#endif
};

constexpr CommandId kGeneratedCommands[] = {
    GeneralCommissioning::Commands::ArmFailSafeResponse::Id,
    GeneralCommissioning::Commands::SetRegulatoryConfigResponse::Id,
    GeneralCommissioning::Commands::CommissioningCompleteResponse::Id,
#if CHIP_CONFIG_TERMS_AND_CONDITIONS_REQUIRED
    GeneralCommissioning::Commands::SetTCAcknowledgementsResponse::Id,
#endif
};

} // namespace

DataModel::ActionReturnStatus GeneralCommissioningCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                                         AttributeValueEncoder & encoder)
{
    switch (request.path.mAttributeId)
    {
    case FeatureMap::Id: {
        return encoder.Encode(mLogic.GetFeatureFlags());
    }
    case ClusterRevision::Id: {
        return encoder.Encode(GeneralCommissioning::kRevision);
    }
    case RegulatoryConfig::Id: {
        return mLogic.ReadIfSupported(&ConfigurationManager::GetRegulatoryLocation, encoder);
    }
    case LocationCapability::Id: {
        return mLogic.ReadIfSupported(&ConfigurationManager::GetLocationCapability, encoder);
    }
    case BasicCommissioningInfo::Id: {
        return mLogic.ReadBasicCommissioningInfo(encoder);
    }
    case SupportsConcurrentConnection::Id: {
        return mLogic.ReadSupportsConcurrentConnection(encoder);
    }
#if CHIP_CONFIG_TERMS_AND_CONDITIONS_REQUIRED
    case TCAcceptedVersion::Id: {
        return mLogic.ReadTCAcceptedVersion(encoder);
    }
    case TCMinRequiredVersion::Id: {
        return mLogic.ReadTCMinRequiredVersion(encoder);
    }
    case TCAcknowledgements::Id: {
        return mLogic.ReadTCAcknowledgements(encoder);
    }
    case TCAcknowledgementsRequired::Id: {
        return mLogic.ReadTCAcknowledgementsRequired(encoder);
    }
    case TCUpdateDeadline::Id: {
        return mLogic.ReadTCUpdateDeadline(encoder);
    }
#endif // CHIP_CONFIG_TERMS_AND_CONDITIONS_REQUIRED
    default:
        return Protocols::InteractionModel::Status::UnsupportedAttribute;
    }
}


std::optional<DataModel::ActionReturnStatus> GeneralCommissioningCluster::InvokeCommand(const DataModel::InvokeRequest & request,
                                                                                        TLV::TLVReader & input_arguments,
                                                                                        CommandHandler * handler)
{
    using namespace GeneralCommissioning::Commands;
    switch (request.path.mCommandId)
    {
    case ArmFailSafe::Id:
        ArmFailSafe::DecodableType request_data;
        ReturnErrorOnFailure(request_data.Decode(input_arguments));
        return mLogic.HandleArmFailSafe(handler,request.path, handler->GetAccessingFabricIndex(),request_data);

    case CommissioningComplete::Id:
        CommissioningComplete::DecodableType request_data;
        ReturnErrorOnFailure(request_data.Decode(input_arguments));
        return mLogic.HandleCommissioningComplete(handler,request.path, handler->GetAccessingFabricIndex(),request_data);

    case SetRegulatoryConfig::Id:
        SetRegulatoryConfig::DecodableType request_data;
        ReturnErrorOnFailure(request_data.Decode(input_arguments));
        return mLogic.HandleSetRegulatoryConfig(handler,request.path, request_data);

#if CHIP_CONFIG_TERMS_AND_CONDITIONS_REQUIRED
    case SetTCAcknowledgements::Id:
        SetTCAcknowledgements::DecodableType request_data;
        ReturnErrorOnFailure(request_data.Decode(input_arguments));
        return mLogic.HandleSetTCAcknowledgements(handler,request.path, request_data);
#endif
    default:
        return Protocols::InteractionModel::Status::UnsupportedCommand;
    }

}

CHIP_ERROR GeneralCommissioningCluster::AcceptedCommands(const ConcreteClusterPath & path,
                                                        ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder)
{
    return builder.ReferenceExisting(kAcceptedCommands);
}

CHIP_ERROR GeneralCommissioningCluster::GeneratedCommands(const ConcreteClusterPath & path,
                                                         ReadOnlyBufferBuilder<CommandId> & builder)
{
    return builder.ReferenceExisting(kGeneratedCommands);
}

CHIP_ERROR GeneralCommissioningCluster::Attributes(const ConcreteClusterPath & path,
                                                   ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    AttributeListBuilder attributeListBuilder(builder);
    const DataModel::AttributeEntry kMandatoryAttributes[] = {
       GeneralCommissioning::Attributes::Breadcrumb::kMetadataEntry,
       GeneralCommissioning::Attributes::BasicCommissioningInfo::kMetadataEntry,
       GeneralCommissioning::Attributes::RegulatoryConfig::kMetadataEntry,
       GeneralCommissioning::Attributes::LocationCapability::kMetadataEntry,
       GeneralCommissioning::Attributes::SupportsConcurrentConnection::kMetadataEntry,
    };
    const BitFlags<GeneralCommissioning::Feature> featureFlags = mLogic.GetFeatureFlags();

    const bool hasTermsAndConditions  = featureFlags.Has(Feature::kTermsAndConditions);

    const AttributeListBuilder::OptionalAttributeEntry optionalEntries[] = {
        { hasTermsAndConditions, TCAcceptedVersion::kMetadataEntry },
        { hasTermsAndConditions, TCMinRequiredVersion::kMetadataEntry },
        { hasTermsAndConditions, TCAcknowledgements::kMetadataEntry },
        { hasTermsAndConditions, TCAcknowledgementsRequired::kMetadataEntry },
        { hasTermsAndConditions, TCUpdateDeadline::kMetadataEntry },
    };

    return attributeListBuilder.Append(Span(kMandatoryAttributes), Span(optionalEntries));
}


} // namespace Clusters
} // namespace app
} // namespace chip

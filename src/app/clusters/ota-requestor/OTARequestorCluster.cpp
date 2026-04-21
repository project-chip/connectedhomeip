/*
 *
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

#include <app/clusters/ota-requestor/OTARequestorCluster.h>

#include <app/clusters/ota-requestor/OTARequestorInterface.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <clusters/OtaSoftwareUpdateRequestor/AttributeIds.h>
#include <clusters/OtaSoftwareUpdateRequestor/ClusterId.h>
#include <clusters/OtaSoftwareUpdateRequestor/Commands.h>
#include <clusters/OtaSoftwareUpdateRequestor/Events.h>
#include <clusters/OtaSoftwareUpdateRequestor/Metadata.h>

namespace chip::app::Clusters {
namespace {

constexpr size_t kMaxMetadataLen = 512; // The maximum length of Metadata in any OTA Requestor command

constexpr DataModel::AcceptedCommandEntry kAcceptedCommands[] = {
    OtaSoftwareUpdateRequestor::Commands::AnnounceOTAProvider::kMetadataEntry,
};

} // namespace

OTARequestorCluster::OTARequestorCluster(EndpointId endpointId, OTARequestorCommandInterface & otaCommands,
                                         OTARequestorAttributes & attributes) :
    DefaultServerCluster(ConcreteClusterPath(endpointId, OtaSoftwareUpdateRequestor::Id)),
    mOtaCommands(otaCommands), mAttributes(attributes)
{}

CHIP_ERROR OTARequestorCluster::Startup(ServerClusterContext & context)
{
    ReturnErrorOnFailure(DefaultServerCluster::Startup(context));
    return mAttributes.SetInteractionModelContext(mPath.mEndpointId, *this, context.interactionContext.eventsGenerator);
}

DataModel::ActionReturnStatus OTARequestorCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                                 AttributeValueEncoder & encoder)
{
    switch (request.path.mAttributeId)
    {
    case OtaSoftwareUpdateRequestor::Attributes::DefaultOTAProviders::Id: {
        return encoder.EncodeList([this](const auto & listEncoder) -> CHIP_ERROR {
            ProviderLocationList::Iterator providerIterator = mAttributes.GetDefaultOtaProviderListIterator();
            while (providerIterator.Next())
            {
                ReturnErrorOnFailure(listEncoder.Encode(providerIterator.GetValue()));
            }
            return CHIP_NO_ERROR;
        });
    }
    case OtaSoftwareUpdateRequestor::Attributes::UpdatePossible::Id:
        return encoder.Encode(mAttributes.GetUpdatePossible());
    case OtaSoftwareUpdateRequestor::Attributes::UpdateState::Id:
        return encoder.Encode(mAttributes.GetUpdateState());
    case OtaSoftwareUpdateRequestor::Attributes::UpdateStateProgress::Id:
        return encoder.Encode(mAttributes.GetUpdateStateProgress());
    case Globals::Attributes::FeatureMap::Id:
        return encoder.Encode<uint32_t>(0);
    case Globals::Attributes::ClusterRevision::Id:
        return encoder.Encode(OtaSoftwareUpdateRequestor::kRevision);
    default:
        return Protocols::InteractionModel::Status::UnsupportedAttribute;
    }
}

DataModel::ActionReturnStatus OTARequestorCluster::WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                                  AttributeValueDecoder & aDecoder)
{
    switch (request.path.mAttributeId)
    {
    case OtaSoftwareUpdateRequestor::Attributes::DefaultOTAProviders::Id:
        return WriteDefaultOtaProviders(request.path, aDecoder);
    default:
        return Protocols::InteractionModel::Status::UnsupportedAttribute;
    }
}

CHIP_ERROR OTARequestorCluster::Attributes(const ConcreteClusterPath & path,
                                           ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    AttributeListBuilder listBuilder(builder);
    return listBuilder.Append(Span(OtaSoftwareUpdateRequestor::Attributes::kMandatoryMetadata), {});
}

CHIP_ERROR OTARequestorCluster::AcceptedCommands(const ConcreteClusterPath & path,
                                                 ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder)
{
    return builder.ReferenceExisting(kAcceptedCommands);
}

CHIP_ERROR OTARequestorCluster::WriteDefaultOtaProviders(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder)
{
    if (!aPath.IsListOperation() || aPath.mListOp == ConcreteDataAttributePath::ListOperation::ReplaceAll)
    {
        DataModel::DecodableList<OtaSoftwareUpdateRequestor::Structs::ProviderLocation::DecodableType> list;
        ReturnErrorOnFailure(aDecoder.Decode(list));

        ReturnErrorOnFailure(mAttributes.RemoveDefaultOtaProvider(aDecoder.AccessingFabricIndex()));

        auto iter = list.begin();
        while (iter.Next())
        {
            ReturnErrorOnFailure(mAttributes.AddDefaultOtaProvider(iter.GetValue()));
        }

        return iter.GetStatus();
    }

    switch (aPath.mListOp)
    {
    case ConcreteDataAttributePath::ListOperation::ReplaceItem:
        return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
    case ConcreteDataAttributePath::ListOperation::DeleteItem:
        return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
    case ConcreteDataAttributePath::ListOperation::AppendItem: {
        OtaSoftwareUpdateRequestor::Structs::ProviderLocation::DecodableType item;
        ReturnErrorOnFailure(aDecoder.Decode(item));
        return mAttributes.AddDefaultOtaProvider(item);
    }
    default:
        return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
    }

    return CHIP_NO_ERROR;
}

std::optional<DataModel::ActionReturnStatus> OTARequestorCluster::InvokeCommand(const DataModel::InvokeRequest & request,
                                                                                chip::TLV::TLVReader & input_arguments,
                                                                                CommandHandler * handler)
{
    switch (request.path.mCommandId)
    {
    case OtaSoftwareUpdateRequestor::Commands::AnnounceOTAProvider::Id: {
        VerifyOrReturnError(handler != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

        OtaSoftwareUpdateRequestor::Commands::AnnounceOTAProvider::DecodableType data;
        ReturnErrorOnFailure(data.Decode(input_arguments));

        auto & metadataForNode = data.metadataForNode;
        if (metadataForNode.HasValue() && metadataForNode.Value().size() > kMaxMetadataLen)
        {
            ChipLogError(Zcl, "Metadata size %u exceeds max %u", static_cast<unsigned>(metadataForNode.Value().size()),
                         static_cast<unsigned>(kMaxMetadataLen));
            return Protocols::InteractionModel::Status::ConstraintError;
        }
        mOtaCommands.HandleAnnounceOTAProvider(handler, request.path, data);
        return std::nullopt;
    }
    default:
        return Protocols::InteractionModel::Status::UnsupportedCommand;
    }
}

CHIP_ERROR OTARequestorCluster::GenerateVersionAppliedEvent(const DefaultOTARequestorEventGenerator::VersionAppliedEvent & event)
{
    VerifyOrReturnError(mContext != nullptr, CHIP_ERROR_INCORRECT_STATE);
    auto eventNumber = mContext->interactionContext.eventsGenerator.GenerateEvent(event, mPath.mEndpointId);
    VerifyOrReturnError(eventNumber.has_value(), CHIP_ERROR_INTERNAL);
    return CHIP_NO_ERROR;
}

CHIP_ERROR OTARequestorCluster::GenerateDownloadErrorEvent(const DefaultOTARequestorEventGenerator::DownloadErrorEvent & event)
{
    VerifyOrReturnError(mContext != nullptr, CHIP_ERROR_INCORRECT_STATE);
    auto eventNumber = mContext->interactionContext.eventsGenerator.GenerateEvent(event, mPath.mEndpointId);
    VerifyOrReturnError(eventNumber.has_value(), CHIP_ERROR_INTERNAL);
    return CHIP_NO_ERROR;
}

} // namespace chip::app::Clusters

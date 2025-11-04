/*
 *
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

#include <app/clusters/ota-requestor/OtaRequestorCluster.h>

#include <app/clusters/ota-requestor/OTARequestorInterface.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <clusters/OtaSoftwareUpdateRequestor/ClusterId.h>
#include <clusters/OtaSoftwareUpdateRequestor/Metadata.h>

namespace chip::app::Clusters {
namespace {

constexpr size_t kMaxMetadataLen = 512; // The maximum length of Metadata in any OTA Requestor command

constexpr DataModel::AcceptedCommandEntry kAcceptedCommands[] = {
    OtaSoftwareUpdateRequestor::Commands::AnnounceOTAProvider::kMetadataEntry,
};

}  // namespace

OtaRequestorCluster::OtaRequestorCluster(EndpointId endpointId, OTARequestorInterface & otaRequestor)
    : DefaultServerCluster(ConcreteClusterPath(endpointId, OtaSoftwareUpdateRequestor::Id)),
      mOtaRequestor(otaRequestor)
{
}

DataModel::ActionReturnStatus OtaRequestorCluster::ReadAttribute(
    const DataModel::ReadAttributeRequest & request,
    AttributeValueEncoder & encoder)
{
    switch (request.path.mAttributeId)
    {
    case OtaSoftwareUpdateRequestor::Attributes::DefaultOTAProviders::Id: {
        return encoder.EncodeList([this](const auto & listEncoder) -> CHIP_ERROR {
            ProviderLocationList::Iterator providerIterator = mOtaRequestor.GetDefaultOTAProviderListIterator();
            CHIP_ERROR error = CHIP_NO_ERROR;
            while (error == CHIP_NO_ERROR && providerIterator.Next()) {
                error = listEncoder.Encode(providerIterator.GetValue());
            }
            return error;
        });
    }
    case OtaSoftwareUpdateRequestor::Attributes::UpdatePossible::Id:
        return encoder.Encode(mOtaRequestor.GetUpdatePossible());
    case OtaSoftwareUpdateRequestor::Attributes::UpdateState::Id:
        return encoder.Encode(mOtaRequestor.GetCurrentUpdateState());
    case OtaSoftwareUpdateRequestor::Attributes::UpdateStateProgress::Id:
        return encoder.Encode(mOtaRequestor.GetCurrentUpdateStateProgress());
    case Globals::Attributes::FeatureMap::Id:
        return encoder.Encode<uint32_t>(0);
    case Globals::Attributes::ClusterRevision::Id:
        return encoder.Encode(OtaSoftwareUpdateRequestor::kRevision);
    default:
        return Protocols::InteractionModel::Status::UnsupportedAttribute;
    }
}

CHIP_ERROR OtaRequestorCluster::Attributes(const ConcreteClusterPath & path,
                                           ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    AttributeListBuilder listBuilder(builder);
    return listBuilder.Append(Span(OtaSoftwareUpdateRequestor::Attributes::kMandatoryMetadata), {});
}

std::optional<DataModel::ActionReturnStatus> OtaRequestorCluster::InvokeCommand(
    const DataModel::InvokeRequest & request, chip::TLV::TLVReader & input_arguments, CommandHandler * handler)
{
    switch (request.path.mCommandId)
    {
    case OtaSoftwareUpdateRequestor::Commands::AnnounceOTAProvider::Id: {
        OtaSoftwareUpdateRequestor::Commands::AnnounceOTAProvider::DecodableType data;
        ReturnErrorOnFailure(data.Decode(input_arguments));

        auto & metadataForNode = data.metadataForNode;
        if (metadataForNode.HasValue() && metadataForNode.Value().size() > kMaxMetadataLen)
        {
            ChipLogError(Zcl, "Metadata size %u exceeds max %u", static_cast<unsigned>(metadataForNode.Value().size()),
                         static_cast<unsigned>(kMaxMetadataLen));
            return Protocols::InteractionModel::Status::InvalidCommand;
        }
        mOtaRequestor.HandleAnnounceOTAProvider(handler, request.path, data);
        return std::nullopt;
    }
    default:
        return Protocols::InteractionModel::Status::UnsupportedCommand;
    }
}

CHIP_ERROR OtaRequestorCluster::AcceptedCommands(const ConcreteClusterPath & path,
                                                 ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder)
{
    return builder.ReferenceExisting(kAcceptedCommands);
}

}  // namespace chip::app::Clusters

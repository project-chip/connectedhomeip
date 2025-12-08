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

OTARequestorCluster::OTARequestorCluster(EndpointId endpointId, OTARequestorInterface * otaRequestor) :
    DefaultServerCluster(ConcreteClusterPath(endpointId, OtaSoftwareUpdateRequestor::Id)),
    mEventHandlerRegistration(*this, endpointId), mOtaRequestor(otaRequestor)
{}

CHIP_ERROR OTARequestorCluster::Startup(ServerClusterContext & context)
{
#if CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
    ChipLogError(SoftwareUpdate, "Initializing OTA requestor on endpoint %u with flag 'chip_enable_ota_requestor' disabled",
                 mPath.mEndpointId);
#endif
    ReturnErrorOnFailure(DefaultServerCluster::Startup(context));
    if (mOtaRequestor)
    {
        return mOtaRequestor->RegisterEventHandler(mEventHandlerRegistration);
    }
    return CHIP_NO_ERROR;
}

void OTARequestorCluster::Shutdown()
{
    if (mOtaRequestor)
    {
        SuccessOrLog(mOtaRequestor->UnregisterEventHandler(mPath.mEndpointId), SoftwareUpdate,
                     "Unable to unregister event handling for endpoint %u during shutdown", mPath.mEndpointId);
    }
    DefaultServerCluster::Shutdown();
}

DataModel::ActionReturnStatus OTARequestorCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                                 AttributeValueEncoder & encoder)
{
    switch (request.path.mAttributeId)
    {
    case OtaSoftwareUpdateRequestor::Attributes::DefaultOTAProviders::Id: {
        if (!mOtaRequestor)
        {
            // There are examples which enable the OTA requestor cluster in their zap configuration but don't
            // enable the build flag that controls the implementation of the OTA requestor. The behaviour here
            // maintains backwards compatibility for those examples.
#if CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
            return CHIP_ERROR_INTERNAL;
#else
            return encoder.EncodeEmptyList();
#endif
        }
        return encoder.EncodeList([this](const auto & listEncoder) -> CHIP_ERROR {
            ProviderLocationList::Iterator providerIterator = mOtaRequestor->GetDefaultOTAProviderListIterator();
            CHIP_ERROR error                                = CHIP_NO_ERROR;
            while (error == CHIP_NO_ERROR && providerIterator.Next())
            {
                error = listEncoder.Encode(providerIterator.GetValue());
            }
            return error;
        });
    }
    case OtaSoftwareUpdateRequestor::Attributes::UpdatePossible::Id:
        return encoder.Encode(mUpdatePossible);
    case OtaSoftwareUpdateRequestor::Attributes::UpdateState::Id:
        if (!mOtaRequestor)
        {
            // There are examples which enable the OTA requestor cluster in their zap configuration but don't
            // enable the build flag that controls the implementation of the OTA requestor. The behaviour here
            // maintains backwards compatibility for those examples.
#if CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
            return CHIP_ERROR_INTERNAL;
#else
            return encoder.Encode(OtaSoftwareUpdateRequestor::UpdateStateEnum::kUnknown);
#endif
        }
        return encoder.Encode(mOtaRequestor->GetCurrentUpdateState());
    case OtaSoftwareUpdateRequestor::Attributes::UpdateStateProgress::Id:
        if (!mOtaRequestor)
        {
            // There are examples which enable the OTA requestor cluster in their zap configuration but don't
            // enable the build flag that controls the implementation of the OTA requestor. The behaviour here
            // maintains backwards compatibility for those examples.
#if CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
            return CHIP_ERROR_INTERNAL;
#else
            return encoder.EncodeNull();
#endif
        }
        return encoder.Encode(mOtaRequestor->GetCurrentUpdateStateProgress());
    case Globals::Attributes::FeatureMap::Id:
        return encoder.Encode<uint32_t>(0);
    case Globals::Attributes::ClusterRevision::Id:
        return encoder.Encode(OtaSoftwareUpdateRequestor::kRevision);
    default:
        return Protocols::InteractionModel::Status::UnsupportedAttribute;
    }
}

DataModel::ActionReturnStatus OTARequestorCluster::WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                                  AttributeValueDecoder & decoder)
{
    switch (request.path.mAttributeId)
    {
    case OtaSoftwareUpdateRequestor::Attributes::DefaultOTAProviders::Id:
        return NotifyAttributeChangedIfSuccess(OtaSoftwareUpdateRequestor::Attributes::DefaultOTAProviders::Id,
                                               WriteDefaultOtaProviders(request.path, decoder));
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

std::optional<DataModel::ActionReturnStatus> OTARequestorCluster::InvokeCommand(const DataModel::InvokeRequest & request,
                                                                                chip::TLV::TLVReader & input_arguments,
                                                                                CommandHandler * handler)
{
    switch (request.path.mCommandId)
    {
    case OtaSoftwareUpdateRequestor::Commands::AnnounceOTAProvider::Id: {
        if (!mOtaRequestor)
        {
            return CHIP_ERROR_INTERNAL;
        }
        OtaSoftwareUpdateRequestor::Commands::AnnounceOTAProvider::DecodableType data;
        ReturnErrorOnFailure(data.Decode(input_arguments));

        auto & metadataForNode = data.metadataForNode;
        if (metadataForNode.HasValue() && metadataForNode.Value().size() > kMaxMetadataLen)
        {
            ChipLogError(Zcl, "Metadata size %u exceeds max %u", static_cast<unsigned>(metadataForNode.Value().size()),
                         static_cast<unsigned>(kMaxMetadataLen));
            return Protocols::InteractionModel::Status::InvalidCommand;
        }
        mOtaRequestor->HandleAnnounceOTAProvider(handler, request.path, data);
        return std::nullopt;
    }
    default:
        return Protocols::InteractionModel::Status::UnsupportedCommand;
    }
}

CHIP_ERROR OTARequestorCluster::AcceptedCommands(const ConcreteClusterPath & path,
                                                 ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder)
{
    return builder.ReferenceExisting(kAcceptedCommands);
}

void OTARequestorCluster::OnStateTransition(OTARequestorEventHandler::UpdateStateEnum previousState,
                                            OTARequestorEventHandler::UpdateStateEnum newState,
                                            OTARequestorEventHandler::ChangeReasonEnum reason,
                                            DataModel::Nullable<uint32_t> const & targetSoftwareVersion)
{
    if (previousState == newState)
    {
        ChipLogError(Zcl, "Previous state and new state are the same (%d), no event to log", to_underlying(newState));
        return;
    }
    OtaSoftwareUpdateRequestor::Events::StateTransition::Type event = { previousState, newState, reason, targetSoftwareVersion };
    mContext->interactionContext.eventsGenerator.GenerateEvent(event, mPath.mEndpointId);
}

void OTARequestorCluster::OnVersionApplied(uint32_t softwareVersion, uint16_t productId)
{
    OtaSoftwareUpdateRequestor::Events::VersionApplied::Type event = { softwareVersion, productId };
    mContext->interactionContext.eventsGenerator.GenerateEvent(event, mPath.mEndpointId);
}

void OTARequestorCluster::OnDownloadError(uint32_t softwareVersion, uint64_t bytesDownloaded,
                                          DataModel::Nullable<uint8_t> progressPercent, DataModel::Nullable<int64_t> platformCode)
{
    OtaSoftwareUpdateRequestor::Events::DownloadError::Type event = { softwareVersion, bytesDownloaded, progressPercent,
                                                                      platformCode };
    mContext->interactionContext.eventsGenerator.GenerateEvent(event, mPath.mEndpointId);
}

CHIP_ERROR OTARequestorCluster::WriteDefaultOtaProviders(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder)
{
    chip::OTARequestorInterface * requestor = mOtaRequestor;
    if (requestor == nullptr)
    {
        return CHIP_ERROR_INTERNAL;
    }

    if (!aPath.IsListOperation() || aPath.mListOp == ConcreteDataAttributePath::ListOperation::ReplaceAll)
    {
        DataModel::DecodableList<OtaSoftwareUpdateRequestor::Structs::ProviderLocation::DecodableType> list;
        ReturnErrorOnFailure(aDecoder.Decode(list));

        ReturnErrorOnFailure(requestor->ClearDefaultOtaProviderList(aDecoder.AccessingFabricIndex()));

        auto iter = list.begin();
        while (iter.Next())
        {
            ReturnErrorOnFailure(requestor->AddDefaultOtaProvider(iter.GetValue()));
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
        return requestor->AddDefaultOtaProvider(item);
    }
    default:
        return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
    }

    return CHIP_NO_ERROR;
}

} // namespace chip::app::Clusters

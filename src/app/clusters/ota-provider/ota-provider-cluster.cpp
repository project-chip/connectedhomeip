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
#include <app/clusters/ota-provider/ota-provider-cluster.h>

#include <clusters/OtaSoftwareUpdateProvider/Commands.h>
#include <clusters/OtaSoftwareUpdateProvider/Ids.h>
#include <clusters/OtaSoftwareUpdateProvider/Metadata.h>
#include <optional>

namespace chip {
namespace app {
namespace Clusters {

namespace {

constexpr size_t kLocationLen          = 2;   // The expected length of the location parameter in QueryImage
constexpr size_t kMaxMetadataLen       = 512; // The maximum length of Metadata in any OTA Provider command
constexpr size_t kUpdateTokenMaxLength = 32;  // The expected length of the Update Token parameter used in multiple commands
constexpr size_t kUpdateTokenMinLength = 8;   // The expected length of the Update Token parameter used in multiple commands

using namespace chip::app::Clusters::OtaSoftwareUpdateProvider::Commands;

constexpr DataModel::AcceptedCommandEntry kAcceptedCommands[] = {
    QueryImage::kMetadataEntry,
    ApplyUpdateRequest::kMetadataEntry,
    NotifyUpdateApplied::kMetadataEntry,
};

constexpr CommandId kGeneratedCommands[] = {
    QueryImageResponse::Id,
    ApplyUpdateResponse::Id,
};

} // namespace

using Protocols::InteractionModel::Status;
using namespace OtaSoftwareUpdateProvider::Commands;

DataModel::ActionReturnStatus OtaProviderServer::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                               AttributeValueEncoder & encoder)
{
    switch (request.path.mAttributeId)
    {
    case Globals::Attributes::FeatureMap::Id:
        // No features defined for the cluster ...
        return encoder.Encode<uint32_t>(0);
    case Globals::Attributes::ClusterRevision::Id:
        return encoder.Encode(OtaSoftwareUpdateProvider::kRevision);
    }

    return Status::UnsupportedAttribute;
}

CHIP_ERROR OtaProviderServer::AcceptedCommands(const ConcreteClusterPath & path,
                                               ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder)
{
    return builder.ReferenceExisting(kAcceptedCommands);
}

CHIP_ERROR OtaProviderServer::GeneratedCommands(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<CommandId> & builder)
{
    return builder.ReferenceExisting(kGeneratedCommands);
}

std::optional<DataModel::ActionReturnStatus> OtaProviderServer::InvokeCommand(const DataModel::InvokeRequest & request,
                                                                              TLV::TLVReader & input_arguments,
                                                                              CommandHandler * handler)
{
    switch (request.path.mCommandId)
    {
    case QueryImage::Id: {
        QueryImage::DecodableType data;
        ReturnErrorOnFailure(data.Decode(input_arguments));
        return QueryImage(request.path, data, handler);
    }
    case ApplyUpdateRequest::Id: {
        ApplyUpdateRequest::DecodableType data;
        ReturnErrorOnFailure(data.Decode(input_arguments));
        return ApplyUpdateRequest(request.path, data, handler);
    }
    case NotifyUpdateApplied::Id: {
        NotifyUpdateApplied::DecodableType data;
        ReturnErrorOnFailure(data.Decode(input_arguments));
        return NotifyUpdateApplied(request.path, data, handler);
    }
    }

    return Status::UnsupportedCommand;
}

bool OtaProviderLogic::IsNullDelegateWithLogging(EndpointId endpointIdForLogging)
{

    if (mDelegate == nullptr)
    {
        ChipLogError(Zcl, "No OTAProviderDelegate set for ep:%u", endpointIdForLogging);
        return true;
    }
    return false;
}

bool OtaProviderLogic::IsValidUpdateTokenWithLogging(ByteSpan updateToken)
{
    if (updateToken.size() > kUpdateTokenMaxLength || updateToken.size() < kUpdateTokenMinLength)
    {
        ChipLogError(Zcl, "Expected size in [%u, %u] for UpdateToken, got %u", static_cast<unsigned int>(kUpdateTokenMinLength),
                     static_cast<unsigned int>(kUpdateTokenMaxLength), static_cast<unsigned int>(updateToken.size()));
        return false;
    }
    return true;
}

std::optional<DataModel::ActionReturnStatus>
OtaProviderLogic::ApplyUpdateRequest(const ConcreteCommandPath & commandPath,
                                     const OtaSoftwareUpdateProvider::Commands::ApplyUpdateRequest::DecodableType & commandData,
                                     CommandHandler * handler)
{
    if (IsNullDelegateWithLogging(commandPath.mEndpointId))
    {
        return Status::UnsupportedCommand;
    }
    auto & updateToken = commandData.updateToken;

    ChipLogProgress(Zcl, "OTA Provider received ApplyUpdateRequest");
    ChipLogDetail(Zcl, "  Update Token: %u", static_cast<unsigned int>(updateToken.size()));
    ChipLogDetail(Zcl, "  New Version: %" PRIu32, commandData.newVersion);

    VerifyOrReturnError(IsValidUpdateTokenWithLogging(updateToken), Status::InvalidCommand);

    mDelegate->HandleApplyUpdateRequest(handler, commandPath, commandData);
    return std::nullopt;
}

std::optional<DataModel::ActionReturnStatus>
OtaProviderLogic::NotifyUpdateApplied(const ConcreteCommandPath & commandPath,
                                      const OtaSoftwareUpdateProvider::Commands::NotifyUpdateApplied::DecodableType & commandData,
                                      CommandHandler * handler)
{
    if (IsNullDelegateWithLogging(commandPath.mEndpointId))
    {
        return Status::UnsupportedCommand;
    }

    auto & updateToken = commandData.updateToken;

    ChipLogProgress(Zcl, "OTA Provider received NotifyUpdateApplied");
    ChipLogDetail(Zcl, "  Update Token: %u", static_cast<unsigned int>(updateToken.size()));
    ChipLogDetail(Zcl, "  Software Version: %" PRIu32, commandData.softwareVersion);

    VerifyOrReturnError(IsValidUpdateTokenWithLogging(updateToken), Status::InvalidCommand);

    mDelegate->HandleNotifyUpdateApplied(handler, commandPath, commandData);
    return std::nullopt;
}

std::optional<DataModel::ActionReturnStatus>
OtaProviderLogic::QueryImage(const ConcreteCommandPath & commandPath,
                             const OtaSoftwareUpdateProvider::Commands::QueryImage::DecodableType & commandData,
                             CommandHandler * handler)
{
    if (IsNullDelegateWithLogging(commandPath.mEndpointId))
    {
        return Status::UnsupportedCommand;
    }

    auto & vendorId            = commandData.vendorID;
    auto & productId           = commandData.productID;
    auto & hardwareVersion     = commandData.hardwareVersion;
    auto & softwareVersion     = commandData.softwareVersion;
    auto & protocolsSupported  = commandData.protocolsSupported;
    auto & location            = commandData.location;
    auto & requestorCanConsent = commandData.requestorCanConsent;
    auto & metadataForProvider = commandData.metadataForProvider;

    (void) vendorId;
    (void) productId;
    (void) softwareVersion;

    ChipLogProgress(Zcl, "OTA Provider received QueryImage");
    ChipLogDetail(Zcl, "  VendorID: 0x%x", vendorId);
    ChipLogDetail(Zcl, "  ProductID: %u", productId);
    ChipLogDetail(Zcl, "  SoftwareVersion: %" PRIu32, softwareVersion);
    ChipLogDetail(Zcl, "  ProtocolsSupported: [");
    auto protocolIter = protocolsSupported.begin();
    while (protocolIter.Next())
    {
        ChipLogDetail(Zcl, "    %u", to_underlying(protocolIter.GetValue()));
    }
    ChipLogDetail(Zcl, "  ]");
    if (hardwareVersion.HasValue())
    {
        ChipLogDetail(Zcl, "  HardwareVersion: %u", hardwareVersion.Value());
    }
    if (location.HasValue())
    {
        ChipLogDetail(Zcl, "  Location: %.*s", static_cast<int>(location.Value().size()), location.Value().data());
    }
    if (requestorCanConsent.HasValue())
    {
        ChipLogDetail(Zcl, "  RequestorCanConsent: %u", requestorCanConsent.Value());
    }
    if (metadataForProvider.HasValue())
    {
        ChipLogDetail(Zcl, "  MetadataForProvider: %u", static_cast<unsigned int>(metadataForProvider.Value().size()));
    }

    if (location.HasValue() && location.Value().size() != kLocationLen)
    {
        ChipLogError(Zcl, "location param length %u != expected length %u", static_cast<unsigned int>(location.Value().size()),
                     static_cast<unsigned int>(kLocationLen));
        return Status::InvalidCommand;
    }

    if (metadataForProvider.HasValue() && metadataForProvider.Value().size() > kMaxMetadataLen)
    {
        ChipLogError(Zcl, "metadata size %u exceeds max %u", static_cast<unsigned int>(metadataForProvider.Value().size()),
                     static_cast<unsigned int>(kMaxMetadataLen));
        return Status::InvalidCommand;
    }

    mDelegate->HandleQueryImage(handler, commandPath, commandData);

    // delegate directly uses command object
    return std::nullopt;
}

} // namespace Clusters
} // namespace app
} // namespace chip

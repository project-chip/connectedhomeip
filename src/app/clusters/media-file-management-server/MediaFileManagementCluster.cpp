/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include <app/clusters/media-file-management-server/MediaFileManagementCluster.h>

#include <app/data-model/Decode.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <clusters/MediaFileManagement/Commands.h>
#include <clusters/MediaFileManagement/Events.h>
#include <clusters/MediaFileManagement/Metadata.h>

#include <lib/support/CodeUtils.h>

namespace chip {
namespace app {
namespace Clusters {

using namespace MediaFileManagement;
using Protocols::InteractionModel::Status;

namespace {

// Upper bounds for the delegate-backed lists, per the spec constraints.
constexpr size_t kMaxAvailableFiles     = 256;
constexpr size_t kMaxSupportedMimeTypes = 64;
// SupportedMimeTypes entries are MIME type strings (spec constraint: max 64 chars per entry).
constexpr size_t kMaxMimeTypeLength = 64;

} // namespace

DataModel::ActionReturnStatus MediaFileManagementCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                                        AttributeValueEncoder & encoder)
{
    using namespace Attributes;
    switch (request.path.mAttributeId)
    {
    case ClusterRevision::Id:
        return encoder.Encode(MediaFileManagement::kRevision);
    case FeatureMap::Id:
        return encoder.Encode(mFeatures.Raw());
    case TotalStorage::Id:
        return encoder.Encode(mDelegate.GetTotalStorage());
    case AvailableStorage::Id:
        return encoder.Encode(mDelegate.GetAvailableStorage());
    case AvailableFiles::Id:
        return encoder.EncodeList([this](const auto & listEncoder) -> CHIP_ERROR { return this->ReadAvailableFiles(listEncoder); });
    case SupportedMimeTypes::Id:
        return encoder.EncodeList(
            [this](const auto & listEncoder) -> CHIP_ERROR { return this->ReadSupportedMimeTypes(listEncoder); });
    default:
        return Status::UnsupportedAttribute;
    }
}

CHIP_ERROR MediaFileManagementCluster::ReadAvailableFiles(const AttributeValueEncoder::ListEncodeHelper & encoder)
{
    for (size_t i = 0; i < kMaxAvailableFiles; i++)
    {
        Structs::FileDescriptionStruct::Type file;
        CHIP_ERROR err = mDelegate.GetFileAtIndex(i, file);
        if (err == CHIP_ERROR_PROVIDER_LIST_EXHAUSTED)
        {
            return CHIP_NO_ERROR;
        }
        ReturnErrorOnFailure(err);
        ReturnErrorOnFailure(encoder.Encode(file));
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR MediaFileManagementCluster::ReadSupportedMimeTypes(const AttributeValueEncoder::ListEncodeHelper & encoder)
{
    for (size_t i = 0; i < kMaxSupportedMimeTypes; i++)
    {
        char buffer[kMaxMimeTypeLength] = {};
        MutableCharSpan mimeType(buffer);
        CHIP_ERROR err = mDelegate.GetSupportedMimeTypeAtIndex(i, mimeType);
        if (err == CHIP_ERROR_PROVIDER_LIST_EXHAUSTED)
        {
            return CHIP_NO_ERROR;
        }
        ReturnErrorOnFailure(err);
        ReturnErrorOnFailure(encoder.Encode(CharSpan(mimeType.data(), mimeType.size())));
    }
    return CHIP_NO_ERROR;
}

std::optional<DataModel::ActionReturnStatus> MediaFileManagementCluster::InvokeCommand(const DataModel::InvokeRequest & request,
                                                                                       chip::TLV::TLVReader & input_arguments,
                                                                                       CommandHandler * handler)
{
    using namespace Commands;
    switch (request.path.mCommandId)
    {
    case AddFile::Id: {
        AddFile::DecodableType req;
        ReturnErrorOnFailure(DataModel::Decode(input_arguments, req));
        AddFileResponse::Type response;
        Status status = mDelegate.HandleAddFile(req.name, req.size, req.mimeType, req.imageUri, response);
        VerifyOrReturnValue(status == Status::Success, status);
        handler->AddResponse(request.path, response);
        return std::nullopt;
    }
    case DeleteFile::Id: {
        DeleteFile::DecodableType req;
        ReturnErrorOnFailure(DataModel::Decode(input_arguments, req));
        return mDelegate.HandleDeleteFile(req.fileID);
    }
    case RequestSharedFiles::Id: {
        RequestSharedFiles::DecodableType req;
        ReturnErrorOnFailure(DataModel::Decode(input_arguments, req));
        return mDelegate.HandleRequestSharedFiles(req.clientName, req.requestID, req.supportedMimeTypes);
    }
    case GetSharedFile::Id: {
        GetSharedFile::DecodableType req;
        ReturnErrorOnFailure(DataModel::Decode(input_arguments, req));
        GetSharedFileResponse::Type response;
        Status status = mDelegate.HandleGetSharedFile(req.responseID, response);
        VerifyOrReturnValue(status == Status::Success, status);
        handler->AddResponse(request.path, response);
        return std::nullopt;
    }
    case OfferFile::Id: {
        OfferFile::DecodableType req;
        ReturnErrorOnFailure(DataModel::Decode(input_arguments, req));
        return mDelegate.HandleOfferFile(req.clientName, req.name, req.size, req.mimeType, req.imageUri);
    }
    default:
        return Status::UnsupportedCommand;
    }
}

CHIP_ERROR MediaFileManagementCluster::Attributes(const ConcreteClusterPath & path,
                                                  ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    AttributeListBuilder listBuilder(builder);
    // All four attributes are mandatory for this cluster; no optional attributes.
    return listBuilder.Append(Span(Attributes::kMandatoryMetadata), Span<const DataModel::AttributeEntry>(), {});
}

CHIP_ERROR MediaFileManagementCluster::AcceptedCommands(const ConcreteClusterPath & path,
                                                        ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder)
{
    using namespace Commands;
    const bool sharing = HasFeature(Feature::kMediaSharing);

    // AddFile and DeleteFile are mandatory; the sharing commands require the MediaSharing feature.
    ReturnErrorOnFailure(builder.EnsureAppendCapacity(sharing ? 5 : 2));
    ReturnErrorOnFailure(builder.Append(AddFile::kMetadataEntry));
    ReturnErrorOnFailure(builder.Append(DeleteFile::kMetadataEntry));
    if (sharing)
    {
        ReturnErrorOnFailure(builder.Append(RequestSharedFiles::kMetadataEntry));
        ReturnErrorOnFailure(builder.Append(GetSharedFile::kMetadataEntry));
        ReturnErrorOnFailure(builder.Append(OfferFile::kMetadataEntry));
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR MediaFileManagementCluster::GeneratedCommands(const ConcreteClusterPath & path,
                                                         ReadOnlyBufferBuilder<CommandId> & builder)
{
    using namespace Commands;
    // AddFileResponse is always generated; GetSharedFileResponse only with MediaSharing.
    const bool sharing = HasFeature(Feature::kMediaSharing);
    ReturnErrorOnFailure(builder.EnsureAppendCapacity(sharing ? 2 : 1));
    ReturnErrorOnFailure(builder.Append(AddFileResponse::Id));
    if (sharing)
    {
        ReturnErrorOnFailure(builder.Append(GetSharedFileResponse::Id));
    }
    return CHIP_NO_ERROR;
}

void MediaFileManagementCluster::GenerateSharedFilesAddedEvent(uint16_t requestID, uint16_t responseID)
{
    VerifyOrReturn(mContext != nullptr, ChipLogError(Zcl, "MediaFileManagementCluster: mContext is null, cannot generate event"));
    Events::SharedFilesAdded::Type event;
    event.requestID  = requestID;
    event.responseID = responseID;
    mContext->interactionContext.eventsGenerator.GenerateEvent(event, mPath.mEndpointId);
}

} // namespace Clusters
} // namespace app
} // namespace chip

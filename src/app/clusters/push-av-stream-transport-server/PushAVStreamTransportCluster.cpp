/**
 *
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
 *
 */

#include "constants.h"
#include <app/AttributeAccessInterface.h>
#include <app/AttributeAccessInterfaceRegistry.h>
#include <app/CommandHandlerInterfaceRegistry.h>
#include <app/EventLogging.h>
#include <app/InteractionModelEngine.h>
#include <app/clusters/push-av-stream-transport-server/PushAVStreamTransportCluster.h>
#include <app/reporting/reporting.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <app/util/util.h>
#include <clusters/PushAvStreamTransport/Commands.h>
#include <clusters/PushAvStreamTransport/Ids.h>
#include <clusters/PushAvStreamTransport/Metadata.h>
#include <lib/core/CHIPSafeCasts.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/DefaultStorageKeyAllocator.h>
#include <protocols/interaction_model/StatusCode.h>

namespace chip {
namespace app {
namespace Clusters {
namespace {

using namespace chip::app::Clusters::PushAvStreamTransport::Commands;

constexpr DataModel::AcceptedCommandEntry kAcceptedCommands[] = {
    AllocatePushTransport::kMetadataEntry, DeallocatePushTransport::kMetadataEntry,  ModifyPushTransport::kMetadataEntry,
    SetTransportStatus::kMetadataEntry,    ManuallyTriggerTransport::kMetadataEntry, FindTransport::kMetadataEntry,
};

constexpr CommandId kGeneratedCommands[] = {
    AllocatePushTransportResponse::Id,
    FindTransportResponse::Id,
};

} // namespace

using Protocols::InteractionModel::Status;
using namespace PushAvStreamTransport::Commands;

CHIP_ERROR PushAvStreamTransportServer::Attributes(const ConcreteClusterPath & path,
                                                   ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    AttributeListBuilder listBuilder(builder);
    return listBuilder.Append(Span(PushAvStreamTransport::Attributes::kMandatoryMetadata), {}, {});
}

CHIP_ERROR PushAvStreamTransportServer::ReadAndEncodeSupportedFormats(const AttributeValueEncoder::ListEncodeHelper & encoder)
{
    for (const auto & supportsFormat : mLogic.mSupportedFormats)
    {
        ReturnErrorOnFailure(encoder.Encode(supportsFormat));
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR
PushAvStreamTransportServer::ReadAndEncodeCurrentConnections(const AttributeValueEncoder::ListEncodeHelper & encoder,
                                                             FabricIndex fabricIndex)
{
    for (const auto & currentConnections : mLogic.mCurrentConnections)
    {
        if (currentConnections.fabricIndex == fabricIndex)
        {
            ReturnErrorOnFailure(encoder.Encode(currentConnections));
        }
    }

    return CHIP_NO_ERROR;
}

DataModel::ActionReturnStatus PushAvStreamTransportServer::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                                         AttributeValueEncoder & aEncoder)
{
    VerifyOrDie(request.path.mClusterId == PushAvStreamTransport::Id);

    ChipLogProgress(Zcl, "Push AV Stream Transport[ep=%d]: Reading Attribute %" PRIu32, request.path.mEndpointId,
                    request.path.mAttributeId);

    switch (request.path.mAttributeId)
    {
    case PushAvStreamTransport::Attributes::FeatureMap::Id:
        return aEncoder.Encode(mLogic.mFeatures);

    case PushAvStreamTransport::Attributes::ClusterRevision::Id:
        return aEncoder.Encode(PushAvStreamTransport::kRevision);

    case PushAvStreamTransport::Attributes::SupportedFormats::Id:
        return aEncoder.EncodeList([this](const auto & encoder) -> CHIP_ERROR { return ReadAndEncodeSupportedFormats(encoder); });

    case PushAvStreamTransport::Attributes::CurrentConnections::Id:
        return aEncoder.EncodeList([this, &aEncoder](const auto & encoder) -> CHIP_ERROR {
            CHIP_ERROR err = ReadAndEncodeCurrentConnections(encoder, aEncoder.AccessingFabricIndex());
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "Push AV Stream Transport: Error reading CurrentConnections %" CHIP_ERROR_FORMAT, err.Format());
            }
            return err;
        });
    }

    return Status::UnsupportedAttribute;
}

CHIP_ERROR PushAvStreamTransportServer::AcceptedCommands(const ConcreteClusterPath & path,
                                                         ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder)
{
    return builder.ReferenceExisting(kAcceptedCommands);
}

CHIP_ERROR PushAvStreamTransportServer::GeneratedCommands(const ConcreteClusterPath & path,
                                                          ReadOnlyBufferBuilder<CommandId> & builder)
{
    return builder.ReferenceExisting(kGeneratedCommands);
}

std::optional<DataModel::ActionReturnStatus> PushAvStreamTransportServer::InvokeCommand(const DataModel::InvokeRequest & request,
                                                                                        TLV::TLVReader & input_arguments,
                                                                                        CommandHandler * handler)
{
    using namespace PushAvStreamTransport::Commands;

    FabricIndex fabricIndex = request.GetAccessingFabricIndex();

    switch (request.path.mCommandId)
    {
    case AllocatePushTransport::Id: {
        AllocatePushTransport::DecodableType data;
        ReturnErrorOnFailure(data.Decode(input_arguments, fabricIndex));
        return mLogic.HandleAllocatePushTransport(*handler, request.path, data);
    }
    case DeallocatePushTransport::Id: {
        DeallocatePushTransport::DecodableType data;
        ReturnErrorOnFailure(data.Decode(input_arguments, fabricIndex));
        return mLogic.HandleDeallocatePushTransport(*handler, request.path, data);
    }
    case ModifyPushTransport::Id: {
        ModifyPushTransport::DecodableType data;
        ReturnErrorOnFailure(data.Decode(input_arguments, fabricIndex));
        return mLogic.HandleModifyPushTransport(*handler, request.path, data);
    }
    case SetTransportStatus::Id: {
        SetTransportStatus::DecodableType data;
        ReturnErrorOnFailure(data.Decode(input_arguments, fabricIndex));
        return mLogic.HandleSetTransportStatus(*handler, request.path, data);
    }
    case ManuallyTriggerTransport::Id: {
        ManuallyTriggerTransport::DecodableType data;
        ReturnErrorOnFailure(data.Decode(input_arguments, fabricIndex));
        return mLogic.HandleManuallyTriggerTransport(*handler, request.path, data);
    }
    case FindTransport::Id: {
        FindTransport::DecodableType data;
        ReturnErrorOnFailure(data.Decode(input_arguments, fabricIndex));
        return mLogic.HandleFindTransport(*handler, request.path, data);
    }
    }

    return Status::UnsupportedCommand;
}

} // namespace Clusters
} // namespace app
} // namespace chip

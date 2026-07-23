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

#include <app/clusters/av-analysis-server/AvAnalysisCluster.h>
#include <app/reporting/reporting.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <app/util/util.h>
#include <clusters/CameraAvSettingsUserLevelManagement/Commands.h>
#include <clusters/CameraAvSettingsUserLevelManagement/Ids.h>
#include <clusters/CameraAvSettingsUserLevelManagement/Metadata.h>
#include <lib/core/CHIPSafeCasts.h>
#include <lib/support/DefaultStorageKeyAllocator.h>
#include <protocols/interaction_model/StatusCode.h>

namespace chip {
namespace app {
namespace Clusters {

using Protocols::InteractionModel::Status;
using namespace AvAnalysis;

CHIP_ERROR AvAnalysisCluster::AcceptedCommands(const ConcreteClusterPath & path,
                                               ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder)
{
    return mLogic.AcceptedCommands(builder);
}

CHIP_ERROR AvAnalysisCluster::Attributes(const ConcreteClusterPath & path,
                                         ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    return mLogic.Attributes(builder);
}

CHIP_ERROR AvAnalysisCluster::Startup(ServerClusterContext & context)
{
    ReturnErrorOnFailure(DefaultServerCluster::Startup(context));

    mLogic.SetMarkDirtyCallback([this](AttributeId attributeId) { MarkAttributeDirty(attributeId); });
    return mLogic.Startup(context.attributeStorage);
}

DataModel::ActionReturnStatus AvAnalysisCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                               AttributeValueEncoder & aEncoder)
{
    switch (request.path.mAttributeId)
    {
    case Attributes::FeatureMap::Id:
        return aEncoder.Encode(mLogic.mFeatures);
    case Attributes::SupportedAmbientContexts::Id:
        return mLogic.ReadAndEncodeSupportedAmbientContexts(aEncoder);
    case Attributes::ActiveAmbientContextTriggers::Id:
        return mLogic.ReadAndEncodeActiveAmbientContextTriggers(aEncoder);
    case Attributes::MaxAnalysisStreamCount::Id:
        return aEncoder.Encode(mLogic.mMaxAnalysisStreamCount);
    case Attributes::CurrentAnalysisStreamCount::Id:
        return aEncoder.Encode(mLogic.mCurrentAnalysisStreamCount);
    case Attributes::AnalysisStreams::Id:
        return mLogic.ReadAndEncodeAnalysisStreams(aEncoder);
    case Attributes::TrackingEnabled::Id:
        return aEncoder.Encode(mLogic.mTrackingEnabled);
    }

    return Status::UnsupportedAttribute;
}

DataModel::ActionReturnStatus AvAnalysisCluster::WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                                AttributeValueDecoder & aDecoder)
{
    VerifyOrDie(request.path.mClusterId == AvAnalysis::Id);

    switch (request.path.mAttributeId)
    {
    case Attributes::TrackingEnabled::Id: {
        bool trackingEnabled;
        ReturnErrorOnFailure(aDecoder.Decode(trackingEnabled));
        return mLogic.SetTrackingEnabled(trackingEnabled);
    }

    default:
        // Unknown attribute
        return CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute);
    }
}

CHIP_ERROR AvAnalysisCluster::SetMaxAnalysisStreamCount(uint8_t aMaxAnalysisStreamCount)
{
    return mLogic.SetMaxAnalysisStreamCount(aMaxAnalysisStreamCount);
}

std::optional<DataModel::ActionReturnStatus> AvAnalysisCluster::InvokeCommand(const DataModel::InvokeRequest & request,
                                                                              chip::TLV::TLVReader & input_arguments,
                                                                              CommandHandler * handler)
{
    ChipLogProgress(Zcl, "AvAnalysis[ep=%d]: InvokeCommand", request.path.mEndpointId);

    switch (request.path.mCommandId)
    {
    case Commands::EnableContextTriggers::Id: {
        ChipLogProgress(Zcl, "AvAnalysis[ep=%d]: Enabling Context Triggers", request.path.mEndpointId);
        Commands::EnableContextTriggers::DecodableType data;
        ReturnErrorOnFailure(data.Decode(input_arguments));
        return mLogic.HandleEnableContextTriggers(*handler, request.path, data);
    }

    case Commands::DisableContextTriggers::Id: {
        ChipLogProgress(Zcl, "AvAnalysis[ep=%d]: Disabling Context Triggers", request.path.mEndpointId);
        Commands::DisableContextTriggers::DecodableType data;
        ReturnErrorOnFailure(data.Decode(input_arguments));
        return mLogic.HandleDisableContextTriggers(*handler, request.path, data);
    }

    case Commands::EstablishAnalysisStream::Id: {
        ChipLogProgress(Zcl, "AvAnalysis[ep=%d]: Establishing Analysis Stream", request.path.mEndpointId);
        Commands::EstablishAnalysisStream::DecodableType data;
        ReturnErrorOnFailure(data.Decode(input_arguments));
        return mLogic.HandleEstablishAnalysisStream(*handler, request.path, data);
    }

    case Commands::ActivateAnalysisStream::Id: {
        ChipLogProgress(Zcl, "AvAnalysis[ep=%d]: Activating Analysis Stream", request.path.mEndpointId);
        Commands::ActivateAnalysisStream::DecodableType data;
        ReturnErrorOnFailure(data.Decode(input_arguments));
        return mLogic.HandleActivateAnalysisStream(*handler, request.path, data);
    }

    case Commands::DeactivateAnalysisStream::Id: {
        ChipLogProgress(Zcl, "AvAnalysis[ep=%d]: Deactivating Analysis Stream", request.path.mEndpointId);
        Commands::DeactivateAnalysisStream::DecodableType data;
        ReturnErrorOnFailure(data.Decode(input_arguments));
        return mLogic.HandleDeactivateAnalysisStream(*handler, request.path, data);
    }

    case Commands::RemoveAnalysisStream::Id: {
        ChipLogProgress(Zcl, "AvAnalysis[ep=%d]: Removing Analysis Stream", request.path.mEndpointId);
        Commands::RemoveAnalysisStream::DecodableType data;
        ReturnErrorOnFailure(data.Decode(input_arguments));
        return mLogic.HandleRemoveAnalysisStream(*handler, request.path, data);
    }
    }

    return Status::UnsupportedCommand;
}

} // namespace Clusters
} // namespace app
} // namespace chip

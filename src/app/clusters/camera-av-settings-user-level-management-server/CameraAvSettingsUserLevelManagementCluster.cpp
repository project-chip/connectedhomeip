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

#include <app/clusters/camera-av-settings-user-level-management-server/CameraAvSettingsUserLevelManagementCluster.h>
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
using namespace CameraAvSettingsUserLevelManagement;

CHIP_ERROR
CameraAvSettingsUserLevelManagementCluster::AcceptedCommands(const ConcreteClusterPath & path,
                                                             ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder)
{
    return mLogic.AcceptedCommands(builder);
}

CHIP_ERROR CameraAvSettingsUserLevelManagementCluster::Attributes(const ConcreteClusterPath & path,
                                                                  ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    return mLogic.Attributes(builder);
}

CHIP_ERROR CameraAvSettingsUserLevelManagementCluster::Startup(ServerClusterContext & context)
{
    ReturnErrorOnFailure(DefaultServerCluster::Startup(context));

    return mLogic.Startup();
}
/**
 * Helper Read functions for complex attribute types
 */
CHIP_ERROR CameraAvSettingsUserLevelManagementCluster::ReadAndEncodeMPTZPresets(AttributeValueEncoder & aEncoder)
{
    return aEncoder.EncodeList([this](const auto & encoder) -> CHIP_ERROR {
        for (const auto & mptzPresets : mLogic.mMptzPresetHelpers)
        {
            // Get the details to encode from the preset helper
            //
            CameraAvSettingsUserLevelManagement::Structs::MPTZPresetStruct::Type presetStruct;
            std::string name      = mptzPresets.GetName();
            uint8_t preset        = mptzPresets.GetPresetID();
            presetStruct.presetID = preset;
            presetStruct.name     = CharSpan(name.c_str(), name.size());
            presetStruct.settings = mptzPresets.GetMptzPosition();
            ChipLogDetail(Zcl, "CameraAVSettingsUserLevelMgmt: Encoding an instance of MPTZPresetStruct. ID = %d. Name = %s",
                          presetStruct.presetID, NullTerminated(presetStruct.name).c_str());
            ReturnErrorOnFailure(encoder.Encode(presetStruct));
        }

        return CHIP_NO_ERROR;
    });
}

CHIP_ERROR CameraAvSettingsUserLevelManagementCluster::ReadAndEncodeDPTZStreams(AttributeValueEncoder & aEncoder)
{
    return aEncoder.EncodeList([this](const auto & encoder) -> CHIP_ERROR {
        for (const auto & dptzStream : mLogic.mDptzStreams)
        {
            ReturnErrorOnFailure(encoder.Encode(dptzStream));
        }

        return CHIP_NO_ERROR;
    });
}

DataModel::ActionReturnStatus
CameraAvSettingsUserLevelManagementCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                          AttributeValueEncoder & aEncoder)
{
    switch (request.path.mAttributeId)
    {
    case Attributes::FeatureMap::Id:
        return aEncoder.Encode(mLogic.mFeatures);
    case Attributes::MPTZPosition::Id:
        return aEncoder.Encode(mLogic.mMptzPosition);
    case Attributes::MaxPresets::Id:
        return aEncoder.Encode(mLogic.mMaxPresets);
    case Attributes::MPTZPresets::Id:
        return ReadAndEncodeMPTZPresets(aEncoder);
    case Attributes::DPTZStreams::Id:
        return ReadAndEncodeDPTZStreams(aEncoder);
    case Attributes::ZoomMax::Id:
        return aEncoder.Encode(mLogic.mZoomMax);
    case Attributes::TiltMin::Id:
        return aEncoder.Encode(mLogic.mTiltMin);
    case Attributes::TiltMax::Id:
        return aEncoder.Encode(mLogic.mTiltMax);
    case Attributes::PanMin::Id:
        return aEncoder.Encode(mLogic.mPanMin);
    case Attributes::PanMax::Id:
        return aEncoder.Encode(mLogic.mPanMax);
    case Attributes::MovementState::Id:
        return aEncoder.Encode(mLogic.mMovementState);
    }

    return Status::UnsupportedAttribute;
}

std::optional<DataModel::ActionReturnStatus>
CameraAvSettingsUserLevelManagementCluster::InvokeCommand(const DataModel::InvokeRequest & request,
                                                          chip::TLV::TLVReader & input_arguments, CommandHandler * handler)
{
    ChipLogProgress(Zcl, "CameraAVSettingsUserLevelMgmt[ep=%d]: InvokeCommand", request.path.mEndpointId);

    switch (request.path.mCommandId)
    {
    case Commands::MPTZSetPosition::Id: {
        ChipLogProgress(Zcl, "CameraAVSettingsUserLevelMgmt[ep=%d]: Setting MPTZ Position", request.path.mEndpointId);
        Commands::MPTZSetPosition::DecodableType data;
        ReturnErrorOnFailure(data.Decode(input_arguments));
        return mLogic.HandleMPTZSetPosition(*handler, request.path, data);
    }

    case Commands::MPTZRelativeMove::Id: {
        ChipLogProgress(Zcl, "CameraAVSettingsUserLevelMgmt[ep=%d]: Relative move of MPTZ Position", request.path.mEndpointId);
        Commands::MPTZRelativeMove::DecodableType data;
        ReturnErrorOnFailure(data.Decode(input_arguments));
        return mLogic.HandleMPTZRelativeMove(*handler, request.path, data);
    }

    case Commands::MPTZMoveToPreset::Id: {
        ChipLogProgress(Zcl, "CameraAVSettingsUserLevelMgmt[ep=%d]: Moving to an indicated MPTZ Preset", request.path.mEndpointId);
        Commands::MPTZMoveToPreset::DecodableType data;
        ReturnErrorOnFailure(data.Decode(input_arguments));
        return mLogic.HandleMPTZMoveToPreset(*handler, request.path, data);
    }

    case Commands::MPTZSavePreset::Id: {
        ChipLogProgress(Zcl, "CameraAVSettingsUserLevelMgmt[ep=%d]: Saving MPTZ Preset", request.path.mEndpointId);
        Commands::MPTZSavePreset::DecodableType data;
        ReturnErrorOnFailure(data.Decode(input_arguments));
        return mLogic.HandleMPTZSavePreset(*handler, request.path, data);
    }

    case Commands::MPTZRemovePreset::Id: {
        ChipLogProgress(Zcl, "CameraAVSettingsUserLevelMgmt[ep=%d]: Remove indicated MPTZ Preset", request.path.mEndpointId);
        Commands::MPTZRemovePreset::DecodableType data;
        ReturnErrorOnFailure(data.Decode(input_arguments));
        return mLogic.HandleMPTZRemovePreset(*handler, request.path, data);
    }

    case Commands::DPTZSetViewport::Id: {
        ChipLogProgress(Zcl, "CameraAVSettingsUserLevelMgmt[ep=%d]: Setting DPTZ Viewport", request.path.mEndpointId);
        Commands::DPTZSetViewport::DecodableType data;
        ReturnErrorOnFailure(data.Decode(input_arguments));
        return mLogic.HandleDPTZSetViewport(*handler, request.path, data);
    }

    case Commands::DPTZRelativeMove::Id: {
        ChipLogProgress(Zcl, "CameraAVSettingsUserLevelMgmt[ep=%d]: Relative move within a defined DPTZ Viewport",
                        request.path.mEndpointId);
        Commands::DPTZRelativeMove::DecodableType data;
        ReturnErrorOnFailure(data.Decode(input_arguments));
        return mLogic.HandleDPTZRelativeMove(*handler, request.path, data);
    }
    }

    return Status::UnsupportedCommand;
}

/**
 * Allows for a delegate or application to provide the ID and default Viewport of an allocated video stream that is capable of
 * digital movement. This should be invoked by a delegate on the conclusion of allocating a video stream via the AV Stream
 * Management cluster.
 */
void CameraAvSettingsUserLevelManagementCluster::AddMoveCapableVideoStream(uint16_t aVideoStreamID,
                                                                           Globals::Structs::ViewportStruct::Type aViewport)
{
    mLogic.AddMoveCapableVideoStream(aVideoStreamID, aViewport);
}

/**
 * Allows for a delegate or application to update the viewport of an already allocated video stream.
 * This should be invoked whenever a viewport is updated by DPTZSetVewport or DPTZRelativeMove
 */
void CameraAvSettingsUserLevelManagementCluster::UpdateMoveCapableVideoStream(uint16_t aVideoStreamID,
                                                                              Globals::Structs::ViewportStruct::Type aViewport)
{
    mLogic.UpdateMoveCapableVideoStream(aVideoStreamID, aViewport);
}

/**
 * Allows for a delegate or application to update all of the viewports for all of the allocated video streams.
 * This should be invoked whenever the device default viewport is updated via a write to Viewport on the
 * AV Stream Management Cluster
 */
void CameraAvSettingsUserLevelManagementCluster::UpdateMoveCapableVideoStreams(Globals::Structs::ViewportStruct::Type aViewport)
{
    mLogic.UpdateMoveCapableVideoStreams(aViewport);
}

/**
 * Allows for a delegate or application to remove a video stream from the set that is capable of digital movement.
 * This should be invoked by a delegate on the conclusion of deallocating a video stream via the AV Stream Management cluster.
 */
void CameraAvSettingsUserLevelManagementCluster::RemoveMoveCapableVideoStream(uint16_t aVideoStreamID)
{
    mLogic.RemoveMoveCapableVideoStream(aVideoStreamID);
}

CHIP_ERROR CameraAvSettingsUserLevelManagementCluster::SetTiltMin(int16_t aTiltMin)
{
    return mLogic.SetTiltMin(aTiltMin);
}

CHIP_ERROR CameraAvSettingsUserLevelManagementCluster::SetTiltMax(int16_t aTiltMax)
{
    return mLogic.SetTiltMax(aTiltMax);
}

CHIP_ERROR CameraAvSettingsUserLevelManagementCluster::SetPanMin(int16_t aPanMin)
{
    return mLogic.SetPanMin(aPanMin);
}

CHIP_ERROR CameraAvSettingsUserLevelManagementCluster::SetPanMax(int16_t aPanMax)
{
    return mLogic.SetPanMax(aPanMax);
}

CHIP_ERROR CameraAvSettingsUserLevelManagementCluster::SetZoomMax(uint8_t aZoomMax)
{
    return mLogic.SetZoomMax(aZoomMax);
}

} // namespace Clusters
} // namespace app
} // namespace chip

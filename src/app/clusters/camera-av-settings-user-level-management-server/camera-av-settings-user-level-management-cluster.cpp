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

#include <app/AttributeAccessInterfaceRegistry.h>
#include <app/clusters/camera-av-settings-user-level-management-server/camera-av-settings-user-level-management-cluster.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <app/reporting/reporting.h>
#include <app/util/attribute-storage.h>
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
namespace {

using namespace chip::app::Clusters::CameraAvSettingsUserLevelManagement::Commands;

constexpr DataModel::AcceptedCommandEntry kAcceptedCommands[] = {
    MPTZSetPosition::kMetadataEntry, MPTZRelativeMove::kMetadataEntry, MPTZMoveToPreset::kMetadataEntry, MPTZSavePreset::kMetadataEntry, 
    MPTZRemovePreset::kMetadataEntry, DPTZSetViewport::kMetadataEntry, DPTZRelativeMove::kMetadataEntry,  
};

constexpr CommandId kGeneratedCommands[] = {

};

} //namespace

using Protocols::InteractionModel::Status;
using namespace CameraAvSettingsUserLevelManagement::Commands;

CHIP_ERROR CameraAvSettingsUserLevelMgmtServer::Attributes(const ConcreteClusterPath & path,
                                                                 ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    AttributeListBuilder listBuilder(builder);
    return listBuilder.Append(Span(CameraAvSettingsUserLevelManagement::Attributes::kMandatoryMetadata), {}, {}); 
}

/**
 * Helper Read functions for complex attribute types
 */
CHIP_ERROR CameraAvSettingsUserLevelMgmtServer::ReadAndEncodeMPTZPresets(AttributeValueEncoder & aEncoder)
{
    return aEncoder.EncodeList([this](const auto & encoder) -> CHIP_ERROR {
        for (const auto & mptzPresets : mMptzPresetHelpers)
        {
            // Get the details to encode from the preset helper
            //
            MPTZPresetStructType presetStruct;
            std::string name      = mptzPresets.GetName();
            uint8_t preset        = mptzPresets.GetPresetID();
            presetStruct.presetID = preset;
            presetStruct.name     = CharSpan(name.c_str(), name.size());
            presetStruct.settings = mptzPresets.GetMptzPosition();
            ChipLogDetail(Zcl, "CameraAVSettingsUserLevelMgmt[ep=%d]: Encoding an instance of MPTZPresetStruct. ID = %d. Name = %s",
                          mEndpointId, presetStruct.presetID, NullTerminated(presetStruct.name).c_str());
            ReturnErrorOnFailure(encoder.Encode(presetStruct));
        }

        return CHIP_NO_ERROR;
    });
}

CHIP_ERROR CameraAvSettingsUserLevelMgmtServer::ReadAndEncodeDPTZStreams(AttributeValueEncoder & aEncoder)
{
    return aEncoder.EncodeList([this](const auto & encoder) -> CHIP_ERROR {
        for (const auto & dptzStream : mDptzStreams)
        {
            ReturnErrorOnFailure(encoder.Encode(dptzStream));
        }

        return CHIP_NO_ERROR;
    });
}


DataModel::ActionReturnStatus CameraAvSettingsUserLevelMgmtServer::ReadAttribute(const DataModel::ReadAttributeRequest & request, AttributeValueEncoder & aEncoder)
{
    VerifyOrDie(request.path.mClusterId == CameraAvSettingsUserLevelManagement::Id);
    ChipLogProgress(Zcl, "CameraAVSettingsUserLevelMgmt[ep=%d]: Reading Attribute %" PRIu32, request.path.mEndpointId);

    switch (request.path.mAttributeId)
    {
    case CameraAvSettingsUserLevelManagement::Attributes::FeatureMap::Id:
        return aEncoder.Encode(mLogic.mFeatures);
    case CameraAvSettingsUserLevelManagement::Attributes::MPTZPosition::Id:
        return aEncoder.Encode(mLogic.mMptzPosition);
    case CameraAvSettingsUserLevelManagement::Attributes::MaxPresets::Id:
        return aEncoder.Encode(mLogic.mMaxPresets);
        break;
    case CameraAvSettingsUserLevelManagement::Attributes::MPTZPresets::Id:
        return ReadAndEncodeMPTZPresets(aEncoder);
    case CameraAvSettingsUserLevelManagement::Attributes::DPTZStreams::Id:
        return ReadAndEncodeDPTZStreams(aEncoder);
    case CameraAvSettingsUserLevelManagement::Attributes::ZoomMax::Id:
        aEncoder.Encode(mLogic.mZoomMax);
        break;
    case CameraAvSettingsUserLevelManagement::Attributes::TiltMin::Id:
        return aEncoder.Encode(mLogic.mTiltMin);
        break;
    case CameraAvSettingsUserLevelManagement::Attributes::TiltMax::Id:
        return aEncoder.Encode(mLogic.mTiltMax);
        break;
    case CameraAvSettingsUserLevelManagement::Attributes::PanMin::Id:
        return aEncoder.Encode(mLogic.mPanMin);
        break;
    case CameraAvSettingsUserLevelManagement::Attributes::PanMax::Id:
        return aEncoder.Encode(mLogic.mPanMax);
        break;
    case CameraAvSettingsUserLevelManagement::Attributes::MovementState::Id:
        return aEncoder.Encode(mLogic.mMovementState);
    }

    return Status::UnsupportedAttribute;
}

std::optional<DataModel::ActionReturnStatus> CameraAvSettingsUserLevelMgmtServer::InvokeCommand(const DataModel::InvokeRequest & request,
                                                                                                chip::TLV::TLVReader & input_arguments,
                                                                                                CommandHandler * handler)
{
    using namespace CameraAvSettingsUserLevelManagement::Commands;

    ChipLogProgress(Zcl, "CameraAVSettingsUserLevelMgmt[ep=%d]: InvokeCommand", request.path.mEndpointId);

    FabricIndex fabricIndex = request.GetAccessingFabricIndex();

    switch (request.path.mCommandId)
    {
    case MPTZSetPosition::Id:
        ChipLogProgress(Zcl, "CameraAVSettingsUserLevelMgmt[ep=%d]: Setting MPTZ Position", request.path.mEndpointId);
        MPTZSetPosition::DecodableType data;
        ReturnErrorOnFailure(data.Decode(input_arguments, fabricIndex));        

        return mLogic.HandleMPTZSetPosition(handler, request.path, data);

    case MPTZRelativeMove::Id:
        ChipLogProgress(Zcl, "CameraAVSettingsUserLevelMgmt[ep=%d]: Relative move of MPTZ Position", request.path.mEndpointId);
        MPTZRelativeMove::DecodableType data;
        ReturnErrorOnFailure(data.Decode(input_arguments, fabricIndex));  

        return mLogic.HandleMPTZRelativeMove(handler, request.path, data);

    case MPTZMoveToPreset::Id:
        ChipLogProgress(Zcl, "CameraAVSettingsUserLevelMgmt[ep=%d]: Moving to an indicated MPTZ Preset", request.path.mEndpointId);
        MPTZMoveToPreset::DecodableType data;
        ReturnErrorOnFailure(data.Decode(input_arguments, fabricIndex));  

        return mLogic.HandleMPTZMoveToPreset(handler, request.path, data);

    case MPTZSavePreset::Id:
        ChipLogProgress(Zcl, "CameraAVSettingsUserLevelMgmt[ep=%d]: Saving MPTZ Preset", request.path.mEndpointId);
        MPTZSavePreset::DecodableType data;
        ReturnErrorOnFailure(data.Decode(input_arguments, fabricIndex));  

        return mLogic.HandleMPTZSavePreset(handler, request.path, data);

    case MPTZRemovePreset::Id:
        ChipLogProgress(Zcl, "CameraAVSettingsUserLevelMgmt[ep=%d]: Remove indicated MPTZ Preset", request.path.mEndpointId);
        MPTZRemovePreset::DecodableType data;
        ReturnErrorOnFailure(data.Decode(input_arguments, fabricIndex));  

        return mLogic.HandleMPTZRemovePreset(handler, request.path, data);

    case DPTZSetViewport::Id:
        ChipLogProgress(Zcl, "CameraAVSettingsUserLevelMgmt[ep=%d]: Setting DPTZ Viewport", request.path.mEndpointId);
        DPTZSetViewport::DecodableType data;
        ReturnErrorOnFailure(data.Decode(input_arguments, fabricIndex));  

        return mLogic.HandleDPTZSetViewport(handler, request.path, data);

    case DPTZRelativeMove::Id:
        ChipLogProgress(Zcl, "CameraAVSettingsUserLevelMgmt[ep=%d]: Relative move within a defined DPTZ Viewport", request.path.mEndpointId);
        DPTZRelativeMove::DecodableType data;
        ReturnErrorOnFailure(data.Decode(input_arguments, fabricIndex));  

        return mLogic.HandleDPTZRelativeMove(handler, request.path, data);
    }

    return Status::UnsupportedCommand;
}

} // namespace Clusters
} // namespace app
} // namespace chip

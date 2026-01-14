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
#include <app/CommandHandlerInterfaceRegistry.h>
#include <app/InteractionModelEngine.h>
#include <app/clusters/camera-av-settings-user-level-management-server/CameraAvSettingsUserLevelManagementCluster.h>
#include <app/persistence/AttributePersistenceProvider.h>
#include <app/persistence/AttributePersistenceProviderInstance.h>
#include <app/reporting/reporting.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <app/util/util.h>
#include <clusters/CameraAvSettingsUserLevelManagement/Metadata.h>
#include <lib/core/CHIPSafeCasts.h>
#include <lib/support/DefaultStorageKeyAllocator.h>
#include <protocols/interaction_model/StatusCode.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::CameraAvSettingsUserLevelManagement;
using namespace chip::app::Clusters::CameraAvSettingsUserLevelManagement::Structs;
using namespace chip::app::Clusters::CameraAvSettingsUserLevelManagement::Attributes;
using namespace Protocols::InteractionModel;

namespace chip {
namespace app {
namespace Clusters {

CameraAvSettingsUserLevelMgmtServerLogic::CameraAvSettingsUserLevelMgmtServerLogic(EndpointId aEndpointId,
                                                                                   BitFlags<Feature> aFeatures,
                                                                                   uint8_t aMaxPresets) :
    mEndpointId(aEndpointId),
    mFeatures(aFeatures), mMaxPresets(aMaxPresets)
{}

CameraAvSettingsUserLevelMgmtServerLogic::~CameraAvSettingsUserLevelMgmtServerLogic() {}

CHIP_ERROR CameraAvSettingsUserLevelMgmtServerLogic::Startup()
{
    ChipLogProgress(Zcl, "CameraAvSettingsUserLevelManagement: Startup");
    // Make sure mandated Features are set
    //
    VerifyOrReturnError(HasFeature(Feature::kMechanicalPan) || HasFeature(Feature::kMechanicalTilt) ||
                            HasFeature(Feature::kMechanicalZoom) || HasFeature(Feature::kDigitalPTZ),
                        CHIP_ERROR_INVALID_ARGUMENT,
                        ChipLogError(Zcl,
                                     "CameraAVSettingsUserLevelMgmt: Feature configuration error. At least one of "
                                     "Mechanical Pan, Tilt, Zoom, or Digital PTZ must be supported"));

    // Set up our defaults
    SetPan(MakeOptional(kDefaultPan));
    SetTilt(MakeOptional(kDefaultTilt));
    SetZoom(MakeOptional(kDefaultZoom));

    SetMovementState(PhysicalMovementEnum::kIdle);

    LoadPersistentAttributes();

    return CHIP_NO_ERROR;
}

void CameraAvSettingsUserLevelMgmtServerLogic::Shutdown()
{
    mDelegate->ShutdownApp();
}

bool CameraAvSettingsUserLevelMgmtServerLogic::HasFeature(Feature aFeature) const
{
    return mFeatures.Has(aFeature);
}

CHIP_ERROR
CameraAvSettingsUserLevelMgmtServerLogic::AcceptedCommands(ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder)
{
    if ((HasFeature(Feature::kMechanicalPan)) || (HasFeature(Feature::kMechanicalTilt)) || (HasFeature(Feature::kMechanicalZoom)))
    {
        ReturnErrorOnFailure(builder.AppendElements({ Commands::MPTZSetPosition::kMetadataEntry }));
        ReturnErrorOnFailure(builder.AppendElements({ Commands::MPTZRelativeMove::kMetadataEntry }));
    }

    if (HasFeature(Feature::kMechanicalPresets))
    {
        ReturnErrorOnFailure(builder.AppendElements({ Commands::MPTZMoveToPreset::kMetadataEntry }));
        ReturnErrorOnFailure(builder.AppendElements({ Commands::MPTZSavePreset::kMetadataEntry }));
        ReturnErrorOnFailure(builder.AppendElements({ Commands::MPTZRemovePreset::kMetadataEntry }));
    }

    if (HasFeature(Feature::kDigitalPTZ))
    {
        ReturnErrorOnFailure(builder.AppendElements({ Commands::DPTZSetViewport::kMetadataEntry }));
        ReturnErrorOnFailure(builder.AppendElements({ Commands::DPTZRelativeMove::kMetadataEntry }));
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR CameraAvSettingsUserLevelMgmtServerLogic::Attributes(ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    AttributeListBuilder listBuilder(builder);

    // All attributes are set dependent on the Feature Flags
    AttributeListBuilder::OptionalAttributeEntry optionalAttributes[] = {
        { HasFeature(Feature::kMechanicalPan) || HasFeature(Feature::kMechanicalTilt) || HasFeature(Feature::kMechanicalZoom),
          MPTZPosition::kMetadataEntry },
        { HasFeature(Feature::kMechanicalPresets), MaxPresets::kMetadataEntry },
        { HasFeature(Feature::kMechanicalPan), MPTZPresets::kMetadataEntry },
        { HasFeature(Feature::kDigitalPTZ), DPTZStreams::kMetadataEntry },
        { HasFeature(Feature::kMechanicalZoom), ZoomMax::kMetadataEntry },
        { HasFeature(Feature::kMechanicalTilt), TiltMin::kMetadataEntry },
        { HasFeature(Feature::kMechanicalTilt), TiltMax::kMetadataEntry },
        { HasFeature(Feature::kMechanicalPan), PanMin::kMetadataEntry },
        { HasFeature(Feature::kMechanicalPan), PanMax::kMetadataEntry },
        { HasFeature(Feature::kMechanicalPan) || HasFeature(Feature::kMechanicalTilt) || HasFeature(Feature::kMechanicalZoom),
          MovementState::kMetadataEntry },
    };

    return listBuilder.Append(Span(Attributes::kMandatoryMetadata), Span(optionalAttributes));
}

void CameraAvSettingsUserLevelMgmtServerLogic::MarkDirty(AttributeId aAttributeId)
{
    MatterReportingAttributeChangeCallback(mEndpointId, CameraAvSettingsUserLevelManagement::Id, aAttributeId);
}

CHIP_ERROR CameraAvSettingsUserLevelMgmtServerLogic::StoreMPTZPosition(
    const CameraAvSettingsUserLevelManagement::Structs::MPTZStruct::Type & mptzPosition)
{
    uint8_t buffer[kMptzPositionStructMaxSerializedSize];
    MutableByteSpan bufferSpan(buffer);
    TLV::TLVWriter writer;

    writer.Init(bufferSpan);
    ReturnErrorOnFailure(mptzPosition.Encode(writer, TLV::AnonymousTag()));

    auto path = ConcreteAttributePath(mEndpointId, CameraAvSettingsUserLevelManagement::Id, Attributes::MPTZPosition::Id);
    bufferSpan.reduce_size(writer.GetLengthWritten());
    return GetAttributePersistenceProvider()->WriteValue(path, bufferSpan);
}

CHIP_ERROR CameraAvSettingsUserLevelMgmtServerLogic::LoadMPTZPosition(
    CameraAvSettingsUserLevelManagement::Structs::MPTZStruct::Type & mptzPosition)
{
    uint8_t buffer[kMptzPositionStructMaxSerializedSize];
    MutableByteSpan bufferSpan(buffer);

    auto path = ConcreteAttributePath(mEndpointId, CameraAvSettingsUserLevelManagement::Id, Attributes::MPTZPosition::Id);
    ReturnErrorOnFailure(GetAttributePersistenceProvider()->ReadValue(path, bufferSpan));

    TLV::TLVReader reader;

    reader.Init(bufferSpan);
    ReturnErrorOnFailure(reader.Next(TLV::AnonymousTag()));
    ReturnErrorOnFailure(mptzPosition.Decode(reader));

    return CHIP_NO_ERROR;
}

/**
 * Attribute mutators. In all cases, given that these may not have been enabled depending on the Feature Flags that are set,
 * the associated Feature presence is checked. The attributes are updated, and marked dirty, only if there is a change in the
 * attribute value after constraint checking has been completed.
 */
CHIP_ERROR CameraAvSettingsUserLevelMgmtServerLogic::SetTiltMin(int16_t aTiltMin)
{
    VerifyOrReturnError(HasFeature(Feature::kMechanicalTilt), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute));
    VerifyOrReturnError((aTiltMin >= kTiltMinMinValue) && (aTiltMin <= kTiltMinMaxValue), CHIP_IM_GLOBAL_STATUS(ConstraintError));

    if (aTiltMin != mTiltMin)
    {
        mTiltMin = aTiltMin;
        MarkDirty(Attributes::TiltMin::Id);
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR CameraAvSettingsUserLevelMgmtServerLogic::SetTiltMax(int16_t aTiltMax)
{
    VerifyOrReturnError(HasFeature(Feature::kMechanicalTilt), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute));
    VerifyOrReturnError((aTiltMax <= kTiltMaxMaxValue) && (aTiltMax >= kTiltMaxMinValue), CHIP_IM_GLOBAL_STATUS(ConstraintError));

    if (aTiltMax != mTiltMax)
    {
        mTiltMax = aTiltMax;
        MarkDirty(Attributes::TiltMax::Id);
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR CameraAvSettingsUserLevelMgmtServerLogic::SetPanMin(int16_t aPanMin)
{
    VerifyOrReturnError(HasFeature(Feature::kMechanicalPan), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute));
    VerifyOrReturnError((aPanMin >= kPanMinMinValue) && (aPanMin <= kPanMinMaxValue), CHIP_IM_GLOBAL_STATUS(ConstraintError));

    if (aPanMin != mPanMin)
    {
        mPanMin = aPanMin;
        MarkDirty(Attributes::PanMin::Id);
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR CameraAvSettingsUserLevelMgmtServerLogic::SetPanMax(int16_t aPanMax)
{
    VerifyOrReturnError(HasFeature(Feature::kMechanicalPan), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute));
    VerifyOrReturnError((aPanMax <= kPanMaxMaxValue) && (aPanMax >= kPanMaxMinValue), CHIP_IM_GLOBAL_STATUS(ConstraintError));

    if (aPanMax != mPanMax)
    {
        mPanMax = aPanMax;
        MarkDirty(Attributes::PanMax::Id);
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR CameraAvSettingsUserLevelMgmtServerLogic::SetZoomMax(uint8_t aZoomMax)
{
    VerifyOrReturnError(HasFeature(Feature::kMechanicalZoom), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute));
    VerifyOrReturnError((aZoomMax <= kZoomMaxMaxValue) && (aZoomMax >= kZoomMaxMinValue), CHIP_IM_GLOBAL_STATUS(ConstraintError));

    if (aZoomMax != mZoomMax)
    {
        mZoomMax = aZoomMax;
        MarkDirty(Attributes::ZoomMax::Id);
    }

    return CHIP_NO_ERROR;
}

/**
 * Mutators for server copies of Pan, Tilt, and Zoom that may be invoked by a delegate or the server itself in responding to command
 * callbacks, or due to local on device changes.
 * Only set the value if the Feature Flag is set.
 * It is entirely possible for a mutator to be called with a parameter that has no value.  Case in point an invoke of
 * MPTZSetPosition, this will be handled and the attributes updated if at least one of the three pan, tilt, or zoom have a value,
 * with all three params passed through once validation is complete. An empty value is just ignored.
 */
void CameraAvSettingsUserLevelMgmtServerLogic::SetPan(Optional<int16_t> aPan)
{
    ChipLogProgress(Zcl, "CameraAvSettingsUserLevelManagement: SetPan");

    if (HasFeature(Feature::kMechanicalPan))
    {
        if (aPan.HasValue())
        {
            mMptzPosition.pan = aPan;
            TEMPORARY_RETURN_IGNORED StoreMPTZPosition(mMptzPosition);
            MarkDirty(Attributes::MPTZPosition::Id);
        }
    }
}

void CameraAvSettingsUserLevelMgmtServerLogic::SetTilt(Optional<int16_t> aTilt)
{
    if (HasFeature(Feature::kMechanicalTilt))
    {
        if (aTilt.HasValue())
        {
            mMptzPosition.tilt = aTilt;
            TEMPORARY_RETURN_IGNORED StoreMPTZPosition(mMptzPosition);
            MarkDirty(Attributes::MPTZPosition::Id);
        }
    }
}

void CameraAvSettingsUserLevelMgmtServerLogic::SetZoom(Optional<uint8_t> aZoom)
{
    if (HasFeature(Feature::kMechanicalZoom))
    {
        if (aZoom.HasValue())
        {
            mMptzPosition.zoom = aZoom;
            TEMPORARY_RETURN_IGNORED StoreMPTZPosition(mMptzPosition);
            MarkDirty(Attributes::MPTZPosition::Id);
        }
    }
}

void CameraAvSettingsUserLevelMgmtServerLogic::SetMovementState(PhysicalMovementEnum aMovementState)
{
    if (HasFeature(Feature::kMechanicalPan) || HasFeature(Feature::kMechanicalTilt) || HasFeature(Feature::kMechanicalZoom))
    {
        // The attribute is only ever changed to a new value, so this will always be dirty
        mMovementState = aMovementState;
        MarkDirty(Attributes::MovementState::Id);
    }
}

/**
 * Methods handling known video stream IDs, the addition and removal thereof.
 */
void CameraAvSettingsUserLevelMgmtServerLogic::AddMoveCapableVideoStream(uint16_t aVideoStreamID,
                                                                         Globals::Structs::ViewportStruct::Type aViewport)
{
    CameraAvSettingsUserLevelManagement::Structs::DPTZStruct::Type dptzEntry;
    dptzEntry.videoStreamID = aVideoStreamID;
    dptzEntry.viewport      = aViewport;
    mDptzStreams.push_back(dptzEntry);
    MarkDirty(Attributes::DPTZStreams::Id);
}

void CameraAvSettingsUserLevelMgmtServerLogic::UpdateMoveCapableVideoStream(uint16_t aVideoStreamID,
                                                                            Globals::Structs::ViewportStruct::Type aViewport)
{
    auto it = std::find_if(mDptzStreams.begin(), mDptzStreams.end(),
                           [aVideoStreamID](const CameraAvSettingsUserLevelManagement::Structs::DPTZStruct::Type & dptzs) {
                               return dptzs.videoStreamID == aVideoStreamID;
                           });

    if (it == mDptzStreams.end())
    {
        ChipLogError(Zcl, "CameraAVSettingsUserLevelMgmt[ep=%d]. No matching video stream ID, update not possible. ID=%d.",
                     mEndpointId, aVideoStreamID);
        return;
    }

    it->viewport = aViewport;
    MarkDirty(Attributes::DPTZStreams::Id);
}

void CameraAvSettingsUserLevelMgmtServerLogic::UpdateMoveCapableVideoStreams(Globals::Structs::ViewportStruct::Type aViewport)
{
    for (auto & dptzStream : mDptzStreams)
    {
        dptzStream.viewport = aViewport;
    }

    MarkDirty(Attributes::DPTZStreams::Id);
}

void CameraAvSettingsUserLevelMgmtServerLogic::RemoveMoveCapableVideoStream(uint16_t aVideoStreamID)
{
    // Verify that this is a known ID, if it is, remove from the list
    //
    auto it = std::find_if(mDptzStreams.begin(), mDptzStreams.end(),
                           [aVideoStreamID](const CameraAvSettingsUserLevelManagement::Structs::DPTZStruct::Type & dptzs) {
                               return dptzs.videoStreamID == aVideoStreamID;
                           });

    if (it == mDptzStreams.end())
    {
        ChipLogError(Zcl, "CameraAVSettingsUserLevelMgmt[ep=%d]. No matching video stream ID, removal not possible. ID=%d.",
                     mEndpointId, aVideoStreamID);
        return;
    }

    mDptzStreams.erase(it);
    MarkDirty(Attributes::DPTZStreams::Id);
}

/**
 * @returns bool  True if the provided video stream ID is known to the server. False if not.
 */
bool CameraAvSettingsUserLevelMgmtServerLogic::KnownVideoStreamID(uint16_t aVideoStreamID)
{
    auto it = std::find_if(mDptzStreams.begin(), mDptzStreams.end(),
                           [aVideoStreamID](const DPTZStruct::Type & dptzs) { return dptzs.videoStreamID == aVideoStreamID; });

    return (it == mDptzStreams.end() ? false : true);
}

/**
 * Helper function for setting the next preset ID to use in advance of reception of an MPTZSavePreset.
 * The method loops over the range of possible IDs, starting with the current ID, if the preset ID is in use,
 * it continues to the next possible value, looping back to 1. The checking is needed as the preset IDs aren't
 * solely server generated, they can also be provided by a client.
 * If there are no free presets (which will happen if all slots are taken), the value is not updated.
 */
void CameraAvSettingsUserLevelMgmtServerLogic::UpdatePresetID()
{
    uint8_t nextIDToCheck = mCurrentPresetID;
    ChipLogDetail(Zcl, "CameraAVSettingsUserLevelMgmt[ep=%d]: UpdatePresetID. Current Preset is %d.", mEndpointId,
                  mCurrentPresetID);

    do
    {
        nextIDToCheck = static_cast<uint8_t>((nextIDToCheck % mMaxPresets) + 1);

        // Have we lapped back round to where we started?  If so, break
        //
        if (nextIDToCheck == mCurrentPresetID)
        {
            break;
        }

        auto it = std::find_if(mMptzPresetHelpers.begin(), mMptzPresetHelpers.end(),
                               [=](const CameraAvSettingsUserLevelManagement::MPTZPresetHelper & mptzph) {
                                   return mptzph.GetPresetID() == nextIDToCheck;
                               });
        if (it == mMptzPresetHelpers.end())
        {
            mCurrentPresetID = nextIDToCheck;
            break;
        }
    } while (true);

    ChipLogDetail(Zcl, "CameraAVSettingsUserLevelMgmt[ep=%d]: Updated PresetID is %d.", mEndpointId, mCurrentPresetID);
}

/**
 * Helper Read functions for complex attribute types
 */
CHIP_ERROR CameraAvSettingsUserLevelMgmtServerLogic::ReadAndEncodeMPTZPresets(AttributeValueEncoder & aEncoder)
{
    return aEncoder.EncodeList([this](const auto & encoder) -> CHIP_ERROR {
        for (const auto & mptzPresets : mMptzPresetHelpers)
        {
            // Get the details to encode from the preset helper
            //
            MPTZPresetStruct::Type presetStruct;
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

CHIP_ERROR CameraAvSettingsUserLevelMgmtServerLogic::ReadAndEncodeDPTZStreams(AttributeValueEncoder & aEncoder)
{
    return aEncoder.EncodeList([this](const auto & encoder) -> CHIP_ERROR {
        for (const auto & dptzStream : mDptzStreams)
        {
            ReturnErrorOnFailure(encoder.Encode(dptzStream));
        }

        return CHIP_NO_ERROR;
    });
}

void CameraAvSettingsUserLevelMgmtServerLogic::LoadPersistentAttributes()
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    // Load MPTZPosition
    MPTZStruct::Type storedMPTZPosition;
    err = LoadMPTZPosition(storedMPTZPosition);
    if (err == CHIP_NO_ERROR)
    {
        mMptzPosition = storedMPTZPosition;
        ChipLogDetail(Zcl, "CameraAVSettingsUserLevelMgmt[ep=%d]: Loaded MPTZPosition", mEndpointId);
    }
    else
    {
        ChipLogDetail(Zcl, "CameraAVSettingsUserLevelMgmt[ep=%d]: Unable to load the MPTZPosition from the KVS.", mEndpointId);
    }

    // Load MPTZPresets
    err = mDelegate->LoadMPTZPresets(mMptzPresetHelpers);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogDetail(Zcl, "CameraAVSettingsUserLevelMgmt[ep=%d]: Unable to load the MPTZPresets from the KVS.", mEndpointId);
    }

    // Load DPTZRelativeMove
    err = mDelegate->LoadDPTZStreams(mDptzStreams);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogDetail(Zcl, "CameraAVSettingsUserLevelMgmt[ep=%d]: Unable to load the DPTZRelativeMove from the KVS.", mEndpointId);
    }

    // Signal delegate that all persistent configuration attributes have been loaded.
    TEMPORARY_RETURN_IGNORED mDelegate->PersistentAttributesLoadedCallback();
}

std::optional<DataModel::ActionReturnStatus> CameraAvSettingsUserLevelMgmtServerLogic::HandleMPTZSetPosition(
    CommandHandler & handler, const ConcreteCommandPath & commandPath,
    const CameraAvSettingsUserLevelManagement::Commands::MPTZSetPosition::DecodableType & commandData)
{
    bool hasAtLeastOneValue = false;

    Optional<int16_t> pan  = commandData.pan;
    Optional<int16_t> tilt = commandData.tilt;
    Optional<uint8_t> zoom = commandData.zoom;

    // Validate the received command fields
    //
    if (pan.HasValue())
    {
        VerifyOrReturnError(
            HasFeature(Feature::kMechanicalPan), Status::InvalidCommand,
            ChipLogError(Zcl, "CameraAVSettingsUserLevelMgmt[ep=%d]: Mechanical Pan not supported although pan value provided",
                         mEndpointId));

        int16_t panValue = pan.Value();
        VerifyOrReturnError(
            (panValue <= mPanMax) && (panValue >= mPanMin), Status::ConstraintError,
            ChipLogError(Zcl, "CameraAVSettingsUserLevelMgmt[ep=%d]: Received Pan value out of range", mEndpointId));

        hasAtLeastOneValue = true;
    }

    if (tilt.HasValue())
    {
        VerifyOrReturnError(
            HasFeature(Feature::kMechanicalTilt), Status::InvalidCommand,
            ChipLogError(Zcl, "CameraAVSettingsUserLevelMgmt[ep=%d]: Mechanical Tilt not supported although tilt value provided",
                         mEndpointId));

        int16_t tiltValue = tilt.Value();
        VerifyOrReturnError(
            (tiltValue <= mTiltMax) && (tiltValue >= mTiltMin), Status::ConstraintError,
            ChipLogError(Zcl, "CameraAVSettingsUserLevelMgmt[ep=%d]: Received Tilt value out of range", mEndpointId));

        hasAtLeastOneValue = true;
    }

    if (zoom.HasValue())
    {
        VerifyOrReturnError(
            HasFeature(Feature::kMechanicalZoom), Status::InvalidCommand,
            ChipLogError(Zcl, "CameraAVSettingsUserLevelMgmt[ep=%d]: Mechanical Zoom not supported although zoom value provided",
                         mEndpointId));

        uint8_t zoomValue = zoom.Value();

        VerifyOrReturnError(
            (zoomValue <= mZoomMax) && (zoomValue >= kZoomMinValue), Status::ConstraintError,
            ChipLogError(Zcl, "CameraAVSettingsUserLevelMgmt[ep=%d]: Received Zoom value out of range", mEndpointId));

        hasAtLeastOneValue = true;
    }

    // Was a value received in the command
    VerifyOrReturnError(hasAtLeastOneValue, Status::InvalidCommand,
                        ChipLogError(Zcl,
                                     "CameraAVSettingsUserLevelMgmt[ep=%d]: MPTZSetPosition command contains no actionable fields",
                                     mEndpointId));

    // If the camera is still in motion, then return Busy.  Subsequently check with the delegate that we're in a position to change
    // any of the PTZ values which may not be possible for other reasons
    //
    VerifyOrReturnError(
        !IsMoving(), Status::Busy,
        ChipLogError(Zcl, "CameraAVSettingsUserLevelMgmt[ep=%d]: cannot execute command as camera is busy with a physical movement",
                     mEndpointId));

    VerifyOrReturnError(
        mDelegate->CanChangeMPTZ(), Status::Busy,
        ChipLogDetail(Zcl, "CameraAVSettingsUserLevelMgmt[ep=%d]: Device not able to process MPTZ change", mEndpointId));

    Status status = mDelegate->MPTZSetPosition(pan, tilt, zoom, this);

    if (status != Status::Success)
    {
        return status;
    }

    // We don't update the server persisted attributes until the delegate confirms via callback that the physical device movement is
    // complete
    //
    mTargetPan  = pan;
    mTargetTilt = tilt;
    mTargetZoom = zoom;

    SetMovementState(PhysicalMovementEnum::kMoving);

    return status;
}

std::optional<DataModel::ActionReturnStatus> CameraAvSettingsUserLevelMgmtServerLogic::HandleMPTZRelativeMove(
    CommandHandler & handler, const ConcreteCommandPath & commandPath,
    const CameraAvSettingsUserLevelManagement::Commands::MPTZRelativeMove::DecodableType & commandData)
{
    bool hasAtLeastOneValue = false;

    Optional<int16_t> panDelta  = commandData.panDelta;
    Optional<int16_t> tiltDelta = commandData.tiltDelta;
    Optional<int8_t> zoomDelta  = commandData.zoomDelta;

    // These will all be set to actual values if there is a delta provided, and constraint checking passes.
    Optional<int16_t> newPan;
    Optional<int16_t> newTilt;
    Optional<uint8_t> newZoom;

    // Validate the received command fields
    //
    if (panDelta.HasValue())
    {
        VerifyOrReturnError(
            HasFeature(Feature::kMechanicalPan), Status::InvalidCommand,
            ChipLogError(Zcl, "CameraAVSettingsUserLevelMgmt[ep=%d]: Mechanical Pan not supported although panDelta value provided",
                         mEndpointId));

        int16_t panDeltaValue = panDelta.Value();
        VerifyOrReturnError(
            (panDeltaValue <= (mPanMax - mPanMin) && panDeltaValue >= -(mPanMax - mPanMin)), Status::ConstraintError,
            ChipLogError(Zcl, "CameraAVSettingsUserLevelMgmt[ep=%d]: PanDelta value received is out of range.", mEndpointId));

        // If we're here, then we'll also have an existing Pan value in MPTZPosition
        // Note that newPan is always between -180 and 180; and the panDeltaValue is constrained to -360 to 360 by the
        // code above, so the addition sum can never overflow int16_t.
        //
        int16_t newPanValue = static_cast<int16_t>(mMptzPosition.pan.Value() + panDeltaValue);
        if (newPanValue > mPanMax)
        {
            newPanValue = mPanMax;
        }
        if (newPanValue < mPanMin)
        {
            newPanValue = mPanMin;
        }

        newPan.Emplace(newPanValue);

        hasAtLeastOneValue = true;
    }

    if (tiltDelta.HasValue())
    {
        VerifyOrReturnError(
            HasFeature(Feature::kMechanicalTilt), Status::InvalidCommand,
            ChipLogError(Zcl,
                         "CameraAVSettingsUserLevelMgmt[ep=%d]: Mechanical Tilt not supported although tiltDelta value provided",
                         mEndpointId));

        int16_t tiltDeltaValue = tiltDelta.Value();
        VerifyOrReturnError(
            (tiltDeltaValue <= (mTiltMax - mTiltMin) && tiltDeltaValue >= -(mTiltMax - mTiltMin)), Status::ConstraintError,
            ChipLogError(Zcl, "CameraAVSettingsUserLevelMgmt[ep=%d]: TiltDelta value received is out of range.", mEndpointId));

        // If we're here, then we'll also have an existing Tilt value in MPTZPosition
        // Note that newTilt is always between -180 and 180; and the tiltDeltaValue is constrained to -360 to 360 by the
        // code above, so the addition sum can never overflow int16_t.
        //
        int16_t newTiltValue = static_cast<int16_t>(mMptzPosition.tilt.Value() + tiltDeltaValue);
        if (newTiltValue > mTiltMax)
        {
            newTiltValue = mTiltMax;
        }
        if (newTiltValue < mTiltMin)
        {
            newTiltValue = mTiltMin;
        }

        newTilt.Emplace(newTiltValue);

        hasAtLeastOneValue = true;
    }

    if (zoomDelta.HasValue())
    {
        VerifyOrReturnError(
            HasFeature(Feature::kMechanicalZoom), Status::InvalidCommand,
            ChipLogError(Zcl,
                         "CameraAVSettingsUserLevelMgmt[ep=%d]: Mechanical Zoom not supported although zoomDelta value provided",
                         mEndpointId));

        int8_t zoomDeltaValue = zoomDelta.Value();
        VerifyOrReturnError(
            (zoomDeltaValue <= (mZoomMax - 1) && zoomDeltaValue >= -(mZoomMax - 1)), Status::ConstraintError,
            ChipLogError(Zcl, "CameraAVSettingsUserLevelMgmt[ep=%d]: ZoomDelta value received is out of range.", mEndpointId));

        // If we're here, then we'll also have an existing Zoom value in MPTZPosition. The zoom values are constrained such that
        // we won't overflow newZoomValue
        //
        int newZoomValue = static_cast<int>(mMptzPosition.zoom.Value()) + zoomDeltaValue;

        if (newZoomValue > mZoomMax)
        {
            newZoomValue = static_cast<int>(mZoomMax);
        }
        if (newZoomValue < 1)
        {
            newZoomValue = 1;
        }

        ChipLogProgress(Zcl, "New Zoom Value %d", newZoomValue);

        newZoom.Emplace(static_cast<uint8_t>(newZoomValue));

        hasAtLeastOneValue = true;
    }

    // Was a value received in the command
    VerifyOrReturnError(hasAtLeastOneValue, Status::InvalidCommand,
                        ChipLogError(Zcl,
                                     "CameraAVSettingsUserLevelMgmt[ep=%d]: MPTZRelativeMove command contains no actionable fields",
                                     mEndpointId));

    // If the camera is still in motion, then return Busy.  Subsequently check with the delegate that we're in a position to change
    // any of the PTZ values which may not be possible for other reasons
    //
    VerifyOrReturnError(
        !IsMoving(), Status::Busy,
        ChipLogError(Zcl, "CameraAVSettingsUserLevelMgmt[ep=%d]: cannot execute command as camera is busy with a physical movement",
                     mEndpointId));

    VerifyOrReturnError(mDelegate->CanChangeMPTZ(), Status::Busy,
                        ChipLogDetail(Zcl,
                                      "CameraAVSettingsUserLevelMgmt[ep=%d]: Device not able to process MPTZ relative value change",
                                      mEndpointId));

    Status status = mDelegate->MPTZRelativeMove(newPan, newTilt, newZoom, this);

    if (status != Status::Success)
    {
        return status;
    }

    // We don't update the server persisted attributes until the delegate confirms via callback that the physical device movement is
    // complete
    //
    mTargetPan  = newPan;
    mTargetTilt = newTilt;
    mTargetZoom = newZoom;
    SetMovementState(PhysicalMovementEnum::kMoving);

    return status;
}

std::optional<DataModel::ActionReturnStatus> CameraAvSettingsUserLevelMgmtServerLogic::HandleMPTZMoveToPreset(
    CommandHandler & handler, const ConcreteCommandPath & commandPath,
    const CameraAvSettingsUserLevelManagement::Commands::MPTZMoveToPreset::DecodableType & commandData)
{
    uint8_t preset = commandData.presetID;

    // Verify the provided presetID is within spec limits
    //
    VerifyOrReturnError((preset <= mMaxPresets) && (preset >= 1), Status::ConstraintError,
                        ChipLogError(Zcl, "CameraAVSettingsUserLevelMgmt[ep=%d]: Preset provided is out of range. Preset: %d",
                                     mEndpointId, preset));

    // Do we have any presets?
    //
    VerifyOrReturnError(
        !mMptzPresetHelpers.empty(), Status::NotFound,
        ChipLogError(Zcl, "CameraAVSettingsUserLevelMgmt[ep=%d]: No stored presets, MoveToPreset not possible", mEndpointId));

    // We have presets, check that the received ID is a valid preset ID
    //
    auto it = std::find_if(
        mMptzPresetHelpers.begin(), mMptzPresetHelpers.end(),
        [preset](const CameraAvSettingsUserLevelManagement::MPTZPresetHelper & mptzph) { return mptzph.GetPresetID() == preset; });

    VerifyOrReturnError(
        it != mMptzPresetHelpers.end(), Status::NotFound,
        ChipLogError(Zcl,
                     "CameraAVSettingsUserLevelMgmt[ep=%d]: No matching presets, MoveToPreset not possible for provided preset: %d",
                     mEndpointId, preset));

    // If the camera is still in motion, then return Busy.  Subsequently check with the delegate that we're in a position to change
    // any of the PTZ values which may not be possible for other reasons
    //
    VerifyOrReturnError(
        !IsMoving(), Status::Busy,
        ChipLogError(Zcl, "CameraAVSettingsUserLevelMgmt[ep=%d]: cannot execute command as camera is busy with a physical movement",
                     mEndpointId));

    VerifyOrReturnError(
        mDelegate->CanChangeMPTZ(), Status::Busy,
        ChipLogDetail(Zcl, "CameraAVSettingsUserLevelMgmt[ep=%d]: Device not able to process move to MPTZ preset", mEndpointId));

    auto presetValues = it->GetMptzPosition();

    // Inform the delegate that the device is requested to move to PTZ values given by the selected preset id
    Status status = mDelegate->MPTZMoveToPreset(preset, presetValues.pan, presetValues.tilt, presetValues.zoom, this);

    if (status != Status::Success)
    {
        return status;
    }

    // We don't update the server persisted attributes until the delegate confirms via callback that the physical device movement is
    // complete
    //
    mTargetPan  = presetValues.pan;
    mTargetTilt = presetValues.tilt;
    mTargetZoom = presetValues.zoom;
    SetMovementState(PhysicalMovementEnum::kMoving);

    return status;
}

std::optional<DataModel::ActionReturnStatus> CameraAvSettingsUserLevelMgmtServerLogic::HandleMPTZSavePreset(
    CommandHandler & handler, const ConcreteCommandPath & commandPath,
    const CameraAvSettingsUserLevelManagement::Commands::MPTZSavePreset::DecodableType & commandData)
{
    Status status = Status::Success;

    Optional<uint8_t> preset  = commandData.presetID;
    chip::CharSpan presetName = commandData.name;
    uint8_t presetToUse       = mCurrentPresetID;

    // Do we have a user provided preset ID? If yes, is it in range?
    //
    if (preset.HasValue())
    {
        VerifyOrReturnError((preset.Value() <= mMaxPresets) && (preset.Value() >= 1), Status::ConstraintError,
                            ChipLogError(Zcl,
                                         "CameraAVSettingsUserLevelMgmt[ep=%d]: Provided preset ID is out of range. Preset: %d",
                                         mEndpointId, preset.Value()));
        presetToUse = preset.Value();
    }

    // Does the preset equate to an already known stored preset? If so we're updating that one rather than creating a new one
    //
    auto it = std::find_if(mMptzPresetHelpers.begin(), mMptzPresetHelpers.end(),
                           [presetToUse](const CameraAvSettingsUserLevelManagement::MPTZPresetHelper & mptzph) {
                               return mptzph.GetPresetID() == presetToUse;
                           });

    // If the current preset ID results in an entry from the current known set and there was a provided preset then we're updating
    // an existing preset.  Only check for exhausting max presets if we're NOT updating.
    // It is possible that mCurrentPresetID equates to a current preset in cases where the collection of presets is full
    //
    bool updatingExistingPreset = (it != mMptzPresetHelpers.end()) && (preset.HasValue());

    if (!updatingExistingPreset)
    {
        // Make sure that the vector will not exceed the max size
        //
        VerifyOrReturnError(
            mMptzPresetHelpers.size() != mMaxPresets, Status::ResourceExhausted,
            ChipLogError(Zcl,
                         "CameraAVSettingsUserLevelMgmt[ep=%d]: No more space for additional presets, MPTZSavePreset not possible",
                         mEndpointId));
    }

    // Call the delegate, make sure that it is ok to save a new preset, given the current
    // delegate aware values for MPTZ
    //
    status = mDelegate->MPTZSavePreset(presetToUse);

    if (status != Status::Success)
    {
        return status;
    }

    // Capture the current MPTZ values in the preset
    //
    CameraAvSettingsUserLevelManagement::MPTZPresetHelper aMptzPresetHelper;

    aMptzPresetHelper.SetPresetID(presetToUse);
    aMptzPresetHelper.SetName(presetName);
    aMptzPresetHelper.SetMptzPosition(mMptzPosition);

    // If an update, replace what is at the iterator, otherwise add to the set as tis is new
    //
    if (updatingExistingPreset)
    {
        ChipLogDetail(Zcl, "CameraAVSettingsUserLevelMgmt[ep=%d]: Updating existing MPTZ Preset.  Preset ID = %d. Preset Name = %s",
                      mEndpointId, presetToUse, aMptzPresetHelper.GetName().c_str());
        *it = aMptzPresetHelper;
    }
    else
    {
        ChipLogDetail(Zcl, "CameraAVSettingsUserLevelMgmt[ep=%d]: Saving new MPTZ Preset. Preset ID = %d. Preset Name = %s",
                      mEndpointId, presetToUse, aMptzPresetHelper.GetName().c_str());
        mMptzPresetHelpers.push_back(aMptzPresetHelper);
    }

    // Update the current preset ID to the next available only if we actually used the current value.  A user provided preset
    // could have any value between 1 and MaxPresets
    //
    if (presetToUse == mCurrentPresetID)
    {
        UpdatePresetID();
    }

    MarkDirty(Attributes::MPTZPresets::Id);

    return Status::Success;
}

std::optional<DataModel::ActionReturnStatus> CameraAvSettingsUserLevelMgmtServerLogic::HandleMPTZRemovePreset(
    CommandHandler & handler, const ConcreteCommandPath & commandPath,
    const CameraAvSettingsUserLevelManagement::Commands::MPTZRemovePreset::DecodableType & commandData)
{
    uint8_t presetToRemove = commandData.presetID;

    // Verify the provided presetID is within spec limits
    //
    VerifyOrReturnError((presetToRemove <= mMaxPresets) && (presetToRemove >= 1), Status::ConstraintError,
                        ChipLogError(Zcl, "CameraAVSettingsUserLevelMgmt[ep=%d]: Preset to remove is out of range. Preset: %d",
                                     mEndpointId, presetToRemove));

    // Is the provided ID known to us?
    //
    auto it = std::find_if(mMptzPresetHelpers.begin(), mMptzPresetHelpers.end(),
                           [presetToRemove](const CameraAvSettingsUserLevelManagement::MPTZPresetHelper & mptzph) {
                               return mptzph.GetPresetID() == presetToRemove;
                           });

    VerifyOrReturnError(
        it != mMptzPresetHelpers.end(), Status::NotFound,
        ChipLogError(Zcl,
                     "CameraAVSettingsUserLevelMgmt[ep=%d]: No matching presets, RemovePreset not possible for provided preset: %d",
                     mEndpointId, presetToRemove));

    // Call the delegate to ensure that it is ok to remove the preset indicated.
    //
    Status status = mDelegate->MPTZRemovePreset(presetToRemove);

    if (status != Status::Success)
    {
        return status;
    }

    // Remove the identified item from the known set of presets
    //
    mMptzPresetHelpers.erase(it);
    MarkDirty(Attributes::MPTZPresets::Id);

    return Status::Success;
}

std::optional<DataModel::ActionReturnStatus> CameraAvSettingsUserLevelMgmtServerLogic::HandleDPTZSetViewport(
    CommandHandler & handler, const ConcreteCommandPath & commandPath,
    const CameraAvSettingsUserLevelManagement::Commands::DPTZSetViewport::DecodableType & commandData)
{
    uint16_t videoStreamID                          = commandData.videoStreamID;
    Globals::Structs::ViewportStruct::Type viewport = commandData.viewport;

    // Is this a video stream ID of which we have already been informed?
    // If not, fail.
    //
    VerifyOrReturnError(KnownVideoStreamID(videoStreamID), Status::NotFound,
                        ChipLogError(Zcl, "CameraAVSettingsUserLevelMgmt[ep=%d]: Unknown Video Stream ID provided. ID: %d",
                                     mEndpointId, videoStreamID));

    // Call the delegate
    Status status = mDelegate->DPTZSetViewport(videoStreamID, viewport);

    if (status == Status::Success)
    {
        // Update the viewport of our stream in DPTZStreams
        //
        UpdateMoveCapableVideoStream(videoStreamID, viewport);
    }

    return status;
}

std::optional<DataModel::ActionReturnStatus> CameraAvSettingsUserLevelMgmtServerLogic::HandleDPTZRelativeMove(
    CommandHandler & handler, const ConcreteCommandPath & commandPath,
    const CameraAvSettingsUserLevelManagement::Commands::DPTZRelativeMove::DecodableType & commandData)
{
    uint16_t videoStreamID     = commandData.videoStreamID;
    Optional<int16_t> deltaX   = commandData.deltaX;
    Optional<int16_t> deltaY   = commandData.deltaY;
    Optional<int8_t> zoomDelta = commandData.zoomDelta;

    // Verify that a received Zoom Delta is within constraints
    //
    if (zoomDelta.HasValue())
    {
        int8_t zoomDeltaValue = zoomDelta.Value();
        VerifyOrReturnError(
            zoomDeltaValue >= -100 && zoomDeltaValue <= 100, Status::ConstraintError,
            ChipLogError(Zcl,
                         "CameraAVSettingsUserLevelMgmt[ep=%d]: Provided Digital Zoom Delta is out of range. Provided Zoom: %d",
                         mEndpointId, zoomDeltaValue));
    }
    // Is this a video stream ID of which we have already been informed via DPTZSetViewport. We can't relative move on a
    // viewport that hasn't already been set, hence we fail if the provided id is not found.
    //
    VerifyOrReturnError(KnownVideoStreamID(videoStreamID), Status::NotFound,
                        ChipLogError(Zcl, "CameraAVSettingsUserLevelMgmt[ep=%d]. Unknown Video Stream ID provided. ID=%d.",
                                     mEndpointId, videoStreamID));

    // Create a viewport and call the delegate; on success update our Stream Viewport with that which was set
    Globals::Structs::ViewportStruct::Type viewport;
    Status status = mDelegate->DPTZRelativeMove(videoStreamID, deltaX, deltaY, zoomDelta, viewport);

    if (status == Status::Success)
    {
        UpdateMoveCapableVideoStream(videoStreamID, viewport);
    }

    return status;
}

// Physical device movement callback
//
void CameraAvSettingsUserLevelMgmtServerLogic::OnPhysicalMovementComplete(Status status)
{
    // Make sure we're running in the Matter thread
    assertChipStackLockedByCurrentThread();

    if (status == Status::Success)
    {
        SetPan(mTargetPan);
        SetTilt(mTargetTilt);
        SetZoom(mTargetZoom);
    }
    else
    {
        ChipLogError(
            Zcl, "CameraAVSettingsUserLevelMgmt[ep=%d]. Camera failed to move to new requested values of Pan, Tilt, and/or Zoom.",
            mEndpointId);
    }

    SetMovementState(PhysicalMovementEnum::kIdle);
}

} // namespace Clusters
} // namespace app
} // namespace chip

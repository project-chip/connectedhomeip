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
#include <app/SafeAttributePersistenceProvider.h>
#include <app/clusters/camera-av-settings-user-level-management-server/camera-av-settings-user-level-management-server.h>
#include <app/reporting/reporting.h>
#include <app/util/attribute-storage.h>
#include <app/util/util.h>
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
namespace CameraAvSettingsUserLevelManagement {

CameraAvSettingsUserLevelMgmtServer::CameraAvSettingsUserLevelMgmtServer(EndpointId aEndpointId, Delegate * aDelegate,
                                                                         BitFlags<Feature> aFeatures,
                                                                         const BitFlags<OptionalAttributes> aOptionalAttrs) :
    AttributeAccessInterface(MakeOptional(aEndpointId), CameraAvSettingsUserLevelManagement::Id),
    CommandHandlerInterface(MakeOptional(aEndpointId), CameraAvSettingsUserLevelManagement::Id), mDelegate(aDelegate),
    mEndpointId(aEndpointId), mFeatures(aFeatures), mOptionalAttrs(aOptionalAttrs)
{
    mDelegate->SetServer(this);
}

CameraAvSettingsUserLevelMgmtServer::~CameraAvSettingsUserLevelMgmtServer()
{
    // Unregister command handler and attribute access interfaces
    CommandHandlerInterfaceRegistry::Instance().UnregisterCommandHandler(this);
    AttributeAccessInterfaceRegistry::Instance().Unregister(this);
}

CHIP_ERROR CameraAvSettingsUserLevelMgmtServer::Init()
{
    // Make sure mandated Features are set
    //
    VerifyOrReturnError(HasFeature(Feature::kMechanicalPan) || HasFeature(Feature::kMechanicalTilt) ||
                            HasFeature(Feature::kMechanicalZoom),
                        CHIP_ERROR_INVALID_ARGUMENT,
                        ChipLogError(Zcl,
                                     "CameraAVSettingsUserLevelManagement[ep=%d]: Feature configuration error. At least one of "
                                     "Pan, Tilt, or Zoom must be supported",
                                     mEndpointId));

    // All of the attributes are dependent on Feature Flags being set, ensure that this is the case
    //
    // However MPTZPosition is dependent on one of Pan, Tilt, or Zoom, we wouldn't be at this point if one of those weren't set, so
    // we explicitly check that it is present
    //
    if (!SupportsOptAttr(OptionalAttributes::kMptzPosition))
    {
        ChipLogError(Zcl, "CameraAVSettingsUserLevelManagement[ep=%d]: Feature configuration error. MPTZPosition must be supported",
                     mEndpointId);
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    if (SupportsOptAttr(OptionalAttributes::kMaxPresets))
    {
        VerifyOrReturnError(HasFeature(Feature::kMechanicalPresets), CHIP_ERROR_INVALID_ARGUMENT,
                            ChipLogError(Zcl,
                                         "CameraAVSettingsUserLevelManagement[ep=%d]: Feature configuration error. If MaxPresets "
                                         "is enabled, then MechanicalPresets feature is required",
                                         mEndpointId));
    }

    if (SupportsOptAttr(OptionalAttributes::kMptzPresets))
    {
        VerifyOrReturnError(HasFeature(Feature::kMechanicalPresets), CHIP_ERROR_INVALID_ARGUMENT,
                            ChipLogError(Zcl,
                                         "CameraAVSettingsUserLevelManagement[ep=%d]: Feature configuration error. If MPTZPresets "
                                         "is enabled, then MechanicalPresets feature is required",
                                         mEndpointId));
    }

    if (SupportsOptAttr(OptionalAttributes::kDptzRelativeMove))
    {
        VerifyOrReturnError(HasFeature(Feature::kDigitalPTZ), CHIP_ERROR_INVALID_ARGUMENT,
                            ChipLogError(Zcl,
                                         "CameraAVSettingsUserLevelManagement[ep=%d]: Feature configuration error. If "
                                         "DPTZRelativeMove is enabled, then DigitalPTZ feature is required",
                                         mEndpointId));
    }

    if (SupportsOptAttr(OptionalAttributes::kZoomMax))
    {
        VerifyOrReturnError(HasFeature(Feature::kMechanicalZoom), CHIP_ERROR_INVALID_ARGUMENT,
                            ChipLogError(Zcl,
                                         "CameraAVSettingsUserLevelManagement[ep=%d]: Feature configuration error. If ZoomMax is "
                                         "enabled, then MechanicalZoom feature is required",
                                         mEndpointId));
    }

    if (SupportsOptAttr(OptionalAttributes::kTiltMin))
    {
        VerifyOrReturnError(HasFeature(Feature::kMechanicalTilt), CHIP_ERROR_INVALID_ARGUMENT,
                            ChipLogError(Zcl,
                                         "CameraAVSettingsUserLevelManagement[ep=%d]: Feature configuration error. If TiltMin is "
                                         "enabled, then MechanicalTilt feature is required",
                                         mEndpointId));
    }

    if (SupportsOptAttr(OptionalAttributes::kTiltMax))
    {
        VerifyOrReturnError(HasFeature(Feature::kMechanicalTilt), CHIP_ERROR_INVALID_ARGUMENT,
                            ChipLogError(Zcl,
                                         "CameraAVSettingsUserLevelManagement[ep=%d]: Feature configuration error. If TiltMax is "
                                         "enabled, then MechanicalTilt feature is required",
                                         mEndpointId));
    }

    if (SupportsOptAttr(OptionalAttributes::kPanMin))
    {
        VerifyOrReturnError(HasFeature(Feature::kMechanicalPan), CHIP_ERROR_INVALID_ARGUMENT,
                            ChipLogError(Zcl,
                                         "CameraAVSettingsUserLevelManagement[ep=%d]: Feature configuration error. If PanMin is "
                                         "enabled, then MechanicalPan feature is required",
                                         mEndpointId));
    }

    if (SupportsOptAttr(OptionalAttributes::kPanMax))
    {
        VerifyOrReturnError(HasFeature(Feature::kMechanicalPan), CHIP_ERROR_INVALID_ARGUMENT,
                            ChipLogError(Zcl,
                                         "CameraAVSettingsUserLevelManagement[ep=%d]: Feature configuration error. If PanMax is "
                                         "enabled, then MechanicalPan feature is required",
                                         mEndpointId));
    }

    LoadPersistentAttributes();

    // Set default MPTZ
    mMptzPosition.pan  = Optional<int16_t>(defaultPan);
    mMptzPosition.tilt = Optional<int16_t>(defaultTilt);
    mMptzPosition.zoom = Optional<int8_t>(defaultZoom);

    VerifyOrReturnError(AttributeAccessInterfaceRegistry::Instance().Register(this), CHIP_ERROR_INTERNAL);
    ReturnErrorOnFailure(CommandHandlerInterfaceRegistry::Instance().RegisterCommandHandler(this));
    return CHIP_NO_ERROR;
}

bool CameraAvSettingsUserLevelMgmtServer::HasFeature(Feature aFeature) const
{
    return mFeatures.Has(aFeature);
}

bool CameraAvSettingsUserLevelMgmtServer::SupportsOptAttr(OptionalAttributes aOptionalAttr) const
{
    return mOptionalAttrs.Has(aOptionalAttr);
}

void CameraAvSettingsUserLevelMgmtServer::MarkDirty(AttributeId aAttributeId)
{
    MatterReportingAttributeChangeCallback(mEndpointId, CameraAvSettingsUserLevelManagement::Id, aAttributeId);
}

// Attribute mutators
//
CHIP_ERROR CameraAvSettingsUserLevelMgmtServer::SetMaxPresets(uint8_t aMaxPresets)
{
    mMaxPresets = aMaxPresets;
    MarkDirty(Attributes::MaxPresets::Id);
    return CHIP_NO_ERROR;
}

CHIP_ERROR CameraAvSettingsUserLevelMgmtServer::SetTiltMin(int16_t aTiltMin)
{
    if ((aTiltMin < kMinTiltValue) || (aTiltMin > kMaxTiltValue - 1) || (aTiltMin > mTiltMax))
    {
        return CHIP_IM_GLOBAL_STATUS(ConstraintError);
    }

    mTiltMin = aTiltMin;
    MarkDirty(Attributes::TiltMin::Id);
    return CHIP_NO_ERROR;
}

CHIP_ERROR CameraAvSettingsUserLevelMgmtServer::SetTiltMax(int16_t aTiltMax)
{
    if ((aTiltMax > kMaxTiltValue) || (aTiltMax < kMinTiltValue + 1) || (aTiltMax < mTiltMin))
    {
        return CHIP_IM_GLOBAL_STATUS(ConstraintError);
    }

    mTiltMax = aTiltMax;
    MarkDirty(Attributes::TiltMax::Id);
    return CHIP_NO_ERROR;
}

CHIP_ERROR CameraAvSettingsUserLevelMgmtServer::SetPanMin(int16_t aPanMin)
{
    if ((aPanMin < kMinPanValue) || (aPanMin > kMaxPanValue - 1) || (aPanMin > mPanMax))
    {
        return CHIP_IM_GLOBAL_STATUS(ConstraintError);
    }

    mPanMin = aPanMin;
    MarkDirty(Attributes::PanMin::Id);
    return CHIP_NO_ERROR;
}

CHIP_ERROR CameraAvSettingsUserLevelMgmtServer::SetPanMax(int16_t aPanMax)
{
    if ((aPanMax > kMaxPanValue) || (aPanMax < kMinPanValue + 1) || (aPanMax < mPanMin))
    {
        return CHIP_IM_GLOBAL_STATUS(ConstraintError);
    }

    mPanMax = aPanMax;
    MarkDirty(Attributes::PanMax::Id);
    return CHIP_NO_ERROR;
}

CHIP_ERROR CameraAvSettingsUserLevelMgmtServer::SetZoomMax(int8_t aZoomMax)
{
    if ((aZoomMax > kMaxZoomValue) || (aZoomMax < kMinZoomValue + 1))
    {
        return CHIP_IM_GLOBAL_STATUS(ConstraintError);
    }

    mZoomMax = aZoomMax;
    MarkDirty(Attributes::ZoomMax::Id);
    return CHIP_NO_ERROR;
}

// Mutators that may be invoked by a delegate in responding to command callbacks, or due to local on device changes
//
void CameraAvSettingsUserLevelMgmtServer::SetPan(Optional<int16_t> aPan)
{
    if (aPan.HasValue())
    {
        mMptzPosition.pan = aPan;
        MarkDirty(Attributes::MPTZPosition::Id);
    }
}

void CameraAvSettingsUserLevelMgmtServer::SetTilt(Optional<int16_t> aTilt)
{
    if (aTilt.HasValue())
    {
        mMptzPosition.tilt = aTilt;
        MarkDirty(Attributes::MPTZPosition::Id);
    }
}

void CameraAvSettingsUserLevelMgmtServer::SetZoom(Optional<int8_t> aZoom)
{
    if (aZoom.HasValue())
    {
        mMptzPosition.zoom = aZoom;
        MarkDirty(Attributes::MPTZPosition::Id);
    }
}

void CameraAvSettingsUserLevelMgmtServer::AddMoveCapableVideoStreamID(uint16_t aVideoStreamID)
{
    // Make sure this isn't a duplicate ID, if not, add to the list
    //
    if (!KnownVideoStreamID(aVideoStreamID))
    {
        // Not found, this is a good add.
        //
        mDptzRelativeMove.push_back(aVideoStreamID);
        MarkDirty(Attributes::DPTZRelativeMove::Id);

    }
}

void CameraAvSettingsUserLevelMgmtServer::RemoveMoveCapableVideoStreamID(uint16_t aVideoStreamID)
{
    // Verify that this is a known ID, if it is, remove from the list
    //
    auto it = std::find(mDptzRelativeMove.begin(), mDptzRelativeMove.end(), aVideoStreamID);

    if (it == mDptzRelativeMove.end())
    {
        ChipLogError(Zcl, "No matching video stream ID, removal not possible");
        return;
    }

    mDptzRelativeMove.erase(it);
    MarkDirty(Attributes::DPTZRelativeMove::Id);
}

bool CameraAvSettingsUserLevelMgmtServer::KnownVideoStreamID(uint16_t aVideoStreamID)
{
    // Verify that this is a known ID, if it is, remove from the list
    //
    auto it = std::find(mDptzRelativeMove.begin(), mDptzRelativeMove.end(), aVideoStreamID);

    return (it == mDptzRelativeMove.end() ? false : true);
}

// Helper function for setting the next preset ID to use
//
void CameraAvSettingsUserLevelMgmtServer::UpdatePresetID()
{

    // Has the next possible incremented ID been used by a user set Preset?
    //
    uint8_t increment = 1;
    do
    {
        auto it = std::find_if(mMptzPresetHelpers.begin(), mMptzPresetHelpers.end(), [=](const MPTZPresetHelper & mptzph) {
            return mptzph.GetPresetID() == mCurrentPresetID + increment;
        });
        if (it == mMptzPresetHelpers.end())
        {
            mCurrentPresetID = static_cast<uint8_t>(mCurrentPresetID + increment);
            break;
        }
        increment++;
    } while (increment < mMaxPresets);
}

// Helper Read functions for complex attribute types
CHIP_ERROR CameraAvSettingsUserLevelMgmtServer::ReadAndEncodeMPTZPresets(AttributeValueEncoder & aEncoder)
{
    return aEncoder.EncodeList([this](const auto & encoder) -> CHIP_ERROR {
        for (const auto & mptzPresets : mMptzPresetHelpers)
        {
            // Get the details to encode from the preset helper
            //
            MPTZPresetStructType aPresetStruct;
            std::string aName = mptzPresets.GetName();
            uint8_t aPreset   = mptzPresets.GetPresetID();
            ChipLogDetail(Zcl, "Encoding based on helper with ID = %d, name = %s", aPreset, aName.c_str());
            aPresetStruct.presetID = aPreset;
            aPresetStruct.name     = CharSpan(aName.c_str(), aName.size());
            aPresetStruct.settings = mptzPresets.GetMptzPosition();
            ChipLogDetail(Zcl, "Encoding an instance of MPTPresetStruct. ID = %d. Name = %s", aPresetStruct.presetID,
                          aPresetStruct.name.data());
            ReturnErrorOnFailure(encoder.Encode(aPresetStruct));
        }

        return CHIP_NO_ERROR;
    });
}

CHIP_ERROR CameraAvSettingsUserLevelMgmtServer::ReadAndEncodeDPTZRelativeMove(AttributeValueEncoder & aEncoder)
{
    return aEncoder.EncodeList([this](const auto & encoder) -> CHIP_ERROR {
        for (const auto & dptzRelativeMove : mDptzRelativeMove)
        {
            ReturnErrorOnFailure(encoder.Encode(dptzRelativeMove));
        }

        return CHIP_NO_ERROR;
    });
}

// AttributeAccessInterface
CHIP_ERROR CameraAvSettingsUserLevelMgmtServer::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    VerifyOrDie(aPath.mClusterId == CameraAvSettingsUserLevelManagement::Id);
    ChipLogError(Zcl, "Camera AV Settings User Level Management: Reading");

    switch (aPath.mAttributeId)
    {
    case FeatureMap::Id:
        ReturnErrorOnFailure(aEncoder.Encode(mFeatures));
        break;
    case MPTZPosition::Id:
        VerifyOrReturnError(HasFeature(Feature::kMechanicalPan) || HasFeature(Feature::kMechanicalTilt) ||
                                HasFeature(Feature::kMechanicalZoom),
                            CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
                            ChipLogError(Zcl, "CameraAVSettingsUserLevelMgmt: can not get MPTZPosition, feature is not supported"));

        ReturnErrorOnFailure(aEncoder.Encode(mMptzPosition));
        break;
    case MaxPresets::Id:
        VerifyOrReturnError(HasFeature(Feature::kMechanicalPresets), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
                            ChipLogError(Zcl, "CameraAVSettingsUserLevelMgmt: can not get MaxPresets, feature is not supported"));

        ReturnErrorOnFailure(aEncoder.Encode(mMaxPresets));
        break;
    case MPTZPresets::Id:
        VerifyOrReturnError(HasFeature(Feature::kMechanicalPresets), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
                            ChipLogError(Zcl, "CameraAVSettingsUserLevelMgmt: can not get MPTZPresets, feature is not supported"));

        return ReadAndEncodeMPTZPresets(aEncoder);
    case DPTZRelativeMove::Id:
        VerifyOrReturnError(
            HasFeature(Feature::kDigitalPTZ), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
            ChipLogError(Zcl, "CameraAVSettingsUserLevelMgmt: can not get DPTZRelativeMove, feature is not supported"));
        return ReadAndEncodeDPTZRelativeMove(aEncoder);
    case ZoomMax::Id:
        VerifyOrReturnError(HasFeature(Feature::kMechanicalZoom), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
                            ChipLogError(Zcl, "CameraAVSettingsUserLevelMgmt: can not get ZoomMax, feature is not supported"));
        ReturnErrorOnFailure(aEncoder.Encode(mZoomMax));
        break;
    case TiltMin::Id:
        VerifyOrReturnError(HasFeature(Feature::kMechanicalTilt), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
                            ChipLogError(Zcl, "CameraAVSettingsUserLevelMgmt: can not get TiltMin, feature is not supported"));
        ReturnErrorOnFailure(aEncoder.Encode(mTiltMin));
        break;
    case TiltMax::Id:
        VerifyOrReturnError(HasFeature(Feature::kMechanicalTilt), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
                            ChipLogError(Zcl, "CameraAVSettingsUserLevelMgmt: can not get TiltMax, feature is not supported"));
        ReturnErrorOnFailure(aEncoder.Encode(mTiltMax));
        break;
    case PanMin::Id:
        VerifyOrReturnError(HasFeature(Feature::kMechanicalPan), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
                            ChipLogError(Zcl, "CameraAVSettingsUserLevelMgmt: can not get PanMin, feature is not supported"));
        ReturnErrorOnFailure(aEncoder.Encode(mPanMin));
        break;
    case PanMax::Id:
        VerifyOrReturnError(HasFeature(Feature::kMechanicalPan), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
                            ChipLogError(Zcl, "CameraAVSettingsUserLevelMgmt: can not get PanMax, feature is not supported"));
        ReturnErrorOnFailure(aEncoder.Encode(mPanMax));
        break;
    }

    return CHIP_NO_ERROR;
}

void CameraAvSettingsUserLevelMgmtServer::LoadPersistentAttributes()
{
    // Currently there are no non-volatile attributes defined in the spec.  This however is under discussion and likely to change.
    // Hence this is here as a placeholder.
    // Signal delegate that all persistent configuration attributes have been loaded.
    mDelegate->PersistentAttributesLoadedCallback();
}

// CommandHandlerInterface
void CameraAvSettingsUserLevelMgmtServer::InvokeCommand(HandlerContext & handlerContext)
{
    ChipLogDetail(Zcl, "CameraAVSettingsUserLevelMgmt: InvokeCommand");
    switch (handlerContext.mRequestPath.mCommandId)
    {
    case Commands::MPTZSetPosition::Id:
        ChipLogDetail(Zcl, "CameraAVSettingsUserLevelMgmt: Setting MPTZ Position");

        if (!HasFeature(Feature::kMechanicalPan) && !HasFeature(Feature::kMechanicalTilt) && !HasFeature(Feature::kMechanicalZoom))
        {
            handlerContext.mCommandHandler.AddStatus(handlerContext.mRequestPath, Status::UnsupportedCommand);
        }
        else
        {
            HandleCommand<Commands::MPTZSetPosition::DecodableType>(
                handlerContext,
                [this](HandlerContext & ctx, const auto & commandData) { HandleMPTZSetPosition(ctx, commandData); });
        }
        return;

    case Commands::MPTZRelativeMove::Id:
        ChipLogDetail(Zcl, "CameraAVSettingsUserLevelMgmt: Relative move of MPTZ Position");

        if (!HasFeature(Feature::kMechanicalPan) && !HasFeature(Feature::kMechanicalTilt) && !HasFeature(Feature::kMechanicalZoom))
        {
            handlerContext.mCommandHandler.AddStatus(handlerContext.mRequestPath, Status::UnsupportedCommand);
        }
        else
        {
            HandleCommand<Commands::MPTZRelativeMove::DecodableType>(
                handlerContext,
                [this](HandlerContext & ctx, const auto & commandData) { HandleMPTZRelativeMove(ctx, commandData); });
        }
        return;

    case Commands::MPTZMoveToPreset::Id:
        ChipLogDetail(Zcl, "CameraAVSettingsUserLevelMgmt: Moving to an indicated MPTZ Preset");

        if (!HasFeature(Feature::kMechanicalPresets))
        {
            handlerContext.mCommandHandler.AddStatus(handlerContext.mRequestPath, Status::UnsupportedCommand);
        }
        else
        {
            HandleCommand<Commands::MPTZMoveToPreset::DecodableType>(
                handlerContext,
                [this](HandlerContext & ctx, const auto & commandData) { HandleMPTZMoveToPreset(ctx, commandData); });
        }
        return;

    case Commands::MPTZSavePreset::Id:
        ChipLogDetail(Zcl, "CameraAVSettingsUserLevelMgmt: Save new MPTZ Preset");

        if (!HasFeature(Feature::kMechanicalPresets))
        {
            handlerContext.mCommandHandler.AddStatus(handlerContext.mRequestPath, Status::UnsupportedCommand);
        }
        else
        {
            HandleCommand<Commands::MPTZSavePreset::DecodableType>(
                handlerContext, [this](HandlerContext & ctx, const auto & commandData) { HandleMPTZSavePreset(ctx, commandData); });
        }
        return;

    case Commands::MPTZRemovePreset::Id:
        ChipLogDetail(Zcl, "CameraAVSettingsUserLevelMgmt: Remove indicated MPTZ Preset");

        if (!HasFeature(Feature::kMechanicalPresets))
        {
            handlerContext.mCommandHandler.AddStatus(handlerContext.mRequestPath, Status::UnsupportedCommand);
        }
        else
        {
            HandleCommand<Commands::MPTZRemovePreset::DecodableType>(
                handlerContext,
                [this](HandlerContext & ctx, const auto & commandData) { HandleMPTZRemovePreset(ctx, commandData); });
        }
        return;

    case Commands::DPTZSetViewport::Id:
        ChipLogDetail(Zcl, "CameraAVSettingsUserLevelMgmt: Setting DPTZ Viewport");

        if (!HasFeature(Feature::kDigitalPTZ))
        {
            handlerContext.mCommandHandler.AddStatus(handlerContext.mRequestPath, Status::UnsupportedCommand);
        }
        else
        {
            HandleCommand<Commands::DPTZSetViewport::DecodableType>(
                handlerContext,
                [this](HandlerContext & ctx, const auto & commandData) { HandleDPTZSetViewport(ctx, commandData); });
        }
        return;

    case Commands::DPTZRelativeMove::Id:
        ChipLogDetail(Zcl, "CameraAVSettingsUserLevelMgmt: Relative move within a defined DPTZ Viewport");

        if (!HasFeature(Feature::kDigitalPTZ))
        {
            handlerContext.mCommandHandler.AddStatus(handlerContext.mRequestPath, Status::UnsupportedCommand);
        }
        else
        {
            HandleCommand<Commands::DPTZRelativeMove::DecodableType>(
                handlerContext,
                [this](HandlerContext & ctx, const auto & commandData) { HandleDPTZRelativeMove(ctx, commandData); });
        }
        return;
    }
}

void CameraAvSettingsUserLevelMgmtServer::HandleMPTZSetPosition(HandlerContext & ctx,
                                                                const Commands::MPTZSetPosition::DecodableType & commandData)
{
    Status status           = Status::Success;
    bool hasAtLeastOneValue = false;

    Optional<int16_t> pan  = commandData.pan;
    Optional<int16_t> tilt = commandData.tilt;
    Optional<int8_t> zoom  = commandData.zoom;

    // Validate the received command fields
    //
    if (pan.HasValue())
    {
        if (!HasFeature(Feature::kMechanicalPan))
        {
            ChipLogError(Zcl, "Mechanical Pan not supported although pan value provided");
            ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::Failure);
            return;
        }
        int16_t panValue = pan.Value();
        if ((panValue > mPanMax) || (panValue < mPanMin))
        {
            ChipLogError(Zcl, "Received Pan value out of range");
            ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::ConstraintError);
            return;
        }
        hasAtLeastOneValue = true;
    }

    if (tilt.HasValue())
    {
        if (!HasFeature(Feature::kMechanicalTilt))
        {
            ChipLogError(Zcl, "Mechanical Tilt not supported although tilt value provided");
            ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::Failure);
            return;
        }
        int16_t tiltValue = tilt.Value();
        if ((tiltValue > mTiltMax) || (tiltValue < mTiltMin))
        {
            ChipLogError(Zcl, "Received Tilt value out of range");
            ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::ConstraintError);
            return;
        }
        hasAtLeastOneValue = true;
    }

    if (zoom.HasValue())
    {
        if (!HasFeature(Feature::kMechanicalZoom))
        {
            ChipLogError(Zcl, "Mechanical Zoom not supported although zoom value provided");
            ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::Failure);
            return;
        }
        int8_t zoomValue = zoom.Value();
        if ((zoomValue > mZoomMax) || (zoomValue < kMinZoomValue))
        {
            ChipLogError(Zcl, "Received Zoom value out of range");
            ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::ConstraintError);
            return;
        }
        hasAtLeastOneValue = true;
    }

    // Was a value received in the command
    if (!hasAtLeastOneValue)
    {
        ChipLogError(Zcl, "MPTZSetPosition contains no actionable fields");
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::InvalidCommand);
        return;
    }

    // Check with the delegate that we're in a position to change any of the PTZ values
    //
    if (!mDelegate->CanChangeMPTZ())
    {
        ChipLogDetail(Zcl, "Device not able to process MPTZ change");
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::Busy);
        return;
    }

    // Call the delegate to set the new values
    //
    status = mDelegate->MPTZSetPosition(pan, tilt, zoom);

    if (status != Status::Success)
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
        return;
    }

    // Set the local values of pan, tilt, and zoom
    SetPan(pan);
    SetTilt(tilt);
    SetZoom(zoom);

    MarkDirty(Attributes::MPTZPosition::Id);
    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
}

void CameraAvSettingsUserLevelMgmtServer::HandleMPTZRelativeMove(HandlerContext & ctx,
                                                                 const Commands::MPTZRelativeMove::DecodableType & commandData)
{

    Status status           = Status::Success;
    bool hasAtLeastOneValue = false;

    Optional<int16_t> panDelta  = commandData.panDelta;
    Optional<int16_t> tiltDelta = commandData.tiltDelta;
    Optional<int8_t> zoomDelta  = commandData.zoomDelta;
    int16_t newPan              = mMptzPosition.pan.Value();
    int16_t newTilt             = mMptzPosition.tilt.Value();
    int8_t newZoom              = mMptzPosition.zoom.Value();

    // Validate the received command fields
    //
    if (panDelta.HasValue())
    {
        if (!HasFeature(Feature::kMechanicalPan))
        {
            ChipLogError(Zcl, "Mechanical Pan not supported although panDelta value provided");
            ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::Failure);
            return;
        }
        int16_t panDeltaValue = panDelta.Value();
        newPan                = static_cast<int16_t>(newPan + panDeltaValue);
        if (newPan > mPanMax)
        {
            newPan = mPanMax;
        }
        if (newPan < mPanMin)
        {
            newPan = mPanMin;
        }

        hasAtLeastOneValue = true;
    }

    if (tiltDelta.HasValue())
    {
        if (!HasFeature(Feature::kMechanicalTilt))
        {
            ChipLogError(Zcl, "Mechanical Tilt not supported although tiltDelta value provided");
            ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::Failure);
            return;
        }
        int16_t tiltDeltaValue = tiltDelta.Value();
        newTilt                = static_cast<int16_t>(newTilt + tiltDeltaValue);
        if (newTilt > mTiltMax)
        {
            newTilt = mTiltMax;
        }
        if (newTilt < mTiltMin)
        {
            newTilt = mTiltMin;
        }

        hasAtLeastOneValue = true;
    }

    if (zoomDelta.HasValue())
    {
        if (!HasFeature(Feature::kMechanicalZoom))
        {
            ChipLogError(Zcl, "Mechanical Zoom not supported although zoomDelta value provided");
            ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::Failure);
            return;
        }
        int8_t zoomDeltaValue = zoomDelta.Value();
        newZoom               = static_cast<int8_t>(newZoom + zoomDeltaValue);
        if (newZoom > mZoomMax)
        {
            newZoom = mZoomMax;
        }
        if (newZoom < 1)
        {
            newZoom = 1;
        }

        hasAtLeastOneValue = true;
    }

    // Was a value received in the command
    if (!hasAtLeastOneValue)
    {
        ChipLogError(Zcl, "MPTZRelativeMove contains no actionable fields");
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::InvalidCommand);
        return;
    }

    // Check with the delegate that we're in a position to change any of the PTZ values
    //
    if (!mDelegate->CanChangeMPTZ())
    {
        ChipLogDetail(Zcl, "Device not able to process MPTZ relative value change");
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::Busy);
        return;
    }

    // Call the delegate to simply set the newly calculated MPTZ values based on the deltas received
    //
    status = mDelegate->MPTZRelativeMove(Optional(static_cast<int16_t>(newPan)), Optional(static_cast<int16_t>(newTilt)),
                                         Optional(static_cast<int8_t>(newZoom)));

    if (status != Status::Success)
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
        return;
    }

    // Set the local values of pan, tilt, and zoom
    SetPan(Optional<int16_t>(newPan));
    SetTilt(Optional<int16_t>(newTilt));
    SetZoom(Optional<int8_t>(newZoom));

    MarkDirty(Attributes::MPTZPosition::Id);

    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
}

void CameraAvSettingsUserLevelMgmtServer::HandleMPTZMoveToPreset(HandlerContext & ctx,
                                                                 const Commands::MPTZMoveToPreset::DecodableType & commandData)
{
    Status status  = Status::Success;
    uint8_t preset = commandData.presetID;

    // This is effectively a manipulation of the current PTZ settings, ensure that the device is in a state wherein a PTZ change is
    // possible
    //
    if (!mDelegate->CanChangeMPTZ())
    {
        ChipLogDetail(Zcl, "Device not able to process move to MPTZ preset");
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::Busy);
        return;
    }

    // Do we have any presets?
    //
    if (mMptzPresetHelpers.empty())
    {
        ChipLogError(Zcl, "No stored presets, MoveToPreset not possible");
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::ConstraintError);
        return;
    }

    // We have presets, check that the received ID is a valid preset ID
    //
    auto it = std::find_if(mMptzPresetHelpers.begin(), mMptzPresetHelpers.end(),
                           [preset](const MPTZPresetHelper & mptzph) { return mptzph.GetPresetID() == preset; });

    if (it == mMptzPresetHelpers.end())
    {
        ChipLogError(Zcl, "No matching presets, MoveToPreset not possible");
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::ConstraintError);
        return;
    }

    auto presetValues = it->GetMptzPosition();

    // Inform the delegate that the device is requested to move to PTZ values given by the selected preset id
    // Call the delegate to allow the device to handle the physical changes, on success set the MPTZ values based on the preset
    status = mDelegate->MPTZMoveToPreset(preset, presetValues.pan, presetValues.tilt, presetValues.zoom);

    if (status != Status::Success)
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
        return;
    }

    mMptzPosition = presetValues;
    MarkDirty(Attributes::MPTZPosition::Id);

    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Protocols::InteractionModel::Status::Success);
}

void CameraAvSettingsUserLevelMgmtServer::HandleMPTZSavePreset(HandlerContext & ctx,
                                                               const Commands::MPTZSavePreset::DecodableType & commandData)
{
    Status status = Status::Success;

    Optional<uint8_t> preset  = commandData.presetID;
    chip::CharSpan presetName = commandData.name;
    uint8_t presetToUse       = mCurrentPresetID;

    // Do we have a user provided preset ID? If yes, is it in range?
    //
    if (preset.HasValue())
    {
        if (preset.Value() > mMaxPresets - 1)
        {
            ChipLogError(Zcl, "Provided preset ID is out of range");
            ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::ConstraintError);
            return;
        }
        presetToUse = preset.Value();
    }

    // Does the preset equate to an already known stored preset? If so we're updating that one rather than creating a new one
    //
    auto it = std::find_if(mMptzPresetHelpers.begin(), mMptzPresetHelpers.end(),
                           [presetToUse](const MPTZPresetHelper & mptzph) { return mptzph.GetPresetID() == presetToUse; });

    bool newPresetValue = (it == mMptzPresetHelpers.end());

    if (newPresetValue)
    {
        // Make sure that the vector will not exceed the max size
        //
        if (mMptzPresetHelpers.size() == mMaxPresets)
        {
            ChipLogError(Zcl, "No more space for additional presets, MPTZSavePreset not possible");
            ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::ResourceExhausted);
            return;
        }
    }

    // Call the delegate, make sure that it is ok to save a new preset, given the current
    // delegate aware values for MPTZ
    //
    status = mDelegate->MPTZSavePreset(presetToUse);

    if (status != Status::Success)
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
        return;
    }

    // Capture the current MPTZ values in the preset
    //
    MPTZPresetHelper aMptzPresetHelper;

    aMptzPresetHelper.SetPresetID(presetToUse);
    aMptzPresetHelper.SetName(presetName);

    ChipLogDetail(Zcl, "Saving new MPTZ Preset.  Preset ID = %d. Preset Name = %s", presetToUse,
                  aMptzPresetHelper.GetName().c_str());

    aMptzPresetHelper.SetMptzPosition(mMptzPosition);

    // Add to the set only if new, otherwise replace what is at the iterator
    //
    if (newPresetValue) 
    {
        mMptzPresetHelpers.push_back(aMptzPresetHelper);
    }
    else
    {
        *it = aMptzPresetHelper;
    }
   
    // Update the current preset ID to the next available
    UpdatePresetID();
    MarkDirty(Attributes::MPTZPresets::Id);

    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Protocols::InteractionModel::Status::Success);
}

void CameraAvSettingsUserLevelMgmtServer::HandleMPTZRemovePreset(HandlerContext & ctx,
                                                                 const Commands::MPTZRemovePreset::DecodableType & commandData)
{
    uint8_t presetToRemove = commandData.presetID;

    // Verify the provided presetID is within spec limits
    //
    if (presetToRemove > mMaxPresets - 1)
    {
        ChipLogError(Zcl, "Preset to remove is out of range");
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::ConstraintError);
        return;
    }

    // Is the provided ID known to us?
    //
    auto it = std::find_if(mMptzPresetHelpers.begin(), mMptzPresetHelpers.end(),
                           [presetToRemove](const MPTZPresetHelper & mptzph) { return mptzph.GetPresetID() == presetToRemove; });

    if (it == mMptzPresetHelpers.end())
    {
        ChipLogError(Zcl, "No matching presets, RemovePreset not possible");
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::NotFound);
        return;
    }

    // Call the delegate to ensure that it is ok to remove the preset indicated.
    //
    Status status = mDelegate->MPTZRemovePreset(presetToRemove);

    if (status != Status::Success)
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
        return;
    }

    // Remove the identified item from the known set of presets
    //
    mMptzPresetHelpers.erase(it);
    MarkDirty(Attributes::MPTZPresets::Id);

    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Protocols::InteractionModel::Status::Success);
}

void CameraAvSettingsUserLevelMgmtServer::HandleDPTZSetViewport(HandlerContext & ctx,
                                                                const Commands::DPTZSetViewport::DecodableType & commandData)
{
    uint16_t videoStreamID                 = commandData.videoStreamID;
    Structs::ViewportStruct::Type viewport = commandData.viewport;

    // Is this a video stream ID of which we have already been informed?
    // If not, ask the delegate if it's ok.  If yes, add to our set and proceed, if not, fail.
    //
    if (!KnownVideoStreamID(videoStreamID))
    {
        // Call the delegate to validate that the videoStreamID is known; if yes then add to our list and proceed
        //
        if (!mDelegate->IsValidVideoStreamID(videoStreamID))
        {
            ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::InvalidAction);
            return;
        }
        AddMoveCapableVideoStreamID(videoStreamID);
        MarkDirty(Attributes::DPTZRelativeMove::Id);
    }

    // Call the delegate
    Status status = mDelegate->DPTZSetViewport(videoStreamID, viewport);

    if (status != Status::Success)
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
        return;
    }

    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Protocols::InteractionModel::Status::Success);
}

void CameraAvSettingsUserLevelMgmtServer::HandleDPTZRelativeMove(HandlerContext & ctx,
                                                                 const Commands::DPTZRelativeMove::DecodableType & commandData)
{
    uint16_t videoStreamID     = commandData.videoStreamID;
    Optional<int16_t> deltaX   = commandData.deltaX;
    Optional<int16_t> deltaY   = commandData.deltaY;
    Optional<int8_t> zoomDelta = commandData.zoomDelta;

    // Is this a video stream ID of which we have already been informed?
    // If not, ask the delegate if it's ok.  If yes, add to our set and proceed, if not, fail.
    //
    if (!KnownVideoStreamID(videoStreamID))
    {
        // Call the delegate to validate that the videoStreamID is known; if yes then add to our list and proceed
        //
        if (!mDelegate->IsValidVideoStreamID(videoStreamID))
        {
            ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::InvalidAction);
            return;
        }
        AddMoveCapableVideoStreamID(videoStreamID);
        MarkDirty(Attributes::DPTZRelativeMove::Id);
    }

    // Call the delegate
    Status status = mDelegate->DPTZRelativeMove(videoStreamID, deltaX, deltaY, zoomDelta);

    if (status != Status::Success)
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
        return;
    }

    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Protocols::InteractionModel::Status::Success);
}

} // namespace CameraAvSettingsUserLevelManagement
} // namespace Clusters
} // namespace app
} // namespace chip

/** @brief Camera AV Settings User Level Management Cluster Server Init
 *
 * Server Init
 *
 */
void MatterCameraAvSettingsUserLevelManagementPluginServerInitCallback() {}

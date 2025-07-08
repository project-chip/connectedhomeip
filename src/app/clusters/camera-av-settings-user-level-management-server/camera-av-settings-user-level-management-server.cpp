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

CameraAvSettingsUserLevelMgmtServer::CameraAvSettingsUserLevelMgmtServer(EndpointId aEndpointId, Delegate & aDelegate,
                                                                         BitFlags<Feature> aFeatures,
                                                                         BitFlags<OptionalAttributes> aOptionalAttrs,
                                                                         uint8_t aMaxPresets) :
    AttributeAccessInterface(MakeOptional(aEndpointId), CameraAvSettingsUserLevelManagement::Id),
    CommandHandlerInterface(MakeOptional(aEndpointId), CameraAvSettingsUserLevelManagement::Id), mDelegate(aDelegate),
    mEndpointId(aEndpointId), mFeatures(aFeatures), mOptionalAttrs(aOptionalAttrs), mMaxPresets(aMaxPresets)
{
    mDelegate.SetServer(this);
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
                            HasFeature(Feature::kMechanicalZoom) || HasFeature(Feature::kDigitalPTZ),
                        CHIP_ERROR_INVALID_ARGUMENT,
                        ChipLogError(Zcl,
                                     "CameraAVSettingsUserLevelMgmt[ep=%d]: Feature configuration error. At least one of "
                                     "Mechanical Pan, Tilt, Zoom or Digital PTZ must be supported",
                                     mEndpointId));

    // All of the attributes are dependent on Feature Flags being set, ensure that this is the case
    //
    if (SupportsOptAttr(OptionalAttributes::kMptzPosition))
    {
        VerifyOrReturnError(
            HasFeature(Feature::kMechanicalPan) || HasFeature(Feature::kMechanicalTilt) || HasFeature(Feature::kMechanicalZoom),
            CHIP_ERROR_INVALID_ARGUMENT,
            ChipLogError(Zcl,
                         "CameraAVSettingsUserLevelMgmt[ep=%d]: Feature configuration error. If MPTZPosition is enabled "
                         "then one of Pan, Tilt, or Zoom is required",
                         mEndpointId));
    }

    if (SupportsOptAttr(OptionalAttributes::kMaxPresets))
    {
        VerifyOrReturnError(HasFeature(Feature::kMechanicalPresets), CHIP_ERROR_INVALID_ARGUMENT,
                            ChipLogError(Zcl,
                                         "CameraAVSettingsUserLevelMgmt[ep=%d]: Feature configuration error. If MaxPresets "
                                         "is enabled, then MechanicalPresets feature is required",
                                         mEndpointId));
    }

    if (SupportsOptAttr(OptionalAttributes::kMptzPresets))
    {
        VerifyOrReturnError(HasFeature(Feature::kMechanicalPresets), CHIP_ERROR_INVALID_ARGUMENT,
                            ChipLogError(Zcl,
                                         "CameraAVSettingsUserLevelMgmt[ep=%d]: Feature configuration error. If MPTZPresets "
                                         "is enabled, then MechanicalPresets feature is required",
                                         mEndpointId));
    }

    if (SupportsOptAttr(OptionalAttributes::kDptzStreams))
    {
        VerifyOrReturnError(HasFeature(Feature::kDigitalPTZ), CHIP_ERROR_INVALID_ARGUMENT,
                            ChipLogError(Zcl,
                                         "CameraAVSettingsUserLevelMgmt[ep=%d]: Feature configuration error. If "
                                         "DPTZStreams is enabled, then DigitalPTZ feature is required",
                                         mEndpointId));
    }

    if (SupportsOptAttr(OptionalAttributes::kZoomMax))
    {
        VerifyOrReturnError(HasFeature(Feature::kMechanicalZoom), CHIP_ERROR_INVALID_ARGUMENT,
                            ChipLogError(Zcl,
                                         "CameraAVSettingsUserLevelMgmt[ep=%d]: Feature configuration error. If ZoomMax is "
                                         "enabled, then MechanicalZoom feature is required",
                                         mEndpointId));
    }

    if (SupportsOptAttr(OptionalAttributes::kTiltMin))
    {
        VerifyOrReturnError(HasFeature(Feature::kMechanicalTilt), CHIP_ERROR_INVALID_ARGUMENT,
                            ChipLogError(Zcl,
                                         "CameraAVSettingsUserLevelMgmt[ep=%d]: Feature configuration error. If TiltMin is "
                                         "enabled, then MechanicalTilt feature is required",
                                         mEndpointId));
    }

    if (SupportsOptAttr(OptionalAttributes::kTiltMax))
    {
        VerifyOrReturnError(HasFeature(Feature::kMechanicalTilt), CHIP_ERROR_INVALID_ARGUMENT,
                            ChipLogError(Zcl,
                                         "CameraAVSettingsUserLevelMgmt[ep=%d]: Feature configuration error. If TiltMax is "
                                         "enabled, then MechanicalTilt feature is required",
                                         mEndpointId));
    }

    if (SupportsOptAttr(OptionalAttributes::kPanMin))
    {
        VerifyOrReturnError(HasFeature(Feature::kMechanicalPan), CHIP_ERROR_INVALID_ARGUMENT,
                            ChipLogError(Zcl,
                                         "CameraAVSettingsUserLevelMgmt[ep=%d]: Feature configuration error. If PanMin is "
                                         "enabled, then MechanicalPan feature is required",
                                         mEndpointId));
    }

    if (SupportsOptAttr(OptionalAttributes::kPanMax))
    {
        VerifyOrReturnError(HasFeature(Feature::kMechanicalPan), CHIP_ERROR_INVALID_ARGUMENT,
                            ChipLogError(Zcl,
                                         "CameraAVSettingsUserLevelMgmt[ep=%d]: Feature configuration error. If PanMax is "
                                         "enabled, then MechanicalPan feature is required",
                                         mEndpointId));
    }

    // Set up our defaults
    SetPan(MakeOptional(kDefaultPan));
    SetTilt(MakeOptional(kDefaultTilt));
    SetZoom(MakeOptional(kDefaultZoom));

    LoadPersistentAttributes();

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

CHIP_ERROR CameraAvSettingsUserLevelMgmtServer::StoreMPTZPosition(const MPTZStructType & mptzPosition)
{
    uint8_t buffer[kMptzPositionStructMaxSerializedSize];
    MutableByteSpan bufferSpan(buffer);
    TLV::TLVWriter writer;

    writer.Init(bufferSpan);
    ReturnErrorOnFailure(mptzPosition.Encode(writer, TLV::AnonymousTag()));

    auto path = ConcreteAttributePath(mEndpointId, CameraAvSettingsUserLevelManagement::Id, Attributes::MPTZPosition::Id);
    bufferSpan.reduce_size(writer.GetLengthWritten());

    return GetSafeAttributePersistenceProvider()->SafeWriteValue(path, bufferSpan);
}

CHIP_ERROR CameraAvSettingsUserLevelMgmtServer::LoadMPTZPosition(MPTZStructType & mptzPosition)
{
    uint8_t buffer[kMptzPositionStructMaxSerializedSize];
    MutableByteSpan bufferSpan(buffer);

    auto path = ConcreteAttributePath(mEndpointId, CameraAvSettingsUserLevelManagement::Id, Attributes::MPTZPosition::Id);
    ReturnErrorOnFailure(GetSafeAttributePersistenceProvider()->SafeReadValue(path, bufferSpan));

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
CHIP_ERROR CameraAvSettingsUserLevelMgmtServer::SetTiltMin(int16_t aTiltMin)
{
    if (!HasFeature(Feature::kMechanicalTilt))
    {
        return CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute);
    }

    if ((aTiltMin < kMinTiltValue) || (aTiltMin > kMaxTiltValue - 1) || (aTiltMin > mTiltMax))
    {
        return CHIP_IM_GLOBAL_STATUS(ConstraintError);
    }

    if (aTiltMin != mTiltMin)
    {
        mTiltMin = aTiltMin;
        MarkDirty(Attributes::TiltMin::Id);
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR CameraAvSettingsUserLevelMgmtServer::SetTiltMax(int16_t aTiltMax)
{
    if (!HasFeature(Feature::kMechanicalTilt))
    {
        return CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute);
    }

    if ((aTiltMax > kMaxTiltValue) || (aTiltMax < kMinTiltValue + 1) || (aTiltMax < mTiltMin))
    {
        return CHIP_IM_GLOBAL_STATUS(ConstraintError);
    }

    if (aTiltMax != mTiltMax)
    {
        mTiltMax = aTiltMax;
        MarkDirty(Attributes::TiltMax::Id);
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR CameraAvSettingsUserLevelMgmtServer::SetPanMin(int16_t aPanMin)
{
    if (!HasFeature(Feature::kMechanicalPan))
    {
        return CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute);
    }

    if ((aPanMin < kMinPanValue) || (aPanMin > kMaxPanValue - 1) || (aPanMin > mPanMax))
    {
        return CHIP_IM_GLOBAL_STATUS(ConstraintError);
    }

    if (aPanMin != mPanMin)
    {
        mPanMin = aPanMin;
        MarkDirty(Attributes::PanMin::Id);
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR CameraAvSettingsUserLevelMgmtServer::SetPanMax(int16_t aPanMax)
{
    if (!HasFeature(Feature::kMechanicalPan))
    {
        return CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute);
    }

    if ((aPanMax > kMaxPanValue) || (aPanMax < kMinPanValue + 1) || (aPanMax < mPanMin))
    {
        return CHIP_IM_GLOBAL_STATUS(ConstraintError);
    }

    if (aPanMax != mPanMax)
    {
        mPanMax = aPanMax;
        MarkDirty(Attributes::PanMax::Id);
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR CameraAvSettingsUserLevelMgmtServer::SetZoomMax(uint8_t aZoomMax)
{
    if (!HasFeature(Feature::kMechanicalZoom))
    {
        return CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute);
    }

    if ((aZoomMax > kMaxZoomValue) || (aZoomMax < kMinZoomValue + 1))
    {
        return CHIP_IM_GLOBAL_STATUS(ConstraintError);
    }

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
void CameraAvSettingsUserLevelMgmtServer::SetPan(Optional<int16_t> aPan)
{
    if (HasFeature(Feature::kMechanicalPan))
    {
        if (aPan.HasValue())
        {
            mMptzPosition.pan = aPan;
            StoreMPTZPosition(mMptzPosition);
            MarkDirty(Attributes::MPTZPosition::Id);
        }
    }
}

void CameraAvSettingsUserLevelMgmtServer::SetTilt(Optional<int16_t> aTilt)
{
    if (HasFeature(Feature::kMechanicalTilt))
    {
        if (aTilt.HasValue())
        {
            mMptzPosition.tilt = aTilt;
            StoreMPTZPosition(mMptzPosition);
            MarkDirty(Attributes::MPTZPosition::Id);
        }
    }
}

void CameraAvSettingsUserLevelMgmtServer::SetZoom(Optional<uint8_t> aZoom)
{
    if (HasFeature(Feature::kMechanicalZoom))
    {
        if (aZoom.HasValue())
        {
            mMptzPosition.zoom = aZoom;
            StoreMPTZPosition(mMptzPosition);
            MarkDirty(Attributes::MPTZPosition::Id);
        }
    }
}

/**
 * Methods handling known video stream IDs, the addition and removal thereof.
 */
void CameraAvSettingsUserLevelMgmtServer::AddMoveCapableVideoStream(uint16_t aVideoStreamID,
                                                                    Globals::Structs::ViewportStruct::Type aViewport)
{
    DPTZStruct dptzEntry;
    dptzEntry.videoStreamID = aVideoStreamID;
    dptzEntry.viewport      = aViewport;
    mDptzStreams.push_back(dptzEntry);
    MarkDirty(Attributes::DPTZStreams::Id);
}

void CameraAvSettingsUserLevelMgmtServer::UpdateMoveCapableVideoStream(uint16_t aVideoStreamID,
                                                                       Globals::Structs::ViewportStruct::Type aViewport)
{
    auto it = std::find_if(mDptzStreams.begin(), mDptzStreams.end(),
                           [aVideoStreamID](const DPTZStruct & dptzs) { return dptzs.videoStreamID == aVideoStreamID; });

    if (it == mDptzStreams.end())
    {
        ChipLogError(Zcl, "CameraAVSettingsUserLevelMgmt[ep=%d]. No matching video stream ID, update not possible. ID=%d.",
                     mEndpointId, aVideoStreamID);
        return;
    }

    it->viewport = aViewport;
    MarkDirty(Attributes::DPTZStreams::Id);
}

void CameraAvSettingsUserLevelMgmtServer::UpdateMoveCapableVideoStreams(Globals::Structs::ViewportStruct::Type aViewport)
{
    for (auto & dptzStream : mDptzStreams)
    {
        dptzStream.viewport = aViewport;
    }

    MarkDirty(Attributes::DPTZStreams::Id);
}

void CameraAvSettingsUserLevelMgmtServer::RemoveMoveCapableVideoStream(uint16_t aVideoStreamID)
{
    // Verify that this is a known ID, if it is, remove from the list
    //
    auto it = std::find_if(mDptzStreams.begin(), mDptzStreams.end(),
                           [aVideoStreamID](const DPTZStruct & dptzs) { return dptzs.videoStreamID == aVideoStreamID; });

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
bool CameraAvSettingsUserLevelMgmtServer::KnownVideoStreamID(uint16_t aVideoStreamID)
{
    auto it = std::find_if(mDptzStreams.begin(), mDptzStreams.end(),
                           [aVideoStreamID](const DPTZStruct & dptzs) { return dptzs.videoStreamID == aVideoStreamID; });

    return (it == mDptzStreams.end() ? false : true);
}

/**
 * Helper function for setting the next preset ID to use in advance of reception of an MPTZSavePreset.
 * The method loops over the range of possible IDs, starting with the current ID, if the preset ID is in use,
 * it continues to the next possible value, looping back to 1. The checking is needed as the preset IDs aren't
 * solely server generated, they can also be provided by a client.
 * If there are no free presets (which will happen if all slots are taken), the value is not updated.
 */
void CameraAvSettingsUserLevelMgmtServer::UpdatePresetID()
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
                               [=](const MPTZPresetHelper & mptzph) { return mptzph.GetPresetID() == nextIDToCheck; });
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
            ChipLogDetail(Zcl,
                          "CameraAVSettingsUserLevelMgmt[ep=%d]: Encoding an instance of MPTZPresetStruct. ID = %d. Name = %.*s",
                          mEndpointId, presetStruct.presetID, static_cast<int>(presetStruct.name.size()), presetStruct.name.data());
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

void CameraAvSettingsUserLevelMgmtServer::LoadPersistentAttributes()
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    // Load MPTZPosition
    MPTZStructType storedMPTZPosition;
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
    err = mDelegate.LoadMPTZPresets(mMptzPresetHelpers);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogDetail(Zcl, "CameraAVSettingsUserLevelMgmt[ep=%d]: Unable to load the MPTZPresets from the KVS.", mEndpointId);
    }

    // Load DPTZRelativeMove
    err = mDelegate.LoadDPTZStreams(mDptzStreams);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogDetail(Zcl, "CameraAVSettingsUserLevelMgmt[ep=%d]: Unable to load the DPTZRelativeMove from the KVS.", mEndpointId);
    }

    // Signal delegate that all persistent configuration attributes have been loaded.
    mDelegate.PersistentAttributesLoadedCallback();
}

/**
 * AttributeAccessInterface
 */
CHIP_ERROR CameraAvSettingsUserLevelMgmtServer::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    VerifyOrDie(aPath.mClusterId == CameraAvSettingsUserLevelManagement::Id);
    ChipLogDetail(Zcl, "CameraAVSettingsUserLevelMgmt[ep=%d]: Camera AV Settings User Level Management: Reading", mEndpointId);

    switch (aPath.mAttributeId)
    {
    case FeatureMap::Id:
        ReturnErrorOnFailure(aEncoder.Encode(mFeatures));
        break;
    case MPTZPosition::Id:
        VerifyOrReturnError(
            HasFeature(Feature::kMechanicalPan) || HasFeature(Feature::kMechanicalTilt) || HasFeature(Feature::kMechanicalZoom),
            CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
            ChipLogError(Zcl, "CameraAVSettingsUserLevelMgmt[ep=%d]: can not get MPTZPosition, feature is not supported",
                         mEndpointId));

        ReturnErrorOnFailure(aEncoder.Encode(mMptzPosition));
        break;
    case MaxPresets::Id:
        VerifyOrReturnError(HasFeature(Feature::kMechanicalPresets), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
                            ChipLogError(Zcl,
                                         "CameraAVSettingsUserLevelMgmt[ep=%d]: can not get MaxPresets, feature is not supported",
                                         mEndpointId));

        ReturnErrorOnFailure(aEncoder.Encode(mMaxPresets));
        break;
    case MPTZPresets::Id:
        VerifyOrReturnError(HasFeature(Feature::kMechanicalPresets), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
                            ChipLogError(Zcl,
                                         "CameraAVSettingsUserLevelMgmt[ep=%d]: can not get MPTZPresets, feature is not supported",
                                         mEndpointId));

        return ReadAndEncodeMPTZPresets(aEncoder);
    case DPTZStreams::Id:
        VerifyOrReturnError(
            HasFeature(Feature::kDigitalPTZ), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
            ChipLogError(Zcl, "CameraAVSettingsUserLevelMgmt[ep=%d]: can not get DPTZRelativeMove, feature is not supported",
                         mEndpointId));
        return ReadAndEncodeDPTZStreams(aEncoder);
    case ZoomMax::Id:
        VerifyOrReturnError(
            HasFeature(Feature::kMechanicalZoom), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
            ChipLogError(Zcl, "CameraAVSettingsUserLevelMgmt[ep=%d]: can not get ZoomMax, feature is not supported", mEndpointId));
        ReturnErrorOnFailure(aEncoder.Encode(mZoomMax));
        break;
    case TiltMin::Id:
        VerifyOrReturnError(
            HasFeature(Feature::kMechanicalTilt), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
            ChipLogError(Zcl, "CameraAVSettingsUserLevelMgmt[ep=%d]: can not get TiltMin, feature is not supported", mEndpointId));
        ReturnErrorOnFailure(aEncoder.Encode(mTiltMin));
        break;
    case TiltMax::Id:
        VerifyOrReturnError(
            HasFeature(Feature::kMechanicalTilt), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
            ChipLogError(Zcl, "CameraAVSettingsUserLevelMgmt[ep=%d]: can not get TiltMax, feature is not supported", mEndpointId));
        ReturnErrorOnFailure(aEncoder.Encode(mTiltMax));
        break;
    case PanMin::Id:
        VerifyOrReturnError(
            HasFeature(Feature::kMechanicalPan), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
            ChipLogError(Zcl, "CameraAVSettingsUserLevelMgmt[ep=%d]: can not get PanMin, feature is not supported", mEndpointId));
        ReturnErrorOnFailure(aEncoder.Encode(mPanMin));
        break;
    case PanMax::Id:
        VerifyOrReturnError(
            HasFeature(Feature::kMechanicalPan), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
            ChipLogError(Zcl, "CameraAVSettingsUserLevelMgmt[ep=%d]: can not get PanMax, feature is not supported", mEndpointId));
        ReturnErrorOnFailure(aEncoder.Encode(mPanMax));
        break;
    }

    return CHIP_NO_ERROR;
}

/**
 * CommandHandlerInterface
 */
void CameraAvSettingsUserLevelMgmtServer::InvokeCommand(HandlerContext & handlerContext)
{
    ChipLogDetail(Zcl, "CameraAVSettingsUserLevelMgmt[ep=%d]: InvokeCommand", mEndpointId);
    switch (handlerContext.mRequestPath.mCommandId)
    {
    case Commands::MPTZSetPosition::Id:
        ChipLogDetail(Zcl, "CameraAVSettingsUserLevelMgmt[ep=%d]: Setting MPTZ Position", mEndpointId);

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
        ChipLogDetail(Zcl, "CameraAVSettingsUserLevelMgmt[ep=%d]: Relative move of MPTZ Position", mEndpointId);

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
        ChipLogDetail(Zcl, "CameraAVSettingsUserLevelMgmt[ep=%d]: Moving to an indicated MPTZ Preset", mEndpointId);

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
        ChipLogDetail(Zcl, "CameraAVSettingsUserLevelMgmt[ep=%d]: Saving MPTZ Preset", mEndpointId);

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
        ChipLogDetail(Zcl, "CameraAVSettingsUserLevelMgmt[ep=%d]: Remove indicated MPTZ Preset", mEndpointId);

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
        ChipLogDetail(Zcl, "CameraAVSettingsUserLevelMgmt[ep=%d]: Setting DPTZ Viewport", mEndpointId);

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
        ChipLogDetail(Zcl, "CameraAVSettingsUserLevelMgmt[ep=%d]: Relative move within a defined DPTZ Viewport", mEndpointId);

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
    Optional<uint8_t> zoom = commandData.zoom;

    // Validate the received command fields
    //
    if (pan.HasValue())
    {
        if (!HasFeature(Feature::kMechanicalPan))
        {
            ChipLogError(Zcl, "CameraAVSettingsUserLevelMgmt[ep=%d]: Mechanical Pan not supported although pan value provided",
                         mEndpointId);
            ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::InvalidCommand);
            return;
        }
        int16_t panValue = pan.Value();
        if ((panValue > mPanMax) || (panValue < mPanMin))
        {
            ChipLogError(Zcl, "CameraAVSettingsUserLevelMgmt[ep=%d]: Received Pan value out of range", mEndpointId);
            ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::ConstraintError);
            return;
        }
        hasAtLeastOneValue = true;
    }

    if (tilt.HasValue())
    {
        if (!HasFeature(Feature::kMechanicalTilt))
        {
            ChipLogError(Zcl, "CameraAVSettingsUserLevelMgmt[ep=%d]: Mechanical Tilt not supported although tilt value provided",
                         mEndpointId);
            ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::InvalidCommand);
            return;
        }
        int16_t tiltValue = tilt.Value();
        if ((tiltValue > mTiltMax) || (tiltValue < mTiltMin))
        {
            ChipLogError(Zcl, "CameraAVSettingsUserLevelMgmt[ep=%d]: Received Tilt value out of range", mEndpointId);
            ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::ConstraintError);
            return;
        }
        hasAtLeastOneValue = true;
    }

    if (zoom.HasValue())
    {
        if (!HasFeature(Feature::kMechanicalZoom))
        {
            ChipLogError(Zcl, "CameraAVSettingsUserLevelMgmt[ep=%d]: Mechanical Zoom not supported although zoom value provided",
                         mEndpointId);
            ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::InvalidCommand);
            return;
        }
        uint8_t zoomValue = zoom.Value();
        if ((zoomValue > mZoomMax) || (zoomValue < kMinZoomValue))
        {
            ChipLogError(Zcl, "CameraAVSettingsUserLevelMgmt[ep=%d]: Received Zoom value out of range", mEndpointId);
            ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::ConstraintError);
            return;
        }
        hasAtLeastOneValue = true;
    }

    // Was a value received in the command
    if (!hasAtLeastOneValue)
    {
        ChipLogError(Zcl, "CameraAVSettingsUserLevelMgmt[ep=%d]: MPTZSetPosition command contains no actionable fields",
                     mEndpointId);
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::InvalidCommand);
        return;
    }

    // Check with the delegate that we're in a position to change any of the PTZ values
    //
    if (!mDelegate.CanChangeMPTZ())
    {
        ChipLogDetail(Zcl, "CameraAVSettingsUserLevelMgmt[ep=%d]: Device not able to process MPTZ change", mEndpointId);
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::Busy);
        return;
    }

    // Call the delegate to set the new values
    //
    status = mDelegate.MPTZSetPosition(pan, tilt, zoom);

    if (status != Status::Success)
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
        return;
    }

    // Set the local values of pan, tilt, and zoom
    SetPan(pan);
    SetTilt(tilt);
    SetZoom(zoom);

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

    // These will all be set to actual values if there is a delta provided, and constraint checking passes.
    Optional<int16_t> newPan;
    Optional<int16_t> newTilt;
    Optional<uint8_t> newZoom;

    // Validate the received command fields
    //
    if (panDelta.HasValue())
    {
        if (!HasFeature(Feature::kMechanicalPan))
        {
            ChipLogError(Zcl, "CameraAVSettingsUserLevelMgmt[ep=%d]: Mechanical Pan not supported although panDelta value provided",
                         mEndpointId);
            ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::InvalidCommand);
            return;
        }
        int16_t panDeltaValue = panDelta.Value();
        if (panDeltaValue > (mPanMax - mPanMin) || panDeltaValue < -(mPanMax - mPanMin))
        {
            ChipLogError(Zcl, "CameraAVSettingsUserLevelMgmt[ep=%d]: PanDelta value received is out of range.", mEndpointId);
            ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::ConstraintError);
            return;
        }

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
        if (!HasFeature(Feature::kMechanicalTilt))
        {
            ChipLogError(Zcl,
                         "CameraAVSettingsUserLevelMgmt[ep=%d]: Mechanical Tilt not supported although tiltDelta value provided",
                         mEndpointId);
            ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::InvalidCommand);
            return;
        }
        int16_t tiltDeltaValue = tiltDelta.Value();
        if (tiltDeltaValue > (mTiltMax - mTiltMin) || tiltDeltaValue < -(mTiltMax - mTiltMin))
        {
            ChipLogError(Zcl, "CameraAVSettingsUserLevelMgmt[ep=%d]: TiltDelta value received is out of range.", mEndpointId);
            ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::ConstraintError);
            return;
        }

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
        if (!HasFeature(Feature::kMechanicalZoom))
        {
            ChipLogError(Zcl,
                         "CameraAVSettingsUserLevelMgmt[ep=%d]: Mechanical Zoom not supported although zoomDelta value provided",
                         mEndpointId);
            ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::InvalidCommand);
            return;
        }
        int8_t zoomDeltaValue = zoomDelta.Value();
        if (zoomDeltaValue > (mZoomMax - 1) || zoomDeltaValue < -(mZoomMax - 1))
        {
            ChipLogError(Zcl, "CameraAVSettingsUserLevelMgmt[ep=%d]: ZoomDelta value received is out of range.", mEndpointId);
            ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::ConstraintError);
            return;
        }

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

        newZoom.Emplace(static_cast<uint8_t>(newZoomValue));

        hasAtLeastOneValue = true;
    }

    // Was a value received in the command
    if (!hasAtLeastOneValue)
    {
        ChipLogError(Zcl, "CameraAVSettingsUserLevelMgmt[ep=%d]: MPTZRelativeMove command contains no actionable fields",
                     mEndpointId);
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::InvalidCommand);
        return;
    }

    // Check with the delegate that we're in a position to change any of the PTZ values
    //
    if (!mDelegate.CanChangeMPTZ())
    {
        ChipLogDetail(Zcl, "CameraAVSettingsUserLevelMgmt[ep=%d]: Device not able to process MPTZ relative value change",
                      mEndpointId);
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::Busy);
        return;
    }

    // Call the delegate to simply set the newly calculated MPTZ values based on the deltas received
    //
    status = mDelegate.MPTZRelativeMove(newPan, newTilt, newZoom);

    if (status != Status::Success)
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
        return;
    }

    // Set the local values of pan, tilt, and zoom
    SetPan(newPan);
    SetTilt(newTilt);
    SetZoom(newZoom);

    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
}

void CameraAvSettingsUserLevelMgmtServer::HandleMPTZMoveToPreset(HandlerContext & ctx,
                                                                 const Commands::MPTZMoveToPreset::DecodableType & commandData)
{
    Status status  = Status::Success;
    uint8_t preset = commandData.presetID;

    // Verify the provided presetID is within spec limits
    //
    if ((preset > mMaxPresets) || (preset < 1))
    {
        ChipLogError(Zcl, "CameraAVSettingsUserLevelMgmt[ep=%d]: Preset provided is out of range. Preset: %d", mEndpointId, preset);
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::ConstraintError);
        return;
    }
    // Do we have any presets?
    //
    if (mMptzPresetHelpers.empty())
    {
        ChipLogError(Zcl, "CameraAVSettingsUserLevelMgmt[ep=%d]: No stored presets, MoveToPreset not possible", mEndpointId);
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::NotFound);
        return;
    }

    // We have presets, check that the received ID is a valid preset ID
    //
    auto it = std::find_if(mMptzPresetHelpers.begin(), mMptzPresetHelpers.end(),
                           [preset](const MPTZPresetHelper & mptzph) { return mptzph.GetPresetID() == preset; });

    if (it == mMptzPresetHelpers.end())
    {
        ChipLogError(Zcl,
                     "CameraAVSettingsUserLevelMgmt[ep=%d]: No matching presets, MoveToPreset not possible for provided preset: %d",
                     mEndpointId, preset);
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::NotFound);
        return;
    }

    // This is effectively a manipulation of the current PTZ settings, ensure that the device is in a state wherein a PTZ change is
    // possible
    //
    if (!mDelegate.CanChangeMPTZ())
    {
        ChipLogDetail(Zcl, "CameraAVSettingsUserLevelMgmt[ep=%d]: Device not able to process move to MPTZ preset", mEndpointId);
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::Busy);
        return;
    }

    auto presetValues = it->GetMptzPosition();

    // Inform the delegate that the device is requested to move to PTZ values given by the selected preset id
    // Call the delegate to allow the device to handle the physical changes, on success set the MPTZ values based on the preset
    status = mDelegate.MPTZMoveToPreset(preset, presetValues.pan, presetValues.tilt, presetValues.zoom);

    if (status != Status::Success)
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
        return;
    }

    // Set individual Pan, Tilt, and Zoom from the preset
    SetPan(presetValues.pan);
    SetTilt(presetValues.tilt);
    SetZoom(presetValues.zoom);

    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::Success);
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
        if ((preset.Value() > mMaxPresets) || (preset.Value() < 1))
        {
            ChipLogError(Zcl, "CameraAVSettingsUserLevelMgmt[ep=%d]: Provided preset ID is out of range. Preset: %d", mEndpointId,
                         preset.Value());
            ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::ConstraintError);
            return;
        }
        presetToUse = preset.Value();
    }

    // Does the preset equate to an already known stored preset? If so we're updating that one rather than creating a new one
    //
    auto it = std::find_if(mMptzPresetHelpers.begin(), mMptzPresetHelpers.end(),
                           [presetToUse](const MPTZPresetHelper & mptzph) { return mptzph.GetPresetID() == presetToUse; });

    // If the current preset ID results in an entry from the current known set and there was a provided preset then we're updating
    // an existing preset.  Only check for exhausting max presets if we're NOT updating.
    // It is possible that mCurrentPresetID equates to a current preset in cases where the collection of presets is full
    //
    bool updatingExistingPreset = (it != mMptzPresetHelpers.end()) && (preset.HasValue());

    if (!updatingExistingPreset)
    {
        // Make sure that the vector will not exceed the max size
        //
        if (mMptzPresetHelpers.size() == mMaxPresets)
        {
            ChipLogError(Zcl,
                         "CameraAVSettingsUserLevelMgmt[ep=%d]: No more space for additional presets, MPTZSavePreset not possible",
                         mEndpointId);
            ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::ResourceExhausted);
            return;
        }
    }

    // Call the delegate, make sure that it is ok to save a new preset, given the current
    // delegate aware values for MPTZ
    //
    status = mDelegate.MPTZSavePreset(presetToUse);

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

    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::Success);
}

void CameraAvSettingsUserLevelMgmtServer::HandleMPTZRemovePreset(HandlerContext & ctx,
                                                                 const Commands::MPTZRemovePreset::DecodableType & commandData)
{
    uint8_t presetToRemove = commandData.presetID;

    // Verify the provided presetID is within spec limits
    //
    if ((presetToRemove > mMaxPresets) || (presetToRemove < 1))
    {
        ChipLogError(Zcl, "CameraAVSettingsUserLevelMgmt[ep=%d]: Preset to remove is out of range. Preset: %d", mEndpointId,
                     presetToRemove);
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::ConstraintError);
        return;
    }

    // Is the provided ID known to us?
    //
    auto it = std::find_if(mMptzPresetHelpers.begin(), mMptzPresetHelpers.end(),
                           [presetToRemove](const MPTZPresetHelper & mptzph) { return mptzph.GetPresetID() == presetToRemove; });

    if (it == mMptzPresetHelpers.end())
    {
        ChipLogError(Zcl,
                     "CameraAVSettingsUserLevelMgmt[ep=%d]: No matching presets, RemovePreset not possible for provided preset: %d",
                     mEndpointId, presetToRemove);
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::NotFound);
        return;
    }

    // Call the delegate to ensure that it is ok to remove the preset indicated.
    //
    Status status = mDelegate.MPTZRemovePreset(presetToRemove);

    if (status != Status::Success)
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
        return;
    }

    // Remove the identified item from the known set of presets
    //
    mMptzPresetHelpers.erase(it);
    MarkDirty(Attributes::MPTZPresets::Id);

    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::Success);
}

void CameraAvSettingsUserLevelMgmtServer::HandleDPTZSetViewport(HandlerContext & ctx,
                                                                const Commands::DPTZSetViewport::DecodableType & commandData)
{
    uint16_t videoStreamID                          = commandData.videoStreamID;
    Globals::Structs::ViewportStruct::Type viewport = commandData.viewport;

    // Is this a video stream ID of which we have already been informed?
    // If not, fail.
    //
    if (!KnownVideoStreamID(videoStreamID))
    {
        ChipLogError(Zcl, "CameraAVSettingsUserLevelMgmt[ep=%d]: Unknown Video Stream ID provided. ID: %d", mEndpointId,
                     videoStreamID);
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::NotFound);
        return;
    }

    // Call the delegate
    Status status = mDelegate.DPTZSetViewport(videoStreamID, viewport);

    if (status == Status::Success)
    {
        // Update the viewport of our stream in DPTZStreams
        //
        UpdateMoveCapableVideoStream(videoStreamID, viewport);
    }

    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
}

void CameraAvSettingsUserLevelMgmtServer::HandleDPTZRelativeMove(HandlerContext & ctx,
                                                                 const Commands::DPTZRelativeMove::DecodableType & commandData)
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
        if (zoomDeltaValue < -100 || zoomDeltaValue > 100)
        {
            ChipLogError(Zcl,
                         "CameraAVSettingsUserLevelMgmt[ep=%d]: Provided Digital Zoom Delta is out of range. Provided Zoom: %d",
                         mEndpointId, zoomDeltaValue);
            ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::ConstraintError);
            return;
        }
    }
    // Is this a video stream ID of which we have already been informed via DPTZSetViewport. We can't relative move on a
    // viewport that hasn't already been set, hence we fail if the provided id is not found.
    //
    if (!KnownVideoStreamID(videoStreamID))
    {
        ChipLogError(Zcl, "CameraAVSettingsUserLevelMgmt[ep=%d]. Unknown Video Stream ID provided. ID=%d.", mEndpointId,
                     videoStreamID);
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::NotFound);
        return;
    }

    // Create a viewport and call the delegate; on success update our Stream Viewport with that which was set
    Globals::Structs::ViewportStruct::Type viewport;
    Status status = mDelegate.DPTZRelativeMove(videoStreamID, deltaX, deltaY, zoomDelta, viewport);

    if (status == Status::Success)
    {
        UpdateMoveCapableVideoStream(videoStreamID, viewport);
    }

    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
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

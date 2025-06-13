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

#pragma once

#include <app-common/zap-generated/cluster-objects.h>
#include <app/AttributeAccessInterface.h>
#include <app/CommandHandlerInterface.h>
#include <app/SafeAttributePersistenceProvider.h>
#include <lib/core/CHIPPersistentStorageDelegate.h>
#include <protocols/interaction_model/StatusCode.h>
#include <string>
#include <vector>

namespace chip {
namespace app {
namespace Clusters {
namespace CameraAvSettingsUserLevelManagement {

using MPTZStructType       = Structs::MPTZStruct::Type;
using MPTZPresetStructType = Structs::MPTZPresetStruct::Type;
using DPTZStruct           = Structs::DPTZStruct::Type;

constexpr int16_t kMinPanValue  = -180;
constexpr int16_t kMaxPanValue  = 180;
constexpr int16_t kMinTiltValue = -180;
constexpr int16_t kMaxTiltValue = 180;
constexpr uint8_t kMinZoomValue = 1;
constexpr uint8_t kMaxZoomValue = 100;

// Spec defined defaults for Pan, Tilt, and Zoom
constexpr int16_t kDefaultPan  = 0;
constexpr int16_t kDefaultTilt = 0;
constexpr uint8_t kDefaultZoom = 1;

constexpr size_t kMptzPositionStructMaxSerializedSize =
    TLV::EstimateStructOverhead(sizeof(int16_t), sizeof(int16_t), sizeof(uint8_t));

class Delegate;

enum class OptionalAttributes : uint32_t
{
    kMptzPosition = 0x0001,
    kMaxPresets   = 0x0002,
    kMptzPresets  = 0x0004,
    kDptzStreams  = 0x0008,
    kZoomMax      = 0x0010,
    kTiltMin      = 0x0020,
    kTiltMax      = 0x0040,
    kPanMin       = 0x0080,
    kPanMax       = 0x0100,
};

struct MPTZPresetHelper
{
private:
    uint8_t mPresetID;
    std::string mName;
    MPTZStructType mMptzPosition;

public:
    virtual ~MPTZPresetHelper() = default;
    MPTZPresetHelper() {}
    MPTZPresetHelper(uint8_t aPreset, CharSpan aName, MPTZStructType aPosition)
    {
        SetPresetID(aPreset);
        SetName(aName);
        SetMptzPosition(aPosition);
    }

    // Accessors and Mutators
    //
    std::string GetName() const { return mName; }
    void SetName(chip::CharSpan aName) { mName = std::string(aName.begin(), aName.end()); }

    uint8_t GetPresetID() const { return mPresetID; }
    void SetPresetID(uint8_t aPreset) { mPresetID = aPreset; }

    MPTZStructType GetMptzPosition() const { return mMptzPosition; }
    void SetMptzPosition(MPTZStructType aPosition) { mMptzPosition = aPosition; }
};

class CameraAvSettingsUserLevelMgmtServer : public AttributeAccessInterface, public CommandHandlerInterface
{
public:
    /**
     * Creates a server instance. The Init() function needs to be called for this instance to be registered and
     * called by the interaction model at the appropriate times.
     * @param aEndpointId       The endpoint on which this cluster exists. This must match the zap configuration.
     * @param aDelegate         A reference to the delegate to be used by this server.
     * @param aFeatures         The bitflags value that identifies which features are supported by this instance.
     * @param aOptionalAttrs    The bitflags value that identifies the optional attributes supported by this instance.
     *                                           instance.
     * Note: the caller must ensure that the delegate lives throughout the instance's lifetime.
     */
    CameraAvSettingsUserLevelMgmtServer(EndpointId aEndpointId, Delegate & aDelegate, BitFlags<Feature> aFeatures,
                                        BitFlags<OptionalAttributes> aOptionalAttrs, uint8_t aMaxPresets);
    ~CameraAvSettingsUserLevelMgmtServer() override;

    CHIP_ERROR Init();
    void Shutdown();

    bool HasFeature(Feature aFeature) const;

    bool SupportsOptAttr(OptionalAttributes aOptionalAttr) const;

    // Attribute Accessors and Mutators
    CHIP_ERROR SetTiltMin(int16_t aTiltMin);

    CHIP_ERROR SetTiltMax(int16_t aTiltMax);

    CHIP_ERROR SetPanMin(int16_t aPanMin);

    CHIP_ERROR SetPanMax(int16_t aPanMax);

    CHIP_ERROR SetZoomMax(uint8_t aZoomMax);

    const MPTZStructType & GetMptzPosition() const { return mMptzPosition; }

    uint8_t GetMaxPresets() const { return mMaxPresets; }

    const std::vector<DPTZStruct> GetDptzRelativeMove() const { return mDptzStreams; }

    uint8_t GetZoomMax() const { return mZoomMax; }

    int16_t GetTiltMin() const { return mTiltMin; }

    int16_t GetTiltMax() const { return mTiltMax; }

    int16_t GetPanMin() const { return mPanMin; }

    int16_t GetPanMax() const { return mPanMax; }

    /**
     * Allows for a delegate or application to set the pan value given physical changes on the device itself, possibly due to direct
     * user changes
     */
    void SetPan(Optional<int16_t> aPan);

    /**
     * Allows for a delegate or application to set the tilt value given physical changes on the device itself, possibly due to
     * direct user changes
     */
    void SetTilt(Optional<int16_t> aTilt);

    /**
     * Allows for a delegate or application to set the zoom value given physical changes on the device itself, possibly due to
     * direct user changes
     */
    void SetZoom(Optional<uint8_t> aZoom);

    /**
     * Allows for a delegate or application to provide the ID and default Viewport of an allocated video stream that is capable of
     * digital movement. This should be invoked by a delegate on the conclusion of allocating a video stream via the AV Stream
     * Management cluster.
     */
    void AddMoveCapableVideoStream(uint16_t aVideoStreamID, Globals::Structs::ViewportStruct::Type aViewport);

    /**
     * Allows for a delegate or application to update the viewport of an already allocated video stream.
     * This should be invoked whenever a viewport is updated by DPTZSetVewport or DPTZRelativeMove
     */
    void UpdateMoveCapableVideoStream(uint16_t aVideoStreamID, Globals::Structs::ViewportStruct::Type aViewport);

    /**
     * Allows for a delegate or application to update all of the viewports for all of the allocated video streams.
     * This should be invoked whenever the device default viewport is updated via a write to Viewport on the
     * AV Stream Management Cluster
     */
    void UpdateMoveCapableVideoStreams(Globals::Structs::ViewportStruct::Type aViewport);

    /**
     * Allows for a delegate or application to remove a video stream from the set that is capable of digital movement.
     * This should be invoked by a delegate on the conclusion of deallocating a video stream via the AV Stream Management cluster.
     */
    void RemoveMoveCapableVideoStream(uint16_t aVideoStreamID);

    EndpointId GetEndpointId() { return AttributeAccessInterface::GetEndpointId().Value(); }

private:
    Delegate & mDelegate;
    EndpointId mEndpointId;
    BitFlags<Feature> mFeatures;
    BitFlags<OptionalAttributes> mOptionalAttrs;

    // Next available preset ID
    uint8_t mCurrentPresetID = 1;

    // My known values for MPTZ.
    MPTZStructType mMptzPosition;

    // Note, where assigned, these are spec defaults, potentially overwritten by the delegate. Exception is MaxPresets that
    // is an F quality attribute and assigned by the constructor
    const uint8_t mMaxPresets;
    int16_t mPanMin  = kMinPanValue;
    int16_t mPanMax  = kMaxPanValue;
    int16_t mTiltMin = -90;
    int16_t mTiltMax = 90;
    uint8_t mZoomMax = kMaxZoomValue;

    std::vector<MPTZPresetHelper> mMptzPresetHelpers;
    std::vector<DPTZStruct> mDptzStreams;

    // Attribute handler interface
    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;

    // Helper Read functions for complex attribute types
    CHIP_ERROR ReadAndEncodeMPTZPresets(AttributeValueEncoder & encoder);
    CHIP_ERROR ReadAndEncodeDPTZStreams(AttributeValueEncoder & encoder);

    CHIP_ERROR StoreMPTZPosition(const MPTZStructType & mptzPosition);
    CHIP_ERROR LoadMPTZPosition(MPTZStructType & mptzPosition);

    /**
     * Helper function that loads all the persistent attributes from the KVS.
     */
    void LoadPersistentAttributes();

    // Command handler interface
    void InvokeCommand(HandlerContext & ctx) override;
    void HandleMPTZSetPosition(HandlerContext & ctx, const Commands::MPTZSetPosition::DecodableType & commandData);
    void HandleMPTZRelativeMove(HandlerContext & ctx, const Commands::MPTZRelativeMove::DecodableType & commandData);
    void HandleMPTZMoveToPreset(HandlerContext & ctx, const Commands::MPTZMoveToPreset::DecodableType & commandData);
    void HandleMPTZSavePreset(HandlerContext & ctx, const Commands::MPTZSavePreset::DecodableType & commandData);
    void HandleMPTZRemovePreset(HandlerContext & ctx, const Commands::MPTZRemovePreset::DecodableType & commandData);
    void HandleDPTZSetViewport(HandlerContext & ctx, const Commands::DPTZSetViewport::DecodableType & commandData);
    void HandleDPTZRelativeMove(HandlerContext & ctx, const Commands::DPTZRelativeMove::DecodableType & commandData);

    /**
     * Helper function that manages preset IDs
     */
    void UpdatePresetID();

    /**
     * Helper function that validates whether a given video stream ID is already known
     */
    bool KnownVideoStreamID(uint16_t aVideoStreamID);

    /**
     * Helper function for attribute handlers to mark the attribute as dirty
     */
    void MarkDirty(AttributeId aAttributeId);
};

/** @brief
 *  Defines interfaces for implementing application-specific logic for various aspects of the CameraAvUserSettingsManagement
 * Cluster. Specifically, it defines interfaces for the interaction with manual and digital pan, tilt, and zoom functions.
 */
class Delegate
{
public:
    Delegate()          = default;
    virtual ~Delegate() = default;

    /**
     * Allows the delegate to determine whether a change in MPTZ is possible given current device status
     */
    virtual bool CanChangeMPTZ() = 0;

    /**
     * DPTZ Stream handling. Invoked on the delegate by an app, providing to the delegate the id of an
     * allocated or deallocated stream, or the viewport when the device level viewport is updated.
     * The delegate shall invoke the appropriate MoveCapableVideoStream methods on its instance of the server
     */
    virtual void VideoStreamAllocated(uint16_t aStreamID)                                 = 0;
    virtual void VideoStreamDeallocated(uint16_t aStreamID)                               = 0;
    virtual void DefaultViewportUpdated(Globals::Structs::ViewportStruct::Type aViewport) = 0;

    /**
     * Delegate command handlers
     */

    /**
     * Allows any needed app handling given provided and already validated pan, tilt, and zoom values that are to be set based on
     * receoption of an MPTZSetPosition command. Returns a failure status if the physical device cannot realize these values. On a
     * success response the server will update the server held attribute values for PTZ.
     * @param aPan  The validated value of the pan that is to be set
     * @param aTilt The validated value of the tilt that is to be set
     * @param aZoom The validated value of the zoom that is to be set
     */
    virtual Protocols::InteractionModel::Status MPTZSetPosition(Optional<int16_t> aPan, Optional<int16_t> aTilt,
                                                                Optional<uint8_t> aZoom) = 0;

    /**
     * Allows any needed app handling given provided and already validated pan, tilt, and zoom values that are to be set based on
     * receoption of an MPTZRelativeMove command.  The server has already validated the received relative values, and provides the
     * app with the new, requested settings for PTZ. Returns a failure status if the physical device cannot realize these values. On
     * a success response the server will update the server held attribute values for PTZ.
     * @param aPan  The validated value of the pan that is to be set
     * @param aTilt The validated value of the tilt that is to be set
     * @param aZoom The validated value of the zoom that is to be set
     */
    virtual Protocols::InteractionModel::Status MPTZRelativeMove(Optional<int16_t> aPan, Optional<int16_t> aTilt,
                                                                 Optional<uint8_t> aZoom) = 0;

    /**
     * Allows any needed app handling given provided and already validated pan, tilt, and zoom values that are to be set based on
     * receoption of an MPTZMoveToPreset command.  The server has already ensured the requested preset ID exists, and obtained the
     * values for PTZ defined by that preset. Returns a failure status if the physical device cannot realize these values. On
     * a success response the server will update the server held attribute values for PTZ.
     * @param aPreset The preset ID associated with the provided PTZ values
     * @param aPan    The value for Pan associated with the preset
     * @param aTilt   The value for Tilt associated with the preset
     * @param aZoom   The value for Zoom associated with the preset
     */
    virtual Protocols::InteractionModel::Status MPTZMoveToPreset(uint8_t aPreset, Optional<int16_t> aPan, Optional<int16_t> aTilt,
                                                                 Optional<uint8_t> aZoom) = 0;

    /**
     * Informs the delegate that a request has been made to save the current PTZ values in a new (or updated) preset ID.
     * The preset ID is provided, the delegate is already aware of the current PTZ values that will be saved. Allows any needed app
     * handling and the possibility that the request is rejected depending on device state. On a success response the server will
     * save the current values of PTZ against the preset ID.
     * @param aPreset The preset ID that will be used for the saved values
     */
    virtual Protocols::InteractionModel::Status MPTZSavePreset(uint8_t aPreset) = 0;

    /**
     * Informs the delegate that a request has been made to remove the preset indicated. Allows any needed app handling and the
     * possibility that the request is rejected depending on device state. On a success response the server will erases the
     * indicated preset and its associated values.
     * @param aPreset The preset ID that will be used for the saved values
     */
    virtual Protocols::InteractionModel::Status MPTZRemovePreset(uint8_t aPreset) = 0;

    /**
     * Informs the delegate that a request has been made to change the Viewport associated with the provided video stream ID. The
     * server has already ensured that the video stream ID is for a valid video stream. The app needs to work with its AV Stream
     * Managament instance to validate the stream ID, the provided new Viewport, ensure that the command is posssible, and apply the
     * command logic to the camera.
     * @param aVideoStreamID The ID for the videa stream that is subject to change
     * @param aViewport      The new values of Viewport that are to be set
     */
    virtual Protocols::InteractionModel::Status DPTZSetViewport(uint16_t aVideoStreamID,
                                                                Globals::Structs::ViewportStruct::Type aViewport) = 0;
    /**
     * Informs the delegate that a request has been made to digitally alter the current rendered stream. The server has already
     * validated that the Zoom Delta (if provided) is in range, and that the video stream ID is valid. The app needs to work with
     * its AV Stream Managament instance to validate the stream ID, ensure that the command is posssible, and apply the command
     * logic to the camera.
     * @param aVideoStreamID The ID for the videa stream that is subject to change
     * @param aDeltaX        Number of pixels to move in the X plane
     * @param aDeltaY        Number of pixels to move in the Y plane
     * @param aZoomDelta     Relative change of digital zoom
     */
    virtual Protocols::InteractionModel::Status DPTZRelativeMove(uint16_t aVideoStreamID, Optional<int16_t> aDeltaX,
                                                                 Optional<int16_t> aDeltaY, Optional<int8_t> aZoomDelta,
                                                                 Globals::Structs::ViewportStruct::Type & aViewport) = 0;

    /**
     *  @brief Callback into the delegate once persistent attributes managed by
     *  the Cluster have been loaded from Storage.
     */
    virtual CHIP_ERROR PersistentAttributesLoadedCallback() = 0;

    /**
     *  Delegate functions to load the Presets and DPTZRelativeMove valid set of video stream IDs.
     *  The delegate application is responsible for creating and persisting this data. The overall
     *  application is already handling the persistence of the allocated video streams themselves.
     *  hese Load APIs would be used to load the known presets and stream ids into the cluster
     *  server list, at initialization.
     */
    virtual CHIP_ERROR LoadMPTZPresets(std::vector<MPTZPresetHelper> & mptzPresetHelpers) = 0;
    virtual CHIP_ERROR LoadDPTZStreams(std::vector<DPTZStruct> dptzStreams)               = 0;

    CameraAvSettingsUserLevelMgmtServer * mServer = nullptr;

    // Sets the Server pointer
    void SetServer(CameraAvSettingsUserLevelMgmtServer * aServer) { mServer = aServer; }

protected:
    CameraAvSettingsUserLevelMgmtServer * GetServer() const { return mServer; }
};

} // namespace CameraAvSettingsUserLevelManagement
} // namespace Clusters
} // namespace app
} // namespace chip

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
#include <protocols/interaction_model/StatusCode.h>
#include <string>
#include <vector>

namespace chip {
namespace app {
namespace Clusters {
namespace CameraAvSettingsUserLevelManagement {

using MPTZStructType       = Structs::MPTZStruct::Type;
using MPTZPresetStructType = Structs::MPTZPresetStruct::Type;

constexpr int16_t kMinPanValue  = -180;
constexpr int16_t kMaxPanValue  = 180;
constexpr int16_t kMinTiltValue = -180;
constexpr int16_t kMaxTiltValue = 180;
constexpr uint8_t kMinZoomValue = 1;
constexpr uint8_t kMaxZoomValue = 100;

// Spec defined defaulta for Pan, Tilt, and Zoom
constexpr int16_t kDefaultPan  = 0;
constexpr int16_t kDefaultTilt = 0;
constexpr uint8_t kDefaultZoom = 1;

class Delegate;

enum class OptionalAttributes : uint32_t
{
    kMptzPosition     = 0x0001,
    kMaxPresets       = 0x0002,
    kMptzPresets      = 0x0004,
    kDptzRelativeMove = 0x0008,
    kZoomMax          = 0x0010,
    kTiltMin          = 0x0020,
    kTiltMax          = 0x0040,
    kPanMin           = 0x0080,
    kPanMax           = 0x0100,
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
    CameraAvSettingsUserLevelMgmtServer(EndpointId aEndpointId, Delegate * aDelegate, BitFlags<Feature> aFeatures,
                                        BitFlags<OptionalAttributes> aOptionalAttrs);
    ~CameraAvSettingsUserLevelMgmtServer() override;

    CHIP_ERROR Init();
    void Shutdown();

    bool HasFeature(Feature aFeature) const;

    bool SupportsOptAttr(OptionalAttributes aOptionalAttr) const;

    // Attribute Accessors and Mutators
    CHIP_ERROR SetMaxPresets(uint8_t aMaxPresets);

    CHIP_ERROR SetTiltMin(int16_t aTiltMin);

    CHIP_ERROR SetTiltMax(int16_t aTiltMax);

    CHIP_ERROR SetPanMin(int16_t aPanMin);

    CHIP_ERROR SetPanMax(int16_t aPanMax);

    CHIP_ERROR SetZoomMax(uint8_t aZoomMax);

    const MPTZStructType & GetMptzPosition() const { return mMptzPosition; }

    uint8_t GetMaxPresets() const { return mMaxPresets; }

    const std::vector<uint16_t> GetDptzRelativeMove() const { return mDptzRelativeMove; }

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
     * Allows for a delegate or application to provide the ID of an allocated video stream that is capable of digital movement.
     * It is expected that this would be done by a delegate on the conclusion of allocating a video stream via the AV Stream
     * Management cluster.
     */
    void AddMoveCapableVideoStreamID(uint16_t aVideoStreamID);

    /**
     * Allows for a delegate or application to remove a video stream from the set that is capable of digital movement.
     * It is expected that this would be done by a delegate on the conclusion of deallocating a video stream via the AV Stream
     * Management cluster.
     */
    void RemoveMoveCapableVideoStreamID(uint16_t aVideoStreamID);

    EndpointId GetEndpointId() { return AttributeAccessInterface::GetEndpointId().Value(); }

private:
    Delegate * mDelegate;
    EndpointId mEndpointId;
    BitMask<Feature> mFeatures;
    BitMask<OptionalAttributes> mOptionalAttrs;

    // Next available preset ID
    uint8_t mCurrentPresetID = 1;

    // My known values for MPTZ.
    MPTZStructType mMptzPosition;

    // Note, spec defaults, potentially overwritten by the delegate
    uint8_t mMaxPresets = 5;
    int16_t mPanMin     = -180;
    int16_t mPanMax     = 180;
    int16_t mTiltMin    = -90;
    int16_t mTiltMax    = 90;
    uint8_t mZoomMax    = 100;

    std::vector<MPTZPresetHelper> mMptzPresetHelpers;
    std::vector<uint16_t> mDptzRelativeMove;

    // Attribute handler interface
    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;

    // Helper Read functions for complex attribute types
    CHIP_ERROR ReadAndEncodeMPTZPresets(AttributeValueEncoder & encoder);
    CHIP_ERROR ReadAndEncodeDPTZRelativeMove(AttributeValueEncoder & encoder);

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
     * Allows the delegate to verify that a received video stream ID is valid
     */
    virtual bool IsValidVideoStreamID(uint16_t aVideoStreamID) = 0;

    /**
     * Delegate command handlers
     */

    /**
     * Allows any needed app handling given provided and already validated pan, tilt, and zoom values that are to be set based on
     * receoption of an MPTZSetPosition command. Returns a failure status if the physical device cannot realize these values. On a
     * success response the server will update the server held attribute values for PTZ.
     * @param pan The validated value of the pan that is to be set
     * @param tilt The validated value of the tilt that is to be set
     * @param zoom The validated value of the zoom that is to be set
     */
    virtual Protocols::InteractionModel::Status MPTZSetPosition(Optional<int16_t> aPan, Optional<int16_t> aTilt,
                                                                Optional<uint8_t> aZoom) = 0;

    /**
     * Allows any needed app handling given provided and already validated pan, tilt, and zoom values that are to be set based on
     * receoption of an MPTZRelativeMove command.  The server has already validated the received relative values, and provides the
     * app with the new, requested settings for PTZ. Returns a failure status if the physical device cannot realize these values. On
     * a success response the server will update the server held attribute values for PTZ.
     * @param pan The validated value of the pan that is to be set
     * @param tilt The validated value of the tilt that is to be set
     * @param zoom The validated value of the zoom that is to be set
     */
    virtual Protocols::InteractionModel::Status MPTZRelativeMove(Optional<int16_t> aPan, Optional<int16_t> aTilt,
                                                                 Optional<uint8_t> aZoom) = 0;

    virtual Protocols::InteractionModel::Status MPTZMoveToPreset(uint8_t aPreset, Optional<int16_t> aPan, Optional<int16_t> aTilt,
                                                                 Optional<uint8_t> aZoom)                                = 0;
    virtual Protocols::InteractionModel::Status MPTZSavePreset(uint8_t aPreset)                                          = 0;
    virtual Protocols::InteractionModel::Status MPTZRemovePreset(uint8_t aPreset)                                        = 0;
    virtual Protocols::InteractionModel::Status DPTZSetViewport(uint16_t aVideoStreamID,
                                                                Structs::ViewportStruct::Type aViewport)                 = 0;
    virtual Protocols::InteractionModel::Status DPTZRelativeMove(uint16_t aVideoStreamID, Optional<int16_t> aDeltaX,
                                                                 Optional<int16_t> aDeltaY, Optional<int8_t> aZoomDelta) = 0;

private:
    friend class CameraAvSettingsUserLevelMgmtServer;

    CameraAvSettingsUserLevelMgmtServer * mServer = nullptr;

    // sets the Server pointer
    void SetServer(CameraAvSettingsUserLevelMgmtServer * aServer) { mServer = aServer; }

protected:
    CameraAvSettingsUserLevelMgmtServer * GetServer() const { return mServer; }
};

} // namespace CameraAvSettingsUserLevelManagement
} // namespace Clusters
} // namespace app
} // namespace chip

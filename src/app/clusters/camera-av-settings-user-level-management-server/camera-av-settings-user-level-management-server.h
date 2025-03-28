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
#include <lib/core/CHIPPersistentStorageDelegate.h>
#include <protocols/interaction_model/StatusCode.h>
#include <vector>

namespace chip {
namespace app {
namespace Clusters {
namespace CameraAvSettingsUserLevelManagement {

using chip::Protocols::InteractionModel::Status;
using MPTZStructType       = Structs::MPTZStruct::Type;
using MPTZPresetStructType = Structs::MPTZPresetStruct::Type;

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
    uint8_t presetID;
    std::string name;
    MPTZStructType mptzPosition;

public:
    virtual ~MPTZPresetHelper() = default;
    MPTZPresetHelper() {}
    MPTZPresetHelper(uint8_t aPreset, chip::CharSpan aName, MPTZStructType aPosition)
    {
        SetPresetID(aPreset);
        SetName(aName);
        SetMptzPosition(aPosition);
    }

    // Accessors and Mutators
    //
    std::string GetName() const { return name; }
    void SetName(chip::CharSpan aName) { name = std::string(aName.begin(), aName.end()); }

    uint8_t GetPresetID() const { return presetID; }
    void SetPresetID(uint8_t aPreset) { presetID = aPreset; }

    MPTZStructType GetMptzPosition() const { return mptzPosition; }
    void SetMptzPosition(MPTZStructType aPosition) { mptzPosition = aPosition; }
};

class CameraAvSettingsUserLevelMgmtServer : public AttributeAccessInterface, public CommandHandlerInterface
{
public:
    /**
     * Creates a server instance. The Init() function needs to be called for this instance to be registered and
     * called by the interaction model at the appropriate times.
     * @param aEndpointId The endpoint on which this cluster exists. This must match the zap configuration.
     * @param aDelegate A reference to the delegate to be used by this server.
     * Note: the caller must ensure that the delegate lives throughout the instance's lifetime.
     */
    CameraAvSettingsUserLevelMgmtServer(EndpointId endpointId, Delegate * delegate, BitMask<Feature> aFeature,
                                        const BitFlags<OptionalAttributes> aOptionalAttrs, uint8_t aMaxPresets, uint16_t aPanMin,
                                        uint16_t aPanMax, uint16_t aTiltMin, uint16_t aTiltMax, uint8_t aZoomMax);
    ~CameraAvSettingsUserLevelMgmtServer() override;

    CHIP_ERROR Init();
    void Shutdown();

    bool HasFeature(Feature aFeature) const;

    bool SupportsOptAttr(OptionalAttributes aOptionalAttrs) const;

    // Attribute Accessors and Mutators
    const MPTZStructType & GetMptzPosition() const { return mMptzPosition; }

    /**
     * Allows for a delegate or application to set the pan value given physical changes on the device itself, possibly due to direct
     * user changes
     */
    void setPan(Optional<int16_t>);

    /**
     * Allows for a delegate or application to set the tilt value given physical changes on the device itself, possibly due to
     * direct user changes
     */
    void setTilt(Optional<int16_t>);

    /**
     * Allows for a delegate or application to set the zoom value given physical changes on the device itself, possibly due to
     * direct user changes
     */
    void setZoom(Optional<uint8_t>);

    uint8_t GetMaxPresets() const { return mMaxPresets; }

    const std::vector<uint16_t> GetDptzRelativeMove() const { return mDptzRelativeMove; }

    uint8_t GetZoomMax() const { return mZoomMax; }

    int16_t GetTiltMin() const { return mTiltMin; }

    int16_t GetTiltMax() const { return mTiltMax; }

    int16_t GetPanMin() const { return mPanMin; }

    int16_t GetPanMax() const { return mPanMax; }

    EndpointId GetEndpointId() { return AttributeAccessInterface::GetEndpointId().Value(); }

private:
    Delegate * mDelegate;
    EndpointId mEndpointId;
    BitMask<Feature> mFeature;
    BitMask<OptionalAttributes> mOptionalAttrs;

    const Optional<int16_t> defaultPan  = Optional(static_cast<int16_t>(0));
    const Optional<int16_t> defaultTilt = Optional(static_cast<int16_t>(0));
    const Optional<uint8_t> defaultZoom = Optional(static_cast<uint8_t>(1));

    uint8_t currentPresetID = 0;

    // Attributes local storage
    MPTZStructType mMptzPosition;

    // Note, spec defaults, overwritten on construction
    uint8_t mMaxPresets = 5;
    int16_t mPanMin     = -180;
    int16_t mPanMax     = 180;
    int16_t mTiltMin    = -90;
    int16_t mTiltMax    = 90;
    uint8_t mZoomMax    = 100;

    std::vector<MPTZPresetHelper> mMptzPresetHelper;
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
     * Helper function that loads all the persistent attributes from the KVS.
     */
    void LoadPersistentAttributes();

    /**
     * Helper function that manages preset IDs
     */
    void UpdatePresetID();
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
     *  @brief Callback into the delegate once persistent attributes managed by
     *  the Cluster have been loaded from Storage.
     */
    virtual Protocols::InteractionModel::Status PersistentAttributesLoadedCallback() { return Status::Success; };

    /**
     * Allows the delegate to determine whether a change in MPTZ is possible given current device status
     */
    virtual bool CanChangeMPTZ() = 0;

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
    virtual Protocols::InteractionModel::Status MPTZSetPosition(Optional<int16_t> pan, Optional<int16_t> tilt,
                                                                Optional<uint8_t> zoom) = 0;

    /**
     * Allows any needed app handling given provided and already validated pan, tilt, and zoom values that are to be set based on
     * receoption of an MPTZRelativeMove command.  The server has already validated the received relative values, and provides the
     * app with the new, requested settings for PTZ. Returns a failure status if the physical device cannot realize these values. On
     * a success response the server will update the server held attribute values for PTZ.
     * @param pan The validated value of the pan that is to be set
     * @param tilt The validated value of the tilt that is to be set
     * @param zoom The validated value of the zoom that is to be set
     */
    virtual Protocols::InteractionModel::Status MPTZRelativeMove(Optional<int16_t> pan, Optional<int16_t> tilt,
                                                                 Optional<uint8_t> zoom) = 0;

    virtual Protocols::InteractionModel::Status MPTZMoveToPreset(uint8_t preset, Optional<int16_t> pan, Optional<int16_t> tilt,
                                                                 Optional<uint8_t> zoom) = 0;
    virtual Protocols::InteractionModel::Status MPTZSavePreset()                         = 0;
    virtual Protocols::InteractionModel::Status MPTZRemovePreset()                       = 0;
    virtual Protocols::InteractionModel::Status DPTZSetViewport()                        = 0;
    virtual Protocols::InteractionModel::Status DPTZRelativeMove()                       = 0;

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

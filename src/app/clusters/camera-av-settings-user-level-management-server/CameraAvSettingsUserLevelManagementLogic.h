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
#include <app/AttributeValueEncoder.h>
#include <app/CommandHandler.h>
#include <app/clusters/camera-av-settings-user-level-management-server/CameraAvSettingsUserLevelManagementCluster.h>
#include <app/clusters/camera-av-settings-user-level-management-server/CameraAvSettingsUserLevelManagementConstants.h>
#include <app/data-model-provider/ActionReturnStatus.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <lib/support/ReadOnlyBuffer.h>
#include <protocols/interaction_model/StatusCode.h>
#include <string>
#include <vector>

namespace chip {
namespace app {
namespace Clusters {

class CameraAvSettingsUserLevelManagementDelegate;

class CameraAvSettingsUserLevelMgmtServerLogic : public CameraAvSettingsUserLevelManagement::PhysicalPTZCallback
{
public:
    /**
     * Creates a server instance. The Init() function needs to be called for this instance to be registered and
     * called by the interaction model at the appropriate times.
     * @param aEndpointId       The endpoint on which this cluster exists. This must match the zap configuration.
     * @param aFeatures         The bitflags value that identifies which features are supported by this instance.

     * Note: the caller must ensure that the delegate lives throughout the instance's lifetime.
     */
    CameraAvSettingsUserLevelMgmtServerLogic(EndpointId aEndpointId,
                                             BitFlags<CameraAvSettingsUserLevelManagement::Feature> aFeatures, uint8_t aMaxPresets);
    ~CameraAvSettingsUserLevelMgmtServerLogic();

    void SetDelegate(CameraAvSettingsUserLevelManagementDelegate * delegate)
    {
        mDelegate = delegate;
        if (mDelegate == nullptr)
        {
            ChipLogError(Zcl, "CameraAVSettingsUserLevelManagement: Trying to set delegate to null");
        }
    }

    EndpointId mEndpointId = kInvalidEndpointId;

    BitFlags<CameraAvSettingsUserLevelManagement::Feature> mFeatures;

    // Next available preset ID
    uint8_t mCurrentPresetID = 1;

    // My known values for MPTZ.
    CameraAvSettingsUserLevelManagement::Structs::MPTZStruct::Type mMptzPosition;

    // Note, where assigned, these are the extreme ends of the spec defined range (or a default if there is one), potentially
    // overwritten by the delegate. Exception is MaxPresets that is an F quality attribute and assigned by the constructor
    const uint8_t mMaxPresets = 5;
    int16_t mPanMin           = CameraAvSettingsUserLevelManagement::kPanMinMinValue;
    int16_t mPanMax           = CameraAvSettingsUserLevelManagement::kPanMaxMaxValue;
    int16_t mTiltMin          = CameraAvSettingsUserLevelManagement::kTiltMinMinValue;
    int16_t mTiltMax          = CameraAvSettingsUserLevelManagement::kTiltMaxMaxValue;
    uint8_t mZoomMax          = CameraAvSettingsUserLevelManagement::kZoomMaxMaxValue;

    CameraAvSettingsUserLevelManagement::PhysicalMovementEnum mMovementState;

    std::vector<CameraAvSettingsUserLevelManagement::MPTZPresetHelper> mMptzPresetHelpers;
    std::vector<CameraAvSettingsUserLevelManagement::Structs::DPTZStruct::Type> mDptzStreams;

    CHIP_ERROR Init() { return CHIP_NO_ERROR; }

    CHIP_ERROR Startup();

    // Handle any dynamic cleanup required prior to the destructor being called on an app shutdown.  To be invoked by
    // an app as part of its own shutdown sequence and prior to the destruction of the app/delegate.
    void Shutdown();

    bool HasFeature(CameraAvSettingsUserLevelManagement::Feature aFeature) const;

    // Returns the commands accepted depending on the Feature Flags that are set
    CHIP_ERROR AcceptedCommands(ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder);

    // Returns supported depending on the Feature Flags that are set
    CHIP_ERROR Attributes(ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder);

    // Attribute Accessors and Mutators
    CHIP_ERROR SetTiltMin(int16_t aTiltMin);

    CHIP_ERROR SetTiltMax(int16_t aTiltMax);

    CHIP_ERROR SetPanMin(int16_t aPanMin);

    CHIP_ERROR SetPanMax(int16_t aPanMax);

    CHIP_ERROR SetZoomMax(uint8_t aZoomMax);

    const CameraAvSettingsUserLevelManagement::Structs::MPTZStruct::Type & GetMptzPosition() const { return mMptzPosition; }

    uint8_t GetMaxPresets() const { return mMaxPresets; }

    const std::vector<CameraAvSettingsUserLevelManagement::Structs::DPTZStruct::Type> GetDptzRelativeMove() const
    {
        return mDptzStreams;
    }

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

    // Command handlers
    std::optional<DataModel::ActionReturnStatus>
    HandleMPTZSetPosition(CommandHandler & handler, const ConcreteCommandPath & commandPath,
                          const CameraAvSettingsUserLevelManagement::Commands::MPTZSetPosition::DecodableType & commandData);
    std::optional<DataModel::ActionReturnStatus>
    HandleMPTZRelativeMove(CommandHandler & handler, const ConcreteCommandPath & commandPath,
                           const CameraAvSettingsUserLevelManagement::Commands::MPTZRelativeMove::DecodableType & commandData);
    std::optional<DataModel::ActionReturnStatus>
    HandleMPTZMoveToPreset(CommandHandler & handler, const ConcreteCommandPath & commandPath,
                           const CameraAvSettingsUserLevelManagement::Commands::MPTZMoveToPreset::DecodableType & commandData);
    std::optional<DataModel::ActionReturnStatus>
    HandleMPTZSavePreset(CommandHandler & handler, const ConcreteCommandPath & commandPath,
                         const CameraAvSettingsUserLevelManagement::Commands::MPTZSavePreset::DecodableType & commandData);
    std::optional<DataModel::ActionReturnStatus>
    HandleMPTZRemovePreset(CommandHandler & handler, const ConcreteCommandPath & commandPath,
                           const CameraAvSettingsUserLevelManagement::Commands::MPTZRemovePreset::DecodableType & commandData);
    std::optional<DataModel::ActionReturnStatus>
    HandleDPTZSetViewport(CommandHandler & handler, const ConcreteCommandPath & commandPath,
                          const CameraAvSettingsUserLevelManagement::Commands::DPTZSetViewport::DecodableType & commandData);
    std::optional<DataModel::ActionReturnStatus>
    HandleDPTZRelativeMove(CommandHandler & handler, const ConcreteCommandPath & commandPath,
                           const CameraAvSettingsUserLevelManagement::Commands::DPTZRelativeMove::DecodableType & commandData);

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

    // Physical PTZ Interface
    void OnPhysicalMovementComplete(Protocols::InteractionModel::Status status) override;

    // Is a command already being processed
    bool IsMoving() const { return mMovementState == CameraAvSettingsUserLevelManagement::PhysicalMovementEnum::kMoving; }

private:
    CameraAvSettingsUserLevelManagementDelegate * mDelegate = nullptr;

    // Holding variables for values subject to successful physical movement
    Optional<int16_t> mTargetPan;
    Optional<int16_t> mTargetTilt;
    Optional<uint8_t> mTargetZoom;

    // Helper Read functions for complex attribute types
    CHIP_ERROR ReadAndEncodeMPTZPresets(AttributeValueEncoder & encoder);
    CHIP_ERROR ReadAndEncodeDPTZStreams(AttributeValueEncoder & encoder);

    CHIP_ERROR StoreMPTZPosition(const CameraAvSettingsUserLevelManagement::Structs::MPTZStruct::Type & mptzPosition);
    CHIP_ERROR LoadMPTZPosition(CameraAvSettingsUserLevelManagement::Structs::MPTZStruct::Type & mptzPosition);

    /**
     * Helper function that loads all the persistent attributes from the KVS.
     */
    void LoadPersistentAttributes();

    /**
     * Helper function that manages preset IDs
     */
    void UpdatePresetID();

    /**
     * Helper function that validates whether a given video stream ID is already known
     */
    bool KnownVideoStreamID(uint16_t aVideoStreamID);

    /**
     * Mutator for MovementState, only accessible by the server instance
     */
    void SetMovementState(CameraAvSettingsUserLevelManagement::PhysicalMovementEnum aMovementState);
    /**
     * Helper function for attribute handlers to mark the attribute as dirty
     */
    void MarkDirty(AttributeId aAttributeId);
};

} // namespace Clusters
} // namespace app
} // namespace chip

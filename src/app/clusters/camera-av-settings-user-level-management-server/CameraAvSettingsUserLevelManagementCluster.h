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

#include <app/clusters/camera-av-settings-user-level-management-server/CameraAvSettingsUserLevelManagementConstants.h>
#include <app/clusters/camera-av-settings-user-level-management-server/CameraAvSettingsUserLevelManagementLogic.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <protocols/interaction_model/StatusCode.h>
#include <string>
#include <vector>

namespace chip {
namespace app {
namespace Clusters {

constexpr size_t kMptzPositionStructMaxSerializedSize =
    TLV::EstimateStructOverhead(sizeof(int16_t), sizeof(int16_t), sizeof(uint8_t));

class CameraAvSettingsUserLevelManagementCluster;

/** @brief
 *  Defines interfaces for implementing application-specific logic for various aspects of the CameraAvUserSettingsManagement
 * Cluster. Specifically, it defines interfaces for the interaction with manual and digital pan, tilt, and zoom functions.
 */
class CameraAvSettingsUserLevelManagementDelegate
{
public:
    CameraAvSettingsUserLevelManagementDelegate()          = default;
    virtual ~CameraAvSettingsUserLevelManagementDelegate() = default;

    /**
     * Allows the delegate to perform any specific functions such as timer cancellation on a shutdown, this is invoked prior to
     * the destructor, it shall not be invoked as part of the destructor.
     */
    virtual void ShutdownApp() = 0;

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
     * reception of an MPTZSetPosition command. Returns a failure status if the physical device cannot realize these values.  The
     * app shall not block on actual physical execution of the command, rather return success on initiation of the movement.  On
     * conclusion of the movement the app shall invoke the provided callback, at which time the server will update the server held
     * attribute values for PTZ, if the motion succeeded.
     * @param aPan  The validated value of the pan that is to be set
     * @param aTilt The validated value of the tilt that is to be set
     * @param aZoom The validated value of the zoom that is to be set
     * @param callback The callback to be invoked once the physical movement of the camera has completed.
     *                 It is the delegate's responsibility to ensure liveness of this server cluster instance before invocation of
     * the callback, which needs to take place in the Matter threading context.
     */
    virtual Protocols::InteractionModel::Status
    MPTZSetPosition(Optional<int16_t> aPan, Optional<int16_t> aTilt, Optional<uint8_t> aZoom,
                    CameraAvSettingsUserLevelManagement::PhysicalPTZCallback * callback) = 0;

    /**
     * Allows any needed app handling given provided and already validated pan, tilt, and zoom values that are to be set based on
     * reception of an MPTZRelativeMove command.  The server has already validated the received relative values, and provides the
     * app with the new, requested settings for PTZ. Returns a failure status if the physical device cannot realize these values.
     * The app shall not block on actual physical execution of the command, rather return success on initiation of the movement.  On
     * conclusion of the movement the app shall invoke the provided callback, at which time the server will update the server held
     * attribute values for PTZ, if the motion succeeded.
     * @param aPan  The validated value of the pan that is to be set
     * @param aTilt The validated value of the tilt that is to be set
     * @param aZoom The validated value of the zoom that is to be set
     * @param callback The callback to be invoked once the physical movement of the camera has completed.
     *                 It is the delegate's responsibility to ensure liveness of this server cluster instance before invocation of
     * the callback. which needs to take place in the Matter threading context.
     */
    virtual Protocols::InteractionModel::Status
    MPTZRelativeMove(Optional<int16_t> aPan, Optional<int16_t> aTilt, Optional<uint8_t> aZoom,
                     CameraAvSettingsUserLevelManagement::PhysicalPTZCallback * callback) = 0;

    /**
     * Allows any needed app handling given provided and already validated pan, tilt, and zoom values that are to be set based on
     * reception of an MPTZMoveToPreset command.  The server has already ensured the requested preset ID exists, and obtained the
     * values for PTZ defined by that preset. Returns a failure status if the physical device cannot realize these values. The app
     * shall not block on actual physical execution of the command, rather return success on initiation of the movement.  On
     * conclusion of the movement the app shall invoke the provided callback, at which time the server will update the server held
     * attribute values for PTZ assuming success. Within the provided callback the server will update the server held attribute
     * values for PTZ, if the motion succeeded.
     * @param aPreset The preset ID associated with the provided PTZ values
     * @param aPan    The value for Pan associated with the preset
     * @param aTilt   The value for Tilt associated with the preset
     * @param aZoom   The value for Zoom associated with the preset
     * @param callback The callback to be invoked once the physical movement of the camera has completed.
     *                 it is the delegate's responsibility to ensure liveness of this server cluster instance before invocation of
     * the callback. which needs to take place in the Matter threading context.
     */
    virtual Protocols::InteractionModel::Status
    MPTZMoveToPreset(uint8_t aPreset, Optional<int16_t> aPan, Optional<int16_t> aTilt, Optional<uint8_t> aZoom,
                     CameraAvSettingsUserLevelManagement::PhysicalPTZCallback * callback) = 0;

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
    virtual CHIP_ERROR LoadMPTZPresets(std::vector<CameraAvSettingsUserLevelManagement::MPTZPresetHelper> & mptzPresetHelpers) = 0;
    virtual CHIP_ERROR
    LoadDPTZStreams(std::vector<CameraAvSettingsUserLevelManagement::Structs::DPTZStruct::Type> & dptzStreams) = 0;

private:
    friend class CameraAvSettingsUserLevelManagementCluster;

    CameraAvSettingsUserLevelManagementCluster * mServer = nullptr;

    // Sets the Server pointer
    void SetServer(CameraAvSettingsUserLevelManagementCluster * aServer) { mServer = aServer; }

protected:
    CameraAvSettingsUserLevelManagementCluster * GetServer() const { return mServer; }
};

class CameraAvSettingsUserLevelManagementCluster : public DefaultServerCluster
{
public:
    /**
     * Creates a server instance. The Init() function needs to be called for this instance to be registered and
     * called by the interaction model at the appropriate times.
     * @param aEndpointId       The endpoint on which this cluster exists. This must match the zap configuration.
     * @param aFeatures         The bitflags value that identifies which features are supported by this instance.

     * Note: the caller must ensure that the delegate lives throughout the instance's lifetime.
     */
    CameraAvSettingsUserLevelManagementCluster(EndpointId aEndpointId,
                                               BitFlags<CameraAvSettingsUserLevelManagement::Feature> aFeatures,
                                               uint8_t aMaxPresets) :
        DefaultServerCluster({ aEndpointId, CameraAvSettingsUserLevelManagement::Id }),
        mLogic(aEndpointId, aFeatures, aMaxPresets)
    {}

    CameraAvSettingsUserLevelMgmtServerLogic & GetLogic() { return mLogic; }

    void SetDelegate(CameraAvSettingsUserLevelManagementDelegate * delegate)
    {
        mLogic.SetDelegate(delegate);
        if (delegate != nullptr)
        {
            delegate->SetServer(this);
        }
    }

    CHIP_ERROR Init() { return mLogic.Init(); }

    CHIP_ERROR Startup(ServerClusterContext & context) override;

    // Handle any dynamic cleanup required prior to the destructor being called on an app shutdown.  To be invoked by
    // an app as part of its own shutdown sequence and prior to the destruction of the app/delegate.
    void Shutdown(ClusterShutdownType shutdownType) override
    {
        DefaultServerCluster::Shutdown(shutdownType);
        mLogic.Shutdown();
    }

    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override;

    std::optional<DataModel::ActionReturnStatus> InvokeCommand(const DataModel::InvokeRequest & request,
                                                               chip::TLV::TLVReader & input_arguments,
                                                               CommandHandler * handler) override;

    CHIP_ERROR AcceptedCommands(const ConcreteClusterPath & path,
                                ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder) override;

    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) override;

    // Attribute Mutators
    CHIP_ERROR SetTiltMin(int16_t aTiltMin);

    CHIP_ERROR SetTiltMax(int16_t aTiltMax);

    CHIP_ERROR SetPanMin(int16_t aPanMin);

    CHIP_ERROR SetPanMax(int16_t aPanMax);

    CHIP_ERROR SetZoomMax(uint8_t aZoomMax);

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

private:
    CameraAvSettingsUserLevelMgmtServerLogic mLogic;
    BitFlags<CameraAvSettingsUserLevelManagement::Feature> mFeatures;

    std::vector<CameraAvSettingsUserLevelManagement::MPTZPresetHelper> mMptzPresetHelpers;
    std::vector<CameraAvSettingsUserLevelManagement::Structs::DPTZStruct::Type> mDptzStreams;

    // Holding variables for values subject to successful physical movement
    Optional<int16_t> mTargetPan;
    Optional<int16_t> mTargetTilt;
    Optional<uint8_t> mTargetZoom;

    // Helper Read functions for complex attribute types
    CHIP_ERROR ReadAndEncodeMPTZPresets(AttributeValueEncoder & encoder);
    CHIP_ERROR ReadAndEncodeDPTZStreams(AttributeValueEncoder & encoder);
};

} // namespace Clusters
} // namespace app
} // namespace chip

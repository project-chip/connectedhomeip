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

namespace chip {
namespace app {
namespace Clusters {
namespace CameraAvSettingsUserLevelManagement {


/** @brief
 *  Defines interfaces for implementing application-specific logic for various aspects of the CameraAvUserSettingsManagement Cluster.
 *  Specifically, it defines interfaces for the interaction with manual and digital pan, tilt, and zoom functions.
 */
class Delegate
{
public:
    Delegate() = default;
    virtual ~Delegate() = default;

    /**
     *  @brief Callback into the delegate once persistent attributes managed by
     *  the Cluster have been loaded from Storage.
     */
    virtual Protocols::InteractionModel::Status PersistentAttributesLoadedCallback() = 0;

    /**
     * delegate command handlers
     */
    virtual Protocols::InteractionModel::Status MPTZSetPosition() = 0;
    virtual Protocols::InteractionModel::Status MPTZRelativeMove() = 0;
    virtual Protocols::InteractionModel::Status MPTZMoveToPreset() = 0;
    virtual Protocols::InteractionModel::Status MPTZSavePreset() = 0;
    virtual Protocols::InteractionModel::Status MPTZRemovePreset() = 0;
    virtual Protocols::InteractionModel::Status DPTZSetViewport() = 0;
    virtual Protocols::InteractionModel::Status DPTZRelativeMove() = 0;

protected:
    friend class CameraAvSettingsUserLevelMgmtServer;

    CameraAvSettingsUserLevelMgmtServer * mServer = nullptr;

    // sets the Server pointer
    void SetServer(CameraAvSettingsUserLevelMgmtServer * aServer) { mServer = aServer; }
    CameraAvSettingsUserLevelMgmtServer * GetServer() const { return mServer; }
};

enum class OptionalAttributes : uint32_t
{
    kMptzPosition      = 0x0001,
    kMaxPresets        = 0x0002,
    kMptzPresets       = 0x0004,
    kDptzRelativeMove  = 0x0008,
    kZoomMax           = 0x0010,
    kTltMin            = 0x0020,
    kTiltMax           = 0x0040,
    kPanMin            = 0x0080,
    kPanMax            = 0x0100,
};

class CameraAvSettingsUserLevelMgmtServer : public CommandHandlerInterface, public AttributeAccessInterface
{
public:
    /**
     * Creates a chime server instance. The Init() function needs to be called for this instance to be registered and
     * called by the interaction model at the appropriate times.
     * @param aEndpointId The endpoint on which this cluster exists. This must match the zap configuration.
     * @param aDelegate A reference to the delegate to be used by this server.
     * Note: the caller must ensure that the delegate lives throughout the instance's lifetime.
     */
    CameraAvSettingsUserLevelMgmtServer(EndpointId endpointId, Delegate & delegate);
    ~CameraAvSettingsUserLevelMgmtServer();

    CHIP_ERROR Init();
    void Shutdown();

    bool HasFeature(Feature aFeature) const;

    bool SupportsOptAttr(OptionalAttributes aOptionalAttrs) const;

    // None of the Attributes are Settable
    // Attribute Getters
    const MPTZStruct & GetMptzPosition() const { return mMaxMptzPosition; }

    uint8_t GetMaxPresets() const { return mMaxPresets; }

    const std::vector<MPTZPresetStruct> & GetMptzPresets() const { return mMptzPresets; }

    const std::vector<videoStreamId> GetDptzRelativeMove() const { return mDptzRelativeMove; }

    uint8_t GetZoomMax() const { return mMZoomMax; }

    int16_t GetTiltMin() const { return mTiltMin; }

    int16_t GetTiltMax() const { return mTiltMax; }

    int16_t GetPanMin() const { return mPanMin; }

    int16_t GetPanMax() const { return mPanMax; }

    EndpointId GetEndpointId() { return AttributeAccessInterface::GetEndpointId().Value(); }

private:
    CameraAVUserSettingsMgmtDelegate & mDelegate;
    EndpointId mEndpointId;
    BitMask<Feature> mFeature;
    BitMask<OptionalAttributes> mOptionalAttrs;

    // Attributes local storage
    const MPTZStruct mMaxMptzPosition;
    const uint8_t mMaxPresets;
    const std::vector<MPTZPresetStruct> mMptzPresets;
    const std::vector<videoStreamId> mDptzRelativeMove;
    const uint8_t mMZoomMax;
    const int16_t mTiltMin;
    const int16_t mTiltMax;
    const int16_t mPanMin;
    const int16_t mPanMax;

    // Attribute handler interface
    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;

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
};

} // namespace CameraAvSettingsUserLevelManagement
} // namespace Clusters
} // namespace app
} // namespace chip

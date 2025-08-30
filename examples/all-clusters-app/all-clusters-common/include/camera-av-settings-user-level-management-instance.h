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

#include <app/clusters/camera-av-settings-user-level-management-server/camera-av-settings-user-level-management-server.h>
#include <protocols/interaction_model/StatusCode.h>

namespace chip {
namespace app {
namespace Clusters {
namespace CameraAvSettingsUserLevelManagement {

class AVSettingsUserLevelManagementDelegate : public Delegate
{
public:
    AVSettingsUserLevelManagementDelegate()  = default;
    ~AVSettingsUserLevelManagementDelegate() = default;

    bool CanChangeMPTZ() override;

    CHIP_ERROR LoadMPTZPresets(std::vector<MPTZPresetHelper> & mptzPresetHelpers) override;
    CHIP_ERROR LoadDPTZStreams(std::vector<Structs::DPTZStruct::Type> & dptzStreams) override;
    CHIP_ERROR PersistentAttributesLoadedCallback() override;

    virtual void VideoStreamAllocated(uint16_t aStreamID) override;
    virtual void VideoStreamDeallocated(uint16_t aStreamID) override;
    virtual void DefaultViewportUpdated(Globals::Structs::ViewportStruct::Type aViewport) override;

    /**
     * delegate command handlers
     */
    Protocols::InteractionModel::Status MPTZSetPosition(Optional<int16_t> aPan, Optional<int16_t> aTilt,
                                                        Optional<uint8_t> aZoom) override;
    Protocols::InteractionModel::Status MPTZRelativeMove(Optional<int16_t> aPan, Optional<int16_t> aTilt,
                                                         Optional<uint8_t> aZoom) override;
    Protocols::InteractionModel::Status MPTZMoveToPreset(uint8_t aPreset, Optional<int16_t> aPan, Optional<int16_t> aTilt,
                                                         Optional<uint8_t> aZoom) override;
    Protocols::InteractionModel::Status MPTZSavePreset(uint8_t aPreset) override;
    Protocols::InteractionModel::Status MPTZRemovePreset(uint8_t aPreset) override;
    Protocols::InteractionModel::Status DPTZSetViewport(uint16_t aVideoStreamID,
                                                        Globals::Structs::ViewportStruct::Type aViewport) override;
    Protocols::InteractionModel::Status DPTZRelativeMove(uint16_t aVideoStreamID, Optional<int16_t> aDeltaX,
                                                         Optional<int16_t> aDeltaY, Optional<int8_t> aZoomDelta,
                                                         Globals::Structs::ViewportStruct::Type & aViewport) override;
};

void Shutdown();

} // namespace CameraAvSettingsUserLevelManagement
} // namespace Clusters
} // namespace app
} // namespace chip

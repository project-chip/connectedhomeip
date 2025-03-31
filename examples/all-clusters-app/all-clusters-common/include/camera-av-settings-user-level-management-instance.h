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
    AVSettingsUserLevelManagementDelegate(){};

    /**
     *  @brief Callback into the delegate once persistent attributes managed by
     *  the Cluster have been loaded from Storage.
     */
    Protocols::InteractionModel::Status PersistentAttributesLoadedCallback() override;

    bool CanChangeMPTZ() override;
    bool IsValidVideoStreamID(uint16_t videoStreamID) override;

    /**
     * delegate command handlers
     */
    Protocols::InteractionModel::Status MPTZSetPosition(Optional<int16_t> pan, Optional<int16_t> tilt,
                                                        Optional<int8_t> zoom) override;
    Protocols::InteractionModel::Status MPTZRelativeMove(Optional<int16_t> pan, Optional<int16_t> tilt,
                                                         Optional<int8_t> zoom) override;
    Protocols::InteractionModel::Status MPTZMoveToPreset(uint8_t preset, Optional<int16_t> pan, Optional<int16_t> tilt,
                                                         Optional<int8_t> zoom) override;
    Protocols::InteractionModel::Status MPTZSavePreset(uint8_t preset) override;
    Protocols::InteractionModel::Status MPTZRemovePreset(uint8_t preset) override;
    Protocols::InteractionModel::Status DPTZSetViewport(uint16_t videoStreamID, Structs::ViewportStruct::Type viewport) override;
    Protocols::InteractionModel::Status DPTZRelativeMove(uint16_t videoStreamID, Optional<int16_t> deltaX, Optional<int16_t> deltaY,
                                                         Optional<int8_t> zoomDelta) override;
};

CameraAvSettingsUserLevelMgmtServer * GetInstance();

void Shutdown();

} // namespace CameraAvSettingsUserLevelManagement
} // namespace Clusters
} // namespace app
} // namespace chip

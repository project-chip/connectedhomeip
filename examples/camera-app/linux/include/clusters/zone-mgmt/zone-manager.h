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

#include "camera-device-interface.h"
#include <app/clusters/zone-management-server/zone-management-server.h>
#include <app/util/config.h>
#include <vector>

constexpr uint16_t kInvalidZoneID = 65500;
namespace chip {
namespace app {
namespace Clusters {
namespace ZoneManagement {

/**
 * The application delegate to define the options & implement commands.
 */
class ZoneManager : public ZoneMgmtDelegate
{
public:
    Protocols::InteractionModel::Status CreateTwoDCartesianZone(const TwoDCartesianZoneDecodableStruct & zone,
                                                                uint16_t & outZoneID) override;

    Protocols::InteractionModel::Status UpdateTwoDCartesianZone(uint16_t zoneID,
                                                                const TwoDCartesianZoneDecodableStruct & zone) override;

    Protocols::InteractionModel::Status RemoveZone(uint16_t zoneID) override;

    Protocols::InteractionModel::Status CreateOrUpdateTrigger(const ZoneTriggerControlStruct & zoneTrigger) override;

    Protocols::InteractionModel::Status RemoveTrigger(uint16_t zoneID) override;

    void OnAttributeChanged(AttributeId attributeId) override;

    ZoneManager()  = default;
    ~ZoneManager() = default;

    void SetCameraDevice(CameraDeviceInterface * aCameraDevice);

private:
    CameraDeviceInterface * mCameraDevice = nullptr;
};

} // namespace ZoneManagement
} // namespace Clusters
} // namespace app
} // namespace chip

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

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <lib/support/logging/CHIPLogging.h>
#include <zone-manager.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::DataModel;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ZoneManagement;
using namespace chip::app::Clusters::ZoneManagement::Attributes;
using chip::Protocols::InteractionModel::Status;

void ZoneManager::SetCameraDevice(CameraDeviceInterface * aCameraDevice)
{
    mCameraDevice = aCameraDevice;
}

Protocols::InteractionModel::Status ZoneManager::CreateTwoDCartesianZone(const TwoDCartesianZoneStruct & zone, uint16_t & outZoneID)
{
    outZoneID = kInvalidZoneID;

    return Status::Success;
}

Protocols::InteractionModel::Status ZoneManager::UpdateTwoDCartesianZone(uint16_t zoneID, const TwoDCartesianZoneStruct & zone)
{

    return Status::Success;
}

Protocols::InteractionModel::Status ZoneManager::GetTwoDCartesianZone(const DataModel::Nullable<uint16_t> zoneID,
                                                                      const std::vector<TwoDCartesianZoneStruct> & outZones)
{

    return Status::Success;
}

Protocols::InteractionModel::Status ZoneManager::RemoveZone(uint16_t zoneID)
{

    return Status::Success;
}

Protocols::InteractionModel::Status ZoneManager::CreateOrUpdateTrigger(const ZoneTriggerControlStruct & zoneTrigger)
{

    return Status::Success;
}

Protocols::InteractionModel::Status ZoneManager::RemoveTrigger(uint16_t zoneID)
{

    return Status::Success;
}

void ZoneManager::OnAttributeChanged(AttributeId attributeId)
{
    ChipLogProgress(Camera, "Attribute changed for AttributeId = " ChipLogFormatMEI, ChipLogValueMEI(attributeId));

    switch (attributeId)
    {
    case Sensitivity::Id: {
        break;
    }
    default:
        ChipLogProgress(Camera, "Unknown Attribute with AttributeId = " ChipLogFormatMEI, ChipLogValueMEI(attributeId));
    }
}

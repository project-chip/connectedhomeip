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

#include "camera-device-interface.h"
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

Protocols::InteractionModel::Status ZoneManager::CreateTwoDCartesianZone(const TwoDCartesianZoneStorage & zone,
                                                                         uint16_t & outZoneID)
{
    TwoDCartZone twoDCartZone;
    outZoneID           = GetNewZoneId();
    twoDCartZone.zoneId = outZoneID;
    twoDCartZone.zone   = zone;

    mTwoDCartZones.push_back(twoDCartZone);

    return Status::Success;
}

Protocols::InteractionModel::Status ZoneManager::UpdateTwoDCartesianZone(uint16_t zoneID, const TwoDCartesianZoneStorage & zone)
{
    // Find an iterator to the item with the matching ID
    auto it = std::find_if(mTwoDCartZones.begin(), mTwoDCartZones.end(),
                           [zoneID](const TwoDCartZone & zone) { return zone.zoneId == zoneID; });

    // If an item with the zoneID was found
    if (it != mTwoDCartZones.end())
    {
        TwoDCartZone twoDCartZone;
        twoDCartZone.zoneId = zoneID;
        twoDCartZone.zone   = zone;
        *it                 = twoDCartZone; // Replace the found item with the newItem

        return Status::Success; // Indicate success
    }

    return Status::NotFound;
}

Protocols::InteractionModel::Status ZoneManager::RemoveZone(uint16_t zoneID)
{
    mTwoDCartZones.erase(std::remove_if(mTwoDCartZones.begin(), mTwoDCartZones.end(),
                                        [&](const TwoDCartZone & zone) { return zone.zoneId == zoneID; }),
                         mTwoDCartZones.end());

    return Status::Success;
}

Protocols::InteractionModel::Status ZoneManager::CreateTrigger(const ZoneTriggerControlStruct & zoneTrigger)
{
    if (mCameraDevice->GetCameraHALInterface().CreateZoneTrigger(zoneTrigger) == CameraError::SUCCESS)
    {
        return Status::Success;
    }
    else
    {
        return Status::Failure;
    }
}

Protocols::InteractionModel::Status ZoneManager::UpdateTrigger(const ZoneTriggerControlStruct & zoneTrigger)
{
    if (mCameraDevice->GetCameraHALInterface().UpdateZoneTrigger(zoneTrigger) == CameraError::SUCCESS)
    {
        return Status::Success;
    }
    else
    {
        return Status::Failure;
    }
}

Protocols::InteractionModel::Status ZoneManager::RemoveTrigger(uint16_t zoneID)
{
    if (mCameraDevice->GetCameraHALInterface().RemoveZoneTrigger(zoneID) == CameraError::SUCCESS)
    {
        return Status::Success;
    }
    else
    {
        return Status::Failure;
    }
}

CHIP_ERROR ZoneManager::LoadZones(std::vector<ZoneInformationStorage> & aZones)
{
    aZones.clear();

    return CHIP_NO_ERROR;
}

CHIP_ERROR ZoneManager::LoadTriggers(std::vector<ZoneTriggerControlStruct> & aTriggers)
{
    aTriggers.clear();

    return CHIP_NO_ERROR;
}

CHIP_ERROR ZoneManager::PersistentAttributesLoadedCallback()
{
    ChipLogError(Camera, "Persistent attributes loaded");

    return CHIP_NO_ERROR;
}

void ZoneManager::OnAttributeChanged(AttributeId attributeId)
{
    ChipLogProgress(Camera, "Attribute changed for AttributeId = " ChipLogFormatMEI, ChipLogValueMEI(attributeId));

    switch (attributeId)
    {
    case Sensitivity::Id: {
        mCameraDevice->GetCameraHALInterface().SetDetectionSensitivity(GetZoneMgmtServer()->GetSensitivity());
        break;
    }
    default:
        ChipLogProgress(Camera, "Unknown Attribute with AttributeId = " ChipLogFormatMEI, ChipLogValueMEI(attributeId));
    }
}

void ZoneManager::OnZoneTriggeredEvent(uint16_t zoneId,
                                       chip::app::Clusters::ZoneManagement::ZoneEventTriggeredReasonEnum triggerReason)
{
    ChipLogProgress(Camera, "Generating ZoneTriggered event for ZoneId = %u", zoneId);
    GetZoneMgmtServer()->GenerateZoneTriggeredEvent(zoneId, triggerReason);
}

void ZoneManager::OnZoneStoppedEvent(uint16_t zoneId, chip::app::Clusters::ZoneManagement::ZoneEventStoppedReasonEnum stopReason)
{
    ChipLogProgress(Camera, "Generating ZoneStopped event for ZoneId = %u", zoneId);
    GetZoneMgmtServer()->GenerateZoneStoppedEvent(zoneId, stopReason);
}

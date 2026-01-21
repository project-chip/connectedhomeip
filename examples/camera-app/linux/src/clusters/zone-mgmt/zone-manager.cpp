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
#include "camera-device.h"
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CHIPDeviceLayer.h>
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
    ChipLogProgress(Camera, "Persistent attributes loaded");

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

void ZoneManager::OnZoneTriggerTimeout(chip::System::Layer * systemLayer, void * appState)
{
    ZoneManager * zoneManager = reinterpret_cast<ZoneManager *>(appState);
    VerifyOrReturn(zoneManager != nullptr, ChipLogError(Camera, "OnZoneTriggerTimeout: context is null"));

    for (auto trigCtxtIter = zoneManager->mTriggerContexts.begin(); trigCtxtIter != zoneManager->mTriggerContexts.end();)
    {
        if (trigCtxtIter->triggerState == TriggerState::InBlindDuration)
        {
            trigCtxtIter->remainingBlindDuration--;

            if (trigCtxtIter->remainingBlindDuration > 0)
            {
                trigCtxtIter++;
            }
            else
            {
                // Remove the trigger context after the expiry of the
                // blindDuration
                trigCtxtIter = zoneManager->mTriggerContexts.erase(trigCtxtIter);
            }

            continue;
        }

        // Advance time since initial trigger by the timer timeout period
        trigCtxtIter->timeSinceInitialTrigger += kTimerPeriod;

        if (trigCtxtIter->timeSinceInitialTrigger > trigCtxtIter->triggerDetectedDuration)
        {
            // Emit ZoneStopped with reason ActionStopped
            ChipLogProgress(Camera, "Generating ZoneStopped event for ZoneId = %u with reason:kActionStopped",
                            trigCtxtIter->triggerCtrl.zoneID);
            zoneManager->GetZoneMgmtServer()->GenerateZoneStoppedEvent(trigCtxtIter->triggerCtrl.zoneID,
                                                                       ZoneEventStoppedReasonEnum::kActionStopped);
            // Set the triggerState to BlindDuration
            trigCtxtIter->triggerState = TriggerState::InBlindDuration;
        }
        else if (trigCtxtIter->timeSinceInitialTrigger > trigCtxtIter->triggerCtrl.maxDuration)
        {
            // Emit ZoneStopped with reason Timeout
            ChipLogProgress(Camera, "Generating ZoneStopped event for ZoneId = %u with reason:kTimeout",
                            trigCtxtIter->triggerCtrl.zoneID);
            zoneManager->GetZoneMgmtServer()->GenerateZoneStoppedEvent(trigCtxtIter->triggerCtrl.zoneID,
                                                                       ZoneEventStoppedReasonEnum::kTimeout);
            // Set the triggerState to BlindDuration
            trigCtxtIter->triggerState = TriggerState::InBlindDuration;
        }
        else
        {
            trigCtxtIter++;
        }
    }
    if (!zoneManager->mTriggerContexts.empty())
    {
        // Start the timer again if there are active triggers
        TEMPORARY_RETURN_IGNORED DeviceLayer::SystemLayer().StartTimer(System::Clock::Seconds32(kTimerPeriod), OnZoneTriggerTimeout,
                                                                       zoneManager);
    }
}

void ZoneManager::OnZoneTriggeredEvent(uint16_t zoneId,
                                       chip::app::Clusters::ZoneManagement::ZoneEventTriggeredReasonEnum triggerReason)
{
    // Ensure that a trigger exists for the zoneId
    auto trigger = GetZoneMgmtServer()->GetTriggerForZone(zoneId);
    VerifyOrReturn(trigger.HasValue(), ChipLogError(Camera, "Trigger not found for ZoneId"));

    ChipLogProgress(Camera, "Zone activity detected for ZoneId = %u", zoneId);
    auto foundTrigCtxt = std::find_if(mTriggerContexts.begin(), mTriggerContexts.end(),
                                      [&](const ZoneTriggerContext & trigCtxt) { return trigCtxt.triggerCtrl.zoneID == zoneId; });

    // If an item with the zoneID was not found, then this is the first trigger
    if (foundTrigCtxt == mTriggerContexts.end())
    {
        // Generate the event for the initial trigger
        ChipLogProgress(Camera, "Generating ZoneTriggered event for ZoneId = %u", zoneId);
        GetZoneMgmtServer()->GenerateZoneTriggeredEvent(zoneId, triggerReason);

        ZoneTriggerContext trigCtxt;
        trigCtxt.triggerState = TriggerState::Triggered;
        // Set the TriggerDetectedDuration = InitialDuration
        trigCtxt.triggerDetectedDuration     = trigger.Value().initialDuration;
        trigCtxt.prevTriggerDetectedDuration = trigCtxt.triggerDetectedDuration;
        trigCtxt.triggerCtrl                 = trigger.Value();
        trigCtxt.remainingBlindDuration      = trigger.Value().blindDuration;
        trigCtxt.triggerCount                = 1;
        mTriggerContexts.push_back(trigCtxt);

        // Schedule the periodic timer
        TEMPORARY_RETURN_IGNORED DeviceLayer::SystemLayer().StartTimer(System::Clock::Seconds32(kTimerPeriod), OnZoneTriggerTimeout,
                                                                       this);
    }
    else
    {
        // Nothing to do for zone if it is in blindDuration
        VerifyOrReturn(foundTrigCtxt->triggerState != TriggerState::InBlindDuration,
                       ChipLogProgress(Camera, "Ignoring ZoneTriggered event for zone in BlindDuration"));

        // Zone has already been triggered at least once.
        foundTrigCtxt->triggerCount++;
        // Spec logic for advancing triggerDetectedDuration
        if (foundTrigCtxt->triggerCount <= 2 || foundTrigCtxt->timeSinceInitialTrigger > foundTrigCtxt->prevTriggerDetectedDuration)
        {
            ChipLogProgress(Camera, "Trigger detected: Advancing TriggerDetectedDuration for ZoneId = %u, count = %u", zoneId,
                            foundTrigCtxt->triggerCount);
            // First increase of triggerDetectedDuration or
            // timeSinceInitialTrigger is greater than previous
            // triggerDetectedDuration; Advance the triggerDetectedDuration
            foundTrigCtxt->prevTriggerDetectedDuration = foundTrigCtxt->triggerDetectedDuration;
            foundTrigCtxt->triggerDetectedDuration += foundTrigCtxt->triggerCtrl.augmentationDuration;
            ChipLogProgress(Camera, "prev = %u, current = %u", foundTrigCtxt->prevTriggerDetectedDuration,
                            foundTrigCtxt->triggerDetectedDuration);
        }
        else
        {
            ChipLogProgress(Camera, "Trigger detected for ZoneId = %u, but ignored. Count = %u", zoneId,
                            foundTrigCtxt->triggerCount);
        }
    }
}

void ZoneManager::OnZoneStoppedEvent(uint16_t zoneId, chip::app::Clusters::ZoneManagement::ZoneEventStoppedReasonEnum stopReason)
{
    ChipLogProgress(Camera, "Generating ZoneStopped event for ZoneId = %u", zoneId);
    GetZoneMgmtServer()->GenerateZoneStoppedEvent(zoneId, stopReason);
}

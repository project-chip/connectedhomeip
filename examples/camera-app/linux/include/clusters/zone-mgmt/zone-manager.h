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
constexpr uint8_t kTimerPeriod    = 1; // 1 second periodicity for checking event
                                       // trigger parameters.
namespace chip {
namespace app {
namespace Clusters {
namespace ZoneManagement {

struct TwoDCartZone
{
    uint16_t zoneId;
    TwoDCartesianZoneStorage zone;
};

/**
 * The application delegate to define the options & implement commands.
 */
class ZoneManager : public Delegate, public CameraDeviceInterface::CameraHALInterface::ZoneEventCallback
{
public:
    Protocols::InteractionModel::Status CreateTwoDCartesianZone(const TwoDCartesianZoneStorage & zone,
                                                                uint16_t & outZoneID) override;

    Protocols::InteractionModel::Status UpdateTwoDCartesianZone(uint16_t zoneID, const TwoDCartesianZoneStorage & zone) override;

    Protocols::InteractionModel::Status RemoveZone(uint16_t zoneID) override;

    Protocols::InteractionModel::Status CreateTrigger(const ZoneTriggerControlStruct & zoneTrigger) override;

    Protocols::InteractionModel::Status UpdateTrigger(const ZoneTriggerControlStruct & zoneTrigger) override;

    Protocols::InteractionModel::Status RemoveTrigger(uint16_t zoneID) override;

    void OnAttributeChanged(AttributeId attributeId) override;

    CHIP_ERROR LoadZones(std::vector<ZoneInformationStorage> & aZones) override;

    CHIP_ERROR LoadTriggers(std::vector<ZoneTriggerControlStruct> & aTriggers) override;

    CHIP_ERROR PersistentAttributesLoadedCallback() override;

    void OnZoneTriggeredEvent(uint16_t zoneId,
                              chip::app::Clusters::ZoneManagement::ZoneEventTriggeredReasonEnum triggerReason) override;

    void OnZoneStoppedEvent(uint16_t zoneId, chip::app::Clusters::ZoneManagement::ZoneEventStoppedReasonEnum stopReason) override;

    ZoneManager()  = default;
    ~ZoneManager() = default;

    void SetCameraDevice(CameraDeviceInterface * aCameraDevice);

private:
    enum class TriggerState : uint8_t
    {
        Idle,            ///< Default state
        Triggered,       ///< ZoneTrigger event generated
        InBlindDuration, ///< In Blind duration state after ZoneStopped event
    };

    struct ZoneTriggerContext
    {
        uint32_t timeSinceInitialTrigger     = 0;
        uint32_t triggerDetectedDuration     = 0;
        uint32_t prevTriggerDetectedDuration = 0;
        TriggerState triggerState            = TriggerState::Idle;
        uint16_t triggerCount                = 0;
        uint16_t remainingBlindDuration      = 0;
        ZoneTriggerControlStruct triggerCtrl;
    };

    CameraDeviceInterface * mCameraDevice = nullptr;
    // Activity zones;
    std::vector<TwoDCartZone> mTwoDCartZones;
    std::vector<ZoneTriggerContext> mTriggerContexts;

    static void OnZoneTriggerTimeout(chip::System::Layer * systemLayer, void * appState);

    uint16_t GetNewZoneId()
    {
        // TODO: Replace with a better algo; Maybe use a std::set
        uint16_t newId = 1;
        while (true)
        {
            bool idExists = false;
            for (const auto & zone : mTwoDCartZones)
            {
                if (zone.zoneId == newId)
                {
                    idExists = true;
                    break;
                }
            }
            if (!idExists)
            {
                return newId;
            }
            newId++;
        }
    }
};

} // namespace ZoneManagement
} // namespace Clusters
} // namespace app
} // namespace chip

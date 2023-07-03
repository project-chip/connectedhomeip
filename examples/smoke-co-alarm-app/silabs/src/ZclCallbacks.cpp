/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

/**
 * @file
 *   This file implements the handler for data model messages.
 */

#include "AppConfig.h"
#include "SmokeCoAlarmManager.h"

#include "SilabsTestEventTriggerDelegate.h"
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/ConcreteAttributePath.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace ::chip;
using namespace ::chip::app::Clusters;

void MatterPostAttributeChangeCallback(const chip::app::ConcreteAttributePath & attributePath, uint8_t type, uint16_t size,
                                       uint8_t * value)
{
    ClusterId clusterId     = attributePath.mClusterId;
    AttributeId attributeId = attributePath.mAttributeId;
    ChipLogProgress(Zcl, "Cluster callback: " ChipLogFormatMEI, ChipLogValueMEI(clusterId));

    if (clusterId == SmokeCoAlarm::Id && attributeId == SmokeCoAlarm::Attributes::ExpressedState::Id)
    {
        static_assert(sizeof(SmokeCoAlarm::ExpressedStateEnum) == 1, "Wrong size");
        SmokeCoAlarm::ExpressedStateEnum expressedState = *(reinterpret_cast<SmokeCoAlarm::ExpressedStateEnum *>(value));
        ChipLogProgress(Zcl, "Smoke CO Alarm cluster: " ChipLogFormatMEI " state %d", ChipLogValueMEI(clusterId),
                        to_underlying(expressedState));
    }
}

/** @brief Smoke CO Alarm Cluster Init
 *
 * This function is called when a specific cluster is initialized. It gives the
 * application an opportunity to take care of cluster initialization procedures.
 * It is called exactly once for each endpoint where cluster is present.
 *
 * @param endpoint   Ver.: always
 *
 */
void emberAfSmokeCoAlarmClusterInitCallback(EndpointId endpoint)
{
    // TODO: implement any additional Cluster Server init actions
}

bool emberAfPluginSmokeCoAlarmSelfTestRequestCommand(EndpointId endpointId)
{
    return AlarmMgr().StartSelfTesting();
}

bool emberAfHandleEventTrigger(uint64_t eventTrigger)
{
    bool success = false;

    switch (eventTrigger)
    {
    case 0xffffffff00000090:
        success = SmokeCoAlarmServer::Instance().SetSmokeState(1, SmokeCoAlarm::AlarmStateEnum::kWarning);
        if (success)
        {
            success = AlarmMgr().SetExpressedState(1, SmokeCoAlarm::ExpressedStateEnum::kSmokeAlarm, true);
        }
        break;

    case 0xffffffff000000a0:
        success = SmokeCoAlarmServer::Instance().SetSmokeState(1, SmokeCoAlarm::AlarmStateEnum::kNormal);
        if (success)
        {
            success = AlarmMgr().SetExpressedState(1, SmokeCoAlarm::ExpressedStateEnum::kSmokeAlarm, false);
        }
        break;

    case 0xffffffff00000091:
        success = SmokeCoAlarmServer::Instance().SetCOState(1, SmokeCoAlarm::AlarmStateEnum::kWarning);
        if (success)
        {
            success = AlarmMgr().SetExpressedState(1, SmokeCoAlarm::ExpressedStateEnum::kCOAlarm, true);
        }
        break;

    case 0xffffffff000000a1:
        success = SmokeCoAlarmServer::Instance().SetCOState(1, SmokeCoAlarm::AlarmStateEnum::kNormal);
        if (success)
        {
            success = AlarmMgr().SetExpressedState(1, SmokeCoAlarm::ExpressedStateEnum::kCOAlarm, false);
        }
        break;

    case 0xffffffff00000095:
        success = SmokeCoAlarmServer::Instance().SetBatteryAlert(1, SmokeCoAlarm::AlarmStateEnum::kWarning);
        if (success)
        {
            success = AlarmMgr().SetExpressedState(1, SmokeCoAlarm::ExpressedStateEnum::kBatteryAlert, true);
        }
        break;

    case 0xffffffff000000a5:
        success = SmokeCoAlarmServer::Instance().SetBatteryAlert(1, SmokeCoAlarm::AlarmStateEnum::kNormal);
        if (success)
        {
            success = AlarmMgr().SetExpressedState(1, SmokeCoAlarm::ExpressedStateEnum::kBatteryAlert, false);
        }
        break;

    case 0xffffffff00000093:
        success = SmokeCoAlarmServer::Instance().SetHardwareFaultAlert(1, true);
        if (success)
        {
            success = AlarmMgr().SetExpressedState(1, SmokeCoAlarm::ExpressedStateEnum::kHardwareFault, true);
        }
        break;

    case 0xffffffff000000a3:
        success = SmokeCoAlarmServer::Instance().SetHardwareFaultAlert(1, false);
        if (success)
        {
            success = AlarmMgr().SetExpressedState(1, SmokeCoAlarm::ExpressedStateEnum::kHardwareFault, false);
        }
        break;

    case 0xffffffff0000009a:
        success = SmokeCoAlarmServer::Instance().SetEndOfServiceAlert(1, SmokeCoAlarm::EndOfServiceEnum::kExpired);
        if (success)
        {
            success = AlarmMgr().SetExpressedState(1, SmokeCoAlarm::ExpressedStateEnum::kEndOfService, true);
        }
        break;

    case 0xffffffff000000aa:
        success = SmokeCoAlarmServer::Instance().SetEndOfServiceAlert(1, SmokeCoAlarm::EndOfServiceEnum::kNormal);
        if (success)
        {
            success = AlarmMgr().SetExpressedState(1, SmokeCoAlarm::ExpressedStateEnum::kEndOfService, false);
        }
        break;

    case 0xffffffff0000009b:
        success = SmokeCoAlarmServer::Instance().SetDeviceMuted(1, SmokeCoAlarm::MuteStateEnum::kMuted);
        break;

    case 0xffffffff000000ab:
        success = SmokeCoAlarmServer::Instance().SetDeviceMuted(1, SmokeCoAlarm::MuteStateEnum::kNotMuted);
        break;

    case 0xffffffff00000092:
        success = SmokeCoAlarmServer::Instance().SetInterconnectSmokeAlarm(1, SmokeCoAlarm::AlarmStateEnum::kWarning);
        if (success)
        {
            success = AlarmMgr().SetExpressedState(1, SmokeCoAlarm::ExpressedStateEnum::kInterconnectSmoke, true);
        }
        break;

    case 0xffffffff000000a2:
        success = SmokeCoAlarmServer::Instance().SetInterconnectSmokeAlarm(1, SmokeCoAlarm::AlarmStateEnum::kNormal);
        if (success)
        {
            success = AlarmMgr().SetExpressedState(1, SmokeCoAlarm::ExpressedStateEnum::kInterconnectSmoke, false);
        }
        break;

    case 0xffffffff00000094:
        success = SmokeCoAlarmServer::Instance().SetInterconnectCOAlarm(1, SmokeCoAlarm::AlarmStateEnum::kWarning);
        if (success)
        {
            success = AlarmMgr().SetExpressedState(1, SmokeCoAlarm::ExpressedStateEnum::kInterconnectCO, true);
        }
        break;

    case 0xffffffff000000a4:
        success = SmokeCoAlarmServer::Instance().SetInterconnectCOAlarm(1, SmokeCoAlarm::AlarmStateEnum::kNormal);
        if (success)
        {
            success = AlarmMgr().SetExpressedState(1, SmokeCoAlarm::ExpressedStateEnum::kInterconnectCO, false);
        }
        break;

    case 0xffffffff00000096:
        success = SmokeCoAlarmServer::Instance().SetContaminationState(1, SmokeCoAlarm::ContaminationStateEnum::kWarning);
        break;

    case 0xffffffff00000097:
        success = SmokeCoAlarmServer::Instance().SetContaminationState(1, SmokeCoAlarm::ContaminationStateEnum::kLow);
        break;

    case 0xffffffff000000a6:
        success = SmokeCoAlarmServer::Instance().SetContaminationState(1, SmokeCoAlarm::ContaminationStateEnum::kNormal);
        break;

    case 0xffffffff00000098:
        success = SmokeCoAlarmServer::Instance().SetSensitivityLevel(1, SmokeCoAlarm::SensitivityEnum::kHigh);
        break;

    case 0xffffffff00000099:
        success = SmokeCoAlarmServer::Instance().SetSensitivityLevel(1, SmokeCoAlarm::SensitivityEnum::kLow);
        break;

    case 0xffffffff000000a8:
        success = SmokeCoAlarmServer::Instance().SetSensitivityLevel(1, SmokeCoAlarm::SensitivityEnum::kStandard);
        break;

    default:
        break;
    }

    return success;
}

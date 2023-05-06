/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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
#include <app/CommandHandler.h>
#include <app/ConcreteCommandPath.h>
#include <app/util/af-types.h>
#include <app/util/basic-types.h>
#include <platform/CHIPDeviceConfig.h>

/**
 * @brief Smoke CO Alarm Server Plugin class
 */
class SmokeCoAlarmServer
{
public:
    /**********************************************************
     * Functions Definitions
     *********************************************************/

    static SmokeCoAlarmServer & Instance();

    EmberAfStatus selfTestRequestCommand(chip::app::CommandHandler * commandObj,
                                         const chip::app::ConcreteCommandPath & commandPath);
    void initSmokeCoAlarmServer(chip::EndpointId endpoint);

    EmberAfStatus getExpressedState(chip::EndpointId endpoint,
                                    chip::app::Clusters::SmokeCoAlarm::ExpressedStateEnum * currentValue);
    EmberAfStatus setExpressedState(chip::EndpointId endpoint, chip::app::Clusters::SmokeCoAlarm::ExpressedStateEnum newValue);
    EmberAfStatus getSmokeState(chip::EndpointId endpoint, chip::app::Clusters::SmokeCoAlarm::AlarmStateEnum * currentValue);
    EmberAfStatus setSmokeState(chip::EndpointId endpoint, chip::app::Clusters::SmokeCoAlarm::AlarmStateEnum newValue);
    EmberAfStatus getCOState(chip::EndpointId endpoint, chip::app::Clusters::SmokeCoAlarm::AlarmStateEnum * currentValue);
    EmberAfStatus setCOState(chip::EndpointId endpoint, chip::app::Clusters::SmokeCoAlarm::AlarmStateEnum newValue);
    EmberAfStatus getBatteryAlert(chip::EndpointId endpoint, chip::app::Clusters::SmokeCoAlarm::AlarmStateEnum * currentValue);
    EmberAfStatus setBatteryAlert(chip::EndpointId endpoint, chip::app::Clusters::SmokeCoAlarm::AlarmStateEnum newValue);
    EmberAfStatus getDeviceMuted(chip::EndpointId endpoint, chip::app::Clusters::SmokeCoAlarm::MuteStateEnum * currentValue);
    EmberAfStatus setDeviceMuted(chip::EndpointId endpoint, chip::app::Clusters::SmokeCoAlarm::MuteStateEnum newValue);
    EmberAfStatus getTestInProgress(chip::EndpointId endpoint, bool * currentValue);
    EmberAfStatus setTestInProgress(chip::EndpointId endpoint, bool newValue);
    EmberAfStatus getHardwareFaultAlert(chip::EndpointId endpoint, bool * currentValue);
    EmberAfStatus setHardwareFaultAlert(chip::EndpointId endpoint, bool newValue);
    EmberAfStatus getEndOfServiceAlert(chip::EndpointId endpoint,
                                       chip::app::Clusters::SmokeCoAlarm::EndOfServiceEnum * currentValue);
    EmberAfStatus setEndOfServiceAlert(chip::EndpointId endpoint, chip::app::Clusters::SmokeCoAlarm::EndOfServiceEnum newValue);
    EmberAfStatus getInterconnectSmokeAlarm(chip::EndpointId endpoint,
                                            chip::app::Clusters::SmokeCoAlarm::AlarmStateEnum * currentValue);
    EmberAfStatus setInterconnectSmokeAlarm(chip::EndpointId endpoint, chip::app::Clusters::SmokeCoAlarm::AlarmStateEnum newValue);
    EmberAfStatus getInterconnectCOAlarm(chip::EndpointId endpoint,
                                         chip::app::Clusters::SmokeCoAlarm::AlarmStateEnum * currentValue);
    EmberAfStatus setInterconnectCOAlarm(chip::EndpointId endpoint, chip::app::Clusters::SmokeCoAlarm::AlarmStateEnum newValue);
    EmberAfStatus getContaminationState(chip::EndpointId endpoint,
                                        chip::app::Clusters::SmokeCoAlarm::ContaminationStateEnum * currentValue);
    EmberAfStatus setContaminationState(chip::EndpointId endpoint,
                                        chip::app::Clusters::SmokeCoAlarm::ContaminationStateEnum newValue);
    EmberAfStatus getSensitivityLevel(chip::EndpointId endpoint, chip::app::Clusters::SmokeCoAlarm::SensitivityEnum * currentValue);
    EmberAfStatus setSensitivityLevel(chip::EndpointId endpoint, chip::app::Clusters::SmokeCoAlarm::SensitivityEnum newValue);
    EmberAfStatus getExpiryDate(chip::EndpointId endpoint, uint32_t * currentValue);
    EmberAfStatus setExpiryDate(chip::EndpointId endpoint, uint32_t newValue);

    bool sendSmokeAlarmEvent(chip::EndpointId endpoint);
    bool sendCOAlarmEvent(chip::EndpointId endpoint);
    bool sendLowBatteryEvent(chip::EndpointId endpoint);
    bool sendHardwareFaultEvent(chip::EndpointId endpoint);
    bool sendEndOfServiceEvent(chip::EndpointId endpoint);
    bool sendSelfTestCompleteEvent(chip::EndpointId endpoint);
    bool sendAlarmMutedEvent(chip::EndpointId endpoint);
    bool sendMuteEndedEvent(chip::EndpointId endpoint);
    bool sendInterconnectSmokeAlarmEvent(chip::EndpointId endpoint);
    bool sendInterconnectCOAlarmEvent(chip::EndpointId endpoint);
    bool sendAllClearEvent(chip::EndpointId endpoint);

private:
    /**********************************************************
     * Functions Definitions
     *********************************************************/

    static SmokeCoAlarmServer instance;
};

/**********************************************************
 * Global
 *********************************************************/

/**********************************************************
 * Callbacks
 *********************************************************/

/** @brief Smoke CO Alarm Cluster Server Post Init
 *
 * Following resolution of the Smoke CO Alarm state at startup for this endpoint,
 * perform any additional initialization needed; e.g., synchronize hardware
 * state.
 *
 * @param endpoint Endpoint that is being initialized  Ver.: always
 */
void emberAfPluginSmokeCoAlarmClusterServerPostInitCallback(chip::EndpointId endpoint);

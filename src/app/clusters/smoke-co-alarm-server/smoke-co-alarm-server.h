/**
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
 *   APIs and defines for the Smoke CO Alarm Server plugin.
 *
 */

#pragma once

#include <app-common/zap-generated/cluster-objects.h>
#include <app/CommandHandler.h>
#include <app/ConcreteCommandPath.h>
#include <app/util/af.h>
#include <app/util/config.h>
#include <protocols/interaction_model/StatusCode.h>

/**
 * @brief Smoke CO Alarm Server Plugin class
 */
class SmokeCoAlarmServer
{
public:
    static SmokeCoAlarmServer & Instance();

    using AlarmStateEnum         = chip::app::Clusters::SmokeCoAlarm::AlarmStateEnum;
    using ContaminationStateEnum = chip::app::Clusters::SmokeCoAlarm::ContaminationStateEnum;
    using EndOfServiceEnum       = chip::app::Clusters::SmokeCoAlarm::EndOfServiceEnum;
    using ExpressedStateEnum     = chip::app::Clusters::SmokeCoAlarm::ExpressedStateEnum;
    using Feature                = chip::app::Clusters::SmokeCoAlarm::Feature;
    using MuteStateEnum          = chip::app::Clusters::SmokeCoAlarm::MuteStateEnum;
    using SensitivityEnum        = chip::app::Clusters::SmokeCoAlarm::SensitivityEnum;

    /**
     * For all the functions below, the return value is true on success, false on failure
     */

    /**
     * @brief Updates the expressed state with new value
     *
     * @note If the value of ExpressedState is not Normal, the attribute corresponding to the value should not be Normal.
     *
     * @param endpointId ID of the endpoint
     * @param newExpressedState new expressed state
     * @return true on success, false on failure
     */
    bool SetExpressedState(chip::EndpointId endpointId, ExpressedStateEnum newExpressedState);

    bool SetSmokeState(chip::EndpointId endpointId, AlarmStateEnum newSmokeState);
    bool SetCOState(chip::EndpointId endpointId, AlarmStateEnum newCOState);
    bool SetBatteryAlert(chip::EndpointId endpointId, AlarmStateEnum newBatteryAlert);
    bool SetDeviceMuted(chip::EndpointId endpointId, MuteStateEnum newDeviceMuted);
    bool SetTestInProgress(chip::EndpointId endpointId, bool newTestInProgress);
    bool SetHardwareFaultAlert(chip::EndpointId endpointId, bool newHardwareFaultAlert);
    bool SetEndOfServiceAlert(chip::EndpointId endpointId, EndOfServiceEnum newEndOfServiceAlert);
    bool SetInterconnectSmokeAlarm(chip::EndpointId endpointId, AlarmStateEnum newInterconnectSmokeAlarm);
    bool SetInterconnectCOAlarm(chip::EndpointId endpointId, AlarmStateEnum newInterconnectCOAlarm);
    bool SetContaminationState(chip::EndpointId endpointId, ContaminationStateEnum newContaminationState);
    bool SetSensitivityLevel(chip::EndpointId endpointId, SensitivityEnum newSensitivityLevel);
    // bool SetExpiryDate(chip::EndpointId endpointId, Date newExpiryDate); // TODO: Date type encoding not defined

    bool GetExpressedState(chip::EndpointId endpointId, ExpressedStateEnum & expressedState);
    bool GetSmokeState(chip::EndpointId endpointId, AlarmStateEnum & smokeState);
    bool GetCOState(chip::EndpointId endpointId, AlarmStateEnum & coState);
    bool GetBatteryAlert(chip::EndpointId endpointId, AlarmStateEnum & batteryAlert);
    bool GetDeviceMuted(chip::EndpointId endpointId, MuteStateEnum & deviceMuted);
    bool GetTestInProgress(chip::EndpointId endpointId, bool & testInProgress);
    bool GetHardwareFaultAlert(chip::EndpointId endpointId, bool & hardwareFaultAlert);
    bool GetEndOfServiceAlert(chip::EndpointId endpointId, EndOfServiceEnum & endOfServiceAlert);
    bool GetInterconnectSmokeAlarm(chip::EndpointId endpointId, AlarmStateEnum & interconnectSmokeAlarm);
    bool GetInterconnectCOAlarm(chip::EndpointId endpointId, AlarmStateEnum & interconnectCOAlarm);
    bool GetContaminationState(chip::EndpointId endpointId, ContaminationStateEnum & contaminationState);
    bool GetSensitivityLevel(chip::EndpointId endpointId, SensitivityEnum & sensitivityLevel);
    // bool GetExpiryDate(chip::EndpointId endpointId, Date & expiryDate); // TODO: Date type encoding not defined

    chip::BitFlags<Feature> GetFeatures(chip::EndpointId endpointId);

    inline bool SupportsSmokeAlarm(chip::EndpointId endpointId) { return GetFeatures(endpointId).Has(Feature::kSmokeAlarm); }

    inline bool SupportsCOAlarm(chip::EndpointId endpointId) { return GetFeatures(endpointId).Has(Feature::kCoAlarm); }

private:
    /**
     * @brief Common handler for SelfTestRequest commands
     *
     * @param commandObj    original command context
     * @param commandPath   original command path
     * @return true         if successful
     * @return false        if error happened
     */
    bool HandleRemoteSelfTestRequest(chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath);

    /**
     * @brief Send generic event
     *
     * @tparam T            Any event type supported by Matter
     * @param endpointId    endpoint where SmokeCoAlarmServer is running
     * @param event         event object built by caller
     */
    template <typename T>
    void SendEvent(chip::EndpointId endpointId, T & event);

    /**
     * @brief Get generic attribute value
     *
     * @tparam T            attribute value type
     * @param endpointId    endpoint where SmokeCoAlarmServer is running
     * @param attributeId   attribute Id (used for logging only)
     * @param getFn         attribute getter function as defined in <Accessors.h>
     * @param value         actual attribute value on success
     * @return true         on success (value is set to the actual attribute value)
     * @return false        if attribute reading failed (value is kept unchanged)
     */
    template <typename T>
    bool GetAttribute(chip::EndpointId endpointId, chip::AttributeId attributeId,
                      EmberAfStatus (*getFn)(chip::EndpointId endpointId, T * value), T & value) const;

    /**
     * @brief Set generic attribute value
     *
     * @tparam T            attribute value type
     * @param endpointId    endpoint where SmokeCoAlarmServer is running
     * @param attributeId   attribute Id (used for logging only)
     * @param setFn         attribute setter function as defined in <Accessors.h>
     * @param value         new attribute value
     * @return true         on success
     * @return false        if attribute writing failed
     */
    template <typename T>
    bool SetAttribute(chip::EndpointId endpointId, chip::AttributeId attributeId,
                      EmberAfStatus (*setFn)(chip::EndpointId endpointId, T value), T value);

    friend bool emberAfSmokeCoAlarmClusterSelfTestRequestCallback(
        chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
        const chip::app::Clusters::SmokeCoAlarm::Commands::SelfTestRequest::DecodableType & commandData);

    static SmokeCoAlarmServer sInstance;
};

// =============================================================================
// Plugin callbacks that are called by cluster server and should be implemented
// by the server app
// =============================================================================

/**
 * @brief User handler for SelfTestRequest command (server)
 *
 * @note The application must set the ExpressedState to "Testing"
 *
 * @param endpointId endpoint for which SelfTestRequest command is called
 *
 * @retval true on success
 * @retval false if error happened (err should be set to appropriate error code)
 */
bool emberAfPluginSmokeCoAlarmSelfTestRequestCommand(chip::EndpointId endpointId);

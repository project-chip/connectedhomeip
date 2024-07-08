/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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
#include <app/clusters/mode-base-server/mode-base-server.h>
#include <app/clusters/water-heater-management-server/water-heater-management-server.h>

#include <protocols/interaction_model/StatusCode.h>

namespace chip {
namespace app {
namespace Clusters {
namespace WaterHeaterManagement {

using ModeTagStructType = detail::Structs::ModeTagStruct::Type;

class WhmManufacturer;

// This is an application level delegate to handle operational state commands according to the specific business logic.
class WaterHeaterManagementDelegate : public WaterHeaterManagement::Delegate, public ModeBase::Delegate
{
public:
    WaterHeaterManagementDelegate(EndpointId clustersEndpoint);

    virtual ~WaterHeaterManagementDelegate() = default;

    void SetWaterHeaterManagementInstance(WaterHeaterManagement::Instance & instance);

    void SetWhmManufacturer(WhmManufacturer & whmManufacturer);

    /*********************************************************************************
     *
     * Methods implementing the WaterHeaterManagement::Delegate interace
     *
     *********************************************************************************/

    /**
     * @brief Delegate should implement a handler to start boosting the water temperature as required.
     *        Upon receipt, the Water Heater SHALL transition into the BOOST state, which SHALL cause the water in the tank (or the
     * TargetPercentage of the water, if included) to be heated towards the set point (or the TemporarySetpoint, if included), which
     * in turn may cause a call for heat, even if the mode is OFF, or is TIMED and it is during one of the Off periods.
     *
     * @param duration  Indicates the time period in seconds for which the BOOST state is activated before it automatically reverts
     * to the previous mode (e.g. OFF, MANUAL or TIMED).
     * @param oneShot  Indicates whether the BOOST state should be automatically canceled once the hot water has first reached the
     * set point temperature (or the TemporarySetpoint temperature, if specified) for the TargetPercentage (if specified).
     * @param emergencyBoost  Indicates that the consumer wants the water to be heated as quickly as practicable. This MAY cause
     * multiple heat sources to be activated (e.g. a heat pump and direct electric heating element).
     * @param temporarySetpoint  Indicates the target temperature to which to heat the hot water for this Boost command. It SHALL be
     * used instead of the normal set point temperature whilst the BOOST state is active.
     * @param targetPercentage  If the tank supports the TankPercent feature, this field indicates the amount of water that SHALL be
     * heated by this Boost command before the heater is switched off.
     * @param targetReheat  If the tank supports the TankPercent feature, and the heating by this Boost command has ceased because
     * the TargetPercentage of the water in the tank has been heated to the set point (or TemporarySetpoint if included), this field
     * indicates the percentage to which the hot water in the tank SHALL be allowed to fall before again beginning to reheat it.
     *
     * @return  Success if the boost command is accepted; otherwise the command SHALL be rejected with appropriate error.
     */
    Protocols::InteractionModel::Status HandleBoost(uint32_t duration, Optional<bool> oneShot, Optional<bool> emergencyBoost,
                                                    Optional<int16_t> temporarySetpoint, Optional<chip::Percent> targetPercentage,
                                                    Optional<chip::Percent> targetReheat) override;

    /**
     * @brief Delegate should implement a handler to cancel a boost command.
     *        Upon receipt, the Water Heater SHALL transition back from the BOOST state to the previous mode (e.g. OFF, MANUAL or
     * TIMED).
     *
     * @return It should report SUCCESS if successful and FAILURE otherwise.
     */
    Protocols::InteractionModel::Status HandleCancelBoost() override;

    // ------------------------------------------------------------------
    // Get attribute methods
    BitMask<WaterHeaterTypeBitmap> GetHeaterTypes() override;
    BitMask<WaterHeaterDemandBitmap> GetHeatDemand() override;
    uint16_t GetTankVolume() override;
    int64_t GetEstimatedHeatRequired() override;
    Percent GetTankPercentage() override;
    BoostStateEnum GetBoostState() override;

    // ------------------------------------------------------------------
    // Set attribute methods
    void SetHeaterTypes(BitMask<WaterHeaterTypeBitmap> heaterTypes);
    void SetHeatDemand(BitMask<WaterHeaterDemandBitmap> heatDemand);
    void SetTankVolume(uint16_t tankVolume);
    void SetEstimatedHeatRequired(int64_t estimatedHeatRequired);
    void SetTankPercentage(Percent tankPercentage);
    void SetBoostState(BoostStateEnum boostState);

    /*********************************************************************************
     *
     * Methods implementing the ModeBase::Delegate interface
     *
     *********************************************************************************/

    CHIP_ERROR Init() override;

    /**
     * Handle application logic when the mode is changing.
     *
     * @param mode The new mode that the device is requested to transition to.
     * @param response A reference to a response that will be sent to the client. The contents of which con be modified by the
     * application.
     */
    void HandleChangeToMode(uint8_t mode, ModeBase::Commands::ChangeToModeResponse::Type & response) override;

    /**
     * Get the mode label of the Nth mode in the list of modes.
     *
     * @param modeIndex The index of the mode to be returned. It is assumed that modes are indexable from 0 and with no gaps.
     * @param label A reference to the mutable char span which will be mutated to receive the label on success. Use
     * CopyCharSpanToMutableCharSpan to copy into the MutableCharSpan.
     *
     * @return Returns a CHIP_NO_ERROR if there was no error and the label was returned successfully.
     * CHIP_ERROR_PROVIDER_LIST_EXHAUSTED if the modeIndex in beyond the list of available labels.
     */
    CHIP_ERROR GetModeLabelByIndex(uint8_t modeIndex, MutableCharSpan & label) override;

    /**
     * Get the mode value of the Nth mode in the list of modes.
     *
     * @param modeIndex The index of the mode to be returned. It is assumed that modes are indexable from 0 and with no gaps.
     * @param value a reference to the uint8_t variable that is to contain the mode value.
     *
     * @return Returns a CHIP_NO_ERROR if there was no error and the value was returned successfully.
     * CHIP_ERROR_PROVIDER_LIST_EXHAUSTED if the modeIndex in beyond the list of available values.
     */
    CHIP_ERROR GetModeValueByIndex(uint8_t modeIndex, uint8_t & value) override;

    /**
     * Get the mode tags of the Nth mode in the list of modes.
     * @param modeIndex The index of the mode to be returned. It is assumed that modes are indexable from 0 and with no gaps.
     * @param tags a reference to an existing and initialised buffer that is to contain the mode tags. std::copy can be used
     * to copy into the buffer.
     *
     * @return Returns a CHIP_NO_ERROR if there was no error and the mode tags were returned successfully.
     * CHIP_ERROR_PROVIDER_LIST_EXHAUSTED if the modeIndex in beyond the list of available mode tags.
     */
    CHIP_ERROR GetModeTagsByIndex(uint8_t modeIndex, DataModel::List<ModeTagStructType> & tags) override;

    /*********************************************************************************
     *
     * WaterHeaterManagementDelegate specific methods
     *
     *********************************************************************************/

    /**
     * @brief Set the Water Header Mode and act accordingly.
     *
     * @param mode  The Water Heater Mode (e.g. OFF, MANUAL or TIMED).
     */
    void SetWaterHeaterMode(uint8_t mode);

    /**
     * @brief Set the water temperature of the tank
     *
     * @param waterTemperature  The water temperature in 100th's Celsius
     */
    void SetWaterTemperature(uint16_t waterTemperature);

    /**
     * @brief Set the target water temperature of the tank
     *
     * @param targetWaterTemperature  The water temperature in 100th's Celsius
     */
    void SetTargetWaterTemperature(uint16_t targetWaterTemperature);

    /**
     * @brief Determine whether the heating sources need to be turned on or off
     */
    Protocols::InteractionModel::Status CheckIfHeatNeedsToBeTurnedOnOrOff();

    /**
     * @brief Static timer callback for when Boost timer expires.
     */
    static void BoostTimerExpiry(System::Layer * systemLayer, void * delegate);

    /**
     * @brief Object timer callback for when Boost timer expires.
     */
    void HandleBoostTimerExpiry();

    /**
     * Determines whether the tank water temperature has reached the target temperature.
     *
     * @return Returns True is tank water temperature has reached the target temperature, False otherwise.
     */
    bool HasWaterTemperatureReachedTarget() const;

    /**
     * Simulates water being drawn from the water tank.
     *
     * @param percentageReplaced  The % of water being replaced with water with a temperature of replacedWaterTemperature.
     * @param replacedWaterTemperature  The temperature of the percentageReplaced water.
     */
    void DrawOffHotWater(uint8_t percentageReplaced, uint16_t replacedWaterTemperature);

    /*********************************************************************************
     *
     * Public constants
     *
     *********************************************************************************/

    // The Water Header Modes

    // The device will not attempt to keep the water warm.
    static constexpr uint8_t ModeOff = 0;

    // The device will attempt to keep the water warm based on the OccupiedHeatingSetpoint attribute of the associated Thermostat
    // cluster.
    static constexpr uint8_t ModeManual = 1;

    // The device will attempt to keep the water warm based on the Schedules attribute of the associated Thermostat cluster.
    static constexpr uint8_t ModeTimed = 2;

private:
    /*********************************************************************************
     *
     * WaterHeaterManagementDelegate specific attributes
     *
     *********************************************************************************/

    // Need the following so can determine which features are supported
    WaterHeaterManagement::Instance * mpWhmInstance;

    // Pointer to the manufacturer specific object which understand the hardware
    WhmManufacturer * mpWhmManufacturer;

    // Target water temperature in 100ths of a C
    uint16_t mTargetWaterTemperature;

    // Actual water temperature in 100ths of a C
    uint16_t mHotWaterTemperature;

    // The % of hot water remaining at mHotWaterTemperature
    uint8_t mPercentageHotWater;

    // The (100 - mPercentageHotWater)% of water at mReplacedWaterTemperature
    uint16_t mReplacedWaterTemperature;

    // Boost command parameters

    // This field SHALL indicate whether the BOOST state should be automatically canceled once the hot water has first reached the
    // set point temperature (or the TemporarySetpoint temperature, if specified) for the TargetPercentage (if specified).
    Optional<bool> mBoostOneShot;

    // This field indicates that the consumer wants the water to be heated as quickly as practicable. This MAY cause multiple heat
    // sources to be activated (e.g. a heat pump and direct electric heating element).
    Optional<bool> mBoostEmergencyBoost;

    // This field indicates the target temperature to which to heat the hot water for this Boost command. It SHALL be used instead
    // of the normal set point temperature whilst the BOOST state is active.
    Optional<int16_t> mBoostTemporarySetpoint;

    // If the tank supports the TankPercent feature, this field indicates the amount of water that SHALL be heated by this Boost
    // command before the heater is switched off. This field is optional, however it SHALL be included if the TargetReheat field is
    // included.
    Optional<chip::Percent> mBoostTargetPercentage;

    // If the tank supports the TankPercent feature, and the heating by this Boost command has ceased because the TargetPercentage
    // of the water in the tank has been heated to the set point (or TemporarySetpoint if included), this field indicates the
    // percentage to which the hot water in the tank SHALL be allowed to fall before again beginning to reheat it. For example if
    // the TargetPercentage was 80%, and the TargetReheat was 40%, then after initial heating to 80% hot water, the tank may have
    // hot water drawn off until only 40% hot water remains. At this point the heater will begin to heat back up to 80% of hot
    // water. If this field and the OneShot field were both omitted, heating would begin again after any water draw which reduced
    // the TankPercentage below 80%.
    Optional<chip::Percent> mBoostTargetReheat;

    // Track whether the water temperature has reached the water temperature specified in the boost command. Used in conjunction
    // with the boost command boostTargetReheat parameter
    bool mBoostTargetTemperatureReached;

    /*********************************************************************************
     *
     * Member variables implementing the WaterHeaterManagement::Delegate interface
     *
     *********************************************************************************/

    // Access to the Water Heater Mode instance
    ModeBase::Instance mWaterHeaterModeInstance;

    // This attribute SHALL indicate the methods to call for heat that the controller supports. If a bit is set then the controller
    // supports the corresponding method.
    BitMask<WaterHeaterTypeBitmap> mHeaterTypes;

    // This attribute SHALL indicate if the controller is asking for heat. If a bit is set then the corresponding call for heat is
    // active.
    BitMask<WaterHeaterDemandBitmap> mHeatDemand;

    // This attribute SHALL indicate the volume of water that the hot water tank can hold (in units of Litres). This allows an
    // energy management system to estimate the required heating energy needed to reach the target temperature.
    uint16_t mTankVolume;

    // This attribute SHALL indicate the estimated heat energy needed to raise the water temperature to the target setpoint. This
    // can be computed by taking the specific heat capacity of water (4182 J/kg °C) and by knowing the current temperature of the
    // water, the tank volume and target temperature.
    int64_t mEstimatedHeatRequired;

    // This attribute SHALL indicate an approximate level of hot water stored in the tank, which may help consumers understand the
    // amount of hot water remaining in the tank.
    Percent mTankPercentage;

    // This attribute SHALL indicate if the BOOST state, as triggered by a Boost command, is currently active.
    BoostStateEnum mBoostState;

    /*********************************************************************************
     *
     * Member variables implementing the ModeBase::Delegate interface
     *
     *********************************************************************************/

    ModeTagStructType modeTagsOff[1]    = { { .value = to_underlying(WaterHeaterMode::ModeTag::kOff) } };
    ModeTagStructType modeTagsManual[1] = { { .value = to_underlying(WaterHeaterMode::ModeTag::kManual) } };
    ModeTagStructType modeTagsTimed[1]  = { { .value = to_underlying(WaterHeaterMode::ModeTag::kTimed) } };

    const detail::Structs::ModeOptionStruct::Type kModeOptions[3] = {
        detail::Structs::ModeOptionStruct::Type{ .label    = CharSpan::fromCharString("Off"),
                                                 .mode     = ModeOff,
                                                 .modeTags = DataModel::List<const ModeTagStructType>(modeTagsOff) },
        detail::Structs::ModeOptionStruct::Type{ .label    = CharSpan::fromCharString("Manual"),
                                                 .mode     = ModeManual,
                                                 .modeTags = DataModel::List<const ModeTagStructType>(modeTagsManual) },
        detail::Structs::ModeOptionStruct::Type{ .label    = CharSpan::fromCharString("Timed"),
                                                 .mode     = ModeTimed,
                                                 .modeTags = DataModel::List<const ModeTagStructType>(modeTagsTimed) }
    };
};

} // namespace WaterHeaterManagement
} // namespace Clusters
} // namespace app
} // namespace chip

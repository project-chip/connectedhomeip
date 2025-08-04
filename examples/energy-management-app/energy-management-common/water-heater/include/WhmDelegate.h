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
#include <app/clusters/water-heater-management-server/water-heater-management-server.h>

#include <protocols/interaction_model/StatusCode.h>

namespace chip {
namespace app {
namespace Clusters {
namespace WaterHeaterManagement {

class WhmManufacturer;

enum HeatingOp
{
    TurnHeatingOn,
    TurnHeatingOff,
    LeaveHeatingUnchanged
};

// This is an application level delegate to handle operational state commands according to the specific business logic.
class WaterHeaterManagementDelegate : public WaterHeaterManagement::Delegate
{
public:
    WaterHeaterManagementDelegate(EndpointId clustersEndpoint);

    virtual ~WaterHeaterManagementDelegate() = default;

    void SetWaterHeaterManagementInstance(WaterHeaterManagement::Instance & instance);

    void SetWhmManufacturer(WhmManufacturer & whmManufacturer);

    /*********************************************************************************
     *
     * Methods implementing the WaterHeaterManagement::Delegate interface
     *
     *********************************************************************************/

    /**
     * @brief Delegate should implement a handler to start boosting the water
     *        temperature as required. Upon receipt, the Water Heater SHALL
     *        transition into the BOOST state, which SHALL cause the water in
     *        the tank (or the TargetPercentage of the water, if included) to be
     *        heated towards the set point (or the TemporarySetpoint, if
     *        included), which in turn may cause a call for heat, even if the
     *        mode is OFF, or is TIMED and it is during one of the Off periods.
     *
     * @param duration           Indicates the time period in seconds for which
     *                           the BOOST state is activated before it
     *                           automatically reverts to the previous mode
     *                           (e.g. OFF, MANUAL or TIMED).
     *
     * @param oneShot            Indicates whether the BOOST state should be
     *                           automatically canceled once the hot water has
     *                           first reached the set point temperature (or the
     *                           TemporarySetpoint temperature, if specified)
     *                           for the TargetPercentage (if specified).
     *
     * @param emergencyBoost     Indicates that the consumer wants the water to
     *                           be heated as quickly as practicable. This MAY
     *                           cause multiple heat sources to be activated
     *                           (e.g. a heat pump and direct electric heating
     *                           element).
     *
     * @param temporarySetpoint  Indicates the target temperature to which to
     *                           heat the hot water for this Boost command. It
     *                           SHALL be used instead of the normal set point
     *                           temperature whilst the BOOST state is active.
     *
     * @param targetPercentage   If the tank supports the TankPercent feature,
     *                           this field indicates the amount of water that
     *                           SHALL be heated by this Boost command before
     *                           the heater is switched off.
     *
     * @param targetReheat       If the tank supports the TankPercent feature,
     *                           and the heating by this Boost command has
     *                           ceased because the TargetPercentage of the
     *                           water in the tank has been heated to the set
     *                           point (or TemporarySetpoint if included), this
     *                           field indicates the percentage to which the hot
     *                           water in the tank SHALL be allowed to fall
     *                           before again beginning to reheat it.
     *
     * @return  Success if the boost command is accepted; otherwise the command
     * SHALL be rejected with appropriate error.
     */
    Protocols::InteractionModel::Status HandleBoost(uint32_t duration, Optional<bool> oneShot, Optional<bool> emergencyBoost,
                                                    Optional<int16_t> temporarySetpoint, Optional<Percent> targetPercentage,
                                                    Optional<Percent> targetReheat) override;

    /**
     * @brief Delegate should implement a handler to cancel a boost command.
     *        Upon receipt, the Water Heater SHALL transition back from the
     *        BOOST state to the previous mode (e.g. OFF, MANUAL or TIMED).
     *
     * @return It should report SUCCESS if successful and FAILURE otherwise.
     */
    Protocols::InteractionModel::Status HandleCancelBoost() override;

    // ------------------------------------------------------------------
    // Get attribute methods
    BitMask<WaterHeaterHeatSourceBitmap> GetHeaterTypes() override;
    BitMask<WaterHeaterHeatSourceBitmap> GetHeatDemand() override;
    uint16_t GetTankVolume() override;
    Energy_mWh GetEstimatedHeatRequired() override;
    Percent GetTankPercentage() override;
    BoostStateEnum GetBoostState() override;

    // ------------------------------------------------------------------
    // Set attribute methods
    void SetHeaterTypes(BitMask<WaterHeaterHeatSourceBitmap> heaterTypes);
    void SetHeatDemand(BitMask<WaterHeaterHeatSourceBitmap> heatDemand);
    void SetTankVolume(uint16_t tankVolume);
    void SetEstimatedHeatRequired(Energy_mWh estimatedHeatRequired);
    void SetTankPercentage(Percent tankPercentage);
    void SetBoostState(BoostStateEnum boostState);

    /***************************************************************************
     *
     * WaterHeaterManagementDelegate specific methods
     *
     ***************************************************************************/

    /**
     * @brief Set the Water Header Mode and act accordingly.
     *
     * @param mode  The Water Heater Mode (e.g. OFF, MANUAL or TIMED).
     */
    Protocols::InteractionModel::Status SetWaterHeaterMode(uint8_t mode);

    /**
     * @brief Set the water temperature of the tank
     *
     * @param waterTemperature  The water temperature in 100th's Celsius
     */
    void SetWaterTemperature(int16_t waterTemperature);

    /**
     * @brief Set the target water temperature of the tank
     *
     * @param targetWaterTemperature  The water temperature in 100th's Celsius
     */
    void SetTargetWaterTemperature(int16_t targetWaterTemperature);

    /**
     * @brief Determine whether the heating sources need to be turned on or off or left unchanged.
     *
     * @return Success if the heating was successfully turned on or off or left unchanged otherwise an error
     *         code is returned if turning the heating on or off failed.
     */
    Protocols::InteractionModel::Status ChangeHeatingIfNecessary();

    /**
     * @brief Static timer callback for when Boost timer expires.
     */
    static void BoostTimerExpiry(System::Layer * systemLayer, void * delegate);

    /**
     * @brief Object timer callback for when Boost timer expires.
     */
    void HandleBoostTimerExpiry();

    /**
     * Determines whether the tank water temperature has reached the target
     * temperature.
     *
     * @return Returns True is tank water temperature has reached the target
     * temperature, False otherwise.
     */
    bool HasWaterTemperatureReachedTarget() const;

    /**
     * Simulates water being drawn from the water tank.
     *
     * @param percentageReplaced  The % of water being replaced with water with
     * a temperature of replacedWaterTemperature.
     *
     * @param replacedWaterTemperature  The temperature of the
     * percentageReplaced water.
     */
    void DrawOffHotWater(Percent percentageReplaced, int16_t replacedWaterTemperature);

    /**
     * Set the temperature of the cold water that fills the tank as the hot water
     * is drawn off.
     *
     * @param coldWaterTemperature  The cold water temperature in 100th of a C
     */
    void SetColdWaterTemperature(int16_t coldWaterTemperature);

private:
    /**
     * Return the target temperature.
     * If a boost command is in progress and has a mBoostTemporarySetpoint value use that as the
     * target temperature otherwise use the temperature set via SetTargetWaterTemperature().
     *
     * @return the target temperature
     */
    int16_t GetActiveTargetWaterTemperature() const;

    /**
     * @brief Calculate the percentage of the water in the tank at the target
     *        temperature.
     *
     * @return  Percentage of water at the target temperature
     */
    uint8_t CalculateTankPercentage() const;

    /**
     * @brief Determine whether heating needs to be turned on or off or left as
     * is.
     *
     * @param heatingOp[out]  Set as determined whether heating needs to be
     * turned on/off or left unchanged.
     *
     * @return  Success if the heating operation could be determined otherwise
     * returns with appropriate error.
     */
    Protocols::InteractionModel::Status DetermineIfChangingHeatingState(HeatingOp & heatingOp);

private:
    /***************************************************************************
     *
     * WaterHeaterManagementDelegate specific attributes
     *
     ***************************************************************************/

    // Need the following so can determine which features are supported
    WaterHeaterManagement::Instance * mpWhmInstance;

    // Pointer to the manufacturer specific object which understand the hardware
    WhmManufacturer * mpWhmManufacturer;

    // Target water temperature in 100ths of a C
    int16_t mTargetWaterTemperature;

    // Actual water temperature in 100ths of a C
    int16_t mWaterTemperature;

    // The cold water temperature in 100ths of a C
    int16_t mColdWaterTemperature;

    // Boost command parameters

    // This field SHALL indicate whether the BOOST state should be automatically
    // canceled once the hot water has first reached the set point temperature
    // (or the TemporarySetpoint temperature, if specified) for the
    // TargetPercentage (if specified).
    Optional<bool> mBoostOneShot;

    // This field indicates that the consumer wants the water to be heated as
    // quickly as practicable. This MAY cause multiple heat sources to be
    // activated (e.g. a heat pump and direct electric heating element).
    Optional<bool> mBoostEmergencyBoost;

    // This field indicates the target temperature to which to heat the hot
    // water for this Boost command. It SHALL be used instead of the normal set
    // point temperature whilst the BOOST state is active.
    Optional<int16_t> mBoostTemporarySetpoint;

    // If the tank supports the TankPercent feature, this field indicates the
    // amount of water that SHALL be heated by this Boost command before the
    // heater is switched off. This field is optional, however it SHALL be
    // included if the TargetReheat field is included.
    Optional<Percent> mBoostTargetPercentage;

    // If the tank supports the TankPercent feature, and the heating by this
    // Boost command has ceased because the TargetPercentage of the water in the
    // tank has been heated to the set point (or TemporarySetpoint if included),
    // this field indicates the percentage to which the hot water in the tank
    // SHALL be allowed to fall before again beginning to reheat it. For example
    // if the TargetPercentage was 80%, and the TargetReheat was 40%, then after
    // initial heating to 80% hot water, the tank may have hot water drawn off
    // until only 40% hot water remains. At this point the heater will begin to
    // heat back up to 80% of hot water. If this field and the OneShot field
    // were both omitted, heating would begin again after any water draw which
    // reduced the TankPercentage below 80%.
    Optional<Percent> mBoostTargetReheat;

    // Track whether the water temperature has reached the water temperature
    // specified in the boost command. Used in conjunction with the boost
    // command boostTargetReheat parameter
    bool mBoostTargetTemperatureReached;

    /*********************************************************************************
     *
     * Member variables implementing the WaterHeaterManagement::Delegate interface
     *
     *********************************************************************************/

    // This attribute SHALL indicate the methods to call for heat that the
    // controller supports. If a bit is set then the controller supports the
    // corresponding method.
    BitMask<WaterHeaterHeatSourceBitmap> mHeaterTypes;

    // This attribute SHALL indicate if the controller is asking for heat. If a
    // bit is set then the corresponding call for heat is active.
    BitMask<WaterHeaterHeatSourceBitmap> mHeatDemand;

    // This attribute SHALL indicate the volume of water that the hot water tank
    // can hold (in units of Litres). This allows an energy management system to
    // estimate the required heating energy needed to reach the target
    // temperature.
    uint16_t mTankVolume;

    // This attribute SHALL indicate the estimated heat energy needed to raise
    // the water temperature to the target setpoint. This can be computed by
    // taking the specific heat capacity of water (4182 J/kg °C) and by knowing
    // the current temperature of the water, the tank volume and target
    // temperature.
    Energy_mWh mEstimatedHeatRequired;

    // This attribute SHALL indicate an approximate level of hot water stored in
    // the tank, which may help consumers understand the amount of hot water
    // remaining in the tank.
    Percent mTankPercentage;

    // This attribute SHALL indicate if the BOOST state, as triggered by a Boost
    // command, is currently active.
    BoostStateEnum mBoostState;
};

} // namespace WaterHeaterManagement
} // namespace Clusters
} // namespace app
} // namespace chip

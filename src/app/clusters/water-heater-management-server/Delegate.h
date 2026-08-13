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
#include <lib/core/CHIPError.h>
#include <protocols/interaction_model/StatusCode.h>

namespace chip {
namespace app {
namespace Clusters {
namespace WaterHeaterManagement {

class WaterHeaterManagementCluster; // forward declaration for Delegate back-pointer

class Delegate
{
public:
    Delegate()          = default;
    virtual ~Delegate() = default;

    void SetEndpointId(EndpointId aEndpoint) { mEndpointId = aEndpoint; }

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
    virtual Protocols::InteractionModel::Status HandleBoost(uint32_t duration, Optional<bool> oneShot,
                                                            Optional<bool> emergencyBoost, Optional<int16_t> temporarySetpoint,
                                                            Optional<Percent> targetPercentage, Optional<Percent> targetReheat) = 0;

    /**
     * @brief Delegate should implement a handler to cancel a boost command.
     *        Upon receipt, the Water Heater SHALL transition back from the
     *        BOOST state to the previous mode (e.g. OFF, MANUAL or TIMED).
     *
     * @return It should report SUCCESS if successful and FAILURE otherwise.
     */
    virtual Protocols::InteractionModel::Status HandleCancelBoost() = 0;

    // ------------------------------------------------------------------
    // Get attribute methods
    virtual BitMask<WaterHeaterHeatSourceBitmap> GetHeaterTypes() = 0;
    virtual BitMask<WaterHeaterHeatSourceBitmap> GetHeatDemand()  = 0;
    virtual uint16_t GetTankVolume()                              = 0;
    virtual Energy_mWh GetEstimatedHeatRequired()                 = 0;
    virtual Percent GetTankPercentage()                           = 0;
    virtual BoostStateEnum GetBoostState()                        = 0;

    // ------------------------------------------------------------------
    // Event generation helpers — concrete delegate implementations may call
    // these to emit WaterHeaterManagement cluster events. Available once the
    // owning WaterHeaterManagementCluster has been constructed.
    void SetCluster(WaterHeaterManagementCluster * cluster) { mCluster = cluster; }
    CHIP_ERROR GenerateBoostStartedEvent(uint32_t durationSecs, Optional<bool> oneShot, Optional<bool> emergencyBoost,
                                         Optional<int16_t> temporarySetpoint, Optional<Percent> targetPercentage,
                                         Optional<Percent> targetReheat);
    CHIP_ERROR GenerateBoostEndedEvent();

protected:
    EndpointId mEndpointId = 0;

private:
    WaterHeaterManagementCluster * mCluster = nullptr;
};

} // namespace WaterHeaterManagement
} // namespace Clusters
} // namespace app
} // namespace chip

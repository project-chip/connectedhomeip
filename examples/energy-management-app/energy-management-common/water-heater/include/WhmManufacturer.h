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

#include <DEMManufacturerDelegate.h>
#include <WhmDelegate.h>
#include <WhmInstance.h>

namespace chip {
namespace app {
namespace Clusters {
namespace WaterHeaterManagement {

/**
 * The WhmManufacturer example class
 *
 * Helps with handling the test triggers.
 */

class WhmManufacturer : public DeviceEnergyManagement::DEMManufacturerDelegate
{
public:
    WhmManufacturer(WaterHeaterManagementInstance * whmInstance) { mWhmInstance = whmInstance; }

    WaterHeaterManagementInstance * GetWhmInstance() { return mWhmInstance; }

    WaterHeaterManagementDelegate * GetWhmDelegate()
    {
        if (mWhmInstance)
        {
            return mWhmInstance->GetDelegate();
        }

        return nullptr;
    }

    /**
     * @brief   Called at start up to apply hardware settings
     */
    CHIP_ERROR Init();

    /**
     * @brief   Called at shutdown
     */
    CHIP_ERROR Shutdown();

    /**
     * @brief   Called to determine which heating sources to use,
     */
    BitMask<WaterHeaterHeatSourceBitmap> DetermineHeatingSources();

    /**
     * @brief   Turn the heating of the water tank on.
     *
     * @param emergencyBoost   Indicates that the consumer wants the water to be heated as quickly as practicable. This MAY cause
     *                         multiple heat sources to be activated (e.g. a heat pump and direct electric heating element).
     * @return An error if a problem occurs turning the heating on
     */
    Protocols::InteractionModel::Status TurnHeatingOn(bool emergencyBoost);

    /**
     * @brief   Turn the heating of the water tank off.
     *
     * @return An error if a problem occurs turning the heating off
     */
    Protocols::InteractionModel::Status TurnHeatingOff();

    /**
     * @brief Called to handle a boost command.
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
     * @return  Success if the boost command is successful; otherwise return the
     * appropriate error.
     */
    Protocols::InteractionModel::Status BoostCommandStarted(uint32_t duration, Optional<bool> oneShot,
                                                            Optional<bool> emergencyBoost, Optional<int16_t> temporarySetpoint,
                                                            Optional<chip::Percent> targetPercentage,
                                                            Optional<chip::Percent> targetReheat);

    /**
     * @brief Called when the Boost command has been cancelled.
     *
     * @return It should report SUCCESS if successful and FAILURE otherwise.
     */
    Protocols::InteractionModel::Status BoostCommandCancelled();

    /**
     * @brief Called when a boost command has completed.
     */
    void BoostCommandFinished();

    /* Implement the DEMManufacturerDelegate interface */

    /**
     * @brief The PowerAdjustEnd event needs to report the approximate energy used by the ESA during the session.
     */
    int64_t GetApproxEnergyDuringSession() override;

private:
    WaterHeaterManagementInstance * mWhmInstance;
};

/** @brief Helper function to return the singleton WhmManufacturer instance
 *
 * This is needed by the WhmManufacturer class to support TestEventTriggers
 * which are called outside of any class context. This allows the WhmManufacturer
 * class to return the relevant Delegate instance in which to invoke the test
 * events on.
 *
 * This function is typically found in main.cpp or wherever the singleton is created.
 */
WhmManufacturer * GetWhmManufacturer();

} // namespace WaterHeaterManagement
} // namespace Clusters
} // namespace app
} // namespace chip

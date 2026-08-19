/**
 *
 *    Copyright (c) 2024-2026 Project CHIP Authors
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

#include <stddef.h>

#include <app-common/zap-generated/cluster-enums.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app/util/basic-types.h>
#include <lib/core/CHIPError.h>
#include <lib/core/Optional.h>
#include <lib/support/Span.h>

namespace chip::app::Clusters::EnergyPreference {

struct Delegate
{
    // Note: This delegate does not handle the "Current Active" indexes attributes storage.
    // eg: Current Energy Balance and Current Low Power Mode Sensitivity.  These can be handled using
    // ember built in storage, or via the external callbacks as desired by the implementer.

    virtual ~Delegate() {}

    /**
     * Get an Energy Balance.
     *
     * The delegate method is called by the cluster to fill out the
     * values for the list in EnergyBalances attribute. Storage for
     * both outStep and outLabel is provided by the caller.
     *
     * @param endpoint The endpoint to query.
     * @param index The index of the balance, with 0 representing the first one.
     * @param outStep The Step value from BalanceStruct
     *
     * @param outLabel The Label value from BalanceStruct. Storage is
     * provided by the caller, and is large enough to accomodate the
     * longest label (64 chars), on successful return the size of the span must be
     * adjusted to reflect the length of the value, or ClearValue() called on the Optional to indicate there is no label.
     *
     * @return CHIP_ERROR_NOT_FOUND if the index is out of range.
     */
    virtual CHIP_ERROR GetEnergyBalanceAtIndex(chip::EndpointId endpoint, size_t index, chip::Percent & outStep,
                                               chip::Optional<chip::MutableCharSpan> & outLabel) = 0;

    /**
     * Get an Energy Priority.
     * @param endpoint The endpoint to query.
     * @param index The index of the priority, with 0 representing the first one.
     * @param outPriority The EnergyPriorityEnum to copy the data into.
     * @return CHIP_ERROR_NOT_FOUND if the index is out of range.
     */
    virtual CHIP_ERROR GetEnergyPriorityAtIndex(chip::EndpointId endpoint, size_t index,
                                                chip::app::Clusters::EnergyPreference::EnergyPriorityEnum & outPriority) = 0;

    /**
     * Get a Power Sensitity Balance Struct data at the specified index.
     *
     * The delegate method is called by the cluster to fill out the
     * values for the list in LowPowerSensitivities attribute. Storage for
     * both outStep and outLabel is provided by the caller.
     *
     * @param endpoint The endpoint to query.
     * @param index The index of the priority, with 0 representing the first one.
     * @param outStep The Step value from BalanceStruct
     *
     * @param outLabel The Label value from BalanceStruct. Storage is
     * provided by the caller, and is large enough to accomodate the
     * longest label (64 chars), on successful return the size of the span must be
     * adjusted to reflect the length of the value, or ClearValue() called on the Optional to indicate there is no label.
     *
     * @return CHIP_ERROR_NOT_FOUND if the index is out of range.
     */
    virtual CHIP_ERROR GetLowPowerModeSensitivityAtIndex(chip::EndpointId endpoint, size_t index, chip::Percent & outStep,
                                                         chip::Optional<chip::MutableCharSpan> & outLabel) = 0;

    /**
     * Get the number of energy balances this endpoint has.
     * @param endpoint The endpoint to query.
     * @return the number of balance structs in the list.
     */
    virtual size_t GetNumEnergyBalances(chip::EndpointId endpoint) = 0;

    /**
     * Get the number of low power mode sensitivities this endpoint has.
     * @param endpoint The endpoint to query.
     * @return the number of balance structs in the list.
     */
    virtual size_t GetNumLowPowerModeSensitivities(chip::EndpointId endpoint) = 0;

    /**
     * Callback invoked when the CurrentEnergyBalance attribute is changed on endpoint `endpoint`.
     * @note Do not call SetCurrentEnergyBalance() on the cluster on the same endpoint from this callback to avoid possible infinite
     * recursion.
     */
    virtual void OnCurrentEnergyBalanceChanged(chip::EndpointId endpoint, uint8_t currentEnergyBalance) {}

    /**
     * Callback invoked when the CurrentLowPowerModeSensitivity attribute is changed on endpoint `endpoint`.
     * @note Do not call SetCurrentLowPowerModeSensitivity() on the cluster on the same endpoint from this callback to avoid
     * possible infinite recursion.
     */
    virtual void OnCurrentLowPowerModeSensitivityChanged(chip::EndpointId endpoint, uint8_t currentLowPowerModeSensitivity) {}
};

} // namespace chip::app::Clusters::EnergyPreference

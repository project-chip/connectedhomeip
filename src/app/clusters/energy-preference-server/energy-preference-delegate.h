/**
 *
 *    Copyright (c) 2024 Project CHIP Authors
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
     * both aOutStep and aOutLabel is provided by the caller.
     *
     * @param aEndpoint The endpoint to query.
     * @param aIndex The index of the balance, with 0 representing the first one.
     * @param aOutStep The Step value from BalanceStruct
     *
     * @param aOutLabel The Label value from BalanceStruct. Storage is
     * provided by the caller, and is large enough to accomodate the
     * longest label (64 chars), on successful return the size of the span must be
     * adjusted to reflect the length of the value, or ClearValue() called on the Optional to indicate there is no label.
     *
     * @return CHIP_ERROR_NOT_FOUND if the index is out of range.
     */
    virtual CHIP_ERROR GetEnergyBalanceAtIndex(chip::EndpointId aEndpoint, size_t aIndex, chip::Percent & aOutStep,
                                               chip::Optional<chip::MutableCharSpan> & aOutLabel) = 0;

    /**
     * Get an Energy Priority.
     * @param aEndpoint The endpoint to query.
     * @param aIndex The index of the priority, with 0 representing the first one.
     * @param aOutPriority The EnergyPriorityEnum to copy the data into.
     * @return CHIP_ERROR_NOT_FOUND if the index is out of range.
     */
    virtual CHIP_ERROR GetEnergyPriorityAtIndex(chip::EndpointId aEndpoint, size_t aIndex,
                                                chip::app::Clusters::EnergyPreference::EnergyPriorityEnum & aOutPriority) = 0;

    /**
     * Get a Power Sensitity Balance Struct data at the specified index.
     *
     * The delegate method is called by the cluster to fill out the
     * values for the list in LowPowerSensitivities attribute. Storage for
     * both aOutStep and aOutLabel is provided by the caller.
     *
     * @param aEndpoint The endpoint to query.
     * @param aIndex The index of the priority, with 0 representing the first one.
     * @param aOutStep The Step value from BalanceStruct
     *
     * @param aOutLabel The Label value from BalanceStruct. Storage is
     * provided by the caller, and is large enough to accomodate the
     * longest label (64 chars), on successful return the size of the span must be
     * adjusted to reflect the length of the value, or ClearValue() called on the Optional to indicate there is no label.
     *
     * @return CHIP_ERROR_NOT_FOUND if the index is out of range.
     */
    virtual CHIP_ERROR GetLowPowerModeSensitivityAtIndex(chip::EndpointId aEndpoint, size_t aIndex, chip::Percent & aOutStep,
                                                         chip::Optional<chip::MutableCharSpan> & aOutLabel) = 0;

    /**
     * Get the number of energy balances this endpoint has.
     * @param aEndpoint The endpoint to query.
     * @return the number of balance structs in the list.
     */
    virtual size_t GetNumEnergyBalances(chip::EndpointId aEndpoint) = 0;

    /**
     * Get the number of low power mode sensitivities this endpoint has.
     * @param aEndpoint The endpoint to query.
     * @return the number of balance structs in the list.
     */
    virtual size_t GetNumLowPowerModeSensitivities(chip::EndpointId aEndpoint) = 0;
};

void SetDelegate(Delegate * aDelegate);
Delegate * GetDelegate();

} // namespace chip::app::Clusters::EnergyPreference

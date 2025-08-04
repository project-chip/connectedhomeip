/*
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

#include <app/icd/server/ICDMonitoringTable.h>
#include <lib/core/CHIPError.h>

namespace chip {
namespace app {

/**
 * @brief This class defines the necessary interface a ICD Check-In BackOff strategy needs to implment to be consummed by the
 *        ICDManager class. The strategy is injected with the init server params when initializing the device Server class.
 */
class ICDCheckInBackOffStrategy
{
public:
    virtual ~ICDCheckInBackOffStrategy() = default;

    /**
     * @brief Function is used by the ICDManager to determine if a Check-In message should be sent to the given entry based on the
     *        Check-In BackOff strategy.
     *
     *        There are no requirements on how the Check-In BackOff strategy should behave.
     *        The only specified requirement is the maximum time between to Check-In message, MaximumCheckInBackOff.
     *        All strategies must respect this requirement.
     *
     * @param entry ICDMonitoringEntry for which we are about to send a Check-In message to.
     *
     * @return true   ICDCheckInBackOffStrategy determines that we SHOULD send a Check-In message to the given entry
     * @return false ICDCheckInBackOffStrategy determines that we SHOULD NOT send a Check-In message to the given entry
     */
    virtual bool ShouldSendCheckInMessage(const ICDMonitoringEntry & entry) = 0;

    /**
     * @brief Function is used within the test event trigger to force the maximum BackOff state of the ICD Check-In BackOff
     *        strategy. This enables to validate the strategy and to certify it respects the  MaximumCheckInBackOff interval during
     *        certification.
     *
     *        Function sets the maxmimum BackOff state for all clients registered with the ICD
     *
     * @return CHIP_ERROR Any error returned during the forcing of the maximum BackOff state
     */
    virtual CHIP_ERROR ForceMaximumCheckInBackoff() = 0;
};

} // namespace app
} // namespace chip

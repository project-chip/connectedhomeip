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

#include <app/icd/server/ICDCheckInBackOffStrategy.h>
#include <lib/core/ClusterEnums.h>

namespace chip {
namespace app {

/**
 * @brief Default ICD Check-In BackOff Strategy.
 *        The default strategy is based on the two types of controllers
 *        - kPermanent : Always send a Check-In message
 *        - kEphemeral : Never send a Check-In message
 *
 *        This implementation represents a no back off strategy.
 */
class DefaultICDCheckInBackOffStrategy : public ICDCheckInBackOffStrategy
{
public:
    DefaultICDCheckInBackOffStrategy()  = default;
    ~DefaultICDCheckInBackOffStrategy() = default;

    /**
     * @brief Function checks if the entry is a permanent or ephemeral client.
     *        If the client is permanent, we should send a Check-In message.
     *        If the client is ephemeral, we should not send a Check-In message.
     *
     * @param entry Entry for which we are deciding whether we need to send a Check-In message or not.
     * @return true If the client is permanent, return true.
     * @return false If the client is not permanent, ephemeral or invalid, return false.
     */
    bool ShouldSendCheckInMessage(const ICDMonitoringEntry & entry) override
    {
        return (entry.clientType == Clusters::IcdManagement::ClientTypeEnum::kPermanent);
    }

    /**
     * @brief The default Check-In BackOff fundamentally implements a no back off strategy.
     *        As such, we don't need to execute anything to force the maximum Check-In BackOff.
     *
     */
    CHIP_ERROR ForceMaximumCheckInBackoff() override { return CHIP_NO_ERROR; }
};

} // namespace app
} // namespace chip

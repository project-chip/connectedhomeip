/*
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

#pragma once

#include <app-common/zap-generated/cluster-objects.h>
#include <app/TestEventTriggerDelegate.h>

namespace chip {

enum class EnergyEvseTrigger : uint64_t
{
    // Scenarios
    kBasicFunctionality = 0x0099000000000000,      // Basic Functionality Test Event       | Simulate installation with
                                                   // _{A_CIRCUIT_CAPACITY}_=32A and _{A_USER_MAXIMUM_CHARGE_CURRENT}_=32A
    kBasicFunctionalityClear = 0x0099000000000001, // Basic Functionality Test Event Clear | End simulation of installation
    kEVPluggedIn             = 0x0099000000000002, // EV Plugged-in Test Event | Simulate plugging
                                                   // the EV into the EVSE using a cable of 63A capacity
    kEVPluggedInClear    = 0x0099000000000003,     // EV Plugged-in Test Event Clear | Simulate unplugging the EV
    kEVChargeDemand      = 0x0099000000000004, // EV Charge Demand Test Event | Simulate the EV presenting charge demand to the EVSE
    kEVChargeDemandClear = 0x0099000000000005, // EV Charge Demand Test Event Clear | Simulate the EV becoming fully charged

};

class EnergyEvseTestEventTriggerDelegate : public TestEventTriggerDelegate
{
public:
    explicit EnergyEvseTestEventTriggerDelegate(const ByteSpan & enableKey, TestEventTriggerDelegate * otherDelegate) :
        mEnableKey(enableKey), mOtherDelegate(otherDelegate)
    {}

    bool DoesEnableKeyMatch(const ByteSpan & enableKey) const override;
    CHIP_ERROR HandleEventTrigger(uint64_t eventTrigger) override;

private:
    ByteSpan mEnableKey;
    TestEventTriggerDelegate * mOtherDelegate;
};

} // namespace chip

/**
 * @brief User handler for handling the test event trigger
 *
 * @note If TestEventTrigger is enabled, it needs to be implemented in the app
 *
 * @param eventTrigger Event trigger to handle
 *
 * @retval true on success
 * @retval false if error happened
 */
bool HandleEnergyEvseTestEventTrigger(uint64_t eventTrigger);

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

namespace chip {

/*
 * These Test EventTrigger values are specified in the TC_EEVSE test plan
 * and are defined conditions used in test events.
 *
 * They are sent along with the enableKey (manufacturer defined secret)
 * in the General Diagnostic cluster TestEventTrigger command
 */
enum class EnergyEvseTrigger : uint64_t
{
    // Scenarios
    // Basic Functionality Test Event       | Simulate installation with _{A_CIRCUIT_CAPACITY}_=32A and
    // _{A_USER_MAXIMUM_CHARGE_CURRENT}_=32A
    kBasicFunctionality = 0x0099000000000000,
    // Basic Functionality Test Event Clear | End simulation of installation
    kBasicFunctionalityClear = 0x0099000000000001,
    // EV Plugged-in Test Event | Simulate plugging the EV into the EVSE using a cable of 63A capacity
    kEVPluggedIn = 0x0099000000000002,
    // EV Plugged-in Test Event Clear | Simulate unplugging the EV
    kEVPluggedInClear = 0x0099000000000003,
    // EV Charge Demand Test Event | Simulate the EV presenting charge demand to the EVSE
    kEVChargeDemand = 0x0099000000000004,
    // EV Charge Demand Test Event Clear | Simulate the EV becoming fully charged
    kEVChargeDemandClear = 0x0099000000000005,
    // EV Charge TimeOfUse Mode | Simulate putting the EVSE into a Mode with the TimeOfUse tag included
    kEVTimeOfUseMode = 0x0099000000000006,
    // EVSE has a GroundFault fault
    kEVSEGroundFault = 0x0099000000000010,
    // EVSE has a OverTemperature fault
    kEVSEOverTemperatureFault = 0x0099000000000011,
    // EVSE faults have cleared
    kEVSEFaultClear = 0x0099000000000012,
    // EVSE Diagnostics Complete | Simulate diagnostics have been completed and return to normal
    kEVSEDiagnosticsComplete = 0x0099000000000020,
    // EV Charge TimeOfUse Mode clear | Simulate clearing the EVSE Mode TimeOfUse tag
    kEVTimeOfUseModeClear = 0x0099000000000021,
};

class EnergyEvseTestEventTriggerHandler : public TestEventTriggerHandler
{
public:
    explicit EnergyEvseTestEventTriggerHandler() {}

    /** This function must return True if the eventTrigger is recognised and handled
     *  It must return False to allow a higher level TestEvent handler to check other
     *  clusters that may handle it.
     */
    CHIP_ERROR HandleEventTrigger(uint64_t eventTrigger) override
    {
        if (HandleEnergyEvseTestEventTrigger(eventTrigger))
        {
            return CHIP_NO_ERROR;
        }
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
};

} // namespace chip

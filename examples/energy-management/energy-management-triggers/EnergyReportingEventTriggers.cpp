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

#include "FakeReadings.h"

#include <app/clusters/electrical-energy-measurement-server/EnergyReportingTestEventTriggerHandler.h>

using namespace chip;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ElectricalEnergyMeasurement;
using namespace chip::app::Clusters::ElectricalEnergyMeasurement::Structs;

void SetTestEventTrigger_FakeReadingsLoadStart()
{
    int64_t aPower_mW              = 1'000'000; // Fake load 1000 W
    uint32_t aPowerRandomness_mW   = 20'000;    // randomness 20W
    int64_t aVoltage_mV            = 230'000;   // Fake Voltage 230V
    uint32_t aVoltageRandomness_mV = 1'000;     // randomness 1V
    int64_t aCurrent_mA            = 4'348;     // Fake Current (at 1kW@230V = 4.3478 Amps)
    uint32_t aCurrentRandomness_mA = 500;       // randomness 500mA
    uint8_t aInterval_s            = 2;         // 2s updates
    bool bReset                    = true;
    FakeReadings::GetInstance().StartFakeReadings(EndpointId(1), aPower_mW, aPowerRandomness_mW, aVoltage_mV, aVoltageRandomness_mV,
                                                  aCurrent_mA, aCurrentRandomness_mA, aInterval_s, bReset);
}

void SetTestEventTrigger_FakeReadingsGeneratorStart()
{
    int64_t aPower_mW              = -3'000'000; // Fake Generator -3000 W
    uint32_t aPowerRandomness_mW   = 20'000;     // randomness 20W
    int64_t aVoltage_mV            = 230'000;    // Fake Voltage 230V
    uint32_t aVoltageRandomness_mV = 1'000;      // randomness 1V
    int64_t aCurrent_mA            = -13'043;    // Fake Current (at -3kW@230V = -13.0434 Amps)
    uint32_t aCurrentRandomness_mA = 500;        // randomness 500mA
    uint8_t aInterval_s            = 5;          // 5s updates
    bool bReset                    = true;
    FakeReadings::GetInstance().StartFakeReadings(EndpointId(1), aPower_mW, aPowerRandomness_mW, aVoltage_mV, aVoltageRandomness_mV,
                                                  aCurrent_mA, aCurrentRandomness_mA, aInterval_s, bReset);
}

void SetTestEventTrigger_FakeReadingsStop()
{
    FakeReadings::GetInstance().StopFakeReadings();
}

bool HandleEnergyReportingTestEventTrigger(uint64_t eventTrigger)
{
    EnergyReportingTrigger trigger = static_cast<EnergyReportingTrigger>(eventTrigger);

    switch (trigger)
    {
    case EnergyReportingTrigger::kFakeReadingsStop:
        ChipLogProgress(Support, "[EnergyReporting-Test-Event] => Stop Fake load");
        SetTestEventTrigger_FakeReadingsStop();
        break;
    case EnergyReportingTrigger::kFakeReadingsLoadStart_1kW_2s:
        ChipLogProgress(Support, "[EnergyReporting-Test-Event] => Start Fake load 1kW @2s Import");
        SetTestEventTrigger_FakeReadingsLoadStart();
        break;
    case EnergyReportingTrigger::kFakeReadingsGenStart_3kW_5s:
        ChipLogProgress(Support, "[EnergyReporting-Test-Event] => Start Fake generator 3kW @5s Export");
        SetTestEventTrigger_FakeReadingsGeneratorStart();
        break;

    default:
        return false;
    }

    return true;
}

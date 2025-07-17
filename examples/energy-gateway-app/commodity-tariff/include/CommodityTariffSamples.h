/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <string>

namespace TariffPresets {
    static constexpr const char* tariff_sample_1 = R"({
        "TariffInfo": {
            "TariffLabel": "Full Tariff One",
            "ProviderName": "Default Provider",
            "Currency": {
                "Currency": 120,
                "DecimalPoints": 0
            },
            "BlockMode": 0
        },
        "TariffUnit": 0,
        "StartDate": 0,
        "DayEntries": [
            {
                "DayEntryID": 10,
                "StartTime": 0,
                "Duration": 240,
                "RandomizationOffset": 0,
                "RandomizationType": 0
            },
            {
                "DayEntryID": 11,
                "StartTime": 240,
                "Duration": 240,
                "RandomizationOffset": 0,
                "RandomizationType": 0
            },
            {
                "DayEntryID": 12,
                "StartTime": 480,
                "Duration": 240,
                "RandomizationOffset": 0,
                "RandomizationType": 0
            },
            {
                "DayEntryID": 13,
                "StartTime": 720,
                "Duration": 240,
                "RandomizationOffset": 0,
                "RandomizationType": 0
            },
            {
                "DayEntryID": 14,
                "StartTime": 960,
                "Duration": 240,
                "RandomizationOffset": 0,
                "RandomizationType": 0
            },
            {
                "DayEntryID": 15,
                "StartTime": 1200,
                "Duration": 240,
                "RandomizationOffset": 0,
                "RandomizationType": 0
            },
            {
                "DayEntryID": 16,
                "StartTime": 1440,
                "Duration": 60,
                "RandomizationOffset": 0,
                "RandomizationType": 0
            },
            {
                "DayEntryID": 20,
                "StartTime": 0,
                "Duration": 240,
                "RandomizationOffset": 0,
                "RandomizationType": 0
            },
            {
                "DayEntryID": 21,
                "StartTime": 240,
                "Duration": 240,
                "RandomizationOffset": 0,
                "RandomizationType": 0
            },
            {
                "DayEntryID": 22,
                "StartTime": 480,
                "Duration": 240,
                "RandomizationOffset": 0,
                "RandomizationType": 0
            },
            {
                "DayEntryID": 23,
                "StartTime": 720,
                "Duration": 240,
                "RandomizationOffset": 0,
                "RandomizationType": 0
            },
            {
                "DayEntryID": 24,
                "StartTime": 960,
                "Duration": 240,
                "RandomizationOffset": 0,
                "RandomizationType": 0
            },
            {
                "DayEntryID": 25,
                "StartTime": 1200,
                "Duration": 240,
                "RandomizationOffset": 0,
                "RandomizationType": 0
            },
            {
                "DayEntryID": 26,
                "StartTime": 1440,
                "Duration": 60,
                "RandomizationOffset": 0,
                "RandomizationType": 0
            },
            {
                "DayEntryID": 30,
                "StartTime": 0,
                "Duration": 1500,
                "RandomizationOffset": 0,
                "RandomizationType": 0
            }
        ],
        "DayPatterns": [
            {
                "DayPatternID": 0,
                "DaysOfWeek": "0x55",
                "DayEntryIDs": [10, 11, 12, 13, 14, 15, 16]
            },
            {
                "DayPatternID": 1,
                "DaysOfWeek": "0x2a",
                "DayEntryIDs": [20, 21, 22, 23, 24, 25, 26]
            }
        ],
        "CalendarPeriods": [
            {
                "StartDate": 0,
                "DayPatternIDs": [0, 1]
            }
        ],
        "IndividualDays": [
            {
                "Date": 1745427104,
                "DayType": 1,
                "DayEntryIDs": [30]
            }
        ],
        "TariffComponents": [
            {
                "TariffComponentID": 10,
                "Price": {
                    "PriceType": 0,
                    "Price": 0.15,
                    "PriceLevel": 1
                },
                "FriendlyCredit": false,
                "AuxiliaryLoad": {
                    "Number": 1,
                    "RequiredState": 0
                },
                "PeakPeriod": {
                    "Severity": 1,
                    "PeakPeriod": 1
                },
                "PowerThreshold": {
                    "PowerThreshold": 2400000,
                    "ApparentPowerThreshold": 120,
                    "PowerThresholdSource": 0
                },
                "Threshold": 120,
                "Label": "Tariff Component 1",
                "Predicted": false
            },
            {
                "TariffComponentID": 20,
                "Price": {
                    "PriceType": 0,
                    "Price": 0.2,
                    "PriceLevel": 0
                },
                "FriendlyCredit": false,
                "AuxiliaryLoad": {
                    "Number": 1,
                    "RequiredState": 0
                },
                "PeakPeriod": {
                    "Severity": 1,
                    "PeakPeriod": 1
                },
                "PowerThreshold": {
                    "PowerThreshold": 4800000,
                    "ApparentPowerThreshold": 240,
                    "PowerThresholdSource": 0
                },
                "Threshold": 240,
                "Label": "Tariff Component 2",
                "Predicted": false
            }
        ],
        "TariffPeriods": [
            {
                "Label": "Period 1",
                "DayEntryIDs": [10, 14, 21, 25],
                "TariffComponentIDs": [10]
            },
            {
                "Label": "Period 2",
                "DayEntryIDs": [11, 15, 22, 26],
                "TariffComponentIDs": [20]
            },
            {
                "Label": "Period 3",
                "DayEntryIDs": [12, 16, 23],
                "TariffComponentIDs": [10]
            },
            {
                "Label": "Period 4",
                "DayEntryIDs": [13, 20, 24],
                "TariffComponentIDs": [20]
            },
            {
                "Label": "Period 5",
                "DayEntryIDs": [30],
                "TariffComponentIDs": [10]
            }
        ],
        "DefaultRandomizationOffset": 0,
        "DefaultRandomizationType": 0
    })";
    static constexpr const char*  tariff_sample_2 = R"({
        "TariffInfo": {
            "TariffLabel": "Full Tariff Two",
            "ProviderName": "Example Provider",
            "Currency": {
                "Currency": 200,
                "DecimalPoints": 1
            },
            "BlockMode": 1
        },
        "TariffUnit": 0,
        "StartDate": 0,
        "DayEntries": [
            {
                "DayEntryID": 10,
                "StartTime": 0,
                "Duration": 240,
                "RandomizationOffset": 0,
                "RandomizationType": 0
            },
            {
                "DayEntryID": 11,
                "StartTime": 240,
                "Duration": 240,
                "RandomizationOffset": 0,
                "RandomizationType": 0
            },
            {
                "DayEntryID": 12,
                "StartTime": 480,
                "Duration": 240,
                "RandomizationOffset": 0,
                "RandomizationType": 0
            },
            {
                "DayEntryID": 13,
                "StartTime": 720,
                "Duration": 240,
                "RandomizationOffset": 0,
                "RandomizationType": 0
            },
            {
                "DayEntryID": 14,
                "StartTime": 960,
                "Duration": 240,
                "RandomizationOffset": 0,
                "RandomizationType": 0
            },
            {
                "DayEntryID": 15,
                "StartTime": 1200,
                "Duration": 240,
                "RandomizationOffset": 0,
                "RandomizationType": 0
            },
            {
                "DayEntryID": 16,
                "StartTime": 1440,
                "Duration": 60,
                "RandomizationOffset": 0,
                "RandomizationType": 0
            }
        ],
        "DayPatterns": [
            {
                "DayPatternID": 0,
                "DaysOfWeek": "0x7F",
                "DayEntryIDs": [10, 11, 12, 13, 14, 15, 16]
            }
        ],
        "CalendarPeriods": [
            {
                "StartDate": 0,
                "DayPatternIDs": [0]
            }
        ],
        "TariffComponents": [
            {
                "TariffComponentID": 10,
                "Price": {
                    "PriceType": 0,
                    "Price": 0.15,
                    "PriceLevel": 1
                },
                "FriendlyCredit": false,
                "AuxiliaryLoad": {
                    "Number": 1,
                    "RequiredState": 0
                },
                "PeakPeriod": {
                    "Severity": 1,
                    "PeakPeriod": 1
                },
                "PowerThreshold": {
                    "PowerThreshold": 2400000,
                    "ApparentPowerThreshold": 120,
                    "PowerThresholdSource": 0
                },
                "Threshold": 120,
                "Label": "Tariff Component 1",
                "Predicted": false
            },
            {
                "TariffComponentID": 20,
                "Price": {
                    "PriceType": 0,
                    "Price": 0.2,
                    "PriceLevel": 0
                },
                "FriendlyCredit": false,
                "AuxiliaryLoad": {
                    "Number": 1,
                    "RequiredState": 0
                },
                "PeakPeriod": {
                    "Severity": 1,
                    "PeakPeriod": 1
                },
                "PowerThreshold": {
                    "PowerThreshold": 4800000,
                    "ApparentPowerThreshold": 240,
                    "PowerThresholdSource": 0
                },
                "Threshold": 240,
                "Label": "Tariff Component 2",
                "Predicted": false
            },
            {
                "TariffComponentID": 30,
                "Price": {
                    "PriceType": 0,
                    "Price": 0.5,
                    "PriceLevel": 0
                },
                "FriendlyCredit": true,
                "AuxiliaryLoad": {
                    "Number": 1,
                    "RequiredState": 0
                },
                "PeakPeriod": {
                    "Severity": 1,
                    "PeakPeriod": 1
                },
                "PowerThreshold": {
                    "PowerThreshold": 3200000,
                    "ApparentPowerThreshold": 320,
                    "PowerThresholdSource": 0
                },
                "Threshold": 320,
                "Label": "Tariff Component 3",
                "Predicted": true
            }
        ],
        "TariffPeriods": [
            {
                "Label": "Period 1",
                "DayEntryIDs": [10, 13, 16],
                "TariffComponentIDs": [10]
            },
            {
                "Label": "Period 2",
                "DayEntryIDs": [11, 14],
                "TariffComponentIDs": [20]
            },
            {
                "Label": "Period 3",
                "DayEntryIDs": [12, 15],
                "TariffComponentIDs": [30]
            }
        ],
        "DefaultRandomizationOffset": 0,
        "DefaultRandomizationType": 0
    })";
} //TariffPresets

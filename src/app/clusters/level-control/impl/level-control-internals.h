/**
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include "../level-control.h"

#include <app/cluster-building-blocks/QuieterReporting.h>
#include <app/util/endpoint-config-api.h>
#include <app/util/util.h>
#include <system/SystemClock.h>

struct CallbackScheduleState {
    chip::System::Clock::Timestamp idealTimestamp;
    chip::System::Clock::Milliseconds32 runTime;
};

struct EmberAfLevelControlState {
    chip::CommandId commandId;
    uint8_t moveToLevel;
    bool increasing;
    uint8_t onLevel;
    uint8_t minLevel;
    uint8_t maxLevel;
    uint16_t storedLevel;
    uint32_t eventDurationMs;
    uint32_t transitionTimeMs;
    uint32_t elapsedTimeMs;
    CallbackScheduleState callbackSchedule;
    chip::app::QuieterReportingAttribute<uint8_t> quietCurrentLevel { chip::app::DataModel::NullNullable };
    chip::app::QuieterReportingAttribute<uint16_t> quietRemainingTime {
        chip::app::DataModel::MakeNullable<uint16_t>(0)
    };
};

constexpr uint8_t kStartUpCurrentLevelUseDeviceMinimum = 0x00;
constexpr uint8_t kStartUpCurrentLevelUsePreviousLevel = 0xFF;

EmberAfLevelControlState * getState(chip::EndpointId endpoint);

void writeRemainingTime(chip::EndpointId endpoint, uint16_t remainingTimeMs, bool isNewTransition = false);

void HandleStartUpCurrentLevel(chip::EndpointId endpoint, EmberAfLevelControlState * state,
    chip::app::DataModel::Nullable<uint8_t> & currentLevel);

void ResolveOnLevel(chip::EndpointId endpoint, const chip::app::DataModel::Nullable<uint8_t> & currentLevelCache,
    chip::app::DataModel::Nullable<uint8_t> & resolvedLevel, bool & useOnLevel);

bool TryGetOnOffTransitionTimeDs(chip::EndpointId endpoint, uint16_t & transitionTimeDs);

chip::Protocols::InteractionModel::Status ComputeTransitionTimeMsForMoveToLevel(
    chip::EndpointId endpoint, chip::app::DataModel::Nullable<uint16_t> transitionTimeDs, uint8_t actualStepSize,
    uint32_t fastestTransitionTimeMs, uint32_t & transitionTimeMs);

chip::Protocols::InteractionModel::Status ComputeEventDurationMsForMove(chip::EndpointId endpoint,
    chip::app::DataModel::Nullable<uint8_t> rate,
    uint32_t fastestTransitionTimeMs,
    uint8_t & eventDurationMs);

uint32_t ComputeTransitionTimeMsForStep(chip::EndpointId endpoint, chip::app::DataModel::Nullable<uint16_t> transitionTimeDs,
    uint8_t stepSize, uint8_t actualStepSize, uint32_t fastestTransitionTimeMs);

chip::Protocols::InteractionModel::Status SetCurrentLevelQuietReport(chip::EndpointId endpoint,
    EmberAfLevelControlState * state,
    chip::app::DataModel::Nullable<uint8_t> newValue,
    bool isEndOfTransition);

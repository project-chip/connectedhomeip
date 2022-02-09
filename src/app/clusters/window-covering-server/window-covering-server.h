/*
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

#include <app-common/zap-generated/attribute-id.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/enums.h>
#include <app/util/af-types.h>

#include <app/data-model/Nullable.h>

#define WC_PERCENT100THS_MIN_OPEN 0
#define WC_PERCENT100THS_MAX_CLOSED 10000

namespace chip {
namespace app {
namespace Clusters {
namespace WindowCovering {

typedef DataModel::Nullable<Percent> NPercent;
typedef DataModel::Nullable<Percent100ths> NPercent100ths;
typedef DataModel::Nullable<uint16_t> NAbsolute;

struct Mode
{
    uint8_t motorDirReversed : 1; // bit 0
    uint8_t calibrationMode : 1;  // bit 1
    uint8_t maintenanceMode : 1;  // bit 2
    uint8_t ledDisplay : 1;       // bit 3
};
static_assert(sizeof(Mode) == sizeof(uint8_t), "Mode Size is not correct");

struct ConfigStatus
{
    uint8_t operational : 1;             // bit 0 M
    uint8_t online : 1;                  // bit 1 M
    uint8_t liftIsReversed : 1;          // bit 2 LF
    uint8_t liftIsPA : 1;                // bit 3 LF & PA
    uint8_t tiltIsPA : 1;                // bit 4 TL & PA
    uint8_t liftIsEncoderControlled : 1; // bit 5 LF & PA
    uint8_t tiltIsEncoderControlled : 1; // bit 6 LF & PA
};
static_assert(sizeof(ConfigStatus) == sizeof(uint8_t), "ConfigStatus Size is not correct");

// Match directly with OperationalStatus 2 bits Fields
enum class OperationalState : uint8_t
{
    Stall             = 0x00, // currently not moving
    MovingUpOrOpen    = 0x01, // is currently opening
    MovingDownOrClose = 0x02, // is currently closing
    Reserved          = 0x03, // dont use
};
static_assert(sizeof(OperationalState) == sizeof(uint8_t), "OperationalState Size is not correct");

struct OperationalStatus
{
    OperationalState global : 2; // bit 0-1 M
    OperationalState lift : 2;   // bit 2-3 LF
    OperationalState tilt : 2;   // bit 4-5 TL
};
static_assert(sizeof(OperationalStatus) == sizeof(uint8_t), "OperationalStatus Size is not correct");

struct SafetyStatus
{
    uint8_t remoteLockout : 1;       // bit 0
    uint8_t tamperDetection : 1;     // bit 1
    uint8_t failedCommunication : 1; // bit 2
    uint8_t positionFailure : 1;     // bit 3
    uint8_t thermalProtection : 1;   // bit 4
    uint8_t obstacleDetected : 1;    // bit 5
    uint8_t powerIssue : 1;          // bit 6
    uint8_t stopInput : 1;           // bit 7
    uint8_t motorJammed : 1;         // bit 8
    uint8_t hardwareFailure : 1;     // bit 9
    uint8_t manualOperation : 1;     // bit 10
};
static_assert(sizeof(SafetyStatus) == sizeof(uint16_t), "SafetyStatus Size is not correct");

// Declare Position Limit Status
enum class LimitStatus : uint8_t
{
    Intermediate      = 0x00,
    IsUpOrOpen        = 0x01,
    IsDownOrClose     = 0x02,
    Inverted          = 0x03,
    IsPastUpOrOpen    = 0x04,
    IsPastDownOrClose = 0x05,
};
static_assert(sizeof(LimitStatus) == sizeof(uint8_t), "LimitStatus Size is not correct");

struct AbsoluteLimits
{
    uint16_t open;
    uint16_t closed;
};

bool HasFeature(chip::EndpointId endpoint, WcFeature feature);
bool HasFeaturePaLift(chip::EndpointId endpoint);
bool HasFeaturePaTilt(chip::EndpointId endpoint);

void TypeSet(chip::EndpointId endpoint, EmberAfWcType type);
EmberAfWcType TypeGet(chip::EndpointId endpoint);

void ConfigStatusSet(chip::EndpointId endpoint, const ConfigStatus & status);
const ConfigStatus ConfigStatusGet(chip::EndpointId endpoint);

void OperationalStatusSet(chip::EndpointId endpoint, const OperationalStatus & status);
void OperationalStatusSetWithGlobalUpdated(chip::EndpointId endpoint, OperationalStatus & status);
const OperationalStatus OperationalStatusGet(chip::EndpointId endpoint);

OperationalState ComputeOperationalState(uint16_t target, uint16_t current);
OperationalState ComputeOperationalState(NPercent100ths target, NPercent100ths current);

void EndProductTypeSet(chip::EndpointId endpoint, EmberAfWcEndProductType type);
EmberAfWcEndProductType EndProductTypeGet(chip::EndpointId endpoint);

void ModeSet(chip::EndpointId endpoint, const Mode & mode);
const Mode ModeGet(chip::EndpointId endpoint);

void SafetyStatusSet(chip::EndpointId endpoint, SafetyStatus & status);
const SafetyStatus SafetyStatusGet(chip::EndpointId endpoint);

LimitStatus CheckLimitState(uint16_t position, AbsoluteLimits limits);

bool IsPercent100thsValid(Percent100ths percent100ths);
bool IsPercent100thsValid(NPercent100ths npercent100ths);

uint16_t LiftToPercent100ths(chip::EndpointId endpoint, uint16_t lift);
uint16_t Percent100thsToLift(chip::EndpointId endpoint, uint16_t percent100ths);
void LiftPositionSet(chip::EndpointId endpoint, uint16_t percent100ths);

uint16_t TiltToPercent100ths(chip::EndpointId endpoint, uint16_t tilt);
uint16_t Percent100thsToTilt(chip::EndpointId endpoint, uint16_t percent100ths);
void TiltPositionSet(chip::EndpointId endpoint, uint16_t percent100ths);

} // namespace WindowCovering
} // namespace Clusters
} // namespace app
} // namespace chip

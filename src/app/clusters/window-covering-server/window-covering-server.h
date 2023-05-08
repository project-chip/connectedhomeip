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

#include "window-covering-delegate.h"
#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/enums.h>
#include <app/util/af-types.h>
#include <protocols/interaction_model/StatusCode.h>

#include <app/data-model/Nullable.h>

#define WC_PERCENT100THS_MIN_OPEN 0
#define WC_PERCENT100THS_MAX_CLOSED 10000
#define WC_PERCENT100THS_MIDDLE 5000
#define WC_PERCENT100THS_COEF 100

namespace chip {
namespace app {
namespace Clusters {
namespace WindowCovering {

typedef DataModel::Nullable<Percent> NPercent;
typedef DataModel::Nullable<Percent100ths> NPercent100ths;
typedef DataModel::Nullable<uint16_t> NAbsolute;
typedef Optional<Percent> OPercent;
typedef Optional<Percent100ths> OPercent100ths;

// Match directly with OperationalStatus 2 bits Fields
enum class OperationalState : uint8_t
{
    Stall             = 0x00, // currently not moving
    MovingUpOrOpen    = 0x01, // is currently opening
    MovingDownOrClose = 0x02, // is currently closing
    Reserved          = 0x03, // dont use
};
static_assert(sizeof(OperationalState) == sizeof(uint8_t), "OperationalState Size is not correct");

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

bool HasFeature(chip::EndpointId endpoint, WindowCoveringFeature feature);
bool HasFeaturePaLift(chip::EndpointId endpoint);
bool HasFeaturePaTilt(chip::EndpointId endpoint);

void TypeSet(chip::EndpointId endpoint, Type type);
Type TypeGet(chip::EndpointId endpoint);

void ConfigStatusPrint(const chip::BitMask<ConfigStatus> & configStatus);
void ConfigStatusSet(chip::EndpointId endpoint, const chip::BitMask<ConfigStatus> & status);
chip::BitMask<ConfigStatus> ConfigStatusGet(chip::EndpointId endpoint);
void ConfigStatusUpdateFeatures(chip::EndpointId endpoint);

void OperationalStatusPrint(const chip::BitMask<OperationalStatus> & opStatus);
void OperationalStatusSet(chip::EndpointId endpoint, chip::BitMask<OperationalStatus> newStatus);
chip::BitMask<OperationalStatus> OperationalStatusGet(chip::EndpointId endpoint);
void OperationalStateSet(chip::EndpointId endpoint, const chip::BitMask<OperationalStatus> field, OperationalState state);
OperationalState OperationalStateGet(chip::EndpointId endpoint, const chip::BitMask<OperationalStatus> field);

OperationalState ComputeOperationalState(uint16_t target, uint16_t current);
OperationalState ComputeOperationalState(NPercent100ths target, NPercent100ths current);
Percent100ths ComputePercent100thsStep(OperationalState direction, Percent100ths previous, Percent100ths delta);

void EndProductTypeSet(chip::EndpointId endpoint, EndProductType type);
EndProductType EndProductTypeGet(chip::EndpointId endpoint);

void ModePrint(const chip::BitMask<Mode> & mode);
void ModeSet(chip::EndpointId endpoint, chip::BitMask<Mode> & mode);
chip::BitMask<Mode> ModeGet(chip::EndpointId endpoint);

void SafetyStatusSet(chip::EndpointId endpoint, const chip::BitMask<SafetyStatus> & status);
chip::BitMask<SafetyStatus> SafetyStatusGet(chip::EndpointId endpoint);

LimitStatus CheckLimitState(uint16_t position, AbsoluteLimits limits);

bool IsPercent100thsValid(Percent100ths percent100ths);
bool IsPercent100thsValid(NPercent100ths npercent100ths);

uint16_t Percent100thsToValue(AbsoluteLimits limits, Percent100ths relative);

uint16_t LiftToPercent100ths(chip::EndpointId endpoint, uint16_t lift);
uint16_t Percent100thsToLift(chip::EndpointId endpoint, uint16_t percent100ths);
void LiftPositionSet(chip::EndpointId endpoint, NPercent100ths position);

uint16_t TiltToPercent100ths(chip::EndpointId endpoint, uint16_t tilt);
uint16_t Percent100thsToTilt(chip::EndpointId endpoint, uint16_t percent100ths);
void TiltPositionSet(chip::EndpointId endpoint, NPercent100ths position);

Protocols::InteractionModel::Status GetMotionLockStatus(chip::EndpointId endpoint);

/**
 * @brief PostAttributeChange is called when an Attribute is modified.
 *
 * The method is called by MatterWindowCoveringClusterServerAttributeChangedCallback
 * to update cluster attributes values. If the application overrides MatterWindowCoveringClusterServerAttributeChangedCallback,
 * it should call the PostAttributeChange on its own.
 *
 * @param[in] endpoint
 * @param[in] attributeId
 */
void PostAttributeChange(chip::EndpointId endpoint, chip::AttributeId attributeId);

void SetDefaultDelegate(EndpointId endpoint, Delegate * delegate);

} // namespace WindowCovering
} // namespace Clusters
} // namespace app
} // namespace chip

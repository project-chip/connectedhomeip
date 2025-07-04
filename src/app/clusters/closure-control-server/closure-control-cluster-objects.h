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

#include <app-common/zap-generated/cluster-objects.h>
#include <lib/support/CommonIterator.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ClosureControl {

/**
 * Structure represents the overall state of a closure control cluster derivation instance.
 */
struct GenericOverallCurrentState : public Structs::OverallCurrentStateStruct::Type
{
    GenericOverallCurrentState(Optional<DataModel::Nullable<CurrentPositionEnum>> positionValue = NullOptional,
                               Optional<DataModel::Nullable<bool>> latchValue                   = NullOptional,
                               Optional<Globals::ThreeLevelAutoEnum> speedValue                 = NullOptional,
                               Optional<DataModel::Nullable<bool>> secureStateValue             = NullOptional)
    {
        Set(positionValue, latchValue, speedValue, secureStateValue);
    }

    GenericOverallCurrentState(const GenericOverallCurrentState & overallCurrentState) { *this = overallCurrentState; }

    GenericOverallCurrentState & operator=(const GenericOverallCurrentState & overallCurrentState)
    {
        Set(overallCurrentState.position, overallCurrentState.latch, overallCurrentState.speed, overallCurrentState.secureState);
        return *this;
    }

    void Set(Optional<DataModel::Nullable<CurrentPositionEnum>> positionValue = NullOptional,
             Optional<DataModel::Nullable<bool>> latchValue                   = NullOptional,
             Optional<Globals::ThreeLevelAutoEnum> speedValue                 = NullOptional,
             Optional<DataModel::Nullable<bool>> secureStateValue             = NullOptional)
    {
        position    = positionValue;
        latch       = latchValue;
        speed       = speedValue;
        secureState = secureStateValue;
    }

    bool operator==(const Structs::OverallCurrentStateStruct::Type & rhs) const
    {
        return position == rhs.position && latch == rhs.latch && speed == rhs.speed && secureState == rhs.secureState;
    }
};

/**
 * Structure represents the overall target state of a closure control cluster derivation instance.
 */
struct GenericOverallTargetState : public Structs::OverallTargetStateStruct::Type
{
    GenericOverallTargetState(Optional<DataModel::Nullable<TargetPositionEnum>> positionValue = NullOptional,
                              Optional<DataModel::Nullable<bool>> latchValue                  = NullOptional,
                              Optional<Globals::ThreeLevelAutoEnum> speedValue                = NullOptional)
    {
        Set(positionValue, latchValue, speedValue);
    }

    GenericOverallTargetState(const GenericOverallTargetState & overallTarget) { *this = overallTarget; }

    GenericOverallTargetState & operator=(const GenericOverallTargetState & overallTarget)
    {
        Set(overallTarget.position, overallTarget.latch, overallTarget.speed);
        return *this;
    }

    void Set(Optional<DataModel::Nullable<TargetPositionEnum>> positionValue = NullOptional,
             Optional<DataModel::Nullable<bool>> latchValue                  = NullOptional,
             Optional<Globals::ThreeLevelAutoEnum> speedValue                = NullOptional)
    {
        position = positionValue;
        latch    = latchValue;
        speed    = speedValue;
    }

    bool operator==(const Structs::OverallTargetStateStruct::Type & rhs) const
    {
        return position == rhs.position && latch == rhs.latch && speed == rhs.speed;
    }
};

} // namespace ClosureControl
} // namespace Clusters
} // namespace app
} // namespace chip

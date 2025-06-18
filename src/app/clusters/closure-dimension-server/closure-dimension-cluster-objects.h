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

namespace chip {
namespace app {
namespace Clusters {
namespace ClosureDimension {

/**
 * Structure represents the current state struct of a closure dimension cluster derivation instance.
 */
struct GenericCurrentStateStruct : public Structs::CurrentStateStruct::Type
{
    GenericCurrentStateStruct(Optional<Percent100ths> positionValue = NullOptional, Optional<bool> latchValue = NullOptional,
                              Optional<Globals::ThreeLevelAutoEnum> speedValue = NullOptional)
    {
        Set(positionValue, latchValue, speedValue);
    }

    GenericCurrentStateStruct(const GenericCurrentStateStruct & currentState) { *this = currentState; }

    GenericCurrentStateStruct & operator=(const GenericCurrentStateStruct & current)
    {
        Set(current.position, current.latch, current.speed);
        return *this;
    }

    void Set(Optional<Percent100ths> positionValue = NullOptional, Optional<bool> latchValue = NullOptional,
             Optional<Globals::ThreeLevelAutoEnum> speedValue = NullOptional)
    {
        position = positionValue;
        latch    = latchValue;
        speed    = speedValue;
    }

    bool operator==(const GenericCurrentStateStruct & rhs) const
    {
        return position == rhs.position && latch == rhs.latch && speed == rhs.speed;
    }

    bool operator!=(const GenericCurrentStateStruct & rhs) const
    {
        return position != rhs.position || latch != rhs.latch || speed != rhs.speed;
    }
};

/**
 * Structure represents the target struct of a closure dimension cluster derivation instance.
 */
struct GenericTargetStruct : public Structs::TargetStruct::Type
{
    GenericTargetStruct(Optional<Percent100ths> positionValue = NullOptional, Optional<bool> latchValue = NullOptional,
                        Optional<Globals::ThreeLevelAutoEnum> speedValue = NullOptional)
    {
        Set(positionValue, latchValue, speedValue);
    }

    GenericTargetStruct(const GenericTargetStruct & target) { *this = target; }

    GenericTargetStruct & operator=(const GenericTargetStruct & target)
    {
        Set(target.position, target.latch, target.speed);
        return *this;
    }

    void Set(Optional<Percent100ths> positionValue = NullOptional, Optional<bool> latchValue = NullOptional,
             Optional<Globals::ThreeLevelAutoEnum> speedValue = NullOptional)
    {
        position = positionValue;
        latch    = latchValue;
        speed    = speedValue;
    }

    bool operator==(const GenericTargetStruct & rhs) const
    {
        return position == rhs.position && latch == rhs.latch && speed == rhs.speed;
    }

    bool operator!=(const GenericTargetStruct & rhs) const
    {
        return position != rhs.position || latch != rhs.latch || speed != rhs.speed;
    }
};
} // namespace ClosureDimension
} // namespace Clusters
} // namespace app
} // namespace chip

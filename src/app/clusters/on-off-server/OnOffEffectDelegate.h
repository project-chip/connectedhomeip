/*
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <app/data-model-provider/ActionReturnStatus.h>
#include <clusters/OnOff/Enums.h>

#include <cstdint>

namespace chip::app::Clusters::OnOff {

/// Interface for handling lighting effects.
class OnOffEffectDelegate
{
public:
    virtual ~OnOffEffectDelegate() = default;

    virtual DataModel::ActionReturnStatus TriggerEffect(EffectIdentifierEnum effectId, uint8_t effectVariant)
    {
        switch (effectId)
        {
        case EffectIdentifierEnum::kDelayedAllOff:
            return TriggerDelayedAllOff();
        case EffectIdentifierEnum::kDyingLight:
            return TriggerDyingLight();
        default:
            return Protocols::InteractionModel::Status::InvalidCommand;
        }
    }

    virtual DataModel::ActionReturnStatus TriggerDelayedAllOff() { return Protocols::InteractionModel::Status::UnsupportedCommand; }

    virtual DataModel::ActionReturnStatus TriggerDyingLight() { return Protocols::InteractionModel::Status::UnsupportedCommand; }
};

} // namespace chip::app::Clusters::OnOff

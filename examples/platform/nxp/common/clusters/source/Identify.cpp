/*
 *    Copyright (c) 2026 Project CHIP Authors
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

#include "Identify.h"

using namespace chip::app;
using namespace chip::NXP::App;
namespace chip::NXP::App {
void IdentifyDelegate::OnIdentifyStart(chip::app::Clusters::IdentifyCluster & cluster)
{
    ChipLogProgress(Zcl, "OnIdentifyStart");
    // TODO: logic to start identification (e.g., start blinking an LED)
}

void IdentifyDelegate::OnIdentifyStop(chip::app::Clusters::IdentifyCluster & cluster)
{
    ChipLogProgress(Zcl, "OnIdentifyStop");
    // TODO: logic to stop identification (e.g., stop blinking an LED)
}

void IdentifyDelegate::OnTriggerEffect(chip::app::Clusters::IdentifyCluster & cluster)
{

    switch (cluster.GetEffectIdentifier())
    {
    case Clusters::Identify::EffectIdentifierEnum::kBlink:
        ChipLogProgress(Zcl, "Clusters::Identify::EffectIdentifierEnum::kBlink");
        break;
    case Clusters::Identify::EffectIdentifierEnum::kBreathe:
        ChipLogProgress(Zcl, "Clusters::Identify::EffectIdentifierEnum::kBreathe");
        break;
    case Clusters::Identify::EffectIdentifierEnum::kOkay:
        ChipLogProgress(Zcl, "Clusters::Identify::EffectIdentifierEnum::kOkay");
        break;
    case Clusters::Identify::EffectIdentifierEnum::kChannelChange:
        ChipLogProgress(Zcl, "Clusters::Identify::EffectIdentifierEnum::kChannelChange");
        break;
    default:
        ChipLogProgress(Zcl, "No identifier effect");
        return;
    }
    // TODO: logic to trigger a specific effect
}

bool IdentifyDelegate::IsTriggerEffectEnabled() const
{
    return true;
}
} // namespace chip::NXP::App

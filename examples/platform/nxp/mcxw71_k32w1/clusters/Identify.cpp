/*
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

#include "Identify.h"
#include "AppTask.h"
#include "UserInterfaceFeedback.h"

#include <app-common/zap-generated/ids/Clusters.h>

using namespace chip;
using namespace chip::app;

static Identify gIdentify = { EndpointId{ 1 }, NXP::App::OnIdentifyStart, NXP::App::OnIdentifyStop,
                              Clusters::Identify::IdentifyTypeEnum::kVisibleIndicator, NXP::App::OnTriggerEffect,
                              // Use invalid value for identifiers to enable TriggerEffect command
                              // to stop Identify command for each effect
                              Clusters::Identify::EffectIdentifierEnum::kUnknownEnumValue,
                              Clusters::Identify::EffectVariantEnum::kDefault };

namespace chip::NXP::App {

void OnIdentifyStart(Identify * identify)
{
    chip::DeviceLayer::SystemLayer().CancelTimer(OnTriggerEffectComplete, identify);
    OnTriggerEffectComplete(&chip::DeviceLayer::SystemLayer(), identify);

    ChipLogProgress(Zcl, "Identify process has started. Status LED should blink with a period of 0.5 seconds.");

#if CONFIG_ENABLE_FEEDBACK
    FeedbackMgr().DisplayOnAction(UserInterfaceFeedback::Action::kIdentify);
#endif
}

void OnIdentifyStop(Identify * identify)
{
    ChipLogProgress(Zcl, "Identify process has stopped.");

#if CONFIG_ENABLE_FEEDBACK
    FeedbackMgr().RestoreState();
#endif
}

void OnTriggerEffectComplete(chip::System::Layer * systemLayer, void * appState)
{
    // Let Identify command take over if called during TriggerEffect already running
    ChipLogProgress(Zcl, "TriggerEffect has stopped.");

    // TriggerEffect finished - reset identifiers
    // Use invalid value for identifiers to enable TriggerEffect command
    // to stop Identify command for each effect
    gIdentify.mCurrentEffectIdentifier = Clusters::Identify::EffectIdentifierEnum::kUnknownEnumValue;
    gIdentify.mTargetEffectIdentifier  = Clusters::Identify::EffectIdentifierEnum::kUnknownEnumValue;
    gIdentify.mEffectVariant           = Clusters::Identify::EffectVariantEnum::kDefault;

#if CONFIG_ENABLE_FEEDBACK
    FeedbackMgr().RestoreState();
#endif
}

void OnTriggerEffect(Identify * identify)
{
    uint16_t timerDelay = 0;

    ChipLogProgress(Zcl, "TriggerEffect has started.");

    switch (identify->mCurrentEffectIdentifier)
    {
    case Clusters::Identify::EffectIdentifierEnum::kBlink:
        timerDelay = 2;
        break;

    case Clusters::Identify::EffectIdentifierEnum::kBreathe:
        timerDelay = 15;
        break;

    case Clusters::Identify::EffectIdentifierEnum::kOkay:
        timerDelay = 4;
        break;

    case Clusters::Identify::EffectIdentifierEnum::kChannelChange:
        ChipLogProgress(Zcl, "Channel Change effect not supported, using effect %d",
                        to_underlying(Clusters::Identify::EffectIdentifierEnum::kBlink));
        timerDelay = 2;
        break;

    case Clusters::Identify::EffectIdentifierEnum::kFinishEffect:
        chip::DeviceLayer::SystemLayer().CancelTimer(OnTriggerEffectComplete, identify);
        timerDelay = 1;
        break;

    case Clusters::Identify::EffectIdentifierEnum::kStopEffect:
        chip::DeviceLayer::SystemLayer().CancelTimer(OnTriggerEffectComplete, identify);
        OnTriggerEffectComplete(&chip::DeviceLayer::SystemLayer(), identify);
        break;

    default:
        ChipLogProgress(Zcl, "Invalid effect identifier.");
    }

    if (timerDelay)
    {
#if CONFIG_ENABLE_FEEDBACK
        FeedbackMgr().DisplayOnAction(UserInterfaceFeedback::Action::kTriggerEffect);
#endif
        chip::DeviceLayer::SystemLayer().StartTimer(chip::System::Clock::Seconds16(timerDelay), OnTriggerEffectComplete, identify);
    }
}

} // namespace chip::NXP::App

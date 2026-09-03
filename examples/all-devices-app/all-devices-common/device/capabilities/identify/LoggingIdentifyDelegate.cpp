/*
 *
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

#include "LoggingIdentifyDelegate.h"

namespace chip {
namespace app {

void LoggingIdentifyDelegate::OnIdentifyStart(Clusters::IdentifyCluster & cluster)
{
    ChipLogProgress(DeviceLayer, "LoggingIdentifyDelegate: Identify START");
}

void LoggingIdentifyDelegate::OnIdentifyStop(Clusters::IdentifyCluster & cluster)
{
    ChipLogProgress(DeviceLayer, "LoggingIdentifyDelegate: Identify STOP");
}

void LoggingIdentifyDelegate::OnTriggerEffect(Clusters::IdentifyCluster & cluster)
{
    StringBuilder<64> msg;

    switch (cluster.GetEffectIdentifier())
    {
    case Clusters::Identify::EffectIdentifierEnum::kBlink:
        msg.Add("BlinkEffect");
        break;
    case Clusters::Identify::EffectIdentifierEnum::kBreathe:
        msg.Add("BreatheEffect");
        break;
    case Clusters::Identify::EffectIdentifierEnum::kOkay:
        msg.Add("OkayEffect");
        break;
    case Clusters::Identify::EffectIdentifierEnum::kChannelChange:
        msg.Add("ChannelChangeEffect");
        break;
    case Clusters::Identify::EffectIdentifierEnum::kFinishEffect:
        msg.Add("FinishEffect");
        break;
    case Clusters::Identify::EffectIdentifierEnum::kStopEffect:
        msg.Add("StopEffect");
        break;
    default:
        msg.AddFormat("UnknownEffect(%d)", static_cast<int>(cluster.GetEffectIdentifier()));
        break;
    }
    msg.Add(" / ");
    switch (cluster.GetEffectVariant())
    {
    case Clusters::Identify::EffectVariantEnum::kDefault:
        msg.Add("DefaultVariant");
        break;
    default:
        msg.AddFormat("UnknownVariant(%d)", static_cast<int>(cluster.GetEffectVariant()));
        break;
    }
    ChipLogProgress(DeviceLayer, "LoggingIdentifyDelegate: TriggerEffect: %s", msg.c_str());
}

bool LoggingIdentifyDelegate::IsTriggerEffectEnabled() const
{
    return true;
}

} // namespace app
} // namespace chip

/*
 *
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
#include "SilabsIdentifyDelegate.h"

#include "BaseApplication.h"

#include <lib/support/logging/CHIPLogging.h>

namespace chip {
namespace app {

void SilabsIdentifyLedDelegate::OnIdentifyStart(Clusters::IdentifyCluster & /*cluster*/)
{
    ChipLogProgress(AppServer, "Identify started (silabs LED)");
    BaseApplication::NotifyCodeDrivenIdentifyStart();
}

void SilabsIdentifyLedDelegate::OnIdentifyStop(Clusters::IdentifyCluster & /*cluster*/)
{
    ChipLogProgress(AppServer, "Identify stopped (silabs LED)");
    BaseApplication::NotifyCodeDrivenIdentifyStop();
}

void SilabsIdentifyLedDelegate::OnTriggerEffect(Clusters::IdentifyCluster & cluster)
{
    BaseApplication::NotifyCodeDrivenTriggerEffect(cluster.GetEffectIdentifier(), cluster.GetEffectVariant());
}

} // namespace app
} // namespace chip

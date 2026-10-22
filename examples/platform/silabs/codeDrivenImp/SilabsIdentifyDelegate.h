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
#pragma once

#include <app/clusters/identify-server/IdentifyCluster.h>

namespace chip {
namespace app {

/**
 * SiLabs status-LED-driving `IdentifyDelegate` for the code-driven
 * `all-devices-app`.
 *
 * Installed once at boot into `PlatformIdentifyIntegration` (see
 * `AppTask::Init`). Bridges IdentifyCluster callbacks to
 * `BaseApplication::NotifyCodeDriven*`, which drive the same status-LED
 * blink patterns used by the Ember-based Identify plugin in other
 * SiLabs sample apps.
 */
class SilabsIdentifyLedDelegate : public Clusters::IdentifyDelegate
{
public:
    SilabsIdentifyLedDelegate()           = default;
    ~SilabsIdentifyLedDelegate() override = default;

    void OnIdentifyStart(Clusters::IdentifyCluster & cluster) override;
    void OnIdentifyStop(Clusters::IdentifyCluster & cluster) override;
    void OnTriggerEffect(Clusters::IdentifyCluster & cluster) override;
    bool IsTriggerEffectEnabled() const override { return true; }
};

} // namespace app
} // namespace chip

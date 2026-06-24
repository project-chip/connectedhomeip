/*
 *    Copyright (c) 2024, 2026 Project CHIP Authors
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

#include <app/clusters/identify-server/identify-server.h>

namespace chip::NXP::App {

// MyIdentifyDelegate class declaration
class IdentifyDelegate : public chip::app::Clusters::IdentifyDelegate
{
public:
    void OnIdentifyStart(chip::app::Clusters::IdentifyCluster & cluster) override;
    void OnIdentifyStop(chip::app::Clusters::IdentifyCluster & cluster) override;
    void OnTriggerEffect(chip::app::Clusters::IdentifyCluster & cluster) override;
    bool IsTriggerEffectEnabled() const override;
};

} // namespace chip::NXP::App

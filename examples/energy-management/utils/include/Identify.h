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

#include <app/clusters/identify-server/IdentifyCluster.h>
#include <lib/core/DataModelTypes.h>

using chip::app::Clusters::IdentifyCluster;
using chip::app::Clusters::IdentifyDelegate;

class IdentifyDelegateImpl : public IdentifyDelegate
{
public:
    void OnIdentifyStart(IdentifyCluster & cluster) override { ChipLogProgress(Zcl, "OnIdentifyStart"); }
    void OnIdentifyStop(IdentifyCluster & cluster) override { ChipLogProgress(Zcl, "OnIdentifyStop"); }
    void OnTriggerEffect(IdentifyCluster & cluster) override {}
    bool IsTriggerEffectEnabled() const override { return false; }
};

// Common interface to get the Identify endpoint ID.
chip::EndpointId GetIdentifyEndpointId();

CHIP_ERROR IdentifyInit();

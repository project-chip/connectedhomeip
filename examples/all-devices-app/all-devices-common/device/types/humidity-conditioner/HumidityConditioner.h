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

#include <app/clusters/humidistat-server/HumidistatCluster.h>
#include <app/clusters/identify-server/IdentifyCluster.h>
#include <app/clusters/on-off-server/OnOffCluster.h>
#include <app/clusters/on-off-server/OnOffDelegate.h>
#include <data-model-providers/codedriven/CodeDrivenDataModelProvider.h>
#include <device/api/SingleEndpoint.h>
#include <lib/support/TimerDelegate.h>

namespace chip::app {

/// Humidity Conditioner (device type 0x007D): Identify + On/Off (Dead Front Behavior) +
/// Humidistat (all features, matching all-clusters-app defaults).
class HumidityConditioner : public SingleEndpoint
{
public:
    struct Config
    {
        TimerDelegate & timerDelegate;
        Clusters::IdentifyDelegate & identifyDelegate;
        Clusters::OnOffDelegate & onOffDelegate;
        Clusters::HumidistatDelegate & humidistatDelegate;
    };

    explicit HumidityConditioner(const Config & config);
    ~HumidityConditioner() override = default;

    CHIP_ERROR Register(EndpointId endpoint, CodeDrivenDataModelProvider & provider, EndpointComposition composition = {}) override;
    void Unregister(CodeDrivenDataModelProvider & provider) override;

    // Public cluster getters for programmatic control
    Clusters::IdentifyCluster & IdentifyCluster() { return mIdentifyCluster.Cluster(); }
    Clusters::OnOffCluster & OnOffCluster() { return mOnOffCluster.Cluster(); }
    Clusters::HumidistatCluster & HumidistatCluster() { return mHumidistatCluster.Cluster(); }

private:
    TimerDelegate & mTimerDelegate;
    Clusters::IdentifyDelegate & mIdentifyDelegate;
    Clusters::OnOffDelegate & mOnOffDelegate;
    Clusters::HumidistatDelegate & mHumidistatDelegate;

    LazyRegisteredServerCluster<Clusters::IdentifyCluster> mIdentifyCluster;
    LazyRegisteredServerCluster<Clusters::OnOffCluster> mOnOffCluster;
    LazyRegisteredServerCluster<Clusters::HumidistatCluster> mHumidistatCluster;
};

} // namespace chip::app

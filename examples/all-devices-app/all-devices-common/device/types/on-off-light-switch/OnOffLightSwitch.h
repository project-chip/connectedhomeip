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

#pragma once

#include <app/clusters/bindings/BindingCluster.h>
#include <app/clusters/bindings/BindingManager.h>
#include <app/clusters/bindings/binding-table.h>
#include <app/clusters/identify-server/IdentifyCluster.h>
#include <binding-handler/BindingHandler.h>
#include <clusters/Identify/Enums.h>
#include <device/api/SingleEndpoint.h>
#include <lib/support/TimerDelegate.h>
#include <platform/PlatformManager.h>

namespace chip::app {

/**
 * On/Off Light Switch device type. Sends OnOff and Identify commands to the peers in the binding
 * table of its endpoint (e.g. an On/Off Light), through the application's BindingHandler.
 *
 * The Send* methods must be called from the Matter event loop or with the Matter stack lock held.
 * They return CHIP_ERROR_INCORRECT_STATE if the device is not registered, and otherwise follow the
 * BindingHandler::Dispatch contract: if no binding matches the command cluster, CHIP_NO_ERROR is
 * returned and nothing is sent.
 */
class OnOffLightSwitch : public SingleEndpoint
{
public:
    struct Config
    {
        TimerDelegate & timerDelegate;
        DeviceLayer::PlatformManager & platformManager;
        Clusters::Binding::Table & bindingTable;
        Clusters::Binding::Manager & bindingManager;
        BindingHandler & bindingHandler;
    };

    explicit OnOffLightSwitch(const Config & config);
    ~OnOffLightSwitch() override = default;

    CHIP_ERROR Register(chip::EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                        EndpointComposition composition = {}) override;
    void Unregister(CodeDrivenDataModelProvider & provider) override;

    // Endpoint interface overrides
    CHIP_ERROR ClientClusters(ReadOnlyBufferBuilder<ClusterId> & out) const override;

    CHIP_ERROR SendOn();
    CHIP_ERROR SendOff();
    CHIP_ERROR SendToggle();
    CHIP_ERROR SendIdentify(uint16_t identifyTimeSeconds);
    CHIP_ERROR SendTriggerEffect(Clusters::Identify::EffectIdentifierEnum effectIdentifier,
                                 Clusters::Identify::EffectVariantEnum effectVariant);

    // Public getters for programmatic control
    Clusters::IdentifyCluster & IdentifyCluster() { return mIdentifyCluster.Cluster(); }
    Clusters::BindingCluster & BindingCluster() { return mBindingCluster.Cluster(); }

private:
    template <typename CommandType>
    CHIP_ERROR SendCommand(const CommandType & command);

    Config mConfig;
    LazyRegisteredServerCluster<Clusters::IdentifyCluster> mIdentifyCluster;
    LazyRegisteredServerCluster<Clusters::BindingCluster> mBindingCluster;
};

} // namespace chip::app

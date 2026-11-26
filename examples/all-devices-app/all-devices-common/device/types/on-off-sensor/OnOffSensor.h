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
#include <app/clusters/identify-server/IdentifyCluster.h>
#include <binding-handler/BindingHandler.h>
#include <device/api/SingleEndpoint.h>
#include <lib/support/TimerDelegate.h>

namespace chip::app {

/**
 * Sends On/Off and Identify commands to the peers bound to this sensor's endpoint.
 *
 * Dependencies must outlive the device. Call Send* from the Matter event loop or with the stack lock held.
 * Send* returns CHIP_ERROR_INCORRECT_STATE while unregistered. Otherwise BindingHandler::Dispatch applies:
 * success means the command was dispatched or queued; no matching binding is a successful no-op.
 */
class OnOffSensor : public SingleEndpoint
{
public:
    struct Context
    {
        TimerDelegate & timerDelegate;
        DeviceLayer::PlatformManager & platformManager;
        Clusters::Binding::Table & bindingTable;
        Clusters::Binding::Manager & bindingManager;
        BindingHandler & bindingHandler;
    };

    OnOffSensor(const Context & context, Clusters::IdentifyDelegate & identifyDelegate);
    ~OnOffSensor() override = default;

    CHIP_ERROR Register(EndpointId endpoint, CodeDrivenDataModelProvider & provider, EndpointComposition composition = {}) override;
    void Unregister(CodeDrivenDataModelProvider & provider) override;
    CHIP_ERROR ClientClusters(ReadOnlyBufferBuilder<ClusterId> & out) const override;

    CHIP_ERROR SendOn();
    CHIP_ERROR SendOff();
    CHIP_ERROR SendToggle();
    CHIP_ERROR SendIdentify(uint16_t identifyTimeSeconds);

    /// Cluster accessors require successful registration.
    Clusters::IdentifyCluster & IdentifyCluster()
    {
        VerifyOrDie(mIdentifyCluster.IsConstructed());
        return mIdentifyCluster.Cluster();
    }
    Clusters::BindingCluster & BindingCluster()
    {
        VerifyOrDie(mBindingCluster.IsConstructed());
        return mBindingCluster.Cluster();
    }

protected:
    virtual CHIP_ERROR RegisterOptionalClusters(EndpointId endpoint, CodeDrivenDataModelProvider & provider)
    {
        return CHIP_NO_ERROR;
    }
    virtual void UnregisterOptionalClusters(CodeDrivenDataModelProvider & provider) {}

private:
    template <typename CommandType>
    CHIP_ERROR SendCommand(const CommandType & command);

    const Context mContext;
    Clusters::IdentifyDelegate & mIdentifyDelegate;
    LazyRegisteredServerCluster<Clusters::IdentifyCluster> mIdentifyCluster;
    LazyRegisteredServerCluster<Clusters::BindingCluster> mBindingCluster;
};

} // namespace chip::app

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

#include "OnOffLightSwitchDevice.h"

#include <app/clusters/bindings/BindingManager.h>
#include <app/clusters/bindings/binding-table.h>
#include <clusters/Identify/Ids.h>
#include <clusters/OnOff/Ids.h>
#include <devices/Types.h>
#include <platform/PlatformManager.h>

using namespace chip::app::Clusters;

namespace chip::app {

namespace {
const ClusterId kClientClusters[] = { OnOff::Id, Identify::Id };
} // namespace

OnOffLightSwitchDevice::OnOffLightSwitchDevice(TimerDelegate & timerDelegate) :
    SingleEndpointDevice(Span<const DataModel::DeviceTypeEntry>(&Device::Type::kOnOffLightSwitch, 1)), mTimerDelegate(timerDelegate)
{}

CHIP_ERROR OnOffLightSwitchDevice::Register(chip::EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                                            EndpointComposition composition)
{
    VerifyOrReturnError(mEndpointId == kInvalidEndpointId, CHIP_ERROR_INCORRECT_STATE);
    DeviceRegistrationTransaction transaction(*this, provider);

    ReturnErrorOnFailure(RegisterDescriptor(endpoint, provider, composition));

    mIdentifyCluster.Create(IdentifyCluster::Config(endpoint, mTimerDelegate));
    ReturnErrorOnFailure(provider.AddCluster(mIdentifyCluster.Registration()));

    mBindingCluster.Create(
        BindingCluster::Context{
            .bindingTable    = Binding::Table::GetInstance(),
            .bindingManager  = Binding::Manager::GetInstance(),
            .platformManager = DeviceLayer::PlatformMgr(),
        },
        endpoint);
    ReturnErrorOnFailure(provider.AddCluster(mBindingCluster.Registration()));

    ReturnErrorOnFailure(provider.AddEndpoint(mEndpointRegistration));
    transaction.Commit();
    return CHIP_NO_ERROR;
}

void OnOffLightSwitchDevice::Unregister(CodeDrivenDataModelProvider & provider)
{
    UnregisterDescriptor(provider);
    if (mBindingCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mBindingCluster.Cluster()));
        mBindingCluster.Destroy();
    }
    if (mIdentifyCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mIdentifyCluster.Cluster()));
        mIdentifyCluster.Destroy();
    }
}

CHIP_ERROR OnOffLightSwitchDevice::ClientClusters(ReadOnlyBufferBuilder<ClusterId> & out) const
{
    return out.ReferenceExisting(Span<const ClusterId>(kClientClusters));
}

} // namespace chip::app

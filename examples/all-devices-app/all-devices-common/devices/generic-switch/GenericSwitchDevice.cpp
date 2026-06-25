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
#include <devices/Types.h>
#include <devices/generic-switch/GenericSwitchDevice.h>

using namespace chip::app::Clusters;

namespace chip::app {

GenericSwitchDevice::GenericSwitchDevice(TimerDelegate & timerDelegate) :
    SingleEndpointDevice(Span<const DataModel::DeviceTypeEntry>(&Device::Type::kGenericSwitch, 1)), mTimerDelegate(timerDelegate)
{}

CHIP_ERROR GenericSwitchDevice::Register(chip::EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                                         EndpointComposition composition)
{
    VerifyOrReturnError(mEndpointId == kInvalidEndpointId, CHIP_ERROR_INCORRECT_STATE);
    DeviceRegistrationTransaction transaction(*this, provider);

    ReturnErrorOnFailure(RegisterDescriptor(endpoint, provider, composition));

    mIdentifyCluster.Create(IdentifyCluster::Config(endpoint, mTimerDelegate));
    ReturnErrorOnFailure(provider.AddCluster(mIdentifyCluster.Registration()));

    mSwitchCluster.Create(endpoint, BitFlags<Switch::Feature>(Switch::Feature::kLatchingSwitch),
                          SwitchCluster::StartupConfiguration{ .numberOfPositions = 2 });
    ReturnErrorOnFailure(provider.AddCluster(mSwitchCluster.Registration()));

    ReturnErrorOnFailure(provider.AddEndpoint(mEndpointRegistration));
    transaction.Commit();
    return CHIP_NO_ERROR;
}

void GenericSwitchDevice::Unregister(CodeDrivenDataModelProvider & provider)
{
    UnregisterDescriptor(provider);
    if (mSwitchCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mSwitchCluster.Cluster()));
        mSwitchCluster.Destroy();
    }
    if (mIdentifyCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mIdentifyCluster.Cluster()));
        mIdentifyCluster.Destroy();
    }
}

} // namespace chip::app

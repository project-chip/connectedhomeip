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

#include "TemperatureControlledCabinetPart.h"

#include <devices/Types.h>

namespace chip::app {

TemperatureControlledCabinetPart::TemperatureControlledCabinetPart(TimerDelegate & timerDelegate,
                                                                   Clusters::IdentifyDelegate & identifyDelegate) :
    TemperatureControlledCabinetPart(timerDelegate, Config{}, identifyDelegate)
{}

TemperatureControlledCabinetPart::TemperatureControlledCabinetPart(TimerDelegate & timerDelegate, Config config,
                                                                   Clusters::IdentifyDelegate & identifyDelegate) :
    SingleEndpoint(Span<const DataModel::DeviceTypeEntry>(&Device::Type::kTemperatureControlledCabinet, 1)),
    mTimerDelegate(timerDelegate), mConfig(config), mIdentifyDelegate(identifyDelegate)
{}

CHIP_ERROR TemperatureControlledCabinetPart::Register(EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                                                      EndpointComposition composition)
{
    DeviceRegistrationTransaction transaction(*this, provider);

    ReturnErrorOnFailure(RegisterDescriptor(endpoint, provider, composition));

    mIdentifyCluster.Create(Clusters::IdentifyCluster::Config(endpoint, mTimerDelegate).WithDelegate(&mIdentifyDelegate));
    mTemperatureControlCluster.Create(
        endpoint, BitFlags<Clusters::TemperatureControl::Feature>(Clusters::TemperatureControl::Feature::kTemperatureNumber),
        Clusters::TemperatureControlCluster::StartupConfiguration{
            .temperatureSetpoint = mConfig.temperatureSetpoint,
            .minTemperature      = mConfig.minTemperature,
            .maxTemperature      = mConfig.maxTemperature,
            .step                = mConfig.step,
        });

    ReturnErrorOnFailure(provider.AddCluster(mIdentifyCluster.Registration()));
    ReturnErrorOnFailure(provider.AddCluster(mTemperatureControlCluster.Registration()));

    mOperationalStateCluster.Create(endpoint, &mOperationalStateDelegate);
    mOperationalStateDelegate.SetCluster(&mOperationalStateCluster.Cluster());
    ReturnErrorOnFailure(provider.AddCluster(mOperationalStateCluster.Registration()));

    ReturnErrorOnFailure(provider.AddEndpoint(mEndpointRegistration));

    transaction.Commit();
    return CHIP_NO_ERROR;
}

void TemperatureControlledCabinetPart::Unregister(CodeDrivenDataModelProvider & provider)
{
    UnregisterDescriptor(provider);

    if (mOperationalStateCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mOperationalStateCluster.Cluster()));
        mOperationalStateCluster.Destroy();
    }

    if (mTemperatureControlCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mTemperatureControlCluster.Cluster()));
        mTemperatureControlCluster.Destroy();
    }

    if (mIdentifyCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mIdentifyCluster.Cluster()));
        mIdentifyCluster.Destroy();
    }
}

} // namespace chip::app

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

#include "ElectricalSensor.h"
#include <devices/Types.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip::app::Clusters;

namespace chip::app {

ElectricalSensor::ElectricalSensor(const Config & config) :
    SingleEndpoint(Span<const DataModel::DeviceTypeEntry>(&Device::Type::kElectricalSensor, 1)), mConfig(config)
{}

CHIP_ERROR ElectricalSensor::Register(chip::EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                                      EndpointComposition composition)
{
    ReturnErrorOnFailure(RegisterDescriptor(endpoint, provider, composition));

    // ElectricalEnergyMeasurement Cluster

    {
        ElectricalEnergyMeasurementClusterT::Config config{
            .endpointId         = endpoint,
            .featureFlags       = mConfig.electricalEnergyMeasurementFeatureFlags,
            .optionalAttributes = mConfig.electricalEnergyMeasurementOptionalAttributes,
            .accuracyStruct     = mConfig.electricalEnergyMeasurementAccuracyStruct,
            .delegate           = mConfig.electricalEnergyDelegate,
            .timerDelegate      = mConfig.timerDelegate,
        };

        mElectricalEnergyMeasurementCluster.Create(config);
        ReturnErrorOnFailure(provider.AddCluster(mElectricalEnergyMeasurementCluster.Registration()));
    }

    // ElectricalPowerMeasurement Cluster

    {
        ElectricalPowerMeasurementClusterT::Config config{
            .endpointId         = endpoint,
            .delegate           = mConfig.electricalPowerDelegate,
            .features           = mConfig.electricalPowerMeasurementFeatureFlags,
            .optionalAttributes = mConfig.electricalPowerMeasurementOptionalAttributes,
        };
        mElectricalPowerMeasurementCluster.Create(config);
        ReturnErrorOnFailure(provider.AddCluster(mElectricalPowerMeasurementCluster.Registration()));
    }

    // PowerTopology Cluster

    {
        PowerTopologyClusterT::Config config{ .endpointId = endpoint,
                                              .delegate   = mConfig.powerTopologyDelegate,
                                              .features   = mConfig.powerTopologyFeatures };
        mPowerTopologyCluster.Create(config);
        ReturnErrorOnFailure(provider.AddCluster(mPowerTopologyCluster.Registration()));
    }

    return provider.AddEndpoint(mEndpointRegistration);
}

void ElectricalSensor::Unregister(CodeDrivenDataModelProvider & provider)
{
    UnregisterDescriptor(provider);
    if (mElectricalEnergyMeasurementCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mElectricalEnergyMeasurementCluster.Cluster()));
        mElectricalEnergyMeasurementCluster.Destroy();
    }
    if (mElectricalPowerMeasurementCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mElectricalPowerMeasurementCluster.Cluster()));
        mElectricalPowerMeasurementCluster.Destroy();
    }
    if (mPowerTopologyCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mPowerTopologyCluster.Cluster()));
        mPowerTopologyCluster.Destroy();
    }
}

ElectricalSensor::ElectricalEnergyMeasurementClusterT & ElectricalSensor::ElectricalEnergyMeasurementCluster()
{
    VerifyOrDie(mElectricalEnergyMeasurementCluster.IsConstructed());
    return mElectricalEnergyMeasurementCluster.Cluster();
}

ElectricalSensor::ElectricalPowerMeasurementClusterT & ElectricalSensor::ElectricalPowerMeasurementCluster()
{
    VerifyOrDie(mElectricalPowerMeasurementCluster.IsConstructed());
    return mElectricalPowerMeasurementCluster.Cluster();
}

ElectricalSensor::PowerTopologyClusterT & ElectricalSensor::PowerTopologyCluster()
{
    VerifyOrDie(mPowerTopologyCluster.IsConstructed());
    return mPowerTopologyCluster.Cluster();
}

} // namespace chip::app

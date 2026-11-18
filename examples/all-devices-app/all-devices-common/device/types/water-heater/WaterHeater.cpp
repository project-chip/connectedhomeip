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

#include <device/types/water-heater/WaterHeater.h>

#include <devices/Types.h>

namespace chip::app {
WaterHeater::WaterHeater(const Config & config) :
    SingleEndpoint(Span<const DataModel::DeviceTypeEntry>(&Device::Type::kWaterHeater, 1)), mConfig(config)
{}

CHIP_ERROR WaterHeater::Register(chip::EndpointId endpoint, CodeDrivenDataModelProvider & provider, EndpointComposition composition)
{
    VerifyOrReturnError(mEndpointId == kInvalidEndpointId, CHIP_ERROR_INCORRECT_STATE);
    DeviceRegistrationTransaction transaction(*this, provider);

    ReturnErrorOnFailure(RegisterDescriptor(endpoint, provider, composition));

    mWaterHeaterManagementCluster.Create(endpoint, mConfig.waterHeaterManagementDelegate, mConfig.whmFeatures);
    mThermostatCluster.Create(endpoint, BitFlags<Clusters::Thermostat::Feature>(Clusters::Thermostat::Feature::kHeating),
                              HeatingThermostat::Config(mConfig.thermostatOptionalAttributes, mConfig.timerDelegate),
                              mConfig.thermostatDelegate, mConfig.heatingDelegate);
    mWaterHeaterModeCluster.Create(endpoint, Clusters::ModeBase::kWaterHeaterMode,
                                   Clusters::ModeBaseCluster::Config{
                                       .feature                = BitMask<Clusters::ModeBase::Feature>(),
                                       .optionalAttributeSet   = {},
                                       .appDelegate            = mConfig.waterHeaterModeDelegate,
                                       .onOffValueForStartUp   = false,
                                       .diagnosticDataProvider = mConfig.diagnosticDataProvider,
                                   });

    ReturnErrorOnFailure(provider.AddCluster(mWaterHeaterManagementCluster.Registration()));
    ReturnErrorOnFailure(provider.AddCluster(mThermostatCluster.Registration()));
    ReturnErrorOnFailure(provider.AddCluster(mWaterHeaterModeCluster.Registration()));

    ReturnErrorOnFailure(RegisterOptionalClusters(endpoint, provider));

    ReturnErrorOnFailure(provider.AddEndpoint(mEndpointRegistration));
    transaction.Commit();
    return CHIP_NO_ERROR;
}

void WaterHeater::Unregister(CodeDrivenDataModelProvider & provider)
{
    UnregisterDescriptor(provider);
    UnregisterOptionalClusters(provider);
    // Tear down in reverse creation order.
    if (mWaterHeaterModeCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mWaterHeaterModeCluster.Cluster()));
        mWaterHeaterModeCluster.Destroy();
    }
    if (mThermostatCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mThermostatCluster.Cluster()));
        mThermostatCluster.Destroy();
    }
    if (mWaterHeaterManagementCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mWaterHeaterManagementCluster.Cluster()));
        mWaterHeaterManagementCluster.Destroy();
    }
}

Clusters::WaterHeaterManagement::WaterHeaterManagementCluster & WaterHeater::WaterHeaterManagementCluster()
{
    VerifyOrDie(mWaterHeaterManagementCluster.IsConstructed());
    return mWaterHeaterManagementCluster.Cluster();
}

WaterHeater::HeatingThermostat & WaterHeater::ThermostatCluster()
{
    VerifyOrDie(mThermostatCluster.IsConstructed());
    return mThermostatCluster.Cluster();
}

Clusters::ModeBaseCluster & WaterHeater::WaterHeaterModeCluster()
{
    VerifyOrDie(mWaterHeaterModeCluster.IsConstructed());
    return mWaterHeaterModeCluster.Cluster();
}
} // namespace chip::app

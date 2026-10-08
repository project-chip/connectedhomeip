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

namespace chip::app {
WaterHeater::WaterHeater(const Config & config) :
    SingleEndpoint(Span<const DataModel::DeviceTypeEntry>(&Device::Type::kWaterHeater, 1)), mConfig(config),
    mTimerDelegate(config.timerDelegate), mWhmDelegate(config.waterHeaterManagementDelegate),
    mWaterHeaterModeDelegate(config.waterHeaterModeDelegate), mThermostatDelegate(config.thermostatDelegate),
    mHeatingDelegate(config.heatingDelegate)
{}
WaterHeater::~WaterHeater() = default;

CHIP_ERROR WaterHeater::Register(chip::EndpointId endpoint, CodeDrivenDataModelProvider & provider, EndpointComposition composition)
{
    VerifyOrReturnError(mEndpointId == kInvalidEndpointId, CHIP_ERROR_INCORRECT_STATE);
    DeviceRegistrationTransaction transaction(*this, provider);

    ReturnErrorOnFailure(RegisterDescriptor(endpoint, provider, composition));
    mProvider   = &provider;
    mEndpointId = endpoint;

    mWaterHeaterManagementCluster.Create(endpoint, mWhmDelegate, mConfig.whmFeatures);
    mThermostatCluster.Create(endpoint, BitFlags<Clusters::Thermostat::Feature>(mConfig.thermostatFeatures),
                              HeatingThermostat::Config(mConfig.thermostatOptionalAttributes, mTimerDelegate), mThermostatDelegate,
                              mHeatingDelegate);
    mWaterHeaterModeCluster.Create(endpoint, Clusters::ModeBase::kWaterHeaterMode,
                                   Clusters::ModeBaseCluster::Config{
                                       .feature                = BitMask<Clusters::ModeBase::Feature>(),
                                       .optionalAttributeSet   = {},
                                       .appDelegate            = mWaterHeaterModeDelegate,
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
    mProvider = nullptr;
    UnregisterDescriptor(provider);
    UnregisterOptionalClusters(provider);
    if (mWaterHeaterManagementCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mWaterHeaterManagementCluster.Cluster()));
        mWaterHeaterManagementCluster.Destroy();
    }
    if (mThermostatCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mThermostatCluster.Cluster()));
        mThermostatCluster.Destroy();
    }
    if (mWaterHeaterModeCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mWaterHeaterModeCluster.Cluster()));
        mWaterHeaterModeCluster.Destroy();
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

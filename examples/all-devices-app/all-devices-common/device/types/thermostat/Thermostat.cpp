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

#include <device/types/thermostat/Thermostat.h>
#include <devices/Types.h>

namespace chip::app {

Thermostat::Thermostat(const Context & context, Clusters::IdentifyDelegate & identifyDelegate,
                       Clusters::Thermostat::Delegate & thermostatDelegate,
                       Clusters::Thermostat::ThermostatHeatingSetpoints::Delegate & heatingDelegate,
                       Clusters::Thermostat::ThermostatCoolingSetpoints::Delegate & coolingDelegate,
                       Clusters::ThermostatUserInterfaceConfiguration::Delegate & userInterfaceDelegate) :
    SingleEndpoint(Span<const DataModel::DeviceTypeEntry>(&Device::Type::kThermostat, 1)),
    mContext(context), mIdentifyDelegate(identifyDelegate), mThermostatDelegate(thermostatDelegate),
    mHeatingDelegate(heatingDelegate), mCoolingDelegate(coolingDelegate), mUserInterfaceDelegate(userInterfaceDelegate)
{}

CHIP_ERROR Thermostat::Register(EndpointId endpoint, CodeDrivenDataModelProvider & provider, EndpointComposition composition)
{
    VerifyOrReturnError(mEndpointId == kInvalidEndpointId, CHIP_ERROR_INCORRECT_STATE);
    DeviceRegistrationTransaction transaction(*this, provider);

    ReturnErrorOnFailure(RegisterDescriptor(endpoint, provider, composition));

    mIdentifyCluster.Create(Clusters::IdentifyCluster::Config(endpoint, mContext.timerDelegate).WithDelegate(&mIdentifyDelegate));
    ReturnErrorOnFailure(provider.AddCluster(mIdentifyCluster.Registration()));

    mGroupsCluster.Create(endpoint,
                          Clusters::GroupsCluster::Context{
                              .groupDataProvider   = mContext.groupDataProvider,
                              .identifyIntegration = &mIdentifyCluster.Cluster(),
                          });
    ReturnErrorOnFailure(provider.AddCluster(mGroupsCluster.Registration()));

    mThermostatCluster.Create(
        endpoint,
        BitFlags<Clusters::Thermostat::Feature>(Clusters::Thermostat::Feature::kHeating, Clusters::Thermostat::Feature::kCooling),
        ThermostatClusterType::Config({}, mContext.timerDelegate), mThermostatDelegate, mHeatingDelegate, mCoolingDelegate);
    ReturnErrorOnFailure(provider.AddCluster(mThermostatCluster.Registration()));

    mUserInterfaceCluster.Create(endpoint);
    mUserInterfaceCluster.Cluster().SetDelegate(&mUserInterfaceDelegate);
    ReturnErrorOnFailure(provider.AddCluster(mUserInterfaceCluster.Registration()));

    ReturnErrorOnFailure(provider.AddEndpoint(mEndpointRegistration));
    transaction.Commit();
    return CHIP_NO_ERROR;
}

void Thermostat::Unregister(CodeDrivenDataModelProvider & provider)
{
    UnregisterDescriptor(provider);
    if (mUserInterfaceCluster.IsConstructed())
    {
        mUserInterfaceCluster.Cluster().SetDelegate(nullptr);
        LogErrorOnFailure(provider.RemoveCluster(&mUserInterfaceCluster.Cluster()));
        mUserInterfaceCluster.Destroy();
    }
    if (mThermostatCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mThermostatCluster.Cluster()));
        mThermostatCluster.Destroy();
    }
    if (mGroupsCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mGroupsCluster.Cluster()));
        mGroupsCluster.Destroy();
    }
    if (mIdentifyCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mIdentifyCluster.Cluster()));
        mIdentifyCluster.Destroy();
    }
}

} // namespace chip::app

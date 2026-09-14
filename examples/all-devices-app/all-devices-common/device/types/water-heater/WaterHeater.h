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

#include <devices/Types.h>
#include <app/clusters/water-heater-management-server/WaterHeaterManagementCluster.h>
#include <app/clusters/mode-base-server/ModeBaseCluster.h>
#include <app/clusters/thermostat-server/ThermostatCluster.h>
#include <app/server-cluster/ServerClusterInterfaceRegistry.h>
#include <device/api/SingleEndpoint.h>
#include <lib/support/TimerDelegate.h>

namespace chip::app {

template <typename... ThermostatDelegates>
class WaterHeater : public SingleEndpoint
{
public:
    using ThermostatClusterType = Clusters::Thermostat::ThermostatCluster<ThermostatDelegates...>;

    struct Config
    {
        TimerDelegate & timerDelegate;
        DeviceLayer::DiagnosticDataProvider & diagnosticDataProvider;
        // WaterHeaterManagement cluster
        BitMask<Clusters::WaterHeaterManagement::Feature> whmFeatures;
        // Thermostat cluster
        BitMask<Clusters::Thermostat::Feature> thermostatFeatures;

    };

    explicit WaterHeater(const Config & config, Clusters::WaterHeaterManagement::Delegate & whmDelegate, Clusters::ModeBase::AppDelegate & waterHeaterModeDelegate) :
        SingleEndpoint(Span<const DataModel::DeviceTypeEntry>(&Device::Type::kWaterHeater, 1)),  
        mConfig(config), mWhmDelegate(whmDelegate), mWaterHeaterModeDelegate(waterHeaterModeDelegate) {}
    ~WaterHeater() = default;

    CHIP_ERROR Register(chip::EndpointId endpoint, CodeDrivenDataModelProvider & provider, 
                        EndpointComposition composition = {}) override
    {
        VerifyOrReturnError(SingleEndpoint::mEndpointId == kInvalidEndpointId, CHIP_ERROR_INCORRECT_STATE);
        DeviceRegistrationTransaction transaction(*this, provider);
    
        mProvider = &provider;
        ReturnErrorOnFailure(RegisterDescriptor(endpoint, provider, composition));
    
        mWaterHeaterManagementCluster.Create(endpoint, mWhmDelegate, mConfig.whmFeatures);
        ReturnErrorOnFailure(provider.AddCluster(mWaterHeaterManagementCluster.Registration()));

        mThermostatDelegates = std::make_tuple(std::make_unique<ThermostatDelegates>(endpoint)...);
    
        std::apply([&](auto &... delegates) {
            mThermostatCluster.Create(
                endpoint, 
                mConfig.thermostatFeatures, 
                Clusters::Thermostat::ThermostatClusterBase::Config(Clusters::Thermostat::OptionalAttributes(), mConfig.timerDelegate),
                *delegates... 
            );
        }, mThermostatDelegates);
    
        ReturnErrorOnFailure(provider.AddCluster(mThermostatCluster.Registration()));

        mWaterHeaterModeCluster.Create(endpoint, Clusters::ModeBase::kWaterHeaterMode,
            Clusters::ModeBaseCluster::Config{
                .feature                = BitMask<Clusters::ModeBase::Feature>(),
                .optionalAttributeSet   = {},
                .appDelegate            = mWaterHeaterModeDelegate,
                .onOffValueForStartUp   = false,
                .diagnosticDataProvider = mConfig.diagnosticDataProvider,
            });
        ReturnErrorOnFailure(provider.AddCluster(mWaterHeaterModeCluster.Registration()));

        ReturnErrorOnFailure(RegisterOptionalClusters(endpoint, provider));
    
        ReturnErrorOnFailure(provider.AddEndpoint(mEndpointRegistration));
        transaction.Commit();
        return CHIP_NO_ERROR;
    }
    void Unregister(CodeDrivenDataModelProvider & provider) override
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

    Clusters::WaterHeaterManagement::WaterHeaterManagementCluster & WaterHeaterManagementCluster()
    {
        VerifyOrDie(mWaterHeaterManagementCluster.IsConstructed());
        return mWaterHeaterManagementCluster.Cluster();
    }

    Clusters::WaterHeaterManagement::WaterHeaterManagementCluster & ThermostatCluster()
    {
        VerifyOrDie(mThermostatCluster.IsConstructed());
        return mThermostatCluster.Cluster();
    }


protected:

    virtual CHIP_ERROR RegisterOptionalClusters(EndpointId endpoint, CodeDrivenDataModelProvider & provider)
    {
        return CHIP_NO_ERROR;
    }

    virtual void UnregisterOptionalClusters(CodeDrivenDataModelProvider & provider) {}

    Config mConfig;
    CodeDrivenDataModelProvider * mProvider = nullptr;

private:
    // Delegates
    std::tuple<std::unique_ptr<ThermostatDelegates>...> mThermostatDelegates;
    Clusters::WaterHeaterManagement::Delegate & mWhmDelegate;
    Clusters::ModeBase::AppDelegate & mWaterHeaterModeDelegate;
    // Clusters
    LazyRegisteredServerCluster<Clusters::WaterHeaterManagement::WaterHeaterManagementCluster> mWaterHeaterManagementCluster;
    LazyRegisteredServerCluster<ThermostatClusterType> mThermostatCluster;
    LazyRegisteredServerCluster<Clusters::ModeBaseCluster> mWaterHeaterModeCluster;
};

} // namespace chip::app

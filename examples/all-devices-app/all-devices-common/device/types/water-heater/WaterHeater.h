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

#include <app/clusters/mode-base-server/ModeBaseCluster.h>
#include <app/clusters/thermostat-server/ThermostatCluster.h>
#include <app/clusters/water-heater-management-server/WaterHeaterManagementCluster.h>
#include <app/server-cluster/ServerClusterInterfaceRegistry.h>
#include <device/api/SingleEndpoint.h>
#include <devices/Types.h>
#include <lib/support/TimerDelegate.h>

namespace chip::app {

class WaterHeater : public SingleEndpoint
{
public:
    using HeatingThermostat = Clusters::Thermostat::ThermostatCluster<Clusters::Thermostat::Delegate,
                                                                      Clusters::Thermostat::ThermostatHeatingSetpoints::Delegate>;

    struct Config
    {
        TimerDelegate & timerDelegate;
        FabricTable & fabricTable;
        DeviceLayer::DiagnosticDataProvider & diagnosticDataProvider;
        // WaterHeaterManagement cluster
        BitMask<Clusters::WaterHeaterManagement::Feature> whmFeatures;
        Clusters::WaterHeaterManagement::Delegate & waterHeaterManagementDelegate;
        // Thermostat cluster
        BitMask<Clusters::Thermostat::Feature> thermostatFeatures;
        Clusters::Thermostat::OptionalAttributes thermostatOptionalAttributes;
        Clusters::Thermostat::Delegate & thermostatDelegate;
        Clusters::Thermostat::ThermostatHeatingSetpoints::Delegate & heatingDelegate;
        // ModeBase cluster
        Clusters::ModeBase::AppDelegate & waterHeaterModeDelegate;
    };

    explicit WaterHeater(const Config & config) :
        SingleEndpoint(Span<const DataModel::DeviceTypeEntry>(&Device::Type::kWaterHeater, 1)),
        mConfig(config), mTimerDelegate(config.timerDelegate), mWhmDelegate(config.waterHeaterManagementDelegate),
        mWaterHeaterModeDelegate(config.waterHeaterModeDelegate),
        mThermostatDelegate(config.thermostatDelegate),
        mHeatingDelegate(config.heatingDelegate)
    {}
    ~WaterHeater() = default;

    CHIP_ERROR Register(chip::EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                        EndpointComposition composition = {}) override
    {
        VerifyOrReturnError(mEndpointId == kInvalidEndpointId, CHIP_ERROR_INCORRECT_STATE);
        DeviceRegistrationTransaction transaction(*this, provider);

        ReturnErrorOnFailure(RegisterDescriptor(endpoint, provider, composition));
        mProvider = &provider;
        mEndpointId = endpoint;

        mWaterHeaterManagementCluster.Create(endpoint, mWhmDelegate, mConfig.whmFeatures);
        ReturnErrorOnFailure(provider.AddCluster(mWaterHeaterManagementCluster.Registration()));

        mThermostatCluster.Create(endpoint, BitFlags<Clusters::Thermostat::Feature>(Clusters::Thermostat::Feature::kCooling),
        HeatingThermostat::Config({}, mTimerDelegate), mThermostatDelegate, mHeatingDelegate);
ReturnErrorOnFailure(provider.AddCluster(mThermostatCluster.Registration()));


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

    HeatingThermostat & ThermostatCluster()
    {
        VerifyOrDie(mThermostatCluster.IsConstructed());
        return mThermostatCluster.Cluster();
    }

    Clusters::ModeBaseCluster & WaterHeaterModeCluster()
    {
        VerifyOrDie(mWaterHeaterModeCluster.IsConstructed());
        return mWaterHeaterModeCluster.Cluster();
    }

protected:
    virtual CHIP_ERROR RegisterOptionalClusters(EndpointId endpoint, CodeDrivenDataModelProvider & provider)
    {
        return CHIP_NO_ERROR;
    }

    virtual void UnregisterOptionalClusters(CodeDrivenDataModelProvider & provider) {}

    Config mConfig;
    CodeDrivenDataModelProvider * mProvider = nullptr;

    // Delegates
    TimerDelegate & mTimerDelegate;
    Clusters::WaterHeaterManagement::Delegate & mWhmDelegate;
    Clusters::ModeBase::AppDelegate & mWaterHeaterModeDelegate;
    Clusters::Thermostat::Delegate & mThermostatDelegate;
    Clusters::Thermostat::ThermostatHeatingSetpoints::Delegate & mHeatingDelegate;

private:
    // Clusters
    LazyRegisteredServerCluster<Clusters::WaterHeaterManagement::WaterHeaterManagementCluster> mWaterHeaterManagementCluster;
    LazyRegisteredServerCluster<HeatingThermostat> mThermostatCluster;
    LazyRegisteredServerCluster<Clusters::ModeBaseCluster> mWaterHeaterModeCluster;
};

} // namespace chip::app

/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
 *    All rights reserved.
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

#include <app/clusters/device-energy-management-server/DeviceEnergyManagementDelegate.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <app/server-cluster/OptionalAttributeSet.h>
#include <clusters/DeviceEnergyManagement/AttributeIds.h>
#include <clusters/DeviceEnergyManagement/ClusterId.h>
#include <clusters/DeviceEnergyManagement/Enums.h>
#include <clusters/DeviceEnergyManagement/Events.h>
#include <clusters/DeviceEnergyManagement/Structs.h>
#include <lib/core/Optional.h>

namespace chip {
namespace app {
namespace Clusters {

class DeviceEnergyManagementCluster : public DefaultServerCluster
{

public:
    using OptionalAttributesSet = OptionalAttributeSet<                    //
        DeviceEnergyManagement::Attributes::PowerAdjustmentCapability::Id, //
        DeviceEnergyManagement::Attributes::Forecast::Id,                  //
        DeviceEnergyManagement::Attributes::OptOutState::Id                //
        >;

    struct Config
    {
        EndpointId endpointId;
        BitMask<DeviceEnergyManagement::Feature> featureFlags;
        DeviceEnergyManagement::Delegate * delegate;

        Config(EndpointId aEndpointId, BitMask<DeviceEnergyManagement::Feature> aFeatures,
               DeviceEnergyManagement::Delegate * aDelegate) :
            endpointId(aEndpointId),
            featureFlags(aFeatures), delegate(aDelegate)
        {}
    };
    // We don't want to allow the default constructor as this cluster requires a delegate to be set
    DeviceEnergyManagementCluster() = delete;

    DeviceEnergyManagementCluster(const Config & config) :
        DefaultServerCluster({ config.endpointId, DeviceEnergyManagement::Id }), mDelegate(config.delegate),
        mFeatureFlags(config.featureFlags), mEnabledOptionalAttributes([&]() {
            OptionalAttributesSet attrs;
            attrs.Set<DeviceEnergyManagement::Attributes::PowerAdjustmentCapability::Id>(
                config.featureFlags.Has(DeviceEnergyManagement::Feature::kPowerAdjustment));
            attrs.Set<DeviceEnergyManagement::Attributes::Forecast::Id>(
                config.featureFlags.HasAny(DeviceEnergyManagement::Feature::kPowerForecastReporting,
                                           DeviceEnergyManagement::Feature::kStateForecastReporting));
            attrs.Set<DeviceEnergyManagement::Attributes::OptOutState::Id>(config.featureFlags.HasAny(
                DeviceEnergyManagement::Feature::kPowerAdjustment, DeviceEnergyManagement::Feature::kStartTimeAdjustment,
                DeviceEnergyManagement::Feature::kForecastAdjustment, DeviceEnergyManagement::Feature::kConstraintBasedAdjustment));
            return attrs;
        }())
    {
        VerifyOrDie(config.delegate != nullptr);
        mDelegate->SetEndpointId(config.endpointId);
    }

    const OptionalAttributesSet & OptionalAttributes() const { return mEnabledOptionalAttributes; }
    const BitFlags<DeviceEnergyManagement::Feature> & Features() const { return mFeatureFlags; }
    DeviceEnergyManagement::Delegate * GetDelegate() const { return mDelegate; }

    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override;
    std::optional<DataModel::ActionReturnStatus> InvokeCommand(const DataModel::InvokeRequest & request,
                                                               TLV::TLVReader & input_arguments, CommandHandler * handler) override;
    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) override;
    CHIP_ERROR AcceptedCommands(const ConcreteClusterPath & path,
                                ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder) override;

private:
    DataModel::ActionReturnStatus CheckOptOutAllowsRequest(DeviceEnergyManagement::AdjustmentCauseEnum adjustmentCause);
    DataModel::ActionReturnStatus HandlePowerAdjustRequest(const DataModel::InvokeRequest & request,
                                                           TLV::TLVReader & input_arguments, CommandHandler * handler);
    DataModel::ActionReturnStatus HandleCancelPowerAdjustRequest(const DataModel::InvokeRequest & request,
                                                                 TLV::TLVReader & input_arguments, CommandHandler * handler);
    DataModel::ActionReturnStatus HandleStartTimeAdjustRequest(const DataModel::InvokeRequest & request,
                                                               TLV::TLVReader & input_arguments, CommandHandler * handler);
    DataModel::ActionReturnStatus HandlePauseRequest(const DataModel::InvokeRequest & request, TLV::TLVReader & input_arguments,
                                                     CommandHandler * handler);
    DataModel::ActionReturnStatus HandleResumeRequest(const DataModel::InvokeRequest & request, TLV::TLVReader & input_arguments,
                                                      CommandHandler * handler);
    DataModel::ActionReturnStatus HandleModifyForecastRequest(const DataModel::InvokeRequest & request,
                                                              TLV::TLVReader & input_arguments, CommandHandler * handler);
    DataModel::ActionReturnStatus HandleRequestConstraintBasedForecast(const DataModel::InvokeRequest & request,
                                                                       TLV::TLVReader & input_arguments, CommandHandler * handler);
    DataModel::ActionReturnStatus HandleCancelRequest(const DataModel::InvokeRequest & request, TLV::TLVReader & input_arguments,
                                                      CommandHandler * handler);

    DeviceEnergyManagement::Delegate * mDelegate = nullptr;
    const BitFlags<DeviceEnergyManagement::Feature> mFeatureFlags;
    const OptionalAttributesSet mEnabledOptionalAttributes;
};

} // namespace Clusters
} // namespace app
} // namespace chip

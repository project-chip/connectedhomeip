/**
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <app-common/zap-generated/cluster-objects.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <app/server-cluster/OptionalAttributeSet.h>
#include <app/server/Server.h>
#include <clusters/GeneralDiagnostics/ClusterId.h>
#include <clusters/GeneralDiagnostics/Metadata.h>
#include <platform/DiagnosticDataProvider.h>
#include <platform/GeneralFaults.h>

namespace chip {
namespace app {
namespace Clusters {

struct GeneralDiagnosticsFunctionsConfig
{
    bool enablePosixTime : 1;
    bool enablePayloadSnapshot : 1;
};

class GeneralDiagnosticsCluster : public DefaultServerCluster
{
public:
    using OptionalAttributeSet =
        chip::app::OptionalAttributeSet<GeneralDiagnostics::Attributes::TotalOperationalHours::Id, //
                                        GeneralDiagnostics::Attributes::BootReason::Id,            //
                                        GeneralDiagnostics::Attributes::ActiveHardwareFaults::Id,  //
                                        GeneralDiagnostics::Attributes::ActiveRadioFaults::Id,     //
                                        GeneralDiagnostics::Attributes::ActiveNetworkFaults::Id,   //
                                        GeneralDiagnostics::Attributes::DeviceLoadStatus::Id       //
                                        // NOTE: Uptime is optional in the XML, however mandatory since revision 2.
                                        //       it will be forced as mandatory by the cluster constructor
                                        >;

    GeneralDiagnosticsCluster(OptionalAttributeSet optionalAttributeSet, BitFlags<GeneralDiagnostics::Feature> featureFlags,
                              DeviceLoadStatusProviderDelegate * deviceLoadStatusProvider) :
        DefaultServerCluster({ kRootEndpointId, GeneralDiagnostics::Id }),
        mOptionalAttributeSet(optionalAttributeSet.ForceSet<GeneralDiagnostics::Attributes::UpTime::Id>()),
        mFeatureFlags(featureFlags), mDeviceLoadStatusProvider(deviceLoadStatusProvider)
    {}

    CHIP_ERROR Startup(ServerClusterContext & context) override;

    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override;

    std::optional<DataModel::ActionReturnStatus> InvokeCommand(const DataModel::InvokeRequest & request,
                                                               chip::TLV::TLVReader & input_arguments,
                                                               CommandHandler * handler) override;

    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) override;

    CHIP_ERROR AcceptedCommands(const ConcreteClusterPath & path,
                                ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder) override;

    CHIP_ERROR GeneratedCommands(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<CommandId> & builder) override;

    /**
     * @brief
     *   Called after the current device is rebooted.
     */
    void OnDeviceReboot(GeneralDiagnostics::BootReasonEnum bootReason);

    /**
     * @brief
     *   Called when the Node detects a hardware fault has been raised.
     */
    void OnHardwareFaultsDetect(const DeviceLayer::GeneralFaults<DeviceLayer::kMaxHardwareFaults> & previous,
                                const DeviceLayer::GeneralFaults<DeviceLayer::kMaxHardwareFaults> & current);

    /**
     * @brief
     *   Called when the Node detects a radio fault has been raised.
     */
    void OnRadioFaultsDetect(const DeviceLayer::GeneralFaults<DeviceLayer::kMaxRadioFaults> & previous,
                             const DeviceLayer::GeneralFaults<DeviceLayer::kMaxRadioFaults> & current);
    /**
     * @brief
     *   Called when the Node detects a network fault has been raised.
     */
    void OnNetworkFaultsDetect(const DeviceLayer::GeneralFaults<DeviceLayer::kMaxNetworkFaults> & previous,
                               const DeviceLayer::GeneralFaults<DeviceLayer::kMaxNetworkFaults> & current);

    CHIP_ERROR GetRebootCount(uint16_t & rebootCount) const
    {
        return DeviceLayer::GetDiagnosticDataProvider().GetRebootCount(rebootCount);
    }
    CHIP_ERROR GetTotalOperationalHours(uint32_t & totalOperationalHours) const
    {
        return DeviceLayer::GetDiagnosticDataProvider().GetTotalOperationalHours(totalOperationalHours);
    }
    CHIP_ERROR GetBootReason(chip::app::Clusters::GeneralDiagnostics::BootReasonEnum & bootReason) const
    {
        return DeviceLayer::GetDiagnosticDataProvider().GetBootReason(bootReason);
    }
    CHIP_ERROR GetActiveHardwareFaults(chip::DeviceLayer::GeneralFaults<DeviceLayer::kMaxHardwareFaults> & hardwareFaults) const
    {
        return DeviceLayer::GetDiagnosticDataProvider().GetActiveHardwareFaults(hardwareFaults);
    }
    CHIP_ERROR GetActiveRadioFaults(chip::DeviceLayer::GeneralFaults<DeviceLayer::kMaxRadioFaults> & radioFaults) const
    {
        return DeviceLayer::GetDiagnosticDataProvider().GetActiveRadioFaults(radioFaults);
    }
    CHIP_ERROR GetActiveNetworkFaults(chip::DeviceLayer::GeneralFaults<DeviceLayer::kMaxNetworkFaults> & networkFaults) const
    {
        return DeviceLayer::GetDiagnosticDataProvider().GetActiveNetworkFaults(networkFaults);
    }

protected:
    OptionalAttributeSet mOptionalAttributeSet;
    CHIP_ERROR ReadNetworkInterfaces(AttributeValueEncoder & aEncoder);
    BitFlags<GeneralDiagnostics::Feature> mFeatureFlags;
    DeviceLoadStatusProviderDelegate * mDeviceLoadStatusProvider;
};

class GeneralDiagnosticsClusterFullConfigurable : public GeneralDiagnosticsCluster
{
public:
    GeneralDiagnosticsClusterFullConfigurable(const GeneralDiagnosticsCluster::OptionalAttributeSet & optionalAttributeSet,
                                              const BitFlags<GeneralDiagnostics::Feature> featureFlags,
                                              DeviceLoadStatusProviderDelegate * deviceLoadStatusProvider,
                                              const GeneralDiagnosticsFunctionsConfig & functionsConfig) :
        GeneralDiagnosticsCluster(optionalAttributeSet, featureFlags, deviceLoadStatusProvider),
        mFunctionConfig(functionsConfig)
    {}

    std::optional<DataModel::ActionReturnStatus> InvokeCommand(const DataModel::InvokeRequest & request,
                                                               chip::TLV::TLVReader & input_arguments,
                                                               CommandHandler * handler) override;

private:
    const GeneralDiagnosticsFunctionsConfig mFunctionConfig;
};

} // namespace Clusters
} // namespace app
} // namespace chip

/*
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <app/clusters/basic-information/ConfigurationVersionDelegate.h>
#include <app/clusters/bridged-device-basic-information-server/BridgedDeviceBasicInformationDelegate.h>
#include <app/clusters/bridged-device-basic-information-server/BridgedDeviceIcdDelegate.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <clusters/BridgedDeviceBasicInformation/ClusterId.h>
#include <clusters/BridgedDeviceBasicInformation/Structs.h>
#include <lib/support/TimerDelegate.h>

#include <optional>
#include <string>
#include <utility>

namespace chip::app::Clusters {

/// This class provides information about bridged devices.
///
/// Since it is intended for bridge uses (assumed higher resources devices)
/// it uses std::string and similar data types for ease of use.
///
/// Note: StartUp/ShutDown events are intentionally NOT supported.
///   - Node-wide startup/shutdown events are provided by the Node's Basic Information cluster.
///   - Per-endpoint (bridged device) lifecycle is detectable via the Descriptor cluster.
///   - They are marked as Optional (O) in the Bridged Device Basic Information specification.
///
/// Note: DeviceLocation attribute (0x0017) is not supported as it is not in the standard Bridged Device Basic Information Cluster
/// XML.
class BridgedDeviceBasicInformationCluster : public DefaultServerCluster, public TimerContext
{
public:
    struct Context
    {
        ConfigurationVersionDelegate & parentVersionConfiguration;
        BridgedDeviceBasicInformationDelegate & delegate;
        TimerDelegate & timerDelegate;
        BridgedDeviceIcdDelegate * icdDelegate = nullptr; // if nullptr, ICD support feature is disabled
    };

    /// Most attributes in the bridged device basic information cluster are fixed and cannot be
    /// changed after construction. This class defines those attributes.
    ///
    /// Attribute will be exposed if the optional values have a value.
    struct FixedData
    {
        std::optional<std::string> vendorName;
        std::optional<VendorId> vendorId;
        std::optional<std::string> productName;
        std::optional<uint16_t> productId;
        std::optional<uint16_t> hardwareVersion;
        std::optional<std::string> hardwareVersionString;
        std::optional<uint32_t> softwareVersion;
        std::optional<std::string> softwareVersionString;
        std::optional<std::string> manufacturingDate;
        std::optional<std::string> partNumber;
        std::optional<std::string> productUrl;
        std::optional<std::string> productLabel;
        std::optional<std::string> serialNumber;
        std::optional<BridgedDeviceBasicInformation::Structs::ProductAppearanceStruct::Type> productAppearance;
    };

    /// Mandatory data for every bridged device
    struct RequiredData
    {
        std::string uniqueId;   // Fixed once set
        bool reachable = false; // initial value for reachable
        std::string nodeLabel;
        uint32_t configurationVersion = 1;
    };

    BridgedDeviceBasicInformationCluster(EndpointId endpointId, RequiredData && required, FixedData && fixedData,
                                         Context && context) :
        DefaultServerCluster({ endpointId, BridgedDeviceBasicInformation::Id }),
        mRequiredData(std::move(required)), mFixedData(std::move(fixedData)), mClusterContext(std::move(context))

    {}

    bool GetReachable() const { return mRequiredData.reachable; }
    void SetReachable(bool reachable);

    const std::string & GetUniqueId() const { return mRequiredData.uniqueId; }
    const FixedData & GetFixedData() const { return mFixedData; }

    const std::string & GetNodeLabel() const { return mRequiredData.nodeLabel; }
    DataModel::ActionReturnStatus SetNodeLabel(CharSpan nodeLabel);

    uint32_t GetConfigurationVersion() const { return mRequiredData.configurationVersion; }

    /// Increases the configuration version and ALSO increases the device
    /// configuration version. Specifically handles the spec requirement of:
    ///
    /// 9.13.5.4 ConfigurationVersion Attribute:
    ///   ...
    ///   If the bridge detects a change on a bridged device, which it deems as a
    ///   change in the configuration of the bridged device, it SHALL increase this
    ///   attribute (and the corresponding attribute on the bridge itself)
    CHIP_ERROR IncreaseConfigurationVersion();

    void GenerateLeaveEvent();

    // Application interface for ICD

    /// Returns the requested "StayActiveDuration" when the cluster is in "pending-active" state.
    ///
    /// When the bridged device is checking in, the application should keep it active
    /// for the time period here and should `NotifyDeviceActive`
    std::optional<uint32_t> GetRequestedStayActiveDurationMs() const { return mStayActiveDurationMs; }
    void NotifyDeviceActive();

    /// ServerClusterInterface (DefaultServerCluster overrides)

    void Shutdown(ClusterShutdownType) override;
    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override;
    DataModel::ActionReturnStatus WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                 AttributeValueDecoder & aDecoder) override;
    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) override;

    CHIP_ERROR AcceptedCommands(const ConcreteClusterPath & path,
                                ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder) override;

    std::optional<DataModel::ActionReturnStatus> InvokeCommand(const DataModel::InvokeRequest & request,
                                                               chip::TLV::TLVReader & input_arguments,
                                                               CommandHandler * handler) override;

private:
    // TimerContext
    void TimerFired() override;

    void StartPendingActiveTimer(System::Clock::Milliseconds32 timeoutMs);
    void CancelPendingActiveTimer();

    RequiredData mRequiredData;
    const FixedData mFixedData;
    const Context mClusterContext;

    std::optional<uint32_t> mStayActiveDurationMs; // Present only when pending active
    System::Clock::Timestamp mPendingActiveExpiryTime{};
};

} // namespace chip::app::Clusters

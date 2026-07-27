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

#include <app/clusters/bridged-device-basic-information-server/BridgedDeviceBasicInformationDelegate.h>
#include <app/clusters/bridged-device-basic-information-server/BridgedDeviceIcdDelegate.h>
#include <app/clusters/bridged-device-basic-information-server/ConfigurationVersionDelegate.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <clusters/BridgedDeviceBasicInformation/ClusterId.h>
#include <clusters/BridgedDeviceBasicInformation/Structs.h>
#include <clusters/shared/Structs.h>
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
class BridgedDeviceBasicInformationCluster : public DefaultServerCluster, public TimerContext
{
public:
    // A device location, however without using Span (i.e. using actual strings and owning the storage)
    struct OwnedDeviceLocation
    {
        std::string locationName;
        std::optional<int16_t> floorNumber;
        std::optional<Globals::AreaTypeTag> areaType;

        OwnedDeviceLocation() = default;
        OwnedDeviceLocation(const Globals::Structs::LocationDescriptorStruct::Type & other) { *this = other; }

        // Return a view of this value as a LocationDescriptorStruct. The locationName
        // will point into "this" so the lifetime of this MUST exceed the usage of the returned value.
        Globals::Structs::LocationDescriptorStruct::Type ToView() const;

        // compare with a non-owned version
        bool operator==(const Globals::Structs::LocationDescriptorStruct::Type & other) const;
        bool operator!=(const Globals::Structs::LocationDescriptorStruct::Type & other) const { return !(*this == other); }

        // set the current value from a non-owned version
        OwnedDeviceLocation & operator=(const Globals::Structs::LocationDescriptorStruct::Type & value);
    };

    struct Context
    {
        // NOTE: These delegate references are used throughout the cluster's lifetime.
        // Their lifetimes MUST be greater than or equal to the lifetime of this cluster instance.
        BridgedDeviceBasicInformationDelegate & delegate;
        TimerDelegate & timerDelegate;
        BridgedDeviceIcdDelegate * icdDelegate = nullptr; // if nullptr, ICD support feature is disabled
    };

    struct Versioning
    {
        uint32_t version;
        ConfigurationVersionDelegate & delegate;
    };

    /// Most attributes in the bridged device basic information cluster are fixed
    /// and cannot be changed after construction. This class defines those attributes.
    ///
    ///
    /// Attribute will be exposed if the optional values have a value.
    struct FixedData
    {
        std::string uniqueId; // Mandatory, fixed once set

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

    /// Mutable data for bridged device
    struct MutableData
    {
        bool reachable = false; // initial value for reachable
        std::string nodeLabel;
        std::optional<DataModel::Nullable<OwnedDeviceLocation>> deviceLocation;
        std::optional<Versioning> configurationVersion;
    };

    BridgedDeviceBasicInformationCluster(EndpointId endpointId, MutableData && mutableData, FixedData && fixedData,
                                         Context && context) :
        DefaultServerCluster({ endpointId, BridgedDeviceBasicInformation::Id }),
        mMutableData(std::move(mutableData)), mFixedData(std::move(fixedData)), mClusterContext(std::move(context))
    {}

    bool GetReachable() const { return mMutableData.reachable; }
    void SetReachable(bool reachable);

    const std::string & GetUniqueId() const { return mFixedData.uniqueId; }
    const FixedData & GetFixedData() const { return mFixedData; }

    const std::string & GetNodeLabel() const { return mMutableData.nodeLabel; }
    DataModel::ActionReturnStatus SetNodeLabel(CharSpan nodeLabel);

    std::optional<uint32_t> GetConfigurationVersion() const
    {
        VerifyOrReturnValue(mMutableData.configurationVersion.has_value(), std::nullopt);
        return mMutableData.configurationVersion->version;
    }

    std::optional<DataModel::Nullable<Globals::Structs::LocationDescriptorStruct::Type>> GetDeviceLocation() const
    {
        VerifyOrReturnValue(mMutableData.deviceLocation.has_value(), std::nullopt);
        VerifyOrReturnValue(!mMutableData.deviceLocation->IsNull(),
                            DataModel::Nullable<Globals::Structs::LocationDescriptorStruct::Type>(DataModel::NullNullable));

        return DataModel::MakeNullable(mMutableData.deviceLocation->Value().ToView());
    }

    /// Device location can only be set if the cluster supports device location
    /// (i.e. the MutableData has a location that is not std::nullopt)
    DataModel::ActionReturnStatus
    SetDeviceLocation(const DataModel::Nullable<Globals::Structs::LocationDescriptorStruct::Type> & location);

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

    CHIP_ERROR Startup(ServerClusterContext & context) override;
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
    enum class PersistenceMode
    {
        kPersist,
        kDoNotPersist,
    };

    /// Updates the NodeLabel attribute value with optional persistence.
    ///
    /// This internal helper validates the new value via the delegate, persists it to NVM if requested,
    /// and then updates the in-memory state and notifies subscribers.
    ///
    /// @param nodeLabel The new node label to set.
    /// @param mode Whether to persist the new value to NVM.
    /// @return Status code indicating the result of the operation.
    DataModel::ActionReturnStatus SetNodeLabelInternal(CharSpan nodeLabel, PersistenceMode mode);

    /// Updates the DeviceLocation attribute value with optional persistence.
    ///
    /// This internal helper validates the new value via the delegate, persists it to NVM if requested,
    /// and then updates the in-memory state and notifies subscribers.
    ///
    /// @param location The new device location to set.
    /// @param mode Whether to persist the new value to NVM.
    /// @return Status code indicating the result of the operation.
    DataModel::ActionReturnStatus
    SetDeviceLocationInternal(const DataModel::Nullable<Globals::Structs::LocationDescriptorStruct::Type> & location,
                              PersistenceMode mode);

    /// Store the current DeviceLocation to persistent storage
    CHIP_ERROR PersistDeviceLocation();

    // TimerContext
    void TimerFired() override;

    void StartPendingActiveTimer(System::Clock::Milliseconds32 timeoutMs);
    void CancelPendingActiveTimer();

    MutableData mMutableData;
    const FixedData mFixedData;
    const Context mClusterContext;

    std::optional<uint32_t> mStayActiveDurationMs; // Present only when pending active
    System::Clock::Timestamp mPendingActiveExpiryTime{};
};

} // namespace chip::app::Clusters

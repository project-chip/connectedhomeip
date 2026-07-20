/*
 *    Copyright (c) 2021-2025 Project CHIP Authors
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

#include <access/AccessControl.h>
#include <app/EventManagement.h>
#include <app/FailSafeContext.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <app/server/CommissioningWindowManager.h>
#include <app/server/Dnssd.h>
#include <clusters/OperationalCredentials/ClusterId.h>
#include <credentials/DeviceAttestationCredsProvider.h>
#include <credentials/GroupDataProvider.h>

namespace chip {
namespace app {
namespace Clusters {

class OperationalCredentialsCluster : public DefaultServerCluster, chip::FabricTable::Delegate
{
public:
    /// Number of vendor_reserved fields supported in CSRResponse NOCSRElements (context tags 3, 4, 5).
    static constexpr size_t kMaxCSRVendorReservedFields = 3;

    /// Identifies which vendor_reserved field to set in the CSRResponse NOCSRElements.
    enum class CSRVendorReservedField : uint8_t
    {
        kVendorReserved1 = 0, // context tag 3
        kVendorReserved2 = 1, // context tag 4
        kVendorReserved3 = 2, // context tag 5
    };

    struct Context
    {
        FabricTable & fabricTable;
        FailSafeContext & failSafeContext;
        SessionManager & sessionManager;
        DnssdServer & dnssdServer;
        CommissioningWindowManager & commissioningWindowManager;
        Credentials::DeviceAttestationCredentialsProvider & dacProvider;
        Credentials::GroupDataProvider & groupDataProvider;
        Access::AccessControl & accessControl;
        DeviceLayer::PlatformManager & platformManager;
        app::EventManagement & eventManagement;
    };

    OperationalCredentialsCluster(EndpointId endpoint, const Context context) :
        DefaultServerCluster({ endpoint, OperationalCredentials::Id }), mOpCredsContext(context){};

    CHIP_ERROR Startup(ServerClusterContext & context) override;
    void Shutdown(ClusterShutdownType type) override;

    /**
     * @brief Set vendor-specific data to be included in CSRResponse NOCSRElements.
     *
     * The caller retains ownership of the underlying memory, which must remain valid for the
     * lifetime of this cluster instance. Pass an empty ByteSpan to clear a field.
     *
     * Note: the resulting nocsr_elements_message (CSR + nonce + all vendor_reserved fields)
     * SHALL be no more than RESP_MAX (900) bytes per the Matter spec. Oversized data causes
     * CSRRequest to fail with CHIP_ERROR_MESSAGE_TOO_LONG at construction time.
     *
     * @param field  Which vendor_reserved field to set.
     * @param data   Vendor-specific data; empty span clears the field.
     * @return CHIP_NO_ERROR on success, CHIP_ERROR_INVALID_ARGUMENT if field is out of range.
     */
    CHIP_ERROR SetCSRVendorReserved(CSRVendorReservedField field, ByteSpan data);

    // Server cluster implementation
    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override;
    std::optional<DataModel::ActionReturnStatus> InvokeCommand(const DataModel::InvokeRequest & request,
                                                               TLV::TLVReader & input_arguments, CommandHandler * handler) override;
    CHIP_ERROR AcceptedCommands(const ConcreteClusterPath & path,
                                ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder) override;
    CHIP_ERROR GeneratedCommands(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<CommandId> & builder) override;
    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) override;

    // Function used to handle event FailSafeTimerExpired
    static void FailSafeCleanup(const DeviceLayer::ChipDeviceEvent * event, OperationalCredentialsCluster * cluster);

    // FabricTable delegate
    void FabricWillBeRemoved(const FabricTable & fabricTable, FabricIndex fabricIndex) override;
    void OnFabricRemoved(const FabricTable & fabricTable, FabricIndex fabricIndex) override;
    void OnFabricUpdated(const FabricTable & fabricTable, FabricIndex fabricIndex) override;
    void OnFabricCommitted(const FabricTable & fabricTable, FabricIndex fabricIndex) override;

private:
    const OperationalCredentialsCluster::Context mOpCredsContext;
    ByteSpan mCsrVendorReserved[kMaxCSRVendorReservedFields] = {};
};

} // namespace Clusters
} // namespace app
} // namespace chip

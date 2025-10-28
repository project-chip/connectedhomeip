/**
 *
 *    Copyright (c) 2022-2025 Project CHIP Authors
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

#include <app/icd/server/ICDConfigurationData.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <app/server-cluster/OptionalAttributeSet.h>
#include <app/util/basic-types.h>
#include <clusters/IcdManagement/Commands.h>
#include <clusters/IcdManagement/Metadata.h>
#include <lib/core/Optional.h>
#include <lib/support/Span.h>

#include <app/icd/server/ICDServerConfig.h>

#if CHIP_CONFIG_ENABLE_ICD_CIP
#include <credentials/FabricTable.h>
#include <crypto/SessionKeystore.h>
#include <lib/core/CHIPPersistentStorageDelegate.h>
#endif // CHIP_CONFIG_ENABLE_ICD_CIP

#include <app/icd/server/ICDStateObserver.h>

namespace chip {

namespace Crypto {
using SymmetricKeystore = Crypto::SessionKeystore;
} // namespace Crypto

namespace app {
namespace Clusters {

namespace IcdManagement {
enum class OptionalCommands : uint8_t
{
    kStayActive = 0x01,
};

constexpr size_t kUserActiveModeTriggerInstructionMaxLength = 128;
} // namespace IcdManagement

/**
 * @brief ICD Management Cluster
 *
 * This class provides the core ICD Management functionality. When CIP (Check-In Protocol)
 * features are needed, use ICDManagementClusterWithCIP which extends this class.
 */
class ICDManagementCluster : public DefaultServerCluster, public chip::app::ICDStateObserver
{
public:
    using OptionalAttributeSet = app::OptionalAttributeSet<IcdManagement::Attributes::UserActiveModeTriggerInstruction::Id>;

    // TODO: The interaction between enabledCommands and feature flags (particularly LITS) needs clarification.
    // According to spec, LITS implies StayActiveRequest support. Options:
    // 1. Document that kStayActive bit in enabledCommands is ignored when LITS feature is set
    // 2. Add Startup() validation to fail if enabledCommands and feature flags are inconsistent,
    //    and simplify AcceptedCommands/GeneratedCommands to only check mEnabledCommands

    /**
     * @brief Constructor for ICDManagementCluster
     *
     * @param userActiveModeTriggerInstruction The instruction string is copied into an internal buffer
     *        during construction. The caller does not need to maintain the lifetime of the CharSpan
     *        or its underlying data after the constructor returns. If the instruction exceeds
     *        kUserActiveModeTriggerInstructionMaxLength (128 bytes), it will be truncated.
     */
    ICDManagementCluster(EndpointId endpointId, Crypto::SymmetricKeystore & symmetricKeystore, FabricTable & fabricTable,
                         ICDConfigurationData & icdConfigurationData, OptionalAttributeSet optionalAttributeSet,
                         BitMask<IcdManagement::OptionalCommands> enabledCommands,
                         BitMask<IcdManagement::UserActiveModeTriggerBitmap> userActiveModeTriggerBitmap,
                         CharSpan userActiveModeTriggerInstruction);

    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) override;

    std::optional<DataModel::ActionReturnStatus> InvokeCommand(const DataModel::InvokeRequest & request,
                                                               TLV::TLVReader & input_arguments, CommandHandler * handler) override;

    CHIP_ERROR AcceptedCommands(const ConcreteClusterPath & path,
                                ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder) override;

    CHIP_ERROR GeneratedCommands(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<CommandId> & builder) override;

private:
    void OnEnterActiveMode() override{};
    void OnEnterIdleMode() override{};
    void OnTransitionToIdle() override{};
    void OnICDModeChange() override;

protected:
#if CHIP_CONFIG_ENABLE_ICD_LIT
    CHIP_ERROR ReadOperatingMode(AttributeValueEncoder & encoder);
#endif // CHIP_CONFIG_ENABLE_ICD_LIT

    Crypto::SymmetricKeystore & mSymmetricKeystore;
    FabricTable & mFabricTable;
    ICDConfigurationData & mICDConfigurationData;
    const OptionalAttributeSet mOptionalAttributeSet;
    const BitMask<IcdManagement::OptionalCommands> mEnabledCommands;
    const BitMask<IcdManagement::UserActiveModeTriggerBitmap> mUserActiveModeTriggerBitmap;
    char mUserActiveModeTriggerInstruction[IcdManagement::kUserActiveModeTriggerInstructionMaxLength];
    const uint8_t mUserActiveModeTriggerInstructionLength;
};

#if CHIP_CONFIG_ENABLE_ICD_CIP

/**
 * @brief Implementation of Fabric Delegate for ICD Management cluster
 */
class ICDManagementFabricDelegate : public FabricTable::Delegate
{
public:
    void Init(PersistentStorageDelegate & storage, Crypto::SymmetricKeystore * symmetricKeystore,
              ICDConfigurationData & icdConfigurationData);

    void OnFabricRemoved(const FabricTable & fabricTable, FabricIndex fabricIndex) override;

private:
    PersistentStorageDelegate * mStorage           = nullptr;
    Crypto::SymmetricKeystore * mSymmetricKeystore = nullptr;
    ICDConfigurationData * mICDConfigurationData   = nullptr;
};

/**
 * @brief ICD Management Cluster with CIP (Check-In Protocol) support
 *
 * This subclass extends ICDManagementCluster with CIP functionality,
 * including client registration/unregistration and fabric delegate management.
 * The fabric delegate is automatically registered/unregistered in Startup/Shutdown.
 *
 * @param userActiveModeTriggerInstruction The instruction string is copied into an internal buffer
 *        during construction (by the base class). The caller does not need to maintain the lifetime
 *        of the CharSpan or its underlying data after the constructor returns.
 */
class ICDManagementClusterWithCIP : public ICDManagementCluster
{
public:
    ICDManagementClusterWithCIP(EndpointId endpointId, Crypto::SymmetricKeystore & symmetricKeystore, FabricTable & fabricTable,
                                ICDConfigurationData & icdConfigurationData, OptionalAttributeSet optionalAttributeSet,
                                BitMask<IcdManagement::OptionalCommands> enabledCommands,
                                BitMask<IcdManagement::UserActiveModeTriggerBitmap> userActiveModeTriggerBitmap,
                                CharSpan userActiveModeTriggerInstruction);

    CHIP_ERROR Startup(ServerClusterContext & context) override;
    void Shutdown() override;

    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & aEncoder) override;

    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) override;

    std::optional<DataModel::ActionReturnStatus> InvokeCommand(const DataModel::InvokeRequest & request,
                                                               TLV::TLVReader & input_arguments, CommandHandler * handler) override;

    CHIP_ERROR AcceptedCommands(const ConcreteClusterPath & path,
                                ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder) override;

    CHIP_ERROR GeneratedCommands(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<CommandId> & builder) override;

private:
    CHIP_ERROR ReadRegisteredClients(AttributeValueEncoder & encoder);

    /**
     * @brief Triggers table update events to notify subscribers that an entry was added or removed
     *        from the ICDMonitoringTable.
     */
    void TriggerICDMTableUpdatedEvent();

    /**
     * @brief Function that executes the business logic of the RegisterClient Command
     *
     * @param[out] icdCounter If function succeeds, icdCounter will have the current value of the ICDCounter stored in the
     * ICDConfigurationData If function fails, icdCounter will be unchanged
     * @return Status
     */
    Protocols::InteractionModel::Status RegisterClient(CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                                                       const IcdManagement::Commands::RegisterClient::DecodableType & commandData,
                                                       uint32_t & icdCounter);

    Protocols::InteractionModel::Status
    UnregisterClient(CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                     const IcdManagement::Commands::UnregisterClient::DecodableType & commandData);

    ICDManagementFabricDelegate mFabricDelegate;
};
#endif // CHIP_CONFIG_ENABLE_ICD_CIP

} // namespace Clusters
} // namespace app
} // namespace chip

/**
 *
 *    Copyright (c) 2023-2025 Project CHIP Authors
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

#include <app/clusters/icd-management-server/ICDManagementCluster.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <clusters/IcdManagement/ClusterId.h>
#if CHIP_CONFIG_ENABLE_ICD_CIP
#include <access/AccessControl.h>
#include <app/icd/server/ICDMonitoringTable.h> // nogncheck
#include <app/icd/server/ICDNotifier.h>        // nogncheck
#endif
#if CHIP_CONFIG_ENABLE_ICD_SERVER
#include <app/server/Server.h> // nogncheck
#endif
#include <app/reporting/reporting.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::IcdManagement;
using namespace Protocols;
using namespace chip::Access;

using chip::Protocols::InteractionModel::Status;

namespace {
#if CHIP_CONFIG_ENABLE_ICD_CIP
/**
 * @brief Function checks if the client has admin permissions to the cluster in the commandPath
 *
 * @param[out] isClientAdmin True : Client has admin permissions
 *                           False : Client does not have admin permissions
 *                           If an error occurs, isClientAdmin is not changed
 * @return CHIP_ERROR
 */
CHIP_ERROR CheckAdmin(CommandHandler * commandObj, const ConcreteCommandPath & commandPath, bool & isClientAdmin)
{
    RequestPath requestPath{ .cluster     = commandPath.mClusterId,
                             .endpoint    = commandPath.mEndpointId,
                             .requestType = RequestType::kCommandInvokeRequest,
                             .entityId    = commandPath.mCommandId };
    CHIP_ERROR err = GetAccessControl().Check(commandObj->GetSubjectDescriptor(), requestPath, Privilege::kAdminister);
    if (CHIP_NO_ERROR == err)
    {
        isClientAdmin = true;
    }
    else if (CHIP_ERROR_ACCESS_DENIED == err)
    {
        isClientAdmin = false;
        err           = CHIP_NO_ERROR;
    }
    return err;
}

#endif // CHIP_CONFIG_ENABLE_ICD_CIP
} // anonymous namespace

namespace chip::app::Clusters {

#if CHIP_CONFIG_ENABLE_ICD_CIP
void ICDManagementFabricDelegate::Init(PersistentStorageDelegate & storage, Crypto::SymmetricKeystore * symmetricKeystore,
                                       ICDConfigurationData & icdConfigurationData)
{
    mStorage              = &storage;
    mSymmetricKeystore    = symmetricKeystore;
    mICDConfigurationData = &icdConfigurationData;
}

void ICDManagementFabricDelegate::OnFabricRemoved(const FabricTable & fabricTable, FabricIndex fabricIndex)
{
    uint16_t supported_clients = mICDConfigurationData->GetClientsSupportedPerFabric();
    ICDMonitoringTable table(*mStorage, fabricIndex, supported_clients, mSymmetricKeystore);
    table.RemoveAll();
    ICDNotifier::GetInstance().NotifyICDManagementEvent(ICDListener::ICDManagementEvents::kTableUpdated);
}
#endif // CHIP_CONFIG_ENABLE_ICD_CIP

ICDManagementCluster::ICDManagementCluster(EndpointId endpointId, PersistentStorageDelegate & storage,
                                           chip::Crypto::SessionKeystore & symmetricKeystore, FabricTable & fabricTable,
                                           ICDConfigurationData & icdConfigurationData, OptionalAttributeSet optionalAttributeSet,
                                           BitMask<IcdManagement::OptionalCommands> aEnabledCommands,
                                           BitMask<IcdManagement::UserActiveModeTriggerBitmap> aUserActiveModeTriggerBitmap,
                                           CharSpan aUserActiveModeTriggerInstruction) :
    DefaultServerCluster({ endpointId, IcdManagement::Id }),
    mStorage(storage), mSymmetricKeystore(symmetricKeystore), mFabricTable(fabricTable),
    mICDConfigurationData(icdConfigurationData), mOptionalAttributeSet(optionalAttributeSet), mEnabledCommands(aEnabledCommands),
    mUserActiveModeTriggerBitmap(aUserActiveModeTriggerBitmap)
{
    MutableCharSpan buffer(mUserActiveModeTriggerInstruction);
    CopyCharSpanToMutableCharSpanWithTruncation(aUserActiveModeTriggerInstruction, buffer);
}

DataModel::ActionReturnStatus ICDManagementCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                                  AttributeValueEncoder & aEncoder)
{
    switch (request.path.mAttributeId)
    {
    case IcdManagement::Attributes::ClusterRevision::Id:
        return aEncoder.Encode(kRevision);

    case IcdManagement::Attributes::FeatureMap::Id:
        return aEncoder.Encode(mICDConfigurationData.GetFeatureMap());

    case IcdManagement::Attributes::IdleModeDuration::Id:
        return aEncoder.Encode(mICDConfigurationData.GetIdleModeDuration().count());

    case IcdManagement::Attributes::ActiveModeDuration::Id:
        return aEncoder.Encode(mICDConfigurationData.GetActiveModeDuration().count());

    case IcdManagement::Attributes::ActiveModeThreshold::Id:
        return aEncoder.Encode(mICDConfigurationData.GetActiveModeThreshold().count());

#if CHIP_CONFIG_ENABLE_ICD_LIT
    case IcdManagement::Attributes::OperatingMode::Id:
        return ReadOperatingMode(aEncoder);
#endif // CHIP_CONFIG_ENABLE_ICD_LIT

    case IcdManagement::Attributes::UserActiveModeTriggerHint::Id:
        return aEncoder.Encode(mUserActiveModeTriggerBitmap);

    case IcdManagement::Attributes::UserActiveModeTriggerInstruction::Id:
        return aEncoder.Encode(
            CharSpan(mUserActiveModeTriggerInstruction,
                     strnlen(mUserActiveModeTriggerInstruction, IcdManagement::kUserActiveModeTriggerInstructionMaxLength)));

    default:
        return Protocols::InteractionModel::Status::UnsupportedAttribute;
    }
}

CHIP_ERROR ICDManagementCluster::Attributes(const ConcreteClusterPath & path,
                                            ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    AttributeListBuilder attributeListBuilder(builder);
    const DataModel::AttributeEntry kMandatoryAttributes[] = {
        IcdManagement::Attributes::IdleModeDuration::kMetadataEntry,
        IcdManagement::Attributes::ActiveModeDuration::kMetadataEntry,
        IcdManagement::Attributes::ActiveModeThreshold::kMetadataEntry,
    };
    BitFlags<IcdManagement::Feature> featureMap = mICDConfigurationData.GetFeatureMap();
    const bool hasCIP                           = featureMap.Has(IcdManagement::Feature::kCheckInProtocolSupport);
    const bool hasUAT                           = featureMap.Has(IcdManagement::Feature::kUserActiveModeTrigger);
    const bool hasLIT                           = featureMap.Has(IcdManagement::Feature::kLongIdleTimeSupport);

    const AttributeListBuilder::OptionalAttributeEntry optionalEntries[] = {
        { hasCIP, IcdManagement::Attributes::RegisteredClients::kMetadataEntry },
        { hasCIP, IcdManagement::Attributes::ICDCounter::kMetadataEntry },
        { hasCIP, IcdManagement::Attributes::ClientsSupportedPerFabric::kMetadataEntry },
        { hasUAT, IcdManagement::Attributes::UserActiveModeTriggerHint::kMetadataEntry },
        { mOptionalAttributeSet.IsSet(IcdManagement::Attributes::UserActiveModeTriggerInstruction::Id),
          IcdManagement::Attributes::UserActiveModeTriggerInstruction::kMetadataEntry },
        { hasLIT, IcdManagement::Attributes::OperatingMode::kMetadataEntry },
        { hasCIP, IcdManagement::Attributes::MaximumCheckInBackOff::kMetadataEntry },
    };
    return attributeListBuilder.Append(Span(kMandatoryAttributes), Span(optionalEntries));
}

std::optional<DataModel::ActionReturnStatus> ICDManagementCluster::InvokeCommand(const DataModel::InvokeRequest & request,
                                                                                 TLV::TLVReader & input_arguments,
                                                                                 CommandHandler * handler)
{
    switch (request.path.mCommandId)
    {
    case IcdManagement::Commands::StayActiveRequest::Id: {
// TODO(#32321): Remove #if after issue is resolved
// Note: We only need this #if statement for platform examples that enable the ICD management server without building the sample
// as an ICD. Since this is not spec compliant, we should remove this #if statement once we stop compiling the ICD management
// server in those examples.
#if CHIP_CONFIG_ENABLE_ICD_SERVER
        Commands::StayActiveRequest::DecodableType commandData;
        ReturnErrorOnFailure(commandData.Decode(input_arguments));

        IcdManagement::Commands::StayActiveResponse::Type response;
        response.promisedActiveDuration = Server::GetInstance().GetICDManager().StayActiveRequest(commandData.stayActiveDuration);
        handler->AddResponse(request.path, response);
#endif // CHIP_CONFIG_ENABLE_ICD_SERVER
        break;
    }
    default:
        return Status::UnsupportedCommand;
    }
    return Status::Success;
}

CHIP_ERROR ICDManagementCluster::AcceptedCommands(const ConcreteClusterPath & path,
                                                  ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder)
{
    BitFlags<IcdManagement::Feature> featureMap = mICDConfigurationData.GetFeatureMap();
    const bool hasCIP                           = featureMap.Has(IcdManagement::Feature::kCheckInProtocolSupport);

    if (hasCIP)
    {
        static constexpr DataModel::AcceptedCommandEntry kAcceptedCommands[] = {
            Commands::RegisterClient::kMetadataEntry,
            Commands::UnregisterClient::kMetadataEntry,
        };
        ReturnErrorOnFailure(builder.ReferenceExisting(kAcceptedCommands));
    }

    if (mICDConfigurationData.GetFeatureMap().Has(IcdManagement::Feature::kLongIdleTimeSupport) ||
        mEnabledCommands.Has(IcdManagement::OptionalCommands::kStayActive))
    {
        ReturnErrorOnFailure(builder.EnsureAppendCapacity(1));
        ReturnErrorOnFailure(builder.Append(Commands::StayActiveRequest::kMetadataEntry));
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR ICDManagementCluster::GeneratedCommands(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<CommandId> & builder)
{
    BitFlags<IcdManagement::Feature> featureMap = mICDConfigurationData.GetFeatureMap();
    const bool hasCIP                           = featureMap.Has(IcdManagement::Feature::kCheckInProtocolSupport);

    if (hasCIP)
    {
        static constexpr CommandId kGeneratedCommands[] = {
            Commands::RegisterClientResponse::Id,
        };
        ReturnErrorOnFailure(builder.ReferenceExisting(kGeneratedCommands));
    }

    if (mICDConfigurationData.GetFeatureMap().Has(IcdManagement::Feature::kLongIdleTimeSupport) ||
        mEnabledCommands.Has(IcdManagement::OptionalCommands::kStayActive))
    {
        ReturnErrorOnFailure(builder.EnsureAppendCapacity(1));
        ReturnErrorOnFailure(builder.Append(Commands::StayActiveResponse::Id));
    }
    return CHIP_NO_ERROR;
}

#if CHIP_CONFIG_ENABLE_ICD_LIT
CHIP_ERROR ICDManagementCluster::ReadOperatingMode(AttributeValueEncoder & encoder)
{
    return mICDConfigurationData.GetICDMode() == ICDConfigurationData::ICDMode::SIT
        ? encoder.Encode(IcdManagement::OperatingModeEnum::kSit)
        : encoder.Encode(IcdManagement::OperatingModeEnum::kLit);
}
#endif // CHIP_CONFIG_ENABLE_ICD_LIT
#if CHIP_CONFIG_ENABLE_ICD_CIP

ICDManagementClusterWithCIP::ICDManagementClusterWithCIP(
    EndpointId endpointId, PersistentStorageDelegate & storage, chip::Crypto::SessionKeystore & symmetricKeystore,
    FabricTable & fabricTable, ICDConfigurationData & icdConfigurationData, OptionalAttributeSet optionalAttributeSet,
    BitMask<IcdManagement::OptionalCommands> aEnabledCommands,
    BitMask<IcdManagement::UserActiveModeTriggerBitmap> aUserActiveModeTriggerBitmap, CharSpan aUserActiveModeTriggerInstruction) :
    ICDManagementCluster(endpointId, storage, symmetricKeystore, fabricTable, icdConfigurationData, optionalAttributeSet,
                         aEnabledCommands, aUserActiveModeTriggerBitmap, aUserActiveModeTriggerInstruction)
{}

CHIP_ERROR ICDManagementClusterWithCIP::Startup(ServerClusterContext & context)
{
    // Initialize and register Fabric delegate
    mFabricDelegate.Init(mStorage, &mSymmetricKeystore, mICDConfigurationData);
    mFabricTable.AddFabricDelegate(&mFabricDelegate);
    return CHIP_NO_ERROR;
}

void ICDManagementClusterWithCIP::Shutdown()
{
    // Unregister Fabric delegate
    mFabricTable.RemoveFabricDelegate(&mFabricDelegate);
}

DataModel::ActionReturnStatus ICDManagementClusterWithCIP::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                                         AttributeValueEncoder & aEncoder)
{
    switch (request.path.mAttributeId)
    {
    case IcdManagement::Attributes::RegisteredClients::Id:
        return ReadRegisteredClients(aEncoder);
    case IcdManagement::Attributes::ICDCounter::Id:
        return aEncoder.Encode(mICDConfigurationData.GetICDCounter().GetValue());
    case IcdManagement::Attributes::ClientsSupportedPerFabric::Id:
        return aEncoder.Encode(mICDConfigurationData.GetClientsSupportedPerFabric());
    case IcdManagement::Attributes::MaximumCheckInBackOff::Id:
        return aEncoder.Encode(mICDConfigurationData.GetMaximumCheckInBackoff().count());
    default:
        // Delegate to base class for all other attributes
        return ICDManagementCluster::ReadAttribute(request, aEncoder);
    }
}

std::optional<DataModel::ActionReturnStatus> ICDManagementClusterWithCIP::InvokeCommand(const DataModel::InvokeRequest & request,
                                                                                        TLV::TLVReader & input_arguments,
                                                                                        CommandHandler * handler)
{
    switch (request.path.mCommandId)
    {
    case IcdManagement::Commands::RegisterClient::Id: {
        Commands::RegisterClient::DecodableType commandData;
        ReturnErrorOnFailure(commandData.Decode(input_arguments, handler->GetAccessingFabricIndex()));

        uint32_t icdCounter = 0;
        Status status       = RegisterClient(handler, request.path, commandData, icdCounter);
        if (status == Status::Success)
        {
            Commands::RegisterClientResponse::Type response;
            response.ICDCounter = icdCounter;
            handler->AddResponse(request.path, response);
        }
        else
        {
            handler->AddStatus(request.path, status);
        }
        break;
    }
    case IcdManagement::Commands::UnregisterClient::Id: {
        Commands::UnregisterClient::DecodableType commandData;
        ReturnErrorOnFailure(commandData.Decode(input_arguments, handler->GetAccessingFabricIndex()));

        Status status = UnregisterClient(handler, request.path, commandData);
        handler->AddStatus(request.path, status);
        break;
    }
    default:
        // Delegate to base class for all other commands
        return ICDManagementCluster::InvokeCommand(request, input_arguments, handler);
    }
    return Protocols::InteractionModel::Status::Success;
}

CHIP_ERROR ICDManagementClusterWithCIP::ReadRegisteredClients(AttributeValueEncoder & encoder)
{
    uint16_t supported_clients                        = mICDConfigurationData.GetClientsSupportedPerFabric();
    PersistentStorageDelegate & storage               = mStorage;
    chip::Crypto::SessionKeystore & symmetricKeystore = mSymmetricKeystore;
    const FabricTable & fabricTable                   = mFabricTable;

    return encoder.EncodeList(
        [supported_clients, &storage, &symmetricKeystore, &fabricTable](const auto & subEncoder) -> CHIP_ERROR {
            ICDMonitoringEntry e(&symmetricKeystore);

            for (const auto & fabricInfo : fabricTable)
            {
                ICDMonitoringTable table(storage, fabricInfo.GetFabricIndex(), supported_clients, &symmetricKeystore);
                for (uint16_t i = 0; i < table.Limit(); ++i)
                {
                    CHIP_ERROR err = table.Get(i, e);
                    if (CHIP_ERROR_NOT_FOUND == err)
                    {
                        // No more entries in the table
                        break;
                    }
                    ReturnErrorOnFailure(err);

                    Structs::MonitoringRegistrationStruct::Type s{ .checkInNodeID    = e.checkInNodeID,
                                                                   .monitoredSubject = e.monitoredSubject,
                                                                   .clientType       = e.clientType,
                                                                   .fabricIndex      = e.fabricIndex };
                    ReturnErrorOnFailure(subEncoder.Encode(s));
                }
            }
            return CHIP_NO_ERROR;
        });
}

void ICDManagementClusterWithCIP::TriggerICDMTableUpdatedEvent()
{
    ICDNotifier::GetInstance().NotifyICDManagementEvent(ICDListener::ICDManagementEvents::kTableUpdated);
}

Status ICDManagementClusterWithCIP::RegisterClient(CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                                                   const Commands::RegisterClient::DecodableType & commandData,
                                                   uint32_t & icdCounter)
{
    FabricIndex fabricIndex            = commandObj->GetAccessingFabricIndex();
    NodeId nodeId                      = commandData.checkInNodeID;
    uint64_t monitoredSubject          = commandData.monitoredSubject;
    ClientTypeEnum clientType          = commandData.clientType;
    ByteSpan key                       = commandData.key;
    Optional<ByteSpan> verificationKey = commandData.verificationKey;
    bool isClientAdmin                 = false;

    // Check if ClientType is valid
    VerifyOrReturnError(clientType != ClientTypeEnum::kUnknownEnumValue, Status::ConstraintError);

    // Check if client is admin
    VerifyOrReturnError(CHIP_NO_ERROR == CheckAdmin(commandObj, commandPath, isClientAdmin), Status::Failure);

    bool isFirstEntryForFabric = false;
    ICDMonitoringTable table(mStorage, fabricIndex, mICDConfigurationData.GetClientsSupportedPerFabric(), &mSymmetricKeystore);

    // Get current entry, if exists
    ICDMonitoringEntry entry(&mSymmetricKeystore);
    CHIP_ERROR err = table.Find(nodeId, entry);
    if (CHIP_NO_ERROR == err)
    {
        // Existing entry: Validate Key if, and only if, the ISD does NOT have administrator permissions
        if (!isClientAdmin)
        {
            VerifyOrReturnError(verificationKey.HasValue(), Status::Failure);
            VerifyOrReturnError(entry.IsKeyEquivalent(verificationKey.Value()), Status::Failure);
        }
    }
    else if (CHIP_ERROR_NOT_FOUND == err)
    {
        // New entry
        VerifyOrReturnError(entry.index < table.Limit(), Status::ResourceExhausted);

        // Check if it's going to be the first entry for fabric
        isFirstEntryForFabric = table.IsEmpty();
    }
    else
    {
        // Error
        return Status::Failure;
    }

    // Save
    entry.checkInNodeID    = nodeId;
    entry.monitoredSubject = monitoredSubject;
    entry.clientType       = clientType;

    if (entry.keyHandleValid)
    {
        entry.DeleteKey();
    }

    err = entry.SetKey(key);
    VerifyOrReturnError(CHIP_ERROR_INVALID_ARGUMENT != err, Status::ConstraintError);
    VerifyOrReturnError(CHIP_NO_ERROR == err, Status::Failure);
    err = table.Set(entry.index, entry);

    // Delete key upon failure to prevent key storage leakage.
    if (err != CHIP_NO_ERROR)
    {
        entry.DeleteKey();
    }

    VerifyOrReturnError(CHIP_ERROR_INVALID_ARGUMENT != err, Status::ConstraintError);
    VerifyOrReturnError(CHIP_NO_ERROR == err, Status::Failure);

    if (isFirstEntryForFabric)
    {
        // Notify subscribers that the first entry for the fabric was successfully added
        TriggerICDMTableUpdatedEvent();
    }
    MatterReportingAttributeChangeCallback(kRootEndpointId, IcdManagement::Id, IcdManagement::Attributes::RegisteredClients::Id);
    icdCounter = mICDConfigurationData->GetICDCounter().GetValue();

    return Status::Success;
}

Status ICDManagementClusterWithCIP::UnregisterClient(CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                                                     const Commands::UnregisterClient::DecodableType & commandData)
{
    FabricIndex fabricIndex            = commandObj->GetAccessingFabricIndex();
    NodeId nodeId                      = commandData.checkInNodeID;
    Optional<ByteSpan> verificationKey = commandData.verificationKey;
    bool isClientAdmin                 = false;

    // Check if client is admin
    VerifyOrReturnError(CHIP_NO_ERROR == CheckAdmin(commandObj, commandPath, isClientAdmin), Status::Failure);

    ICDMonitoringTable table(mStorage, fabricIndex, mICDConfigurationData.GetClientsSupportedPerFabric(), &mSymmetricKeystore);

    // Get current entry, if exists
    ICDMonitoringEntry entry(&mSymmetricKeystore);
    CHIP_ERROR err = table.Find(nodeId, entry);
    VerifyOrReturnError(CHIP_ERROR_NOT_FOUND != err, Status::NotFound);
    VerifyOrReturnError(CHIP_NO_ERROR == err, Status::Failure);

    // Existing entry: Validate Key if, and only if, the ISD has NOT administrator permissions
    if (!isClientAdmin)
    {
        VerifyOrReturnError(verificationKey.HasValue(), Status::Failure);
        VerifyOrReturnError(entry.IsKeyEquivalent(verificationKey.Value()), Status::Failure);
    }

    err = table.Remove(entry.index);
    VerifyOrReturnError(CHIP_NO_ERROR == err, Status::Failure);

    if (table.IsEmpty())
    {
        TriggerICDMTableUpdatedEvent();
    }

    MatterReportingAttributeChangeCallback(kRootEndpointId, IcdManagement::Id, IcdManagement::Attributes::RegisteredClients::Id);
    return Status::Success;
}

#endif // CHIP_CONFIG_ENABLE_ICD_CIP

void ICDManagementServer::OnICDModeChange()
{
    // Notify attribute change for OperatingMode attribute
    MatterReportingAttributeChangeCallback(kRootEndpointId, IcdManagement::Id, IcdManagement::Attributes::OperatingMode::Id);
}

} // namespace chip::app::Clusters

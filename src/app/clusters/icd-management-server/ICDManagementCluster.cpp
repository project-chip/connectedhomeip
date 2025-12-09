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
    TEMPORARY_RETURN_IGNORED table.RemoveAll();
    ICDNotifier::GetInstance().NotifyICDManagementEvent(ICDListener::ICDManagementEvents::kTableUpdated);
}
#endif // CHIP_CONFIG_ENABLE_ICD_CIP

ICDManagementCluster::ICDManagementCluster(EndpointId endpointId, Crypto::SymmetricKeystore & symmetricKeystore,
                                           FabricTable & fabricTable, ICDConfigurationData & icdConfigurationData,
                                           OptionalAttributeSet optionalAttributeSet,
                                           BitMask<IcdManagement::OptionalCommands> enabledCommands,
                                           BitMask<IcdManagement::UserActiveModeTriggerBitmap> userActiveModeTriggerBitmap,
                                           CharSpan userActiveModeTriggerInstruction) :
    DefaultServerCluster({ endpointId, IcdManagement::Id }),
    mSymmetricKeystore(symmetricKeystore), mFabricTable(fabricTable), mICDConfigurationData(icdConfigurationData),
    mOptionalAttributeSet(optionalAttributeSet), mUserActiveModeTriggerBitmap(userActiveModeTriggerBitmap),
    mEnabledCommands(enabledCommands), mUserActiveModeTriggerInstructionLength(0)
{
    static_assert(sizeof(mUserActiveModeTriggerInstruction) <= UINT8_MAX,
                  "mUserActiveModeTriggerInstruction size must fit in uint8_t");

    MutableCharSpan buffer(mUserActiveModeTriggerInstruction);
    CopyCharSpanToMutableCharSpanWithTruncation(userActiveModeTriggerInstruction, buffer);
    mUserActiveModeTriggerInstructionLength = static_cast<uint8_t>(buffer.size());
}

CHIP_ERROR ICDManagementCluster::Startup(ServerClusterContext & context)
{
    ReturnErrorOnFailure(DefaultServerCluster::Startup(context));
// TODO(#32321): Remove #if after issue is resolved
// Note: We only need this #if statement for platform examples that enable the ICD management server without building the sample
// as an ICD. Since this is not spec compliant, we should remove this #if statement once we stop compiling the ICD management
// server in those examples.
#if CHIP_CONFIG_ENABLE_ICD_SERVER
    Server::GetInstance().GetICDManager().RegisterObserver(this);
#endif
    return CHIP_NO_ERROR;
}

void ICDManagementCluster::Shutdown(ClusterShutdownType type)
{
// TODO(#32321): Remove #if after issue is resolved
// Note: We only need this #if statement for platform examples that enable the ICD management server without building the sample
// as an ICD. Since this is not spec compliant, we should remove this #if statement once we stop compiling the ICD management
// server in those examples.
#if CHIP_CONFIG_ENABLE_ICD_SERVER
    Server::GetInstance().GetICDManager().ReleaseObserver(this);
#endif
    DefaultServerCluster::Shutdown(type);
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
        return aEncoder.Encode(CharSpan(mUserActiveModeTriggerInstruction, mUserActiveModeTriggerInstructionLength));

    default:
        return Protocols::InteractionModel::Status::UnsupportedAttribute;
    }
}

CHIP_ERROR ICDManagementCluster::Attributes(const ConcreteClusterPath & path,
                                            ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    AttributeListBuilder attributeListBuilder(builder);
    BitFlags<IcdManagement::Feature> featureMap = mICDConfigurationData.GetFeatureMap();
    const bool hasUAT                           = featureMap.Has(IcdManagement::Feature::kUserActiveModeTrigger);
    const bool hasLIT                           = featureMap.Has(IcdManagement::Feature::kLongIdleTimeSupport);
    const bool hasUATInstruction = mOptionalAttributeSet.IsSet(IcdManagement::Attributes::UserActiveModeTriggerInstruction::Id);

    const AttributeListBuilder::OptionalAttributeEntry optionalEntries[] = {
        { hasUAT, IcdManagement::Attributes::UserActiveModeTriggerHint::kMetadataEntry },
        { hasUATInstruction, IcdManagement::Attributes::UserActiveModeTriggerInstruction::kMetadataEntry },
        { hasLIT, IcdManagement::Attributes::OperatingMode::kMetadataEntry },
    };
    return attributeListBuilder.Append(Span(IcdManagement::Attributes::kMandatoryMetadata), Span(optionalEntries));
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

        Commands::StayActiveResponse::Type response;
        response.promisedActiveDuration = Server::GetInstance().GetICDManager().StayActiveRequest(commandData.stayActiveDuration);
        handler->AddResponse(request.path, response);
        return std::nullopt;
#else
        return Status::UnsupportedCommand;
#endif // CHIP_CONFIG_ENABLE_ICD_SERVER
    }
    default:
        return Status::UnsupportedCommand;
    }
}

CHIP_ERROR ICDManagementCluster::AcceptedCommands(const ConcreteClusterPath & path,
                                                  ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder)
{
    if (mICDConfigurationData.GetFeatureMap().Has(Feature::kLongIdleTimeSupport) ||
        mEnabledCommands.Has(OptionalCommands::kStayActive))
    {
        static constexpr DataModel::AcceptedCommandEntry kStayActiveCommand[] = {
            Commands::StayActiveRequest::kMetadataEntry,
        };
        ReturnErrorOnFailure(builder.ReferenceExisting(kStayActiveCommand));
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR ICDManagementCluster::GeneratedCommands(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<CommandId> & builder)
{
    if (mICDConfigurationData.GetFeatureMap().Has(Feature::kLongIdleTimeSupport) ||
        mEnabledCommands.Has(OptionalCommands::kStayActive))
    {
        static constexpr CommandId kStayActiveResponse[] = {
            Commands::StayActiveResponse::Id,
        };
        ReturnErrorOnFailure(builder.ReferenceExisting(kStayActiveResponse));
    }
    return CHIP_NO_ERROR;
}

void ICDManagementCluster::OnICDModeChange()
{
    // Notify attribute change for OperatingMode attribute
    NotifyAttributeChanged(IcdManagement::Attributes::OperatingMode::Id);
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
    EndpointId endpointId, Crypto::SymmetricKeystore & symmetricKeystore, FabricTable & fabricTable,
    ICDConfigurationData & icdConfigurationData, OptionalAttributeSet optionalAttributeSet,
    BitMask<IcdManagement::OptionalCommands> enabledCommands,
    BitMask<IcdManagement::UserActiveModeTriggerBitmap> userActiveModeTriggerBitmap, CharSpan userActiveModeTriggerInstruction) :
    ICDManagementCluster(endpointId, symmetricKeystore, fabricTable, icdConfigurationData, optionalAttributeSet, enabledCommands,
                         userActiveModeTriggerBitmap, userActiveModeTriggerInstruction)
{}

CHIP_ERROR ICDManagementClusterWithCIP::Startup(ServerClusterContext & context)
{
    ReturnErrorOnFailure(ICDManagementCluster::Startup(context));

    mFabricDelegate.Init(context.storage, &mSymmetricKeystore, mICDConfigurationData);
    return mFabricTable.AddFabricDelegate(&mFabricDelegate);
}

void ICDManagementClusterWithCIP::Shutdown(ClusterShutdownType shutdownType)
{
    mFabricTable.RemoveFabricDelegate(&mFabricDelegate);

    ICDManagementCluster::Shutdown(shutdownType);
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

CHIP_ERROR ICDManagementClusterWithCIP::Attributes(const ConcreteClusterPath & path,
                                                   ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    // First get the base class attributes
    ReturnErrorOnFailure(ICDManagementCluster::Attributes(path, builder));

    // Add CIP-specific attributes
    BitFlags<IcdManagement::Feature> featureMap = mICDConfigurationData.GetFeatureMap();
    const bool hasCIP                           = featureMap.Has(Feature::kCheckInProtocolSupport);

    if (hasCIP)
    {
        static constexpr DataModel::AttributeEntry kCIPAttributes[] = {
            Attributes::RegisteredClients::kMetadataEntry,
            Attributes::ICDCounter::kMetadataEntry,
            Attributes::ClientsSupportedPerFabric::kMetadataEntry,
            Attributes::MaximumCheckInBackOff::kMetadataEntry,
        };
        ReturnErrorOnFailure(builder.ReferenceExisting(kCIPAttributes));
    }

    return CHIP_NO_ERROR;
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
    return std::nullopt;
}

CHIP_ERROR ICDManagementClusterWithCIP::AcceptedCommands(const ConcreteClusterPath & path,
                                                         ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder)
{
    ReturnErrorOnFailure(ICDManagementCluster::AcceptedCommands(path, builder));

    BitFlags<IcdManagement::Feature> featureMap = mICDConfigurationData.GetFeatureMap();
    const bool hasCIP                           = featureMap.Has(Feature::kCheckInProtocolSupport);

    if (hasCIP)
    {
        static constexpr DataModel::AcceptedCommandEntry kCIPCommands[] = {
            Commands::RegisterClient::kMetadataEntry,
            Commands::UnregisterClient::kMetadataEntry,
        };
        ReturnErrorOnFailure(builder.ReferenceExisting(kCIPCommands));
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR ICDManagementClusterWithCIP::GeneratedCommands(const ConcreteClusterPath & path,
                                                          ReadOnlyBufferBuilder<CommandId> & builder)
{
    ReturnErrorOnFailure(ICDManagementCluster::GeneratedCommands(path, builder));

    BitFlags<IcdManagement::Feature> featureMap = mICDConfigurationData.GetFeatureMap();
    const bool hasCIP                           = featureMap.Has(Feature::kCheckInProtocolSupport);

    if (hasCIP)
    {
        static constexpr CommandId kCIPResponses[] = {
            Commands::RegisterClientResponse::Id,
        };
        ReturnErrorOnFailure(builder.ReferenceExisting(kCIPResponses));
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR ICDManagementClusterWithCIP::ReadRegisteredClients(AttributeValueEncoder & encoder)
{
    uint16_t supported_clients                    = mICDConfigurationData.GetClientsSupportedPerFabric();
    PersistentStorageDelegate & storage           = mContext->storage;
    Crypto::SymmetricKeystore & symmetricKeystore = mSymmetricKeystore;
    const FabricTable & fabricTable               = mFabricTable;

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
    ICDMonitoringTable table(mContext->storage, fabricIndex, mICDConfigurationData.GetClientsSupportedPerFabric(),
                             &mSymmetricKeystore);

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
        TEMPORARY_RETURN_IGNORED entry.DeleteKey();
    }

    err = entry.SetKey(key);
    VerifyOrReturnError(CHIP_ERROR_INVALID_ARGUMENT != err, Status::ConstraintError);
    VerifyOrReturnError(CHIP_NO_ERROR == err, Status::Failure);
    err = table.Set(entry.index, entry);

    // Delete key upon failure to prevent key storage leakage.
    if (err != CHIP_NO_ERROR)
    {
        TEMPORARY_RETURN_IGNORED entry.DeleteKey();
    }

    VerifyOrReturnError(CHIP_ERROR_INVALID_ARGUMENT != err, Status::ConstraintError);
    VerifyOrReturnError(CHIP_NO_ERROR == err, Status::Failure);

    if (isFirstEntryForFabric)
    {
        // Notify subscribers that the first entry for the fabric was successfully added
        TriggerICDMTableUpdatedEvent();
    }
    MarkRegisteredClientsListChanged();
    icdCounter = mICDConfigurationData.GetICDCounter().GetValue();

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

    ICDMonitoringTable table(mContext->storage, fabricIndex, mICDConfigurationData.GetClientsSupportedPerFabric(),
                             &mSymmetricKeystore);

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

    MarkRegisteredClientsListChanged();
    return Status::Success;
}

void ICDManagementClusterWithCIP::MarkRegisteredClientsListChanged()
{
    NotifyAttributeChanged(IcdManagement::Attributes::RegisteredClients::Id);
}
#endif // CHIP_CONFIG_ENABLE_ICD_CIP

} // namespace chip::app::Clusters

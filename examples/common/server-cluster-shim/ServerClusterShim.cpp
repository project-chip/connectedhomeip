/*
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
#include "ServerClusterShim.h"

#include <access/Privilege.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app/AttributeAccessInterface.h>
#include <app/AttributeAccessInterfaceRegistry.h>
#include <app/CommandHandlerInterface.h>
#include <app/CommandHandlerInterfaceRegistry.h>
#include <app/ConcreteClusterPath.h>
#include <app/GlobalAttributes.h>
#include <app/RequiredPrivilege.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <app/util/IMClusterCommandHandler.h>
#include <app/util/attribute-storage-detail.h>
#include <app/util/attribute-storage.h>
#include <app/util/attribute-table-detail.h>
#include <app/util/attribute-table.h>
#include <crypto/RandUtils.h>
#include <data-model-providers/codegen/EmberAttributeDataBuffer.h>
#include <lib/support/BitFlags.h>
#include <lib/support/ReadOnlyBuffer.h>
#include <optional>
#include <protocols/interaction_model/StatusCode.h>

namespace chip {
namespace app {

namespace {

using namespace chip::app::Clusters;
using namespace chip::app::DataModel;
using namespace chip::app::Compatibility::Internal;
using Protocols::InteractionModel::Status;

/// Attempts to read via an attribute access interface (AAI)
///
/// If it returns a CHIP_ERROR, then this is a FINAL result (i.e. either failure or success).
///
/// If it returns std::nullopt, then there is no AAI to handle the given path
/// and processing should figure out the value otherwise (generally from other ember data)
std::optional<CHIP_ERROR> TryReadViaAccessInterface(const ConcreteAttributePath & path, AttributeAccessInterface * aai,
                                                    AttributeValueEncoder & encoder)
{
    // Processing can happen only if an attribute access interface actually exists..
    if (aai == nullptr)
    {
        return std::nullopt;
    }

    CHIP_ERROR err = aai->Read(path, encoder);

    if (err != CHIP_NO_ERROR)
    {
        // Implementation of 8.4.3.2 (Incoming Read Request and Subscribe Request Action Processing)
        // of the spec for path expansion
        if (path.mExpanded && (err == CHIP_IM_GLOBAL_STATUS(UnsupportedRead)))
        {
            return CHIP_NO_ERROR;
        }

        return err;
    }

    // If the encoder tried to encode, then a value should have been written.
    //   - if encode, assume DONE (i.e. FINAL CHIP_NO_ERROR)
    //   - if no encode, say that processing must continue
    return encoder.TriedEncode() ? std::make_optional(CHIP_NO_ERROR) : std::nullopt;
}

/// Attempts to write via an attribute access interface (AAI)
///
/// If it returns a CHIP_ERROR, then this is a FINAL result (i.e. either failure or success)
///
/// If it returns std::nullopt, then there is no AAI to handle the given path
/// and processing should figure out the value otherwise (generally from other ember data)
std::optional<CHIP_ERROR> TryWriteViaAccessInterface(const ConcreteDataAttributePath & path, AttributeAccessInterface * aai,
                                                     AttributeValueDecoder & decoder)
{
    // Processing can happen only if an attribute access interface actually exists..
    if (aai == nullptr)
    {
        return std::nullopt;
    }

    CHIP_ERROR err = aai->Write(path, decoder);

    if (err != CHIP_NO_ERROR)
    {
        return std::make_optional(err);
    }

    // If the decoder tried to decode, then a value should have been read for processing.
    //   - if decoding was done, assume DONE (i.e. final CHIP_NO_ERROR)
    //   -  otherwise, if no decoding done, return that processing must continue via nullopt
    return decoder.TriedDecode() ? std::make_optional(CHIP_NO_ERROR) : std::nullopt;
}

DataModel::AttributeEntry AttributeEntryFrom(const ConcreteClusterPath & clusterPath, const EmberAfAttributeMetadata & attribute)
{
    const ConcreteAttributePath attributePath(clusterPath.mEndpointId, clusterPath.mClusterId, attribute.attributeId);

    using DataModel::AttributeQualityFlags;

    DataModel::AttributeEntry entry(
        attribute.attributeId,
        BitFlags<DataModel::AttributeQualityFlags>{}
            .Set(AttributeQualityFlags::kListAttribute, (attribute.attributeType == ZCL_ARRAY_ATTRIBUTE_TYPE))
            .Set(DataModel::AttributeQualityFlags::kTimed, attribute.MustUseTimedWrite()),
        attribute.IsReadable() ? std::make_optional(RequiredPrivilege::ForReadAttribute(attributePath)) : std::nullopt,
        attribute.IsWritable() ? std::make_optional(RequiredPrivilege::ForWriteAttribute(attributePath)) : std::nullopt);

    // TODO: Set additional flags:
    // entry.flags.Set(DataModel::AttributeQualityFlags::kFabricScoped)
    // entry.flags.Set(DataModel::AttributeQualityFlags::kFabricSensitive)
    // entry.flags.Set(DataModel::AttributeQualityFlags::kChangesOmitted)
    return entry;
}

DataModel::AcceptedCommandEntry AcceptedCommandEntryFor(const ConcreteCommandPath & path)
{
    const CommandId commandId = path.mCommandId;

    DataModel::AcceptedCommandEntry entry(
        path.mCommandId,
        BitFlags<DataModel::CommandQualityFlags>{}
            .Set(DataModel::CommandQualityFlags::kTimed, CommandNeedsTimedInvoke(path.mClusterId, commandId))
            .Set(DataModel::CommandQualityFlags::kFabricScoped, CommandIsFabricScoped(path.mClusterId, commandId))
            .Set(DataModel::CommandQualityFlags::kLargeMessage, CommandHasLargePayload(path.mClusterId, commandId)),
        RequiredPrivilege::ForInvokeCommand(path));

    return entry;
}

bool PathsContainsOrLogError(const ConcreteClusterPath & path, ServerClusterInterface & serverCluster)
{
    if (!serverCluster.PathsContains({ path.mEndpointId, path.mClusterId }))
    {
        ChipLogError(DataManagement,
                     "[Configuration Error] The cluster path has not been added to this "
                     "ServerClusterShim instance: Endpoint=0x%x Cluster " ChipLogFormatMEI,
                     path.mEndpointId, ChipLogValueMEI(path.mClusterId));
        return false;
    }
    return true;
}

} // namespace

ServerClusterShim::~ServerClusterShim() {}

CHIP_ERROR ServerClusterShim::Startup(ServerClusterContext & context)
{
    mContext = &context;
    return CHIP_NO_ERROR;
}

void ServerClusterShim::Shutdown(ClusterShutdownType)
{
    mContext = nullptr;
}

DataVersion ServerClusterShim::GetDataVersion(const ConcreteClusterPath & path) const
{
    DataVersion * versionPtr = emberAfDataVersionStorage(path);
    if (versionPtr != nullptr)
    {
        return *versionPtr;
    }
    // Should not happen. We don't have a way to return error on this API.
    return 0;
}

CHIP_ERROR ServerClusterShim::Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<AttributeEntry> & builder)
{
    const EmberAfCluster * cluster = emberAfFindServerCluster(path.mEndpointId, path.mClusterId);

    VerifyOrReturnValue(cluster != nullptr, CHIP_ERROR_NOT_FOUND);
    VerifyOrReturnValue(cluster->attributeCount > 0, CHIP_NO_ERROR);
    VerifyOrReturnValue(cluster->attributes != nullptr, CHIP_NO_ERROR);

    // TODO: if ember would encode data in AttributeEntry form, we could reference things directly (shorter code,
    //       although still allocation overhead due to global attributes not in metadata)
    //
    // We have Attributes from ember + global attributes that are NOT in ember metadata.
    // We have to report them all
    constexpr size_t kGlobalAttributeNotInMetadataCount = MATTER_ARRAY_SIZE(GlobalAttributesNotInMetadata);

    ReturnErrorOnFailure(builder.EnsureAppendCapacity(cluster->attributeCount + kGlobalAttributeNotInMetadataCount));

    Span<const EmberAfAttributeMetadata> attributeSpan(cluster->attributes, cluster->attributeCount);

    for (auto & attribute : attributeSpan)
    {
        ReturnErrorOnFailure(builder.Append(AttributeEntryFrom(path, attribute)));
    }

    for (auto & attributeId : GlobalAttributesNotInMetadata)
    {

        // This "GlobalListEntry" is specific for metadata that ember does not include
        // in its attribute list metadata.
        //
        // By spec these Attribute/AcceptedCommands/GeneratedCommants lists are:
        //   - lists of elements
        //   - read-only, with read privilege view
        //   - fixed value (no such flag exists, so this is not a quality flag we set/track)
        DataModel::AttributeEntry globalListEntry(attributeId, DataModel::AttributeQualityFlags::kListAttribute,
                                                  Access::Privilege::kView, std::nullopt);

        ReturnErrorOnFailure(builder.Append(std::move(globalListEntry)));
    }

    return CHIP_NO_ERROR;
}

DataModel::ActionReturnStatus ServerClusterShim::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                               AttributeValueEncoder & encoder)
{
    ChipLogDetail(DataManagement,
                  "Reading attribute: Cluster=" ChipLogFormatMEI " Endpoint=0x%x AttributeId=" ChipLogFormatMEI " (expanded=%d)",
                  ChipLogValueMEI(request.path.mClusterId), request.path.mEndpointId, ChipLogValueMEI(request.path.mAttributeId),
                  request.path.mExpanded);

    const EmberAfAttributeMetadata * attributeMetadata =
        emberAfLocateAttributeMetadata(request.path.mEndpointId, request.path.mClusterId, request.path.mAttributeId);

    // ReadAttribute requirement is that request.path is a VALID path inside the provider
    // metadata tree. Clients are supposed to validate this (and data version and other flags)
    // This SHOULD NEVER HAPPEN hence the general return code (seemed preferable to VerifyOrDie)
    VerifyOrReturnError(attributeMetadata != nullptr, Status::Failure);

    if (!PathsContainsOrLogError({ request.path.mEndpointId, request.path.mClusterId }, *this))
    {
        return Status::Failure;
    }

    // Read via AAI
    std::optional<CHIP_ERROR> aai_result = TryReadViaAccessInterface(
        request.path, AttributeAccessInterfaceRegistry::Instance().Get(request.path.mEndpointId, request.path.mClusterId), encoder);
    VerifyOrReturnError(!aai_result.has_value(), *aai_result);

    // At this point, we have to use ember directly to read the data.
    EmberAfAttributeSearchRecord record;
    record.endpoint                            = request.path.mEndpointId;
    record.clusterId                           = request.path.mClusterId;
    record.attributeId                         = request.path.mAttributeId;
    Protocols::InteractionModel::Status status = emAfReadOrWriteAttribute(
        &record, &attributeMetadata, gEmberAttributeIOBufferSpan.data(), static_cast<uint16_t>(gEmberAttributeIOBufferSpan.size()),
        /* write = */ false);

    if (status != Protocols::InteractionModel::Status::Success)
    {
        return CHIP_ERROR_IM_GLOBAL_STATUS_VALUE(status);
    }

    VerifyOrReturnError(attributeMetadata != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    MutableByteSpan data = gEmberAttributeIOBufferSpan;
    Ember::EmberAttributeDataBuffer emberData(attributeMetadata, data);
    return encoder.Encode(emberData);
}

ActionReturnStatus ServerClusterShim::WriteAttribute(const WriteAttributeRequest & request, AttributeValueDecoder & decoder)
{
    // Context not initialized. Need to call Startup(context) before writing.
    VerifyOrReturnError(mContext != nullptr, Status::InvalidInState);

    const EmberAfAttributeMetadata * attributeMetadata =
        emberAfLocateAttributeMetadata(request.path.mEndpointId, request.path.mClusterId, request.path.mAttributeId);

    // WriteAttribute requirement is that request.path is a VALID path inside the provider
    // metadata tree. Clients are supposed to validate this (and data version and other flags)
    // This SHOULD NEVER HAPPEN hence the general return code (seemed preferable to VerifyOrDie)
    VerifyOrReturnError(attributeMetadata != nullptr, Status::Failure);

    if (!PathsContainsOrLogError({ request.path.mEndpointId, request.path.mClusterId }, *this))
    {
        return Status::Failure;
    }

    // Extra check: internal requests can bypass the read only check, however global attributes
    // have no underlying storage, so write still cannot be done
    //
    // I.e. if we get a `EmberAfCluster*` value from finding metadata, we fail here.
    VerifyOrReturnError(attributeMetadata != nullptr, Status::UnsupportedWrite);

    if (request.path.mDataVersion.HasValue())
    {
        DataVersion * versionPtr = emberAfDataVersionStorage(request.path);

        if (versionPtr == nullptr)
        {
            ChipLogError(DataManagement, "Unable to get cluster info for Endpoint 0x%x, Cluster " ChipLogFormatMEI,
                         request.path.mEndpointId, ChipLogValueMEI(request.path.mClusterId));
            return Status::DataVersionMismatch;
        }

        if (request.path.mDataVersion.Value() != *versionPtr)
        {
            ChipLogError(DataManagement, "Write Version mismatch for Endpoint 0x%x, Cluster " ChipLogFormatMEI,
                         request.path.mEndpointId, ChipLogValueMEI(request.path.mClusterId));
            return Status::DataVersionMismatch;
        }
    }

    AttributeAccessInterface * aai =
        AttributeAccessInterfaceRegistry::Instance().Get(request.path.mEndpointId, request.path.mClusterId);
    std::optional<CHIP_ERROR> aai_result = TryWriteViaAccessInterface(request.path, aai, decoder);
    if (aai_result.has_value())
    {
        if (*aai_result == CHIP_NO_ERROR)
        {
            // TODO: this is awkward since it provides AAI no control over this, specifically
            //       AAI may not want to increase versions for some attributes that are Q
            emberAfAttributeChanged(request.path.mEndpointId, request.path.mClusterId, request.path.mAttributeId,
                                    &mContext->interactionContext.dataModelChangeListener);
        }
        return *aai_result;
    }

    MutableByteSpan dataBuffer = gEmberAttributeIOBufferSpan;
    {
        Ember::EmberAttributeDataBuffer emberData(attributeMetadata, dataBuffer);
        ReturnErrorOnFailure(decoder.Decode(emberData));
    }

    if (dataBuffer.size() > attributeMetadata->size)
    {
        ChipLogDetail(Zcl, "Data to write exceeds the attribute size claimed.");
        return Status::InvalidValue;
    }

    Protocols::InteractionModel::Status status;
    EmberAfWriteDataInput dataInput(dataBuffer.data(), attributeMetadata->attributeType);
    dataInput.SetChangeListener(&mContext->interactionContext.dataModelChangeListener);
    // TODO: dataInput.SetMarkDirty() should be according to `ChangesOmmited`

    if (request.operationFlags.Has(DataModel::OperationFlags::kInternal))
    {
        // Internal requests use the non-External interface that has less enforcement
        // than the external version (e.g. does not check/enforce writable settings, does not
        // validate attribute types) - see attribute-table.h documentation for details.
        status = emberAfWriteAttribute(request.path, dataInput);
    }
    else
    {
        status = emAfWriteAttributeExternal(request.path, dataInput);
    }

    if (status != Protocols::InteractionModel::Status::Success)
    {
        return status;
    }

    return CHIP_NO_ERROR;
}

std::optional<ActionReturnStatus> ServerClusterShim::InvokeCommand(const InvokeRequest & request,
                                                                   chip::TLV::TLVReader & input_arguments, CommandHandler * handler)
{
    CommandHandlerInterface * handler_interface =
        CommandHandlerInterfaceRegistry::Instance().GetCommandHandler(request.path.mEndpointId, request.path.mClusterId);

    if (handler_interface && handler != nullptr)
    {
        CommandHandlerInterface::HandlerContext context(*handler, request.path, input_arguments);
        handler_interface->InvokeCommand(context);

        // If the command was handled, don't proceed any further and return successfully.
        if (context.mCommandHandled)
        {
            return std::nullopt;
        }
    }

    if (!PathsContainsOrLogError({ request.path.mEndpointId, request.path.mClusterId }, *this))
    {
        return Status::Failure;
    }

    // Ember always sets the return in the handler
    DispatchSingleClusterCommand(request.path, input_arguments, handler);
    return std::nullopt;
}

CHIP_ERROR ServerClusterShim::AcceptedCommands(const ConcreteClusterPath & path,
                                               ReadOnlyBufferBuilder<AcceptedCommandEntry> & builder)
{

    // Make sure first that the cluster actually exists on this endpoint before asking the
    // CommandHandlerInterface what commands it claims to support.
    const EmberAfCluster * serverCluster = emberAfFindServerCluster(path.mEndpointId, path.mClusterId);
    VerifyOrReturnError(serverCluster != nullptr, CHIP_ERROR_NOT_FOUND);

    // If path exists in ember storage but not added by the user to the ServerClusterShim instance
    if (!PathsContainsOrLogError({ path.mEndpointId, path.mClusterId }, *this))
    {
        return CHIP_ERROR_NOT_FOUND;
    }

    CommandHandlerInterface * interface =
        CommandHandlerInterfaceRegistry::Instance().GetCommandHandler(path.mEndpointId, path.mClusterId);

    if (interface != nullptr)
    {
        CHIP_ERROR err = interface->RetrieveAcceptedCommands(path, builder);
        // If retrieving the accepted commands returns CHIP_ERROR_NOT_IMPLEMENTED then continue with normal procesing.
        // Otherwise we finished.
        VerifyOrReturnError(err == CHIP_ERROR_NOT_IMPLEMENTED, err);
    }

    VerifyOrReturnError(serverCluster->acceptedCommandList != nullptr, CHIP_NO_ERROR);

    const chip::CommandId * endOfList = serverCluster->acceptedCommandList;
    while (*endOfList != kInvalidCommandId)
    {
        endOfList++;
    }
    const auto commandCount = static_cast<size_t>(endOfList - serverCluster->acceptedCommandList);

    // TODO: if ember would store command entries, we could simplify this code to use static data
    ReturnErrorOnFailure(builder.EnsureAppendCapacity(commandCount));

    ConcreteCommandPath commandPath = ConcreteCommandPath(path.mEndpointId, path.mClusterId, kInvalidCommandId);
    for (const chip::CommandId * p = serverCluster->acceptedCommandList; p != endOfList; p++)
    {
        commandPath.mCommandId = *p;
        ReturnErrorOnFailure(builder.Append(AcceptedCommandEntryFor(commandPath)));
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR ServerClusterShim::GeneratedCommands(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<CommandId> & builder)
{
    // Make sure first that the cluster actually exists on this endpoint before asking the
    // CommandHandlerInterface what commands it claims to support.
    const EmberAfCluster * serverCluster = emberAfFindServerCluster(path.mEndpointId, path.mClusterId);
    VerifyOrReturnError(serverCluster != nullptr, CHIP_ERROR_NOT_FOUND);

    // If path exists in ember storage but not added by the user to the ServerClusterShim instance
    if (!PathsContainsOrLogError({ path.mEndpointId, path.mClusterId }, *this))
    {
        return CHIP_ERROR_NOT_FOUND;
    }

    CommandHandlerInterface * interface =
        CommandHandlerInterfaceRegistry::Instance().GetCommandHandler(path.mEndpointId, path.mClusterId);
    if (interface != nullptr)
    {
        CHIP_ERROR err = interface->RetrieveGeneratedCommands(path, builder);
        // If retrieving generated commands returns CHIP_ERROR_NOT_IMPLEMENTED then continue with normal procesing.
        // Otherwise we finished.
        VerifyOrReturnError(err == CHIP_ERROR_NOT_IMPLEMENTED, err);
    }

    VerifyOrReturnError(serverCluster->generatedCommandList != nullptr, CHIP_NO_ERROR);

    const chip::CommandId * endOfList = serverCluster->generatedCommandList;
    while (*endOfList != kInvalidCommandId)
    {
        endOfList++;
    }
    const auto commandCount = static_cast<size_t>(endOfList - serverCluster->generatedCommandList);
    return builder.ReferenceExisting({ serverCluster->generatedCommandList, commandCount });
}

void ServerClusterShim::ListAttributeWriteNotification(const ConcreteAttributePath & aPath, DataModel::ListWriteOperation opType,
                                                       FabricIndex accessingFabric)
{
    AttributeAccessInterface * aai = AttributeAccessInterfaceRegistry::Instance().Get(aPath.mEndpointId, aPath.mClusterId);

    if (aai != nullptr && PathsContains({ aPath.mEndpointId, aPath.mClusterId }))
    {
        switch (opType)
        {
        case DataModel::ListWriteOperation::kListWriteBegin:
            aai->OnListWriteBegin(aPath);
            break;
        case DataModel::ListWriteOperation::kListWriteFailure:
            aai->OnListWriteEnd(aPath, false);
            break;
        case DataModel::ListWriteOperation::kListWriteSuccess:
            aai->OnListWriteEnd(aPath, true);
            break;
        }
    }
}

CHIP_ERROR ServerClusterShim::EventInfo(const ConcreteEventPath & path, DataModel::EventEntry & eventInfo)
{
    eventInfo.readPrivilege = RequiredPrivilege::ForReadEvent(path);
    return CHIP_NO_ERROR;
}

} // namespace app
} // namespace chip

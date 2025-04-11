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
#include <data-model-providers/codegen/CodegenServerCluster.h>

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
#include <data-model-providers/codegen/EmberMetadata.h>
#include <lib/support/BitFlags.h>
#include <optional>
#include <protocols/interaction_model/StatusCode.h>

namespace chip {
namespace app {

namespace {

using namespace chip::app::Clusters;
using namespace chip::app::DataModel;
using namespace chip::app::Compatibility::Internal;
using Protocols::InteractionModel::Status;

class ContextAttributesChangeListener : public AttributesChangedListener
{
public:
    ContextAttributesChangeListener(const DataModel::InteractionModelContext & context) : mListener(context.dataModelChangeListener)
    {}
    void MarkDirty(const AttributePathParams & path) override { mListener->MarkDirty(path); }

private:
    DataModel::ProviderChangeListener * mListener;
};

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
        // Implementation of 8.4.3.2 of the spec for path expansion
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
    DataModel::AttributeEntry entry;

    const ConcreteAttributePath attributePath(clusterPath.mEndpointId, clusterPath.mClusterId, attribute.attributeId);

    entry.attributeId   = attribute.attributeId;
    entry.readPrivilege = RequiredPrivilege::ForReadAttribute(attributePath);
    if (!attribute.IsReadOnly())
    {
        entry.writePrivilege = RequiredPrivilege::ForWriteAttribute(attributePath);
    }

    entry.flags.Set(DataModel::AttributeQualityFlags::kListAttribute, (attribute.attributeType == ZCL_ARRAY_ATTRIBUTE_TYPE));
    entry.flags.Set(DataModel::AttributeQualityFlags::kTimed, attribute.MustUseTimedWrite());

    // NOTE: we do NOT provide additional info for:
    //    - IsExternal/IsSingleton/IsAutomaticallyPersisted is not used by IM handling
    //    - IsSingleton spec defines it for CLUSTERS where as we have it for ATTRIBUTES
    //    - Several specification flags are not available (reportable, quieter reporting,
    //      fixed, source attribution)

    // TODO: Set additional flags:
    // entry.flags.Set(DataModel::AttributeQualityFlags::kFabricScoped)
    // entry.flags.Set(DataModel::AttributeQualityFlags::kFabricSensitive)
    // entry.flags.Set(DataModel::AttributeQualityFlags::kChangesOmitted)
    return entry;
}

DataModel::AcceptedCommandEntry AcceptedCommandEntryFor(const ConcreteCommandPath & path)
{
    const CommandId commandId = path.mCommandId;

    DataModel::AcceptedCommandEntry entry;

    entry.commandId       = path.mCommandId;
    entry.invokePrivilege = RequiredPrivilege::ForInvokeCommand(path);
    entry.flags.Set(DataModel::CommandQualityFlags::kTimed, CommandNeedsTimedInvoke(path.mClusterId, commandId));
    entry.flags.Set(DataModel::CommandQualityFlags::kFabricScoped, CommandIsFabricScoped(path.mClusterId, commandId));
    entry.flags.Set(DataModel::CommandQualityFlags::kLargeMessage, CommandHasLargePayload(path.mClusterId, commandId));

    return entry;
}

} // namespace

CHIP_ERROR CodegenServerCluster::Startup(ServerClusterContext & context)
{
    mContext = &context;
    return CHIP_NO_ERROR;
}

void CodegenServerCluster::Shutdown()
{
    mContext = nullptr;
}

DataVersion CodegenServerCluster::GetDataVersion(const ConcreteClusterPath & path) const
{
    DataVersion * versionPtr = emberAfDataVersionStorage(path);
    if (versionPtr != nullptr)
    {
        return *versionPtr;
    }
    return 0;
}

CHIP_ERROR CodegenServerCluster::Attributes(const ConcreteClusterPath & path, DataModel::ListBuilder<AttributeEntry> & builder)
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

    // This "GlobalListEntry" is specific for metadata that ember does not include
    // in its attribute list metadata.
    //
    // By spec these Attribute/AcceptedCommands/GeneratedCommants lists are:
    //   - lists of elements
    //   - read-only, with read privilege view
    //   - fixed value (no such flag exists, so this is not a quality flag we set/track)
    DataModel::AttributeEntry globalListEntry;

    globalListEntry.readPrivilege = Access::Privilege::kView;
    globalListEntry.flags.Set(DataModel::AttributeQualityFlags::kListAttribute);

    for (auto & attribute : GlobalAttributesNotInMetadata)
    {
        globalListEntry.attributeId = attribute;
        ReturnErrorOnFailure(builder.Append(globalListEntry));
    }

    return CHIP_NO_ERROR;
}

DataModel::ActionReturnStatus CodegenServerCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                                  AttributeValueEncoder & encoder)
{
    ChipLogDetail(DataManagement,
                  "Reading attribute: Cluster=" ChipLogFormatMEI " Endpoint=0x%x AttributeId=" ChipLogFormatMEI " (expanded=%d)",
                  ChipLogValueMEI(request.path.mClusterId), request.path.mEndpointId, ChipLogValueMEI(request.path.mAttributeId),
                  request.path.mExpanded);

    auto metadata = Ember::FindAttributeMetadata(request.path);

    // Explicit failure in finding a suitable metadata
    if (const Status * status = std::get_if<Status>(&metadata))
    {
        VerifyOrDie((*status == Status::UnsupportedEndpoint) || //
                    (*status == Status::UnsupportedCluster) ||  //
                    (*status == Status::UnsupportedAttribute));
        return *status;
    }

    // Read via AAI
    std::optional<CHIP_ERROR> aai_result = TryReadViaAccessInterface(
        request.path, AttributeAccessInterfaceRegistry::Instance().Get(request.path.mEndpointId, request.path.mClusterId), encoder);
    VerifyOrReturnError(!aai_result.has_value(), *aai_result);

    const EmberAfAttributeMetadata * attributeMetadata = std::get<const EmberAfAttributeMetadata *>(metadata);
    // We can only get a status or metadata.
    VerifyOrDie(attributeMetadata != nullptr);

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

ActionReturnStatus CodegenServerCluster::WriteAttribute(const WriteAttributeRequest & request, AttributeValueDecoder & decoder)
{
    auto metadata = Ember::FindAttributeMetadata(request.path);

    // Explicit failure in finding a suitable metadata
    if (const Status * status = std::get_if<Status>(&metadata))
    {
        VerifyOrDie((*status == Status::UnsupportedEndpoint) || //
                    (*status == Status::UnsupportedCluster) ||  //
                    (*status == Status::UnsupportedAttribute));

        // Check if this is an attribute that ember does not know about but is valid after all and
        // adjust the return code. All these global attributes are `read only` hence the return
        // of unsupported write.
        //
        // If the cluster or endpoint does not exist, though, keep that return code.
        if ((*status == Protocols::InteractionModel::Status::UnsupportedAttribute) &&
            IsSupportedGlobalAttributeNotInMetadata(request.path.mAttributeId))
        {
            return Status::UnsupportedWrite;
        }

        return *status;
    }

    const EmberAfAttributeMetadata ** attributeMetadata = std::get_if<const EmberAfAttributeMetadata *>(&metadata);
    VerifyOrDie(*attributeMetadata != nullptr);

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

    // Context not initialized. Need to call Startup(context) before writing.
    if (mContext == nullptr || mContext->interactionContext == nullptr ||
        mContext->interactionContext->dataModelChangeListener == nullptr)
    {
        return Status::InvalidInState;
    }

    ContextAttributesChangeListener change_listener(*mContext->interactionContext);

    AttributeAccessInterface * aai =
        AttributeAccessInterfaceRegistry::Instance().Get(request.path.mEndpointId, request.path.mClusterId);
    std::optional<CHIP_ERROR> aai_result = TryWriteViaAccessInterface(request.path, aai, decoder);
    if (aai_result.has_value())
    {
        if (*aai_result == CHIP_NO_ERROR)
        {
            // TODO: this is awkward since it provides AAI no control over this, specifically
            //       AAI may not want to increase versions for some attributes that are Q
            emberAfAttributeChanged(request.path.mEndpointId, request.path.mClusterId, request.path.mAttributeId, &change_listener);
        }
        return *aai_result;
    }

    MutableByteSpan dataBuffer = gEmberAttributeIOBufferSpan;
    {
        Ember::EmberAttributeDataBuffer emberData(*attributeMetadata, dataBuffer);
        ReturnErrorOnFailure(decoder.Decode(emberData));
    }

    if (dataBuffer.size() > (*attributeMetadata)->size)
    {
        ChipLogDetail(Zcl, "Data to write exceeds the attribute size claimed.");
        return Status::InvalidValue;
    }

    Protocols::InteractionModel::Status status;
    EmberAfWriteDataInput dataInput(dataBuffer.data(), (*attributeMetadata)->attributeType);
    dataInput.SetChangeListener(&change_listener);
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

std::optional<ActionReturnStatus>
CodegenServerCluster::InvokeCommand(const InvokeRequest & request, chip::TLV::TLVReader & input_arguments, CommandHandler * handler)
{
    CommandHandlerInterface * handler_interface =
        CommandHandlerInterfaceRegistry::Instance().GetCommandHandler(request.path.mEndpointId, request.path.mClusterId);

    if (handler_interface)
    {
        CommandHandlerInterface::HandlerContext context(*handler, request.path, input_arguments);
        handler_interface->InvokeCommand(context);

        // If the command was handled, don't proceed any further and return successfully.
        if (context.mCommandHandled)
        {
            return std::nullopt;
        }
    }

    // Ember always sets the return in the handler
    DispatchSingleClusterCommand(request.path, input_arguments, handler);
    return std::nullopt;
}

CHIP_ERROR CodegenServerCluster::AcceptedCommands(const ConcreteClusterPath & path,
                                                  DataModel::ListBuilder<AcceptedCommandEntry> & builder)
{

    // Some CommandHandlerInterface instances are registered of ALL endpoints, so make sure first that
    // the cluster actually exists on this endpoint before asking the CommandHandlerInterface what commands
    // it claims to support.
    const EmberAfCluster * serverCluster = emberAfFindServerCluster(path.mEndpointId, path.mClusterId);
    VerifyOrReturnError(serverCluster != nullptr, CHIP_ERROR_NOT_FOUND);

    CommandHandlerInterface * interface =
        CommandHandlerInterfaceRegistry::Instance().GetCommandHandler(path.mEndpointId, path.mClusterId);
    if (interface != nullptr)
    {
        size_t commandCount = 0;

        CHIP_ERROR err = interface->EnumerateAcceptedCommands(
            path,
            [](CommandId id, void * context) -> Loop {
                *reinterpret_cast<size_t *>(context) += 1;
                return Loop::Continue;
            },
            reinterpret_cast<void *>(&commandCount));

        if (err == CHIP_NO_ERROR)
        {
            using EnumerationData = struct
            {
                ConcreteCommandPath commandPath;
                DataModel::ListBuilder<DataModel::AcceptedCommandEntry> * acceptedCommandList;
                CHIP_ERROR processingError;
            };

            EnumerationData enumerationData;
            enumerationData.commandPath         = ConcreteCommandPath(path.mEndpointId, path.mClusterId, kInvalidCommandId);
            enumerationData.processingError     = CHIP_NO_ERROR;
            enumerationData.acceptedCommandList = &builder;

            ReturnErrorOnFailure(builder.EnsureAppendCapacity(commandCount));

            ReturnErrorOnFailure(interface->EnumerateAcceptedCommands(
                path,
                [](CommandId commandId, void * context) -> Loop {
                    auto input                    = reinterpret_cast<EnumerationData *>(context);
                    input->commandPath.mCommandId = commandId;
                    CHIP_ERROR appendError        = input->acceptedCommandList->Append(AcceptedCommandEntryFor(input->commandPath));
                    if (appendError != CHIP_NO_ERROR)
                    {
                        input->processingError = appendError;
                        return Loop::Break;
                    }
                    return Loop::Continue;
                },
                reinterpret_cast<void *>(&enumerationData)));
            ReturnErrorOnFailure(enumerationData.processingError);

            // the two invocations MUST return the same sizes.
            VerifyOrReturnError(builder.Size() == commandCount, CHIP_ERROR_INTERNAL);
            return CHIP_NO_ERROR;
        }
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

CHIP_ERROR CodegenServerCluster::GeneratedCommands(const ConcreteClusterPath & path, DataModel::ListBuilder<CommandId> & builder)
{
    // Some CommandHandlerInterface instances are registered of ALL endpoints, so make sure first that
    // the cluster actually exists on this endpoint before asking the CommandHandlerInterface what commands
    // it claims to support.
    const EmberAfCluster * serverCluster = emberAfFindServerCluster(path.mEndpointId, path.mClusterId);
    VerifyOrReturnError(serverCluster != nullptr, CHIP_ERROR_NOT_FOUND);

    CommandHandlerInterface * interface =
        CommandHandlerInterfaceRegistry::Instance().GetCommandHandler(path.mEndpointId, path.mClusterId);
    if (interface != nullptr)
    {
        size_t commandCount = 0;

        CHIP_ERROR err = interface->EnumerateGeneratedCommands(
            path,
            [](CommandId id, void * context) -> Loop {
                *reinterpret_cast<size_t *>(context) += 1;
                return Loop::Continue;
            },
            reinterpret_cast<void *>(&commandCount));

        if (err == CHIP_NO_ERROR)
        {
            ReturnErrorOnFailure(builder.EnsureAppendCapacity(commandCount));

            using EnumerationData = struct
            {
                DataModel::ListBuilder<CommandId> * generatedCommandList;
                CHIP_ERROR processingError;
            };
            EnumerationData enumerationData;
            enumerationData.processingError      = CHIP_NO_ERROR;
            enumerationData.generatedCommandList = &builder;

            ReturnErrorOnFailure(interface->EnumerateGeneratedCommands(
                path,
                [](CommandId id, void * context) -> Loop {
                    auto input = reinterpret_cast<EnumerationData *>(context);

                    CHIP_ERROR appendError = input->generatedCommandList->Append(id);
                    if (appendError != CHIP_NO_ERROR)
                    {
                        input->processingError = appendError;
                        return Loop::Break;
                    }
                    return Loop::Continue;
                },
                reinterpret_cast<void *>(&enumerationData)));
            ReturnErrorOnFailure(enumerationData.processingError);

            // the two invocations MUST return the same sizes.
            VerifyOrReturnError(builder.Size() == commandCount, CHIP_ERROR_INTERNAL);
            return CHIP_NO_ERROR;
        }
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

void CodegenServerCluster::ListAttributeWriteNotification(const ConcreteAttributePath & aPath, DataModel::ListWriteOperation opType)
{
    AttributeAccessInterface * aai = AttributeAccessInterfaceRegistry::Instance().Get(aPath.mEndpointId, aPath.mClusterId);

    if (aai != nullptr)
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

} // namespace app
} // namespace chip

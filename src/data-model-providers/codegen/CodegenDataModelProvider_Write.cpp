/*
 *    Copyright (c) 2024 Project CHIP Authors
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
#include <data-model-providers/codegen/CodegenDataModelProvider.h>

#include <access/AccessControl.h>
#include <app-common/zap-generated/attribute-type.h>
#include <app/AttributeAccessInterface.h>
#include <app/AttributeAccessInterfaceRegistry.h>
#include <app/GlobalAttributes.h>
#include <app/RequiredPrivilege.h>
#include <app/data-model-provider/ProviderChangeListener.h>
#include <app/data-model/FabricScoped.h>
#include <app/reporting/reporting.h>
#include <app/util/af-types.h>
#include <app/util/attribute-metadata.h>
#include <app/util/attribute-storage-detail.h>
#include <app/util/attribute-storage-null-handling.h>
#include <app/util/attribute-storage.h>
#include <app/util/attribute-table-detail.h>
#include <app/util/attribute-table.h>
#include <app/util/ember-io-storage.h>
#include <app/util/ember-strings.h>
#include <app/util/odd-sized-integers.h>
#include <data-model-providers/codegen/EmberAttributeDataBuffer.h>
#include <lib/core/CHIPError.h>
#include <lib/support/CodeUtils.h>

#include <zap-generated/endpoint_config.h>

namespace chip {
namespace app {
namespace {

using namespace chip::app::Compatibility::Internal;
using Protocols::InteractionModel::Status;

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

} // namespace

DataModel::ActionReturnStatus CodegenDataModelProvider::WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                                       AttributeValueDecoder & decoder)
{
    // we must be started up to accept writes (we make use of the context below)
    VerifyOrReturnError(mContext.has_value(), CHIP_ERROR_INCORRECT_STATE);

    const EmberAfAttributeMetadata * attributeMetadata =
        emberAfLocateAttributeMetadata(request.path.mEndpointId, request.path.mClusterId, request.path.mAttributeId);

    if (attributeMetadata != nullptr)
    {
        // AAI is only allowed on ember-attributes
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
                                        &mContext->dataModelChangeListener);
            }
            return *aai_result;
        }
    }

    // If ServerClusterInterface is available, it provides the final answer
    if (auto * cluster = mRegistry.Get(request.path); cluster != nullptr)
    {
        return cluster->WriteAttribute(request, decoder);
    }

    // WriteAttribute requirement is that request.path is a VALID path inside the provider
    // metadata tree. Clients are supposed to validate this (and data version and other flags)
    // This SHOULD NEVER HAPPEN hence the general return code (seemed preferable to VerifyOrDie)
    VerifyOrReturnError(attributeMetadata != nullptr, Status::Failure);

    MutableByteSpan dataBuffer = gEmberAttributeIOBufferSpan;
    {
        Ember::EmberAttributeDataBuffer emberData(attributeMetadata, dataBuffer);
        ReturnErrorOnFailure(decoder.Decode(emberData));
    }

    Protocols::InteractionModel::Status status;

    if (dataBuffer.size() > attributeMetadata->size)
    {
        ChipLogDetail(Zcl, "Data to write exceeds the attribute size claimed.");
        return Status::InvalidValue;
    }

    EmberAfWriteDataInput dataInput(dataBuffer.data(), attributeMetadata->attributeType);

    dataInput.SetChangeListener(&mContext->dataModelChangeListener);
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

void CodegenDataModelProvider::ListAttributeWriteNotification(const ConcreteAttributePath & aPath,
                                                              DataModel::ListWriteOperation opType, FabricIndex accessingFabric)
{
    // NOTE: for backwards compatibility, we process AAI logic BEFORE Server Cluster Interface
    //       so that AttributeAccessInterface logic works if one was installed before Server Cluster Interface
    //       support was introduced in the SDK.
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

        // We fall through here and will notify any ServerClusterInterface as well.
        // This is NOT ideal because AAI may or may not fully intercept the write,
        // So we do not know which of the ::Write behavior AAI uses:
        //   - write succeeds (so SCI should not be notified)
        //   - AAI falls-through (so SCI should process the request)
        //
        // for now we err on the side of notifying both.
    }

    if (auto * cluster = mRegistry.Get(aPath); cluster != nullptr)
    {
        cluster->ListAttributeWriteNotification(aPath, opType, accessingFabric);
        return;
    }
}

void CodegenDataModelProvider::Temporary_ReportAttributeChanged(const AttributePathParams & path)
{
    // we must be started up to process changes since we use the context
    VerifyOrReturn(mContext.has_value());

    if (path.mClusterId != kInvalidClusterId)
    {
        emberAfAttributeChanged(path.mEndpointId, path.mClusterId, path.mAttributeId, &mContext->dataModelChangeListener);
    }
    else
    {
        // When the path has wildcard cluster Id, call the emberAfEndpointChanged to mark attributes on the given endpoint
        // as having changing, but do NOT increase/alter any cluster data versions, as this happens when a bridged endpoint is
        // added or removed from a bridge and the cluster data is not changed during the process.
        emberAfEndpointChanged(path.mEndpointId, &mContext->dataModelChangeListener);
    }
}

} // namespace app
} // namespace chip

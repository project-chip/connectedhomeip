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
#include <data-model-providers/codegen/EmberMetadata.h>
#include <lib/core/CHIPError.h>
#include <lib/support/CodeUtils.h>

#include <zap-generated/endpoint_config.h>

namespace chip {
namespace app {
namespace {

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
    if (auto * cluster = mRegistry.Get(request.path); cluster != nullptr)
    {
        return cluster->WriteAttribute(request, decoder);
    }

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

    ContextAttributesChangeListener change_listener(CurrentContext());

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

    Protocols::InteractionModel::Status status;

    if (dataBuffer.size() > (*attributeMetadata)->size)
    {
        ChipLogDetail(Zcl, "Data to write exceeds the attribute size claimed.");
        return Status::InvalidValue;
    }

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

void CodegenDataModelProvider::ListAttributeWriteNotification(const ConcreteAttributePath & aPath,
                                                              DataModel::ListWriteOperation opType)
{
    if (auto * cluster = mRegistry.Get(aPath); cluster != nullptr)
    {
        cluster->ListAttributeWriteNotification(aPath, opType);
        return;
    }

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

void CodegenDataModelProvider::Temporary_ReportAttributeChanged(const AttributePathParams & path)
{
    ContextAttributesChangeListener change_listener(CurrentContext());
    if (path.mClusterId != kInvalidClusterId)
    {
        emberAfAttributeChanged(path.mEndpointId, path.mClusterId, path.mAttributeId, &change_listener);
    }
    else
    {
        // When the path has wildcard cluster Id, call the emberAfEndpointChanged to mark attributes on the given endpoint
        // as having changing, but do NOT increase/alter any cluster data versions, as this happens when a bridged endpoint is
        // added or removed from a bridge and the cluster data is not changed during the process.
        emberAfEndpointChanged(path.mEndpointId, &change_listener);
    }
}

} // namespace app
} // namespace chip

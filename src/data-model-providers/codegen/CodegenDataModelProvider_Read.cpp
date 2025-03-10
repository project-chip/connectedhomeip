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

#include <optional>
#include <variant>

#include <access/AccessControl.h>
#include <access/Privilege.h>
#include <access/RequestPath.h>
#include <app-common/zap-generated/attribute-type.h>
#include <app/AttributeAccessInterface.h>
#include <app/AttributeAccessInterfaceRegistry.h>
#include <app/AttributeValueEncoder.h>
#include <app/RequiredPrivilege.h>
#include <app/data-model/FabricScoped.h>
#include <app/util/af-types.h>
#include <app/util/attribute-metadata.h>
#include <app/util/attribute-storage-detail.h>
#include <app/util/attribute-storage-null-handling.h>
#include <app/util/attribute-storage.h>
#include <app/util/ember-io-storage.h>
#include <app/util/endpoint-config-api.h>
#include <app/util/odd-sized-integers.h>
#include <data-model-providers/codegen/EmberAttributeDataBuffer.h>
#include <data-model-providers/codegen/EmberMetadata.h>
#include <lib/core/CHIPError.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/Span.h>

#include <zap-generated/endpoint_config.h>

namespace chip {
namespace app {

namespace {

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

} // namespace

/// separated-out ReadAttribute implementation (given existing complexity)
///
/// Generally will:
///    - validate ACL (only for non-internal requests)
///    - Try to read attribute via the AttributeAccessInterface
///    - Try to read the value from ember RAM storage
DataModel::ActionReturnStatus CodegenDataModelProvider::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                                      AttributeValueEncoder & encoder)
{
    ChipLogDetail(DataManagement,
                  "Reading attribute: Cluster=" ChipLogFormatMEI " Endpoint=0x%x AttributeId=" ChipLogFormatMEI " (expanded=%d)",
                  ChipLogValueMEI(request.path.mClusterId), request.path.mEndpointId, ChipLogValueMEI(request.path.mAttributeId),
                  request.path.mExpanded);

    if (auto * cluster = mRegistry.Get(request.path); cluster != nullptr)
    {
        return cluster->ReadAttribute(request, encoder);
    }

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

} // namespace app
} // namespace chip

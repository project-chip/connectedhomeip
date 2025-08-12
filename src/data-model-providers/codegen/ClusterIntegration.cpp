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
#include <data-model-providers/codegen/ClusterIntegration.h>

#include <app/util/attribute-storage-null-handling.h>
#include <app/util/attribute-storage.h>
#include <app/util/attribute-table.h>
#include <app/util/endpoint-config-api.h>
#include <data-model-providers/codegen/CodegenDataModelProvider.h>

namespace chip::app {

namespace {

bool findEndpointWithLog(EndpointId endpointId, ClusterId clusterId, uint16_t fixedClusterServerEndpointCount,
                         uint16_t maxEndpointCount, uint16_t & zeroBasedArrayIndex)
{
    zeroBasedArrayIndex = emberAfGetClusterServerEndpointIndex(endpointId, clusterId, fixedClusterServerEndpointCount);

    if (zeroBasedArrayIndex >= maxEndpointCount)
    {
        ChipLogError(AppServer,
                     "Could not find a valid endpoint index for endpoint %u/" ChipLogFormatMEI " (Index %u was not valid)",
                     endpointId, ChipLogValueMEI(clusterId), zeroBasedArrayIndex);
        return false;
    }
    return true;
}

/// Fetch the featuremap from ember for the given endpoint/cluster
///
/// Input feature map is unchanged if load fails.
void LoadFeatureMap(EndpointId endpointId, ClusterId clusterId, uint32_t & featureMap)
{
    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp;
    uint8_t * readable = Traits::ToAttributeStoreRepresentation(temp);
    Protocols::InteractionModel::Status status =
        emberAfReadAttribute(endpointId, clusterId, Clusters::Globals::Attributes::FeatureMap::Id, readable, sizeof(temp));
    if (status != Protocols::InteractionModel::Status::Success)
    {
        ChipLogError(AppServer, "Failed to load featuremap for %u/" ChipLogFormatMEI " (Status %d)", endpointId,
                     ChipLogValueMEI(clusterId), static_cast<int>(status));
        return;
    }
    // note: we do not try to check if value is representable: all the uint32_t values are representable
    featureMap = Traits::StorageToWorking(temp);
}

} // namespace

void CodegenClusterIntegration::RegisterServer(const RegisterServerOptions & options, Delegate & delegate)
{
    uint16_t zeroBasedArrayIndex;
    if (!findEndpointWithLog(options.endpointId, options.clusterId, options.fixedClusterServerEndpointCount,
                             options.maxEndpointCount, zeroBasedArrayIndex))
    {
        return;
    }

    uint32_t featureMap = 0;
    if (options.fetchFeatureMap)
    {
        LoadFeatureMap(options.endpointId, options.clusterId, featureMap);
    }

    uint32_t optionalAttributes = 0;
    if (options.fetchOptionalAttributes)
    {
        for (AttributeId attributeId = 0; attributeId < 32; attributeId++)
        {
            if (emberAfContainsAttribute(options.endpointId, options.clusterId, attributeId))
            {
                optionalAttributes |= 1u << attributeId;
            }
        }
    }

    CHIP_ERROR err = CodegenDataModelProvider::Instance().Registry().Register(
        delegate.CreateRegistration(options.endpointId, zeroBasedArrayIndex, optionalAttributes, featureMap));

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Failed to register cluster %u/" ChipLogFormatMEI ":   %" CHIP_ERROR_FORMAT, options.endpointId,
                     ChipLogValueMEI(options.clusterId), err.Format());
    }
}

void CodegenClusterIntegration::UnregisterServer(const UnregisterServerOptions & options, Delegate & delegate)
{
    uint16_t zeroBasedArrayIndex;
    if (!findEndpointWithLog(options.endpointId, options.clusterId, options.fixedClusterServerEndpointCount,
                             options.maxEndpointCount, zeroBasedArrayIndex))
    {
        return;
    }

    CHIP_ERROR err = CodegenDataModelProvider::Instance().Registry().Unregister(&delegate.FindRegistration(zeroBasedArrayIndex));
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Failed to unregister cluster %u/" ChipLogFormatMEI ":   %" CHIP_ERROR_FORMAT, options.endpointId,
                     ChipLogValueMEI(options.clusterId), err.Format());
        // NOTE: there is no sane way to handle this failure:
        //   - returning here means we never free resources and a future registration will fail
        //   - not returning (as we do now) will free resources but it is unclear why unregistration failed (is it still in use?)
        //
        // For now assume that unregistration failed due to "already missing" so it is safe to delete
        // however this should never happen in practice.
    }

    delegate.DestroyRegistration(zeroBasedArrayIndex);
}

} // namespace chip::app

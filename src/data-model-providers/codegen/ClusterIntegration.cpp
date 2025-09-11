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
#include <data-model-providers/codegen/CodegenProcessingConfig.h>

#include <limits>

namespace chip::app {

namespace {

bool findEndpointWithLog(EndpointId endpointId, ClusterId clusterId, uint16_t fixedClusterServerEndpointCount,
                         uint16_t maxEndpointCount, uint16_t & emberEndpointIndex)
{
    emberEndpointIndex = emberAfGetClusterServerEndpointIndex(endpointId, clusterId, fixedClusterServerEndpointCount);

    if (emberEndpointIndex >= maxEndpointCount)
    {
#if CHIP_CODEGEN_CONFIG_ENABLE_CODEGEN_INTEGRATION_LOOKUP_ERRORS
        ChipLogError(AppServer,
                     "Could not find a valid endpoint index for endpoint %u/" ChipLogFormatMEI " (Index %u was not valid)",
                     endpointId, ChipLogValueMEI(clusterId), emberEndpointIndex);
#endif // CHIP_CODEGEN_CONFIG_ENABLE_CODEGEN_INTEGRATION_LOOKUP_ERRORS
        return false;
    }
    return true;
}

/// Fetch the featuremap from ember for the given endpoint/cluster
///
/// on error 0 is returned
uint32_t LoadFeatureMap(EndpointId endpointId, ClusterId clusterId)
{
    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp;
    uint8_t * readable = Traits::ToAttributeStoreRepresentation(temp);
    Protocols::InteractionModel::Status status =
        emberAfReadAttribute(endpointId, clusterId, Clusters::Globals::Attributes::FeatureMap::Id, readable, sizeof(temp));
    if (status != Protocols::InteractionModel::Status::Success)
    {
#if CHIP_CODEGEN_CONFIG_ENABLE_CODEGEN_INTEGRATION_LOOKUP_ERRORS
        ChipLogError(AppServer, "Failed to load feature map for %u/" ChipLogFormatMEI " (Status %d)", endpointId,
                     ChipLogValueMEI(clusterId), static_cast<int>(status));
#endif // CHIP_CODEGEN_CONFIG_ENABLE_CODEGEN_INTEGRATION_LOOKUP_ERRORS
        return 0;
    }
    // note: we do not try to check if value is representable: all the uint32_t values are representable
    return Traits::StorageToWorking(temp);
}

} // namespace

void CodegenClusterIntegration::RegisterServer(const RegisterServerOptions & options, Delegate & delegate)
{
    uint16_t emberEndpointIndex;
    if (!findEndpointWithLog(options.endpointId, options.clusterId, options.fixedClusterServerEndpointCount,
                             options.maxEndpointCount, emberEndpointIndex))
    {
        return;
    }

    uint32_t featureMap = 0;
    if (options.fetchFeatureMap)
    {
        featureMap = LoadFeatureMap(options.endpointId, options.clusterId);
    }

    // NOTE: we fetch low ID attributes only here as a convenience/speedup for the very frequent cluster case
    //       where attributes are few and with low IDS.
    //
    // This is NOT a general rule. Specific examples are:
    //   - LevelControl::StartupCurrentLevel has ID 0x4000
    //   - OnOff has several: GlobalSceneControl, OnTime, OffWaitTime, StartupOnOff with id >= 0x4000
    //   - Thermostat and DoorLock have more than 32 attributes in general
    //   - ColorControl has a lot of high-ID attributes
    //
    // The above examples however are few compared to the large number of clusters that matter supports,
    // so this optimization is considered worth it at this time.
    uint32_t optionalAttributes = 0;
    if (options.fetchOptionalAttributes)
    {
        for (AttributeId attributeId = 0; attributeId < std::numeric_limits<uint32_t>::digits; attributeId++)
        {
            if (emberAfContainsAttribute(options.endpointId, options.clusterId, attributeId))
            {
                optionalAttributes |= 1u << attributeId;
            }
        }
    }

    CHIP_ERROR err = CodegenDataModelProvider::Instance().Registry().Register(
        delegate.CreateRegistration(options.endpointId, emberEndpointIndex, optionalAttributes, featureMap));

    if (err != CHIP_NO_ERROR)
    {
#if CHIP_CODEGEN_CONFIG_ENABLE_CODEGEN_INTEGRATION_LOOKUP_ERRORS
        ChipLogError(AppServer, "Failed to register cluster %u/" ChipLogFormatMEI ":   %" CHIP_ERROR_FORMAT, options.endpointId,
                     ChipLogValueMEI(options.clusterId), err.Format());
#endif // CHIP_CODEGEN_CONFIG_ENABLE_CODEGEN_INTEGRATION_LOOKUP_ERRORS
    }
}

void CodegenClusterIntegration::UnregisterServer(const UnregisterServerOptions & options, Delegate & delegate)
{
    uint16_t emberEndpointIndex;
    if (!findEndpointWithLog(options.endpointId, options.clusterId, options.fixedClusterServerEndpointCount,
                             options.maxEndpointCount, emberEndpointIndex))
    {
        return;
    }

    CHIP_ERROR err = CodegenDataModelProvider::Instance().Registry().Unregister(&delegate.FindRegistration(emberEndpointIndex));
    if (err != CHIP_NO_ERROR)
    {
#if CHIP_CODEGEN_CONFIG_ENABLE_CODEGEN_INTEGRATION_LOOKUP_ERRORS
        ChipLogError(AppServer, "Failed to unregister cluster %u/" ChipLogFormatMEI ":   %" CHIP_ERROR_FORMAT, options.endpointId,
                     ChipLogValueMEI(options.clusterId), err.Format());
#endif // CHIP_CODEGEN_CONFIG_ENABLE_CODEGEN_INTEGRATION_LOOKUP_ERRORS
       // NOTE: There is no sane way to handle this failure:
       //   - Returning here means we never free resources and a future registration will fail.
       //   - Not returning (as we do now) will free resources, but it is unclear why unregistration failed (is it still in use?).
       //
       // For now, assume that unregistration failed due to "already missing", so it is safe to delete.
       // However, this should never happen in practice.
    }

    delegate.ReleaseRegistration(emberEndpointIndex);
}

ServerClusterInterface * CodegenClusterIntegration::GetClusterForEndpointIndex(const GetClusterForEndpointIndexOptions & options,
                                                                               Delegate & delegate)
{
    uint16_t emberEndpointIndex;
    if (!findEndpointWithLog(options.endpointId, options.clusterId, options.fixedClusterServerEndpointCount,
                             options.maxEndpointCount, emberEndpointIndex))
    {
        return nullptr;
    }

    return &delegate.FindRegistration(emberEndpointIndex);
}

} // namespace chip::app

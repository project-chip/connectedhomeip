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
#include <app/clusters/ota-provider/ota-provider-cluster.h>
#include <app/static-cluster-config/OtaSoftwareUpdateProvider.h>
#include <app/util/attribute-storage.h>
#include <data-model-providers/codegen/CodegenDataModelProvider.h>

#include <array>
#include <cstdint>

using namespace chip::app;
using namespace chip::app::Clusters;

namespace {

static constexpr size_t kOtaProviderMaxClusterCount =
    OtaSoftwareUpdateProvider::StaticApplicationConfig::kFixedClusterConfig.size() + CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT;

OtaProviderServer mClusters[kOtaProviderMaxClusterCount];
ServerClusterRegistration mRegistrations[kOtaProviderMaxClusterCount];

// Find the 0-based array index corresponding to the given endpoint id.
// Log an error if not found.
bool findEndpoint(chip::EndpointId endpointId, uint16_t & outArrayIndex)
{
    uint16_t arrayIndex = emberAfGetClusterServerEndpointIndex(
        endpointId, OtaSoftwareUpdateProvider::Id, MATTER_DM_OTA_SOFTWARE_UPDATE_PROVIDER_CLUSTER_SERVER_ENDPOINT_COUNT);

    if (arrayIndex >= kOtaProviderMaxClusterCount)
    {
        ChipLogError(AppServer, "Cound not find endpoint index for endpoint %u", endpointId);
        return false;
    }
    return true;
}

} // namespace

void emberAfOtaSoftwareUpdateProviderClusterInitCallback(chip::EndpointId endpointId)
{
    uint16_t arrayIndex = 0;
    if (!findEndpoint(endpointId, arrayIndex))
    {
        return;
    }

    mClusters[arrayIndex].SetEndpointId(endpointId);
    CHIP_ERROR err = CodegenDataModelProvider::Instance().Registry().Register(mRegistrations[arrayIndex]);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Failed to register OTA on endpoint %u: %" CHIP_ERROR_FORMAT, endpointId, err.Format());
    }
}

void emberAfOtaSoftwareUpdateProviderClusterShutdownCallback(chip::EndpointId endpointId)
{
    uint16_t arrayIndex = 0;
    if (!findEndpoint(endpointId, arrayIndex))
    {
        return;
    }

    CHIP_ERROR err = CodegenDataModelProvider::Instance().Registry().Unregister(&mClusters[arrayIndex]);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Failed to unregister OTA on endpoint %u: %" CHIP_ERROR_FORMAT, endpointId, err.Format());
    }
}

void MatterOtaSoftwareUpdateProviderPluginServerInitCallback()
{
    for (unsigned i = 0; i < kOtaProviderMaxClusterCount; i++)
    {
        mRegistrations[i].serverClusterInterface = &mClusters[i];
    }
}

void MatterOtaSoftwareUpdateProviderPluginServerShutdownCallback() {}

namespace chip {
namespace app {
namespace Clusters {
namespace OTAProvider {

void SetDelegate(chip::EndpointId endpointId, OTAProviderDelegate * delegate)
{
    uint16_t arrayIndex = 0;
    if (!findEndpoint(endpointId, arrayIndex))
    {
        return;
    }
    mClusters[arrayIndex].SetDelegate(delegate);
}

} // namespace OTAProvider
} // namespace Clusters
} // namespace app
} // namespace chip

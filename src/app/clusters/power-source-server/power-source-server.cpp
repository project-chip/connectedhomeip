/*
 *    Copyright (c) 2022 Project CHIP Authors
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

/****************************************************************************
 * @file
 * @brief Implementation for the Power Source Server Cluster
 ***************************************************************************/

#include "power-source-server.h"

#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app/util/attribute-storage.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip;
using namespace app;
using namespace app::Clusters;
using namespace app::Clusters::PowerSource::Attributes;

namespace {

PowerSourceServer gPowerSourceServer;

PowerSourceAttrAccess gAttrAccess;

} // anonymous namespace

void MatterPowerSourcePluginServerInitCallback()
{
    registerAttributeAccessOverride(&gAttrAccess);
}

namespace chip {
namespace app {
namespace Clusters {

CHIP_ERROR PowerSourceAttrAccess::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    switch (aPath.mAttributeId)
    {
    case ActiveBatFaults::Id:
        // TODO: Needs implementation.
        err = aEncoder.EncodeEmptyList();
        break;
    case EndpointList::Id: {
        PowerSourceServer & server    = PowerSourceServer::Instance();
        const Span<EndpointId> * span = server.GetEndpointList(aPath.mEndpointId);
        if (span == nullptr)
        {
            err = aEncoder.EncodeEmptyList();
        }
        else
        {
            DataModel::List<EndpointId> list(*span);
            err = aEncoder.Encode(list);
        }
        break;
    }
    default:
        break;
    }

    return err;
}

PowerSourceAttrAccess & TestOnlyGetPowerSourceAttrAccess()
{
    return gAttrAccess;
}

PowerSourceServer & PowerSourceServer::Instance()
{
    return gPowerSourceServer;
}

// Caller does not need to retain the span past the call point as these are copied into an internal storage
CHIP_ERROR PowerSourceServer::SetEndpointList(EndpointId powerSourceClusterEndpoint, Span<EndpointId> endpointList)
{
    // TODO: should check here that the power source cluster exists on the endpoint, but for now let's take the caller's word for it

    size_t idx = PowerSourceClusterEndpointIndex(powerSourceClusterEndpoint);
    if (idx == std::numeric_limits<size_t>::max())
    {
        idx = NextEmptyIndex();
    }
    if (idx == std::numeric_limits<size_t>::max())
    {
        return CHIP_ERROR_NO_MEMORY;
    }
    if (endpointList.size() == 0)
    {
        mPowerSourceClusterInfo[idx] = PowerSourceClusterInfo();
    }
    else
    {
        mPowerSourceClusterInfo[idx] = PowerSourceClusterInfo(powerSourceClusterEndpoint);
        mPowerSourceClusterInfo[idx].SetEndpointList(endpointList);
    }
    return CHIP_NO_ERROR;
}
const Span<EndpointId> * PowerSourceServer::GetEndpointList(EndpointId powerSourceClusterEndpoint) const
{
    size_t idx = PowerSourceClusterEndpointIndex(powerSourceClusterEndpoint);
    if (idx != std::numeric_limits<size_t>::max())
    {
        return &mPowerSourceClusterInfo[idx].mEndpointList;
    }
    return nullptr;
}

size_t PowerSourceServer::PowerSourceClusterEndpointIndex(EndpointId endpointId) const
{
    for (size_t i = 0; i < kNumSupportedEndpoints; ++i)
    {
        if (mPowerSourceClusterInfo[i].mClusterEndpoint == endpointId)
        {
            return i;
        }
    }
    return std::numeric_limits<size_t>::max();
}

size_t PowerSourceServer::NextEmptyIndex() const
{
    for (size_t i = 0; i < kNumSupportedEndpoints; ++i)
    {
        if (mPowerSourceClusterInfo[i].mClusterEndpoint == kInvalidEndpointId)
        {
            return i;
        }
    }
    return std::numeric_limits<size_t>::max();
}

} // namespace Clusters
} // namespace app
} // namespace chip

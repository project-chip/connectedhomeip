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
        PowerSourceServer & server = PowerSourceServer::Instance();
        // EndpointId id[2]           = { 1, 2 };
        // server.SetEndpointList(aPath.mEndpointId, Span<EndpointId>(id));
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

#if CONFIG_BUILD_FOR_HOST_UNIT_TEST
PowerSourceAttrAccess & GetPowerSourceAttrAccess()
{
    return gAttrAccess;
}
#endif

PowerSourceServer & PowerSourceServer::Instance()
{
    return gPowerSourceServer;
}

// Caller does not need to retain the span past the call point as these are copied into an internal storage
CHIP_ERROR PowerSourceServer::SetEndpointList(EndpointId powerSourceClusterEndpoint, Span<EndpointId> endpointList)
{
    // TODO: should check here that the power source cluster exists on the endpoint, but for now let's take the caller's word for it

    int idx = PowerSourceClusterEndpointIndex(powerSourceClusterEndpoint);
    if (idx == -1)
    {
        idx = NextEmptyIndex();
    }
    if (idx == -1)
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
    int idx = PowerSourceClusterEndpointIndex(powerSourceClusterEndpoint);
    if (idx != -1)
    {
        return &mPowerSourceClusterInfo[idx].mEndpointList;
    }
    return nullptr;
}

int PowerSourceServer::PowerSourceClusterEndpointIndex(EndpointId endpointId) const
{
    for (int i = 0; i < nSupportedEndpoints; ++i)
    {
        if (mPowerSourceClusterInfo[i].mClusterEndpoint == endpointId)
        {
            return i;
        }
    }
    return -1;
}

int PowerSourceServer::NextEmptyIndex() const
{
    for (int i = 0; i < nSupportedEndpoints; ++i)
    {
        if (mPowerSourceClusterInfo[i].mClusterEndpoint == kInvalidEndpointId)
        {
            return i;
        }
    }
    return -1;
}

} // namespace Clusters
} // namespace app
} // namespace chip

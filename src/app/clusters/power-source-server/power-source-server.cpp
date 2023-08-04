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
#include <zap-generated/gen_config.h>

using namespace chip;
using namespace app;
using namespace app::Clusters;
using namespace app::Clusters::PowerSource::Attributes;

namespace {

struct PowerSourceClusterInfo
{
    PowerSourceClusterInfo() : mClusterEndpoint(kInvalidEndpointId) {}
    explicit PowerSourceClusterInfo(EndpointId powerClusterEndpointId) : mClusterEndpoint(powerClusterEndpointId) {}
    void Clear()
    {
        mBuf.Free();
        mEndpointList = Span<EndpointId>();
    }
    CHIP_ERROR SetEndpointList(Span<EndpointId> endpointList)
    {
        Clear();
        if (endpointList.size() == 0)
        {
            mEndpointList = Span<EndpointId>();
            return CHIP_NO_ERROR;
        }
        mBuf.Calloc(endpointList.size());
        if (mBuf.Get() == nullptr)
        {
            return CHIP_ERROR_NO_MEMORY;
        }
        memcpy(mBuf.Get(), endpointList.data(), endpointList.size() * sizeof(EndpointId));
        mEndpointList = Span<EndpointId>(mBuf.Get(), endpointList.size());
        return CHIP_NO_ERROR;
    }
    EndpointId mClusterEndpoint = kInvalidEndpointId;
    Platform::ScopedMemoryBuffer<EndpointId> mBuf;
    Span<EndpointId> mEndpointList;
};

PowerSourceServer gPowerSourceServer;

PowerSourceAttrAccess gAttrAccess;

#ifdef ZCL_USING_POWER_SOURCE_CLUSTER_SERVER
#define POWER_SERVER_NUM_SUPPORTED_ENDPOINTS                                                                                       \
    (EMBER_AF_POWER_SOURCE_CLUSTER_SERVER_ENDPOINT_COUNT + CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT)
#else
#define POWER_SERVER_NUM_SUPPORTED_ENDPOINTS CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT
#endif
static constexpr size_t kNumSupportedEndpoints = POWER_SERVER_NUM_SUPPORTED_ENDPOINTS;

#if POWER_SERVER_NUM_SUPPORTED_ENDPOINTS > 0
PowerSourceClusterInfo sPowerSourceClusterInfo[kNumSupportedEndpoints] = {};
#else
PowerSourceClusterInfo * sPowerSourceClusterInfo = nullptr;
#endif

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
            err = aEncoder.EncodeList([span](const auto & encoder) -> CHIP_ERROR {
                for (auto id : *span)
                {
                    ReturnErrorOnFailure(encoder.Encode(id));
                }
                return CHIP_NO_ERROR;
            });
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
    // TODO: should check here that the power source cluster exists on the endpoint, but for now let's take the caller's word
    // for it

    size_t idx = PowerSourceClusterEndpointIndex(powerSourceClusterEndpoint);
    if (idx >= kNumSupportedEndpoints)
    {
        idx = NextEmptyIndex();
    }
    if (idx >= kNumSupportedEndpoints)
    {
        return CHIP_ERROR_NO_MEMORY;
    }

    sPowerSourceClusterInfo[idx].Clear();
    if (endpointList.size() == 0)
    {
        sPowerSourceClusterInfo[idx] = PowerSourceClusterInfo();
    }
    else
    {
        sPowerSourceClusterInfo[idx] = PowerSourceClusterInfo(powerSourceClusterEndpoint);
        sPowerSourceClusterInfo[idx].SetEndpointList(endpointList);
    }
    return CHIP_NO_ERROR;
}
const Span<EndpointId> * PowerSourceServer::GetEndpointList(EndpointId powerSourceClusterEndpoint) const
{
    size_t idx = PowerSourceClusterEndpointIndex(powerSourceClusterEndpoint);
    if (idx != std::numeric_limits<size_t>::max())
    {
        return &sPowerSourceClusterInfo[idx].mEndpointList;
    }
    return nullptr;
}

void PowerSourceServer::Shutdown()
{
    for (size_t i = 0; i < kNumSupportedEndpoints; ++i)
    {
        sPowerSourceClusterInfo[i].Clear();
    }
}

size_t PowerSourceServer::GetNumSupportedEndpointLists() const
{
    return kNumSupportedEndpoints;
}

size_t PowerSourceServer::PowerSourceClusterEndpointIndex(EndpointId endpointId) const
{
    for (size_t i = 0; i < kNumSupportedEndpoints; ++i)
    {
        if (sPowerSourceClusterInfo[i].mClusterEndpoint == endpointId)
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
        if (sPowerSourceClusterInfo[i].mClusterEndpoint == kInvalidEndpointId)
        {
            return i;
        }
    }
    return std::numeric_limits<size_t>::max();
}

} // namespace Clusters
} // namespace app
} // namespace chip

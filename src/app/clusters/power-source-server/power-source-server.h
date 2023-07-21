/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#pragma once

#include <app-common/zap-generated/ids/Clusters.h>
#include <app/AttributeAccessInterface.h>
#include <app/util/af-types.h>
#include <app/util/basic-types.h>
#include <lib/support/Span.h>
#include <platform/CHIPDeviceConfig.h>

namespace chip {
namespace app {
namespace Clusters {

struct PowerSourceClusterInfo
{
    PowerSourceClusterInfo() : mClusterEndpoint(kInvalidEndpointId) {}
    explicit PowerSourceClusterInfo(EndpointId powerClusterEndpointId) : mClusterEndpoint(powerClusterEndpointId) {}
    void Shutdown() {
      mEndpointList = Span<EndpointId>();
      mBuf.Free();
    }
    CHIP_ERROR SetEndpointList(Span<EndpointId> endpointList)
    {
        Shutdown();
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

class PowerSourceServer
{
public:
    static PowerSourceServer & Instance();

    // Caller does not need to retain the span past the call point as these are copied into an internal storage
    CHIP_ERROR SetEndpointList(EndpointId powerSourceClusterEndpoint, Span<EndpointId> endpointList);
    // returns nullptr if there's not endpoint list set for this power source cluster endpoint id.
    const Span<EndpointId> * GetEndpointList(EndpointId powerSourceClusterEndpoint) const;
    void Shutdown()
    {
        for (size_t i = 0; i < kNumSupportedEndpoints; ++i)
        {
            mPowerSourceClusterInfo[i].Shutdown();
        }
    }
    size_t GetNumSupportedEndpointLists() const { return kNumSupportedEndpoints; };

private:
    // There's probably a good way to properly set this from zap or have this grow dynamically as dynamic endpoints are
    // finalized, but for now, let's just say 10 power clusters is probably ok.
    // TODO: is there a way to get a static count of endpoints? CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT?
    static constexpr size_t kNumSupportedEndpoints                         = 10;
    PowerSourceClusterInfo mPowerSourceClusterInfo[kNumSupportedEndpoints] = {};
    // Both return std::numeric_limits<size_t>::max() for not found
    size_t PowerSourceClusterEndpointIndex(EndpointId endpointId) const;
    size_t NextEmptyIndex() const;
};

class PowerSourceAttrAccess : public AttributeAccessInterface
{
public:
    // Register on all endpoints.
    PowerSourceAttrAccess() : AttributeAccessInterface(Optional<EndpointId>::Missing(), PowerSource::Id) {}

    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;
};

PowerSourceAttrAccess & TestOnlyGetPowerSourceAttrAccess();

} // namespace Clusters
} // namespace app
} // namespace chip

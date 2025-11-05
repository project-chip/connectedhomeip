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
#include "SpanEndpoint.h"

#include <app/ConcreteClusterPath.h>
#include <app/server-cluster/ServerClusterContext.h>
#include <clusters/Descriptor/ClusterId.h>
#include <lib/core/CHIPError.h>
#include <lib/support/Span.h>
#include <lib/support/logging/CHIPLogging.h>

namespace chip {
namespace app {

// Builder implementation
SpanEndpoint::Builder & SpanEndpoint::Builder::SetClientClusters(Span<const ClusterId> clientClusters)
{
    mClientClusters = clientClusters;
    return *this;
}

SpanEndpoint::Builder & SpanEndpoint::Builder::SetDeviceTypes(Span<const DataModel::DeviceTypeEntry> deviceTypes)
{
    mDeviceTypes = deviceTypes;
    return *this;
}

#if CHIP_CONFIG_USE_ENDPOINT_UNIQUE_ID
SpanEndpoint::Builder & SpanEndpoint::Builder::SetEndpointUniqueId(CharSpan endpointUniqueId)
{
    mEndpointUniqueId = endpointUniqueId;
    return *this;
}
#endif

SpanEndpoint SpanEndpoint::Builder::Build()
{
#if CHIP_CONFIG_USE_ENDPOINT_UNIQUE_ID
    return SpanEndpoint(mClientClusters, mDeviceTypes, mEndpointUniqueId);
#else
    return SpanEndpoint(mClientClusters, mDeviceTypes);
#endif
}

CHIP_ERROR SpanEndpoint::DeviceTypes(ReadOnlyBufferBuilder<DataModel::DeviceTypeEntry> & out) const
{
    return out.ReferenceExisting(mDeviceTypes);
}

CHIP_ERROR SpanEndpoint::ClientClusters(ReadOnlyBufferBuilder<ClusterId> & out) const
{
    return out.ReferenceExisting(mClientClusters);
}

#if CHIP_CONFIG_USE_ENDPOINT_UNIQUE_ID
CharSpan SpanEndpoint::EndpointUniqueId() const
{
    return mEndpointUniqueId;
}
#endif

// Private constructor for Builder
#if CHIP_CONFIG_USE_ENDPOINT_UNIQUE_ID
SpanEndpoint::SpanEndpoint(const Span<const ClusterId> & clientClusters, const Span<const DataModel::DeviceTypeEntry> & deviceTypes,
                           const CharSpan & uniqueEndpointId) :
    mDeviceTypes(deviceTypes),
    mClientClusters(clientClusters), mEndpointUniqueId(uniqueEndpointId)
{}
#else
SpanEndpoint::SpanEndpoint(const Span<const ClusterId> & clientClusters,
                           const Span<const DataModel::DeviceTypeEntry> & deviceTypes) :
    mDeviceTypes(deviceTypes),
    mClientClusters(clientClusters)
{}
#endif

} // namespace app
} // namespace chip

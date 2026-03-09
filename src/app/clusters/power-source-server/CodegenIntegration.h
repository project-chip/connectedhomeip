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

#include "PowerSourceCluster.h"

#include <app/server-cluster/ServerClusterInterfaceRegistry.h>

namespace chip {
namespace app {
namespace Clusters {
namespace PowerSource {

/// Makes integration of a PowerSourceCluster with CodegenDataModelProvider easier.
///
/// This class provides a very thin wrapper around the PowerSourceCluster and its
/// registration within the CodegenDataModelProvider.
///

template <typename DelegateT, typename = std::enable_if_t<
    std::is_same_v<DelegateT, PowerSourceCluster::Wired> ||
    std::is_same_v<DelegateT, PowerSourceCluster::Battery>>>
class Instance
{
public:
    using OptionalAttributeSet = PowerSourceCluster::OptionalAttributeSet;

    PowerSourceCluster & Cluster() { return mCluster.Cluster(); }

    /// Registers the cluster within the CodegenDataModelProvider Registry
    CHIP_ERROR Register()
    {
        return CodegenDataModelProvider::Instance().Registry().Register(mCluster.Registration());
    }

    /// Unregisters the cluster from the CodegenDataModelProvider Registry
    void Unregister()
    {
        CodegenDataModelProvider::Instance().Registry().Unregister(&mCluster.Cluster());
    }

    Instance(EndpointId endpointId, Span<AttributeId> supportedOptionalAttributes, const DelegateT & delegate) : mCluster(endpointId, GetOptionalAttributeSet(supportedOptionalAttributes), mDelegate) {}

private:
    static OptionalAttributeSet GetOptionalAttributeSet(Span<AttributeId> supportedOptionalAttributes)
    {
        uint32_t optionalAttributeBits = 0;
        for (AttributeId attributeId : supportedOptionalAttributes)
        {
            optionalAttributeBits |= (1 << attributeId);
        }
        return OptionalAttributeSet(optionalAttributeBits);
    }
    RegisteredServerCluster<PowerSourceCluster> mCluster;
};

} // namespace PowerSource
} // namespace Clusters
} // namespace app
} // namespace chip

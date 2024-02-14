/*
 *
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
#pragma once

#include <lib/core/Optional.h>

#include <app-common/zap-generated/cluster-objects.h>
#include <app/AttributeAccessInterface.h>
#include <lib/support/CommonIterator.h>

namespace chip {
namespace app {
namespace Clusters {
namespace PowerTopology {

class Delegate
{
public:
    virtual ~Delegate() = default;

    /**
     * Get the Nth EndpointId in the list of available endpoints.
     * @param index The index of the EndpointId to be returned.
     * @param endpointId A reference to the EndpointId.
     * @return Returns a CHIP_NO_ERROR if there was no error and the EndpointId was returned successfully.
     * CHIP_ERROR_PROVIDER_LIST_EXHAUSTED if the index is greater than or equal to the length of the list of available endpoints.
     */
    virtual CHIP_ERROR GetAvailableEndpointAtIndex(size_t index, EndpointId & endpointId) = 0;

    /**
     * Get the Nth EndpointId in the list of active endpoints.
     * @param index The index of the EndpointId to be returned.
     * @param endpointId A reference to the EndpointId.
     * @return Returns a CHIP_NO_ERROR if there was no error and the EndpointId was returned successfully.
     * CHIP_ERROR_PROVIDER_LIST_EXHAUSTED if the index is greater than or equal to the length of the list of active endpoints.
     */
    virtual CHIP_ERROR GetActiveEndpointAtIndex(size_t index, EndpointId & endpointId) = 0;
};

enum class OptionalAttributes : uint32_t
{
    kOptionalAttributeAvailableEndpoints = 0x1,
    kOptionalAttributeActiveEndpoints    = 0x2,
};

class Instance : public AttributeAccessInterface
{
public:
    Instance(EndpointId aEndpointId, Delegate & aDelegate, BitMask<Feature> aFeature,
             BitMask<OptionalAttributes> aOptionalAttributes) :
        AttributeAccessInterface(MakeOptional(aEndpointId), Id),
        mDelegate(aDelegate), mFeature(aFeature), mOptionalAttrs(aOptionalAttributes)
    {}
    ~Instance() { Shutdown(); }

    CHIP_ERROR Init();
    void Shutdown();

    bool HasFeature(Feature aFeature) const;
    bool SupportsOptAttr(OptionalAttributes aOptionalAttrs) const;

private:
    Delegate & mDelegate;
    BitMask<Feature> mFeature;
    BitMask<OptionalAttributes> mOptionalAttrs;

    // AttributeAccessInterface
    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR ReadAvailableEndpoints(AttributeValueEncoder & aEncoder);
    CHIP_ERROR ReadActiveEndpoints(AttributeValueEncoder & aEncoder);
};

} // namespace PowerTopology
} // namespace Clusters
} // namespace app
} // namespace chip

/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <app/util/basic-types.h>
#include <lib/support/BitFlags.h>

namespace chip {
namespace app {
struct AttributePathParams
{
    enum class Flags : uint8_t
    {
        kFieldIdValid   = 0x01,
        kListIndexValid = 0x02,
    };

    //
    // TODO: (Issue #10596) Need to ensure that we do not encode the NodeId over the wire
    // if it is either not 'set', or is set to a value that matches accessing fabric
    // on which the interaction is undertaken.
    //
    AttributePathParams(EndpointId aEndpointId, ClusterId aClusterId) : AttributePathParams(0, aEndpointId, aClusterId, 0, 0, {}) {}

    AttributePathParams(EndpointId aEndpointId, ClusterId aClusterId, AttributeId aFieldId) :
        AttributePathParams(0, aEndpointId, aClusterId, aFieldId, 0, chip::app::AttributePathParams::Flags::kFieldIdValid)
    {}

    AttributePathParams(EndpointId aEndpointId, ClusterId aClusterId, AttributeId aFieldId, ListIndex aListIndex) :
        AttributePathParams(0, aEndpointId, aClusterId, aFieldId, aListIndex,
                            BitFlags<Flags>(chip::app::AttributePathParams::Flags::kFieldIdValid,
                                            chip::app::AttributePathParams::Flags::kListIndexValid))
    {}

    AttributePathParams(NodeId aNodeId, EndpointId aEndpointId, ClusterId aClusterId, AttributeId aFieldId, ListIndex aListIndex,
                        const BitFlags<Flags> aFlags) :
        mNodeId(aNodeId),
        mEndpointId(aEndpointId), mClusterId(aClusterId), mAttributeId(aFieldId), mListIndex(aListIndex), mFlags(aFlags)
    {}

    AttributePathParams() {}
    NodeId mNodeId           = 0;
    EndpointId mEndpointId   = 0;
    ClusterId mClusterId     = 0;
    AttributeId mAttributeId = 0;
    ListIndex mListIndex     = 0;
    BitFlags<Flags> mFlags;
};
} // namespace app
} // namespace chip

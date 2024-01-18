/**
 *
 *    Copyright (c) 2021 Project CHIP Authors
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
/**
 *    @file
 *      This file defines EventFilterIB parser and builder in CHIP interaction model
 *
 */

#pragma once

#include "StructBuilder.h"
#include "StructParser.h"

#include <app/AppConfig.h>
#include <app/util/basic-types.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/NodeId.h>
#include <lib/core/TLV.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

namespace chip {
namespace app {
namespace EventFilterIB {
enum class Tag : uint8_t
{
    kNode     = 0,
    kEventMin = 1,
};

class Parser : public StructParser
{
public:
#if CHIP_CONFIG_IM_PRETTY_PRINT
    CHIP_ERROR PrettyPrint() const;
#endif // CHIP_CONFIG_IM_PRETTY_PRINT

    /**
     *  @brief Get a TLVReader for the NodeId. Next() must be called before accessing them.
     *
     *  @param [in] apNodeId    A pointer to apNodeId
     *
     *  @return #CHIP_NO_ERROR on success
     *          #CHIP_ERROR_WRONG_TLV_TYPE if there is such element but it's not any of the defined unsigned integer types
     *          #CHIP_END_OF_TLV if there is no such element
     */
    CHIP_ERROR GetNode(NodeId * const apNodeId) const;

    /**
     *  @brief Get a TLVReader for the ClusterId. Next() must be called before accessing them.
     *
     *  @param [in] apEventMin    A pointer to EventMin
     *
     *  @return #CHIP_NO_ERROR on success
     *          #CHIP_ERROR_WRONG_TLV_TYPE if there is such element but it's not any of the defined unsigned integer types
     *          #CHIP_END_OF_TLV if there is no such element
     */
    CHIP_ERROR GetEventMin(uint64_t * const apEventMin) const;
};

class Builder : public StructBuilder
{
public:
    /**
     *  @brief Inject Node into the TLV stream to indicate the nodeId referenced by the event filter.
     *
     *  @param [in] aNodeId refer to the NodeId
     *
     *  @return A reference to *this
     */
    EventFilterIB::Builder & Node(const NodeId aNodeId);

    /**
     *  @brief Inject EventId into the TLV stream.
     *
     *  @param [in] aEventMin EventMin for this event filter
     *
     *  @return A reference to *this
     */
    EventFilterIB::Builder & EventMin(const uint64_t aEventMin);

    /**
     *  @brief Mark the end of this EventFilterIB
     *
     *  @return The builder's final status.
     */
    CHIP_ERROR EndOfEventFilterIB();
};
}; // namespace EventFilterIB
}; // namespace app
}; // namespace chip

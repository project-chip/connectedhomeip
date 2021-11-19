/**
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2016-2017 Nest Labs, Inc.
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

#include "AttributePathIBs.h"
#include "EventFilters.h"
#include "EventPaths.h"

#include <app/AppBuildConfig.h>
#include <app/util/basic-types.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/CHIPTLV.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

namespace chip {
namespace app {
namespace ReadRequestMessage {
enum class Tag : uint8_t
{
    kAttributeRequests  = 0,
    kEventRequests      = 1,
    kDataVersionFilters = 2,
    kEventFilters       = 3,
    kIsFabricFiltered   = 4,
};

class Parser : public StructParser
{
public:
#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
    /**
     *  @brief Roughly verify the message is correctly formed
     *   1) all mandatory tags are present
     *   2) all elements have expected data type
     *   3) any tag can only appear once
     *   4) At the top level of the structure, unknown tags are ignored for forward compatibility
     *  @note The main use of this function is to print out what we're
     *    receiving during protocol development and debugging.
     *    The encoding rule has changed in IM encoding spec so this
     *    check is only "roughly" conformant now.
     *
     *  @return #CHIP_NO_ERROR on success
     */
    CHIP_ERROR CheckSchemaValidity() const;
#endif

    /**
     *  @brief Get a TLVReader for the AttributePathIBs. Next() must be called before accessing them.
     *
     *  @param [in] apAttributeRequests    A pointer to an attribute path list parser.
     *
     *  @return #CHIP_NO_ERROR on success
     *          #CHIP_END_OF_TLV if there is no such element
     */
    CHIP_ERROR GetAttributeRequests(AttributePathIBs::Parser * const apAttributeRequests) const;

    /**
     *  @brief Get a TLVReader for the EventRequests. Next() must be called before accessing them.
     *
     *  @param [in] apEventPaths    A pointer to apEventPaths
     *
     *  @return #CHIP_NO_ERROR on success
     *          #CHIP_END_OF_TLV if there is no such element
     */
    CHIP_ERROR GetEventRequests(EventPaths::Parser * const apEventRequests) const;

    /**
     *  @brief Get a TLVReader for the EventFilters. Next() must be called before accessing them.
     *
     *  @return #CHIP_NO_ERROR on success
     *          #CHIP_END_OF_TLV if there is no such element
     */
    CHIP_ERROR GetEventFilters(EventFilters::Parser * const apEventFilters) const;

    /**
     *  @brief Get IsFabricFiltered boolean
     *
     *  @param [in] apIsFabricFiltered    A pointer to apIsFabricFiltered
     *
     *  @return #CHIP_NO_ERROR on success
     *          #CHIP_END_OF_TLV if there is no such element
     */
    CHIP_ERROR GetIsFabricFiltered(bool * const apIsFabricFiltered) const;
};

class Builder : public StructBuilder
{
public:
    /**
     *  @brief Initialize a AttributePathIBs::Builder for writing into the TLV stream
     *
     *  @return A reference to AttributePathIBs::Builder
     */
    AttributePathIBs::Builder & CreateAttributeRequests();

    /**
     *  @brief Initialize a EventPaths::Builder for writing into the TLV stream
     *
     *  @return A reference to EventPaths::Builder
     */
    EventPaths::Builder & CreateEventRequests();

    /**
     *  @brief Initialize a EventFilters::Builder for writing into the TLV stream
     *
     *  @return A reference to EventFilters::Builder
     */
    EventFilters::Builder & CreateEventFilters();

    /**
     *  @brief  limits the data written within fabric-scoped lists to the accessing fabric
     *  @return A reference to *this
     */
    ReadRequestMessage::Builder & IsFabricFiltered(const bool aIsFabricFiltered);

    /**
     *  @brief Mark the end of this ReadRequestMessage
     *
     *  @return A reference to *this
     */
    ReadRequestMessage::Builder & EndOfReadRequestMessage();

private:
    AttributePathIBs::Builder mAttributeRequests;
    EventPaths::Builder mEventRequests;
    EventFilters::Builder mEventFilters;
};
} // namespace ReadRequestMessage
} // namespace app
} // namespace chip

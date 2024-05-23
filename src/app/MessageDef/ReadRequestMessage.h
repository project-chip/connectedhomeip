/**
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
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
#include "DataVersionFilterIBs.h"
#include "EventFilterIBs.h"
#include "EventPathIBs.h"
#include "MessageBuilder.h"
#include "MessageParser.h"
#include <app/AppConfig.h>
#include <app/util/basic-types.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/TLV.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

namespace chip {
namespace app {
namespace ReadRequestMessage {
enum class Tag : uint8_t
{
    kAttributeRequests  = 0,
    kEventRequests      = 1,
    kEventFilters       = 2,
    kIsFabricFiltered   = 3,
    kDataVersionFilters = 4,
};

class Parser : public MessageParser
{
public:
#if CHIP_CONFIG_IM_PRETTY_PRINT
    CHIP_ERROR PrettyPrint() const;
#endif // CHIP_CONFIG_IM_PRETTY_PRINT
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
     *  @brief Get a TLVReader for the DataVersionFilterIBs. Next() must be called before accessing them.
     *
     *  @return #CHIP_NO_ERROR on success
     *          #CHIP_END_OF_TLV if there is no such element
     */
    CHIP_ERROR GetDataVersionFilters(DataVersionFilterIBs::Parser * const apDataVersionFilters) const;

    /**
     *  @brief Get a TLVReader for the EventRequests. Next() must be called before accessing them.
     *
     *  @param [in] apEventRequests    A pointer to apEventRequests
     *
     *  @return #CHIP_NO_ERROR on success
     *          #CHIP_END_OF_TLV if there is no such element
     */
    CHIP_ERROR GetEventRequests(EventPathIBs::Parser * const apEventRequests) const;

    /**
     *  @brief Get a TLVReader for the EventFilterIBs. Next() must be called before accessing them.
     *
     *  @return #CHIP_NO_ERROR on success
     *          #CHIP_END_OF_TLV if there is no such element
     */
    CHIP_ERROR GetEventFilters(EventFilterIBs::Parser * const apEventFilters) const;

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

class Builder : public MessageBuilder
{
public:
    /**
     *  @brief Initialize a AttributePathIBs::Builder for writing into the TLV stream
     *
     *  @return A reference to AttributePathIBs::Builder
     */
    AttributePathIBs::Builder & CreateAttributeRequests();

    /**
     *  @brief Initialize a DataVersionFilterIBs::Builder for writing into the TLV stream
     *
     *  @return A reference to DataVersionFilterIBs::Builder
     */
    DataVersionFilterIBs::Builder & CreateDataVersionFilters();

    /**
     *  @brief Initialize a EventPathIBs::Builder for writing into the TLV stream
     *
     *  @return A reference to EventPathIBs::Builder
     */
    EventPathIBs::Builder & CreateEventRequests();

    /**
     *  @brief Initialize a EventFilterIBs::Builder for writing into the TLV stream
     *
     *  @return A reference to EventFilterIBs::Builder
     */
    EventFilterIBs::Builder & CreateEventFilters();

    /**
     *  @brief  limits the data written within fabric-scoped lists to the accessing fabric
     *  @return A reference to *this
     */
    ReadRequestMessage::Builder & IsFabricFiltered(const bool aIsFabricFiltered);

    /**
     *  @brief Mark the end of this ReadRequestMessage
     *
     *  @return The builder's final status.
     */
    CHIP_ERROR EndOfReadRequestMessage();

private:
    AttributePathIBs::Builder mAttributeRequests;
    DataVersionFilterIBs::Builder mDataVersionFilters;
    EventPathIBs::Builder mEventRequests;
    EventFilterIBs::Builder mEventFilters;
};
} // namespace ReadRequestMessage
} // namespace app
} // namespace chip

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

#pragma once

#include "EventDataIB.h"
#include "EventStatusIB.h"
#include "StructBuilder.h"
#include "StructParser.h"

#include <app/AppBuildConfig.h>
#include <app/util/basic-types.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/TLV.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

namespace chip {
namespace app {
namespace EventReportIB {
enum class Tag : uint8_t
{
    kEventStatus = 0,
    kEventData   = 1,
};

class Parser : public StructParser
{
public:
#if CHIP_CONFIG_IM_PRETTY_PRINT
    CHIP_ERROR PrettyPrint() const;
#endif // CHIP_CONFIG_IM_PRETTY_PRINT

    /**
     *  @brief Get a TLVReader for the StatusIB. Next() must be called before accessing them.
     *
     *  @param [in] apEventStatus    A pointer to apEventStatus
     *
     *  @return #CHIP_NO_ERROR on success
     *          # CHIP_ERROR_WRONG_TLV_TYPE if there is such element but it's not a structure
     *          #CHIP_END_OF_TLV if there is no such element
     */
    CHIP_ERROR GetEventStatus(EventStatusIB::Parser * const apEventStatus) const;

    /**
     *  @brief Get a TLVReader for the EventDataIB. Next() must be called before accessing them.
     *
     *  @param [in] apEventData    A pointer to apEventData
     *
     *  @return #CHIP_NO_ERROR on success
     *          #CHIP_ERROR_WRONG_TLV_TYPE if there is such element but it's not a EventData
     *          #CHIP_END_OF_TLV if there is no such element
     */
    CHIP_ERROR GetEventData(EventDataIB::Parser * const apEventData) const;
};

class Builder : public StructBuilder
{
public:
    /**
     *  @brief Initialize a EventDataIB::Builder for writing into the TLV stream
     *
     *  @return A reference to EventDataIB::Builder
     */
    EventDataIB::Builder & CreateEventData();

    /**
     *  @brief Initialize a StatusIB::Builder for writing into the TLV stream
     *
     *  @return A reference to StatusIB::Builder
     */
    EventStatusIB::Builder & CreateEventStatus();

    /**
     *  @brief Mark the end of this EventReportIB
     *
     *  @return A reference to *this
     */
    EventReportIB::Builder & EndOfEventReportIB();

private:
    EventStatusIB::Builder mEventStatus;
    EventDataIB::Builder mEventData;
};

/**
 * @brief construct EventStatusIB to target buffer for report
 */
CHIP_ERROR ConstructEventStatusIB(TLV::TLVWriter & aWriter, const ConcreteEventPath & aEvent, StatusIB aStatus);
} // namespace EventReportIB
} // namespace app
} // namespace chip

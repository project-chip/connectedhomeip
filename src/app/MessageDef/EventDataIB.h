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
/**
 *    @file
 *      This file defines EventDataIB parser and builder in CHIP interaction model
 *
 */

#pragma once

#include "EventPathIB.h"
#include "StructBuilder.h"
#include "StructParser.h"
#include <app/AppBuildConfig.h>
#include <app/EventHeader.h>
#include <app/EventLoggingTypes.h>
#include <app/util/basic-types.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/TLV.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

namespace chip {
namespace app {
namespace EventDataIB {
enum class Tag : uint8_t
{
    kPath                 = 0,
    kEventNumber          = 1,
    kPriority             = 2,
    kEpochTimestamp       = 3,
    kSystemTimestamp      = 4,
    kDeltaEpochTimestamp  = 5,
    kDeltaSystemTimestamp = 6,
    kData                 = 7,
};

class Parser : public StructParser
{
public:
#if CHIP_CONFIG_IM_PRETTY_PRINT
    CHIP_ERROR PrettyPrint() const;
#endif // CHIP_CONFIG_IM_PRETTY_PRINT

    /**
     *  @brief Get a TLVReader for the EventPath. Next() must be called before accessing them.
     *
     *  @param [in] apPath    A pointer to apPath
     *
     *  @return #CHIP_NO_ERROR on success
     *          #CHIP_ERROR_WRONG_TLV_TYPE if there is such element but it's not a Path
     *          #CHIP_END_OF_TLV if there is no such element
     */
    CHIP_ERROR GetPath(EventPathIB::Parser * const apPath);

    /**
     *  @brief Get a TLVReader for the EventNumber. Next() must be called before accessing them.
     *
     *  @param [in] apEventNumber    A pointer to apEventNumber
     *
     *  @return #CHIP_NO_ERROR on success
     *          #CHIP_ERROR_WRONG_TLV_TYPE if there is such element but it's not any of the defined unsigned integer types
     *          #CHIP_END_OF_TLV if there is no such element
     */
    CHIP_ERROR GetEventNumber(uint64_t * const apEventNumber);

    /**
     *  @brief Get a TLVReader for the Number. Next() must be called before accessing them.
     *
     *  @param [in] apPriority    A pointer to apPriorityLevel
     *
     *  @return #CHIP_NO_ERROR on success
     *          #CHIP_ERROR_WRONG_TLV_TYPE if there is such element but it's not any of the defined unsigned integer types
     *          #CHIP_END_OF_TLV if there is no such element
     */
    CHIP_ERROR GetPriority(uint8_t * const apPriority);

    /**
     *  @brief Get a TLVReader for the EpochTimestamp. Next() must be called before accessing them.
     *
     *  @param [in] apEpochTimestamp    A pointer to apEpochTimestamp
     *
     *  @return #CHIP_NO_ERROR on success
     *          #CHIP_ERROR_WRONG_TLV_TYPE if there is such element but it's not any of the defined unsigned integer types
     *          #CHIP_END_OF_TLV if there is no such element
     */
    CHIP_ERROR GetEpochTimestamp(uint64_t * const apEpochTimestamp);

    /**
     *  @brief Get a TLVReader for the SystemTimestamp. Next() must be called before accessing them.
     *
     *  @param [in] apSystemTimestamp    A pointer to apSystemTimestamp
     *
     *  @return #CHIP_NO_ERROR on success
     *          #CHIP_ERROR_WRONG_TLV_TYPE if there is such element but it's not any of the defined unsigned integer types
     *          #CHIP_END_OF_TLV if there is no such element
     */
    CHIP_ERROR GetSystemTimestamp(uint64_t * const apSystemTimestamp);

    /**
     *  @brief Get a TLVReader for the DeltaEpochTimestamp. Next() must be called before accessing them.
     *
     *  @param [in] apDeltaEpochTimestamp   A pointer to apDeltaEpochTimestamp
     *
     *  @return #CHIP_NO_ERROR on success
     *          #CHIP_ERROR_WRONG_TLV_TYPE if there is such element but it's not any of the defined unsigned integer types
     *          #CHIP_END_OF_TLV if there is no such element
     */
    CHIP_ERROR GetDeltaEpochTimestamp(uint64_t * const apDeltaEpochTimestamp);

    /**
     *  @brief Get a TLVReader for the DeltaSystemTimestamp. Next() must be called before accessing them.
     *
     *  @param [in] apDeltaSystemTimestamp   A pointer to apDeltaSystemTimestamp
     *
     *  @return #CHIP_NO_ERROR on success
     *          #CHIP_ERROR_WRONG_TLV_TYPE if there is such element but it's not any of the defined unsigned integer types
     *          #CHIP_END_OF_TLV if there is no such element
     */
    CHIP_ERROR GetDeltaSystemTimestamp(uint64_t * const apDeltaSystemTimestamp);

    /**
     *  @brief Get a TLVReader for the Data. Next() must be called before accessing them.
     *
     *  @param [in] apReader    A pointer to apReader
     *
     *  @return #CHIP_NO_ERROR on success
     *          #CHIP_END_OF_TLV if there is no such element
     */
    CHIP_ERROR GetData(TLV::TLVReader * const apReader) const;

    CHIP_ERROR DecodeEventHeader(EventHeader & aEventHeader);

protected:
    CHIP_ERROR ProcessEventPath(EventPathIB::Parser & aEventPath, ConcreteEventPath & aConcreteEventPath);
    CHIP_ERROR ProcessEventTimestamp(EventHeader & aEventHeader);
};

class Builder : public StructBuilder
{
public:
    /**
     *  @brief Initialize a EventPathIB::Builder for writing into the TLV stream
     *
     *  @return A reference to EventPathIB::Builder
     */
    EventPathIB::Builder & CreatePath();

    /**
     *  @brief Inject PriorityLevel into the TLV stream to indicate the priority level associated with
     *  the cluster that is referenced by the path.
     *
     *  @param [in] aPriority This is an integer representation of the priority level.
     *
     *  @return A reference to *this
     */
    EventDataIB::Builder & Priority(const uint8_t aPriority);

    /**
     *  @brief Inject Number into the TLV stream to indicate the number associated with
     *  the cluster that is referenced by the path. The event number is a monotonically increasing number that
     *  uniquely identifies each emitted event. This number is scoped to the PriorityLevel.
     *
     *  @param [in] aEventNumber The uint64_t variable to reflect the event number
     *
     *  @return A reference to *this
     */
    EventDataIB::Builder & EventNumber(const EventNumber aEventNumber);

    /**
     *  @brief Inject EpochTimestamp into the TLV stream.
     *  This is encoded as a 64-bit millisecond number since UNIX epoch (Jan 1 1970 00:00:00 GMT).
     *
     *  @param [in] aEpochTimestamp The uint64_t variable to reflect the Epoch timestamp of the Event.
     *
     *  @return A reference to *this
     */
    EventDataIB::Builder & EpochTimestamp(const uint64_t aEpochTimestamp);

    /**
     *  @brief Inject SystemTimestamp into the TLV stream. If Epoch time is not available, time since boot
     *  SHALL be encoded in this field as 64-bit, milliseconds.
     *
     *  @param [in] aSystemTimestamp The uint64_t variable to reflect system time
     *
     *  @return A reference to *this
     */
    EventDataIB::Builder & SystemTimestamp(const uint64_t aSystemTimestamp);

    /**
     *  @brief Inject DeltaEpochTimestamp into the TLV stream.
     *      This field is present if delta encoding of the Epoch timestamp relative to a prior event is desired for compression
     *      reasons. When this field is present, the Epoch Timestamp field SHALL be omitted.
     *
     *  @param [in] aDeltaEpochTimestamp The uint64_t variable to reflect DeltaEpochTimestamp
     *
     *  @return A reference to *this
     */
    EventDataIB::Builder & DeltaEpochTimestamp(const uint64_t aDeltaEpochTimestamp);

    /**
     *  @brief Inject DeltaSystemTimestamp into the TLV stream.
     *  This field is present if delta encoding of the System timestamp relative to a prior event is desired for compression
     * reasons. When this field is present, the System Timestamp field SHALL be omitted.
     *
     *  @param [in] aDeltaSystemTimestamp The uint64_t variable to reflect DeltaSystemTimestamp
     *
     *  @return A reference to *this
     */
    EventDataIB::Builder & DeltaSystemTimestamp(const uint64_t aDeltaSystemTimestamp);

    /**
     *  @brief Mark the end of this EventDataIB
     *
     *  @return A reference to *this
     */
    EventDataIB::Builder & EndOfEventDataIB();

private:
    EventPathIB::Builder mPath;
};
} // namespace EventDataIB
} // namespace app
} // namespace chip

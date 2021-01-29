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
 *      This file defines EventDataElement parser and builder in CHIP interaction model
 *
 */

#pragma once

#ifndef _CHIP_INTERACTION_MODEL_MESSAGE_DEF_EVENT_DATA_ELEMENT_H
#define _CHIP_INTERACTION_MODEL_MESSAGE_DEF_EVENT_DATA_ELEMENT_H

#include "Builder.h"
#include "EventPath.h"

#include "Parser.h"
#include <core/CHIPCore.h>
#include <core/CHIPTLV.h>
#include <support/CodeUtils.h>
#include <support/logging/CHIPLogging.h>
#include <util/basic-types.h>

namespace chip {
namespace app {
namespace EventDataElement {
enum
{
    kCsTag_EventPath            = 0,
    kCsTag_ImportanceLevel      = 1,
    kCsTag_Number               = 2,
    kCsTag_UTCTimestamp         = 3,
    kCsTag_SystemTimestamp      = 4,
    kCsTag_DeltaUTCTimestamp    = 5,
    kCsTag_DeltaSystemTimestamp = 6,
    kCsTag_Data                 = 7,
};

class Parser : public chip::app::Parser
{
public:
    /**
     *  @brief Initialize the parser object with TLVReader
     *
     *  @param [in] aReader A pointer to a TLVReader, which should point to the beginning of this EventDataElement
     *
     *  @return #CHIP_NO_ERROR on success
     */
    CHIP_ERROR Init(const chip::TLV::TLVReader & aReader);

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

    /**
     *  @brief Get a TLVReader for the EventPath. Next() must be called before accessing them.
     *
     *  @param [in] apEventPath    A pointer to apEventPath
     *
     *  @return #CHIP_NO_ERROR on success
     *          #CHIP_ERROR_WRONG_TLV_TYPE if there is such element but it's not a Path
     *          #CHIP_END_OF_TLV if there is no such element
     */
    CHIP_ERROR GetEventPath(EventPath::Parser * const apEventPath);

    /**
     *  @brief Get a TLVReader for the Number. Next() must be called before accessing them.
     *
     *  @param [in] apImportanceLevel    A pointer to apImportanceLevel
     *
     *  @return #CHIP_NO_ERROR on success
     *          #CHIP_ERROR_WRONG_TLV_TYPE if there is such element but it's not any of the defined unsigned integer types
     *          #CHIP_END_OF_TLV if there is no such element
     */
    CHIP_ERROR GetImportanceLevel(uint8_t * const apImportanceLevel);

    /**
     *  @brief Get a TLVReader for the Number. Next() must be called before accessing them.
     *
     *  @param [in] apNumber    A pointer to apNumber
     *
     *  @return #CHIP_NO_ERROR on success
     *          #CHIP_ERROR_WRONG_TLV_TYPE if there is such element but it's not any of the defined unsigned integer types
     *          #CHIP_END_OF_TLV if there is no such element
     */
    CHIP_ERROR GetNumber(uint64_t * const apNumber);

    /**
     *  @brief Get a TLVReader for the UTCTimestamp. Next() must be called before accessing them.
     *
     *  @param [in] apUTCTimestamp    A pointer to apUTCTimestamp
     *
     *  @return #CHIP_NO_ERROR on success
     *          #CHIP_ERROR_WRONG_TLV_TYPE if there is such element but it's not any of the defined unsigned integer types
     *          #CHIP_END_OF_TLV if there is no such element
     */
    CHIP_ERROR GetUTCTimestamp(uint64_t * const apUTCTimestamp);

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
     *  @brief Get a TLVReader for the DeltaUTCTime. Next() must be called before accessing them.
     *
     *  @param [in] apDeltaUTCTimestamp   A pointer to apDeltaUTCTime
     *
     *  @return #CHIP_NO_ERROR on success
     *          #CHIP_ERROR_WRONG_TLV_TYPE if there is such element but it's not any of the defined unsigned integer types
     *          #CHIP_END_OF_TLV if there is no such element
     */
    CHIP_ERROR GetDeltaUTCTime(uint64_t * const apDeltaUTCTime);

    /**
     *  @brief Get a TLVReader for the DeltaSystemTime. Next() must be called before accessing them.
     *
     *  @param [in] apDeltaSystemTimestamp   A pointer to apDeltaSystemTime
     *
     *  @return #CHIP_NO_ERROR on success
     *          #CHIP_ERROR_WRONG_TLV_TYPE if there is such element but it's not any of the defined unsigned integer types
     *          #CHIP_END_OF_TLV if there is no such element
     */
    CHIP_ERROR GetDeltaSystemTime(uint64_t * const apDeltaSystemTime);

    /**
     *  @brief Get a TLVReader for the Data. Next() must be called before accessing them.
     *
     *  @param [in] apReader    A pointer to apReader
     *
     *  @return #CHIP_NO_ERROR on success
     *          #CHIP_END_OF_TLV if there is no such element
     */
    CHIP_ERROR GetData(chip::TLV::TLVReader * const apReader) const;

protected:
    // A recursively callable function to parse a data element and pretty-print it.
    CHIP_ERROR ParseData(chip::TLV::TLVReader & aReader, int aDepth) const;
};

class Builder : public chip::app::Builder
{
public:
    /**
     *  @brief Initialize a EventDataElement::Builder for writing into a TLV stream
     *
     *  @param [in] apWriter    A pointer to TLVWriter
     *
     *  @return #CHIP_NO_ERROR on success
     */
    CHIP_ERROR Init(chip::TLV::TLVWriter * const apWriter);

    /**
     *  @brief Initialize a EventPath::Builder for writing into the TLV stream
     *
     *  @return A reference to EventPath::Builder
     */
    EventPath::Builder & CreateEventPathBuilder();

    /**
     *  @brief Inject ImportanceLevel into the TLV stream to indicate the importance level associated with
     *  the cluster that is referenced by the path.
     *
     *  @param [in] aImportanceLevel This is an integer representation of the importance level.
     *
     *  @return A reference to *this
     */
    EventDataElement::Builder ImportanceLevel(const uint8_t aImportanceLevel);

    /**
     *  @brief Inject Number into the TLV stream to indicate the number associated with
     *  the cluster that is referenced by the path. The event number is a monotonically increasing number that
     *  uniquely identifies each emitted event. This number is scoped to the ImportanceLevel.
     *
     *  @param [in] aNumber The uint64_t variable to reflectt the event number
     *
     *  @return A reference to *this
     */
    EventDataElement::Builder Number(const uint64_t aNumber);

    /**
     *  @brief Inject UTCTimestamp into the TLV stream.
     *  This is encoded as a 64-bit millisecond number since UNIX epoch (Jan 1 1970 00:00:00 GMT).
     *
     *  @param [in] aUTCTimestamp The uint64_t variable to reflect the UTC timestamp of the Event.
     *
     *  @return A reference to *this
     */
    EventDataElement::Builder UTCTimestamp(const uint64_t aUTCTimestamp);

    /**
     *  @brief Inject SystemTimestamp into the TLV stream. If UTC time is not available, time since boot
     *  SHALL be encoded in this field as 64-bit, milliseconds.
     *
     *  @param [in] aSystemTimestamp The uint64_t variable to reflect system time
     *
     *  @return A reference to *this
     */
    EventDataElement::Builder SystemTimestamp(const uint64_t aSystemTimestamp);

    /**
     *  @brief Inject DeltaUTCTime into the TLV stream.
     *      This field is present if delta encoding of the UTC timestamp relative to a prior event is desired for compression
     *      reasons. When this field is present, the UTC Timestamp field SHALL be omitted.
     *
     *  @param [in] aDeltaUTCTime The uint64_t variable to reflect DeltaUTCTime
     *
     *  @return A reference to *this
     */
    EventDataElement::Builder DeltaUTCTime(const uint64_t aDeltaUTCTime);

    /**
     *  @brief Inject DeltaSystemTimestamp into the TLV stream.
     *  This field is present if delta encoding of the System timestamp relative to a prior event is desired for compression
     * reasons. When this field is present, the System Timestamp field SHALL be omitted.
     *
     *  @param [in] DeltaSystemTimestamp The uint64_t variable to reflect DeltaSystemTime
     *
     *  @return A reference to *this
     */
    EventDataElement::Builder DeltaSystemTime(const uint64_t aDeltaSystemTime);

    /**
     *  @brief Mark the end of this EventDataElement
     *
     *  @return A reference to *this
     */
    EventDataElement::Builder & EndOfEventDataElement();

private:
    EventPath::Builder mEventPathBuilder;
};
}; // namespace EventDataElement
}; // namespace app
}; // namespace chip

#endif // _CHIP_INTERACTION_MODEL_MESSAGE_DEF_EVENT_DATA_ELEMENT_H

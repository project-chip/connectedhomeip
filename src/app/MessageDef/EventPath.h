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
 *      This file defines EventPath parser and builder in CHIP interaction model
 *
 */

#pragma once

#ifndef _CHIP_INTERACTION_MODEL_MESSAGE_DEF_EVENT_PATH_H
#define _CHIP_INTERACTION_MODEL_MESSAGE_DEF_EVENT_PATH_H

#include "Builder.h"
#include "Parser.h"
#include <core/CHIPCore.h>
#include <core/CHIPTLV.h>
#include <support/CodeUtils.h>
#include <support/logging/CHIPLogging.h>
#include <util/basic-types.h>

namespace chip {
namespace app {
namespace EventPath {
enum
{
    kCsTag_NodeId              = 0,
    kCsTag_EndpointId          = 1,
    kCsTag_NamespacedClusterId = 2,
    kCsTag_EventId             = 3,
};

class Parser : public chip::app::Parser
{
public:
    /**
     *  @brief Initialize the parser object with TLVReader
     *
     *  @param [in] aReader A pointer to a TLVReader, which should point to the beginning of this EventPath
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
     *  @brief Get a TLVReader for the NodeId. Next() must be called before accessing them.
     *
     *  @param [in] apNodeId    A pointer to apNodeId
     *
     *  @return #CHIP_NO_ERROR on success
     *          #CHIP_ERROR_WRONG_TLV_TYPE if there is such element but it's not any of the defined unsigned integer types
     *          #CHIP_END_OF_TLV if there is no such element
     */
    CHIP_ERROR GetNodeId(chip::NodeId * const apNodeId) const;

    /**
     *  @brief Get a TLVReader for the EndpointId. Next() must be called before accessing them.
     *
     *  @param [in] apEndpointId    A pointer to apEndpointId
     *
     *  @return #CHIP_NO_ERROR on success
     *          #CHIP_ERROR_WRONG_TLV_TYPE if there is such element but it's not any of the defined unsigned integer types
     *          #CHIP_END_OF_TLV if there is no such element
     */
    CHIP_ERROR GetEndpointId(chip::EndpointId * const apEndpointId) const;

    /**
     *  @brief Get a TLVReader for the NamespacedClusterId. Next() must be called before accessing them.
     *
     *  @param [in] apNamespacedClusterId    A pointer to apNamespacedClusterId
     *
     *  @return #CHIP_NO_ERROR on success
     *          #CHIP_ERROR_WRONG_TLV_TYPE if there is such element but it's not any of the defined unsigned integer types
     *          #CHIP_END_OF_TLV if there is no such element
     */
    CHIP_ERROR GetNamespacedClusterId(chip::ClusterId * const apNamespacedClusterId) const;

    /**
     *  @brief Get a TLVReader for the EventId. Next() must be called before accessing them.
     *
     *  @param [in] apEventId    A pointer to apEventId
     *
     *  @return #CHIP_NO_ERROR on success
     *          #CHIP_ERROR_WRONG_TLV_TYPE if there is such element but it's not any of the defined unsigned integer types
     *          #CHIP_END_OF_TLV if there is no such element
     */
    CHIP_ERROR GetEventId(chip::EventId * const apEventId) const;
};

class Builder : public chip::app::Builder
{
public:
    /**
     *  @brief Initialize a EventPath::Builder for writing into a TLV stream
     *
     *  @param [in] apWriter    A pointer to TLVWriter
     *
     *  @return #CHIP_NO_ERROR on success
     */
    CHIP_ERROR Init(chip::TLV::TLVWriter * const apWriter);

    /**
     * Init the EventPath container with an particular context tag.
     * Required to implement arrays of arrays, and to test ListBuilder.
     *
     * @param[in]   apWriter    Pointer to the TLVWriter that is encoding the message.
     * @param[in]   aContextTagToUse    A contextTag to use.
     *
     * @return                  CHIP_ERROR codes returned by chip::TLV objects.
     */
    CHIP_ERROR Init(chip::TLV::TLVWriter * const apWriter, const uint8_t aContextTagToUse);

    /**
     *  @brief Inject NodeId into the TLV stream.
     *
     *  @param [in] aNodeId NodeId for this event path
     *
     *  @return A reference to *this
     */
    EventPath::Builder & NodeId(const chip::NodeId aNodeId);

    /**
     *  @brief Inject EndpointId into the TLV stream.
     *
     *  @param [in] aEndpointId EndpointId for this eevent path
     *
     *  @return A reference to *this
     */
    EventPath::Builder & EndpointId(const chip::EndpointId aEndpointId);

    /**
     *  @brief Inject NamespacedClusterId into the TLV stream.
     *
     *  @param [in] aNamespacedClusterId NamespacedClusterId for this event path
     *
     *  @return A reference to *this
     */
    EventPath::Builder & NamespacedClusterId(const chip::ClusterId aNamespacedClusterId);

    /**
     *  @brief Inject EventId into the TLV stream.
     *
     *  @param [in] aEventId NamespacedClusterId for this event path
     *
     *  @return A reference to *this
     */
    EventPath::Builder & EventId(const chip::EventId aEventId);

    /**
     *  @brief Mark the end of this EventPath
     *
     *  @return A reference to *this
     */
    EventPath::Builder & EndOfEventPath();

private:
    CHIP_ERROR _Init(chip::TLV::TLVWriter * const apWriter, const uint64_t aTag);
};
}; // namespace EventPath
}; // namespace app
}; // namespace chip

#endif // _CHIP_INTERACTION_MODEL_MESSAGE_DEF_EVENT_PATH_H

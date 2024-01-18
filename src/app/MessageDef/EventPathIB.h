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

#include "ListBuilder.h"
#include "ListParser.h"

#include <app/AppConfig.h>
#include <app/ConcreteEventPath.h>
#include <app/EventPathParams.h>
#include <app/util/basic-types.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/NodeId.h>
#include <lib/core/TLV.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

namespace chip {
namespace app {
namespace EventPathIB {
enum class Tag : uint8_t
{
    kNode     = 0,
    kEndpoint = 1,
    kCluster  = 2,
    kEvent    = 3,
    kIsUrgent = 4,
};

class Parser : public ListParser
{
public:
#if CHIP_CONFIG_IM_PRETTY_PRINT
    CHIP_ERROR PrettyPrint() const;
#endif // CHIP_CONFIG_IM_PRETTY_PRINT

    /**
     *  @brief Get a TLVReader for the NodeId. Next() must be called before accessing them.
     *
     *  @param [in] apNode    A pointer to apNode
     *
     *  @return #CHIP_NO_ERROR on success
     *          #CHIP_ERROR_WRONG_TLV_TYPE if there is such element but it's not any of the defined unsigned integer types
     *          #CHIP_END_OF_TLV if there is no such element
     */
    CHIP_ERROR GetNode(NodeId * const apNode) const;

    /**
     *  @brief Get a TLVReader for the EndpointId. Next() must be called before accessing them.
     *
     *  @param [in] apEndpoint    A pointer to apEndpoint
     *
     *  @return #CHIP_NO_ERROR on success
     *          #CHIP_ERROR_WRONG_TLV_TYPE if there is such element but it's not any of the defined unsigned integer types
     *          #CHIP_END_OF_TLV if there is no such element
     */
    CHIP_ERROR GetEndpoint(EndpointId * const apEndpoint) const;

    /**
     *  @brief Get a TLVReader for the ClusterId. Next() must be called before accessing them.
     *
     *  @param [in] apCluster    A pointer to apCluster
     *
     *  @return #CHIP_NO_ERROR on success
     *          #CHIP_ERROR_WRONG_TLV_TYPE if there is such element but it's not any of the defined unsigned integer types
     *          #CHIP_END_OF_TLV if there is no such element
     */
    CHIP_ERROR GetCluster(ClusterId * const apCluster) const;

    /**
     *  @brief Get a TLVReader for the EventId. Next() must be called before accessing them.
     *
     *  @param [in] apEvent    A pointer to apEvent
     *
     *  @return #CHIP_NO_ERROR on success
     *          #CHIP_ERROR_WRONG_TLV_TYPE if there is such element but it's not any of the defined unsigned integer types
     *          #CHIP_END_OF_TLV if there is no such element
     */
    CHIP_ERROR GetEvent(EventId * const apEvent) const;

    /**
     *  @brief Get a TLVReader for the bool isUrgent. Next() must be called before accessing them.
     *
     *  @param [in] apIsUrgent  A pointer to apIsUrgent
     *
     *  @return #CHIP_NO_ERROR on success
     *          #CHIP_ERROR_WRONG_TLV_TYPE if there is such element but it's not any of the defined unsigned integer types
     *          #CHIP_END_OF_TLV if there is no such element
     */
    CHIP_ERROR GetIsUrgent(bool * const apIsUrgent) const;

    /**
     *  @brief Fill the fields in apPath from the parser, the path in the parser should be a concrete path.
     *
     *  @param [in] apPath    A pointer to the path to fill in.
     *
     *  @return #CHIP_NO_ERROR on success
     *          #CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB if the path from the reader is not a valid concrere event path.
     */
    CHIP_ERROR GetEventPath(ConcreteEventPath * const apPath) const;

    /**
     * @brief Parse the event path into an EventPathParams object. As part of parsing,
     * validity checks for each path item will be done as well.
     *
     * If any errors are encountered, an IM error of 'InvalidAction' will be returned.
     */
    CHIP_ERROR ParsePath(EventPathParams & aEvent) const;
};

class Builder : public ListBuilder
{
public:
    /**
     *  @brief Inject Node into the TLV stream.
     *
     *  @param [in] aNode NodeId for this event path
     *
     *  @return A reference to *this
     */
    EventPathIB::Builder & Node(const NodeId aNode);

    /**
     *  @brief Inject Endpoint into the TLV stream.
     *
     *  @param [in] aEndpoint EndpointId for this event path
     *
     *  @return A reference to *this
     */
    EventPathIB::Builder & Endpoint(const EndpointId aEndpoint);

    /**
     *  @brief Inject Cluster into the TLV stream.
     *
     *  @param [in] aCluster ClusterId for this event path
     *
     *  @return A reference to *this
     */
    EventPathIB::Builder & Cluster(const ClusterId aCluster);

    /**
     *  @brief Inject Event into the TLV stream.
     *
     *  @param [in] aEvent EventId for this event path
     *
     *  @return A reference to *this
     */
    EventPathIB::Builder & Event(const EventId aEvent);

    /**
     *  @brief Inject urgent boolean into the TLV stream.
     *
     *  @param [in] aIsUrgent boolean for this event path
     *
     *  @return A reference to *this
     */
    EventPathIB::Builder & IsUrgent(const bool aIsUrgent);

    /**
     *  @brief Mark the end of this EventPath
     *
     *  @return The builder's final status.
     */
    CHIP_ERROR EndOfEventPathIB();

    CHIP_ERROR Encode(const EventPathParams & aEventPathParams);
};
} // namespace EventPathIB
} // namespace app
} // namespace chip

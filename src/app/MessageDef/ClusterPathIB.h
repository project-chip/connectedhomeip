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

#include "ListBuilder.h"
#include "ListParser.h"

#include <app/AppBuildConfig.h>
#include <app/util/basic-types.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/CHIPTLV.h>
#include <lib/core/NodeId.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

namespace chip {
namespace app {
namespace ClusterPathIB {
enum class Tag : uint8_t
{
    kNode     = 0,
    kEndpoint = 1,
    kCluster  = 2,
};

class Parser : public ListParser
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
     *  @brief Get the NodeId
     *
     *  @param [in] apNodeId    A pointer to apNodeId
     *
     *  @return #CHIP_NO_ERROR on success
     *          #CHIP_ERROR_WRONG_TLV_TYPE if there is such element but it's not any of the defined unsigned integer types
     *          #CHIP_END_OF_TLV if there is no such element
     */
    CHIP_ERROR GetNode(NodeId * const apNodeId) const;

    /**
     *  @brief Get the Endpoint.
     *
     *  @param [in] apEndpoint    A pointer to apEndpoint
     *
     *  @return #CHIP_NO_ERROR on success
     *          #CHIP_ERROR_WRONG_TLV_TYPE if there is such element but it's not any of the defined unsigned integer types
     *          #CHIP_END_OF_TLV if there is no such element
     */
    CHIP_ERROR GetEndpoint(EndpointId * const apEndpoint) const;

    /**
     *  @brief Get the Cluster.
     *
     *  @param [in] apCluster    A pointer to apCluster
     *
     *  @return #CHIP_NO_ERROR on success
     *          #CHIP_ERROR_WRONG_TLV_TYPE if there is such element but it's not any of the defined unsigned integer types
     *          #CHIP_END_OF_TLV if there is no such element
     */
    CHIP_ERROR GetCluster(ClusterId * const apCluster) const;
};

class Builder : public ListBuilder
{
public:
    /**
     *  @brief Inject Node into the TLV stream.
     *
     *  @param [in] aNode Node for this cluster path
     *
     *  @return A reference to *this
     */
    ClusterPathIB::Builder & Node(const NodeId aNode);

    /**
     *  @brief Inject Endpoint into the TLV stream.
     *
     *  @param [in] aEndpoint Endpoint for this cluster path
     *
     *  @return A reference to *this
     */
    ClusterPathIB::Builder & Endpoint(const EndpointId aEndpoint);

    /**
     *  @brief Inject Cluster into the TLV stream.
     *
     *  @param [in] aCluster Cluster for this cluster path
     *
     *  @return A reference to *this
     */
    ClusterPathIB::Builder & Cluster(const ClusterId aCluster);

    /**
     *  @brief Mark the end of this ClusterPathIB
     *
     *  @return A reference to *this
     */
    ClusterPathIB::Builder & EndOfClusterPathIB();
};
} // namespace ClusterPathIB
} // namespace app
} // namespace chip

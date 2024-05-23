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
#include <app/CommandPathParams.h>
#include <app/ConcreteCommandPath.h>
#include <app/util/basic-types.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/TLV.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

namespace chip {
namespace app {
namespace CommandPathIB {
enum class Tag : uint8_t
{
    kEndpointId = 0,
    kClusterId  = 1,
    kCommandId  = 2,
};

class Parser : public ListParser
{
public:
#if CHIP_CONFIG_IM_PRETTY_PRINT
    CHIP_ERROR PrettyPrint() const;
#endif // CHIP_CONFIG_IM_PRETTY_PRINT

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
     *  @brief Get a TLVReader for the ClusterId. Next() must be called before accessing them.
     *
     *  @param [in] apClusterId    A pointer to ClusterId
     *
     *  @return #CHIP_NO_ERROR on success
     *          #CHIP_ERROR_WRONG_TLV_TYPE if there is such element but it's not any of the defined unsigned integer types
     *          #CHIP_END_OF_TLV if there is no such element
     */
    CHIP_ERROR GetClusterId(chip::ClusterId * const apClusterId) const;

    /**
     *  @brief Get a TLVReader for the CommandId. Next() must be called before accessing them.
     *
     *  @param [in] apCommandId    A pointer to CommandId
     *
     *  @return #CHIP_NO_ERROR on success
     *          #CHIP_ERROR_WRONG_TLV_TYPE if there is such element but it's not any of the defined unsigned integer types
     *          #CHIP_END_OF_TLV if there is no such element
     */
    CHIP_ERROR GetCommandId(chip::CommandId * const apCommandId) const;

    /**
     * @brief Get the concrete command path, if this command path is a concrete
     *        path.
     *
     * This will validate that the cluster id and command id are actually valid for a
     * concrete path.
     *
     *  @param [in] aCommandPath    The command path object to write to.
     */
    CHIP_ERROR GetConcreteCommandPath(ConcreteCommandPath & aCommandPath) const;

    /**
     * @brief Get a group command path.
     *
     * This will validate that the cluster id and command id are actually valid for a
     * group path.
     *
     *  @param [out] apClusterId    The cluster id in the path.
     *  @param [out] apCommandId    The command id in the path.
     *
     *  @return #CHIP_NO_ERROR on success
     */
    CHIP_ERROR GetGroupCommandPath(ClusterId * apClusterId, CommandId * apCommandId) const;
};

class Builder : public ListBuilder
{
public:
    /**
     *  @brief Inject EndpointId into the TLV stream to indicate the endpointId referenced by the path.
     *
     *  @param [in] aEndpointId refer to the ID of the endpoint as described in the descriptor cluster.
     *
     *  @return A reference to *this
     */
    CommandPathIB::Builder & EndpointId(const chip::EndpointId aEndpointId);

    /**
     *  @brief Inject ClusterId into the TLV stream.
     *
     *  @param [in] aClusterId ClusterId for this command path
     *
     *  @return A reference to *this
     */
    CommandPathIB::Builder & ClusterId(const chip::ClusterId aClusterId);

    /**
     *  @brief Inject CommandId into the TLV stream
     *
     *  @param [in] aCommandId Command Id for ClusterId for this command path
     *
     *  @return A reference to *this
     */
    CommandPathIB::Builder & CommandId(const chip::CommandId aCommandId);

    /**
     *  @brief Mark the end of this CommandPathIB
     *
     *  @return The builder's final status.
     */
    CHIP_ERROR EndOfCommandPathIB();

    CHIP_ERROR Encode(const CommandPathParams & aCommandPathParams);
    CHIP_ERROR Encode(const ConcreteCommandPath & aConcreteCommandPath);
};
} // namespace CommandPathIB
} // namespace app
} // namespace chip

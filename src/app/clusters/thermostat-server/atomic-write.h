/**
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include <app-common/zap-generated/cluster-objects.h>
#include <app/CommandHandler.h>
#include <app/ConcreteAttributePath.h>
#include <app/ConcreteCommandPath.h>
#include <app/data-model/DecodableList.h>
#include <protocols/interaction_model/StatusCode.h>

using imcode = chip::Protocols::InteractionModel::Status;

namespace chip {
namespace app {
namespace Clusters {
namespace Thermostat {

class AtomicWriteDelegate
{
public:
    AtomicWriteDelegate() = default;

    virtual ~AtomicWriteDelegate() = default;

    virtual imcode OnBeginWrite(EndpointId endpoint, AttributeId attributeId)     = 0;
    virtual imcode OnPreCommitWrite(EndpointId endpoint, AttributeId attributeId) = 0;
    virtual imcode OnCommitWrite(EndpointId endpoint, AttributeId attributeId)    = 0;
    virtual imcode OnRollbackWrite(EndpointId endpoint, AttributeId attributeId)  = 0;

    virtual std::optional<System::Clock::Milliseconds16> GetWriteTimeout(EndpointId endpoint, AttributeId attributeId) = 0;
};

class AtomicWriteManager
{
public:
    AtomicWriteManager() = default;

    virtual ~AtomicWriteManager() = default;

    /**
     * @brief Gets whether an atomic write is in progress for the given endpoint and attribute
     *
     * @param[in] attributeId The attribute ID.
     * @param[in] endpoint The endpoint.
     *
     * @return Whether an atomic write is in progress for the given endpoint
     */
    virtual bool InWrite(const std::optional<AttributeId> attributeId, EndpointId endpoint) = 0;

    /**
     * @brief Gets whether an atomic write is in progress for the given endpoint and attribute
     *
     * @param[in] attributeId The attribute ID.
     * @param[in] subjectDescriptor The subject descriptor.
     * @param[in] endpoint The endpoint.
     *
     * @return Whether an atomic write is in progress for the given endpoint
     */
    virtual bool InWrite(const std::optional<AttributeId> attributeId, const Access::SubjectDescriptor & subjectDescriptor,
                         EndpointId endpoint) = 0;

    /**
     * @brief Gets whether an atomic write is in progress for the given endpoint and attribute
     *
     * @param[in] attributeId The attribute ID.
     * @param[in] commandObj The command handler.
     * @param[in] endpoint The endpoint.
     *
     * @return Whether an atomic write is in progress for the given endpoint
     */
    virtual bool InWrite(const std::optional<AttributeId> attributeId, CommandHandler * commandObj, EndpointId endpoint) = 0;

    virtual bool InWrite(const DataModel::DecodableList<AttributeId>::Iterator attributeIds, CommandHandler * commandObj,
                         EndpointId endpoint) = 0;

    virtual bool BeginWrite(chip::app::CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                            const Commands::AtomicRequest::DecodableType & commandData) = 0;

    virtual bool CommitWrite(chip::app::CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                             const Commands::AtomicRequest::DecodableType & commandData) = 0;

    virtual bool RollbackWrite(chip::app::CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                               const Commands::AtomicRequest::DecodableType & commandData) = 0;

    virtual void ResetWrite(EndpointId endpoint)     = 0;
    virtual void ResetWrite(FabricIndex fabricIndex) = 0;

    virtual void SetDelegate(AtomicWriteDelegate * delegate) = 0;
};

} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip

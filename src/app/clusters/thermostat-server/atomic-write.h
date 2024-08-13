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

/**
 * @brief The AtomicWriteDelegate handles actual execution of the atomic write by the cluster
 *
 */
class AtomicWriteDelegate
{
public:
    AtomicWriteDelegate() = default;

    virtual ~AtomicWriteDelegate() = default;

    /**
     * @brief OnBeginWrite is called when the client has successfully started an atomic write. The server should begin pending any
     * writes to the associated attribute; if it is not capable for any reason, it should return an error code, and the atomic write
     * will be discarded
     *
     * @param endpoint The endpoint for the atomic write
     * @param attributeId The attribute for the associated atomic write
     * @return Success, if the server is able to pend writes; otherwise an error code.
     */
    virtual imcode OnBeginWrite(EndpointId endpoint, AttributeId attributeId) = 0;

    /**
     * @brief OnPreCommitWrite is called when a client attempts to commit an atomic write. The server should evaluate writes to this
     * attribute, and return an error code if they would not be successful. This method should not have any side effects; even if it
     * returns Success, any pending changes may still be discarded if OnPreCommit fails for a separate attribute included in the
     * atomic write
     *
     * @param endpoint The endpoint for the atomic write
     * @param attributeId The attribute for the associated atomic write
     * @return Success, if the pending writes would succeed; otherwise an error code.
     */
    virtual imcode OnPreCommitWrite(EndpointId endpoint, AttributeId attributeId) = 0;

    /**
     * @brief OnCommitWrite is called when a client attempts to commit an atomic write, and all calls to OnPreCommitWrite were
     * successful.
     *
     * @param endpoint The endpoint for the atomic write
     * @param attributeId The attribute for the associated atomic write
     * @return Success, if the pending writes succeeded; otherwise an error code.
     */
    virtual imcode OnCommitWrite(EndpointId endpoint, AttributeId attributeId) = 0;

    /**
     * @brief OnRollbackWrite is called when a client attempts to rollback an atomic write, or when the timeout expires without the
     * client attempting to commit or rollback the atomic write
     *
     * @param endpoint The endpoint for the atomic write
     * @param attributeId The attribute for the associated atomic write
     * @return Success, if rolling back pending writes succeeded; otherwise an error code.
     */
    virtual imcode OnRollbackWrite(EndpointId endpoint, AttributeId attributeId) = 0;

    /**
     * @brief Get the maximum write time out for a given attribute
     *
     * @param endpoint The endpoint for the atomic write
     * @param attributeId The attribute for the associated atomic write
     * @return The maximum milliseconds a server is willing to wait for an atomic write to succeed for the given attribute, or
     * nullopt if the server can not determine the maximum
     */
    virtual std::optional<System::Clock::Milliseconds16> GetWriteTimeout(EndpointId endpoint, AttributeId attributeId) = 0;
};

class AtomicWriteManager
{
public:
    AtomicWriteManager() = default;

    virtual ~AtomicWriteManager() = default;

    /**
     * @brief Check if there is an open atomic write on an endpoint, optionally associated with a specific attribute
     *
     * @param attributeId Optional attribute filter
     * @param endpoint The endpoint to check atomic write state
     * @return true if there is an open atomic write
     * @return false if there is not at open atomic write
     */
    virtual bool InWrite(const std::optional<AttributeId> attributeId, EndpointId endpoint) = 0;

    /**
     * @brief Check if there is an open atomic write on an endpoint, associated with a given SubjectDescriptor, and optionally
     * associated with a specific attribute
     *
     * @param attributeId Optional attribute filter
     * @param subjectDescriptor The subject descriptor to check against
     * @param endpoint The endpoint to check atomic write state
     * @return true if there is an open atomic write
     * @return false if there is not an open atomic write
     */
    virtual bool InWrite(const std::optional<AttributeId> attributeId, const Access::SubjectDescriptor & subjectDescriptor,
                         EndpointId endpoint) = 0;

    /**
     * @brief Check if there is an open atomic write on an endpoint, associated with the source node for a given command invocation,
     * and optionally associated with a specific attribute
     *
     * @param attributeId
     * @param commandObj
     * @param endpoint
     * @return true
     * @return false
     */
    virtual bool InWrite(const std::optional<AttributeId> attributeId, CommandHandler * commandObj, EndpointId endpoint) = 0;

    /**
     * @brief Check if there is an open atomic write on an endpoint, associated with the source node for a given command invocation,
     * and associated with the set of attribute IDs
     *
     * @param attributeIds The set of attribute IDs to check against
     * @param commandObj The command being invoked
     * @param endpoint The endpoint for the atomic write
     * @return true if there is an open atomic write
     * @return false if there is not an open atomic write
     */
    virtual bool InWrite(const DataModel::DecodableList<AttributeId>::Iterator attributeIds, CommandHandler * commandObj,
                         EndpointId endpoint) = 0;

    /**
     * @brief Attempt to start an atomic write
     *
     * @param commandObj The AtomicRequest command
     * @param commandPath The path for the command
     * @param commandData The payload of the command
     * @return true if the atomic write was opened
     * @return false if the atomic write was not opened
     */
    virtual bool BeginWrite(chip::app::CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                            const Commands::AtomicRequest::DecodableType & commandData) = 0;

    /**
     * @brief Attempt to commit an atomic write; returns true if the server is able to commit or failed trying, false if it was
     * unable to find a matching atomic write
     *
     * @param commandObj The AtomicRequest command
     * @param commandPath The path for the command
     * @param commandData The payload of the command
     * @return true if the atomic write was committed or rolled back
     * @return false if the atomic write was not found
     */
    virtual bool CommitWrite(chip::app::CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                             const Commands::AtomicRequest::DecodableType & commandData) = 0;

    /**
     * @brief Attempt to roll back an atomic write; returns true if the server is able to commit or failed trying, false if it was
     * unable to find a matching atomic write
     *
     * @param commandObj The AtomicRequest command
     * @param commandPath The path for the command
     * @param commandData The payload of the command
     * @return true if the atomic write was rolled back
     * @return false if the atomic write was not found
     */
    virtual bool RollbackWrite(chip::app::CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                               const Commands::AtomicRequest::DecodableType & commandData) = 0;

    /**
     * @brief Reset any atomic writes associated with the given endpoint
     *
     * @param endpoint
     */
    virtual void ResetWrite(EndpointId endpoint) = 0;

    /**
     * @brief Reset any atomic writes originating from a given fabric index
     *
     * @param fabricIndex
     */
    virtual void ResetWrite(FabricIndex fabricIndex) = 0;

    /**
     * @brief Sets a delegate for actually executing atomic writes
     *
     * @param delegate
     */
    virtual void SetDelegate(AtomicWriteDelegate * delegate) = 0;
};

} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip

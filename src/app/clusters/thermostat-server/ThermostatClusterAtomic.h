/*
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
#include <app/data-model/List.h>
#include <app/persistence/AttributePersistence.h>
#include <lib/core/ScopedNodeId.h>
#include <lib/support/ReadOnlyBuffer.h>
#include <protocols/interaction_model/Constants.h>
#include <system/SystemClock.h>

#include <optional>

namespace chip {
namespace app {
namespace Clusters {
namespace Thermostat {

/****************************************************************************
 * @file
 * @brief APIs for managing atomic write sessions for the Thermostat cluster.
 *
 * Tracks active atomic write operations associated with a particular subject descriptor,
 * locking out other writers until the write is committed, rolled back, or times out
 ******************************************************************************/

using AtomicAttributes = Platform::ScopedMemoryBufferWithSize<Globals::Structs::AtomicAttributeStatusStruct::Type>;
class AtomicWriteSession
{
public:
    class Delegate
    {
    public:
        Delegate() = default;

        virtual ~Delegate() = default;

        virtual Protocols::InteractionModel::Status OnAtomicWriteBegin(AttributeId attributeId)     = 0;
        virtual Protocols::InteractionModel::Status OnAtomicWritePrecommit(AttributeId attributeId) = 0;
        virtual Protocols::InteractionModel::Status OnAtomicWriteCommit(AttributeId attributeId)    = 0;
        virtual Protocols::InteractionModel::Status OnAtomicWriteRollback(AttributeId attributeId)  = 0;

        virtual std::optional<System::Clock::Milliseconds16> GetMaxAtomicWriteTimeout(chip::AttributeId attributeId) = 0;
        virtual bool HasAttribute(chip::AttributeId attributeId)                                                     = 0;
    };

    enum class State
    {
        Closed = 0,
        Open,
    };

    AtomicWriteSession(Delegate & delegate) : mDelegate(delegate) {}
    /**
     * @brief Sets the atomic write state for the given originatorNodeId
     *
     * @param[in] originatorNodeId The originator scoped node id.
     * @param[in] state Whether or not an atomic write is open or closed.
     * @param attributeStatuses The set of attribute status structs the atomic write should be associated with
     * @return true if it was able to update the atomic write state
     * @return false if it was unable to update the atomic write state
     */
    bool SetAtomicWrite(ScopedNodeId originatorNodeId, State state, AtomicAttributes & attributeStatuses);

    /**
     * @brief Processes a request to begin an atomic write operation
     * @param[in] commandObj The CommandHandler for the invoked AtomicRequest
     * @param[in] commandPath The ConcreteCommandPath for the invoked AtomicRequest
     * @param[in] commandData The DecodableType for the invoked AtomicRequest
     * @return Success if the atomic write operation was initiated, error code otherwise
     */
    std::optional<DataModel::ActionReturnStatus> BeginAtomicWrite(CommandHandler * commandObj,
                                                                  const ConcreteCommandPath & commandPath,
                                                                  const Commands::AtomicRequest::DecodableType & commandData);

    /**
     * @brief Processes a request to commit an atomic write operation
     * @param[in] commandObj The CommandHandler for the invoked AtomicRequest
     * @param[in] commandPath The ConcreteCommandPath for the invoked AtomicRequest
     * @param[in] commandData The DecodableType for the invoked AtomicRequest
     * @return Success if the atomic write operation was committed, error code otherwise
     */
    std::optional<DataModel::ActionReturnStatus> CommitAtomicWrite(CommandHandler * commandObj,
                                                                   const ConcreteCommandPath & commandPath,
                                                                   const Commands::AtomicRequest::DecodableType & commandData);

    /**
     * @brief Processes a request to rollback an atomic write operation
     * @param[in] commandObj The CommandHandler for the invoked AtomicRequest
     * @param[in] commandPath The ConcreteCommandPath for the AtomicRequest
     * @param[in] commandData The DecodableType for the invoked AtomicRequest
     * @return Success if the atomic write operation was rolled back, error code otherwise
     */
    std::optional<DataModel::ActionReturnStatus> RollbackAtomicWrite(CommandHandler * commandObj,
                                                                     const ConcreteCommandPath & commandPath,
                                                                     const Commands::AtomicRequest::DecodableType & commandData);
    /**
     * @brief Resets the atomic write session and cancels its timer
     */
    void ResetAtomicWrite();

    /**
     * @brief Checks if the thermostat cluster has an atomic write open, optionally filtered by an attribute ID
     *
     * @param attributeId The optional attribute ID to filter on
     * @return true if the endpoint has an open atomic write
     * @return false if the endpoint does not have an open atomic write
     */
    bool InAtomicWrite(std::optional<AttributeId> attributeId = std::nullopt);

    /**
     * @brief Checks if the thermostat cluster has an atomic write open, filtered by fabric index
     *
     * @param fabricIndex The fabric index to filter on
     * @return true if the endpoint has an open atomic write
     * @return false if the endpoint does not have an open atomic write
     */
    bool InAtomicWrite(FabricIndex fabricIndex);

    /**
     * @brief Checks if a thermostat cluster has an atomic write open for a given subject descriptor, optionally filtered by an
     * attribute ID
     *
     * @param subjectDescriptor The subject descriptor for the client making a read or write request
     * @param attributeId The optional attribute ID to filter on
     * @return true if the thermostat cluster has an open atomic write
     * @return false if the thermostat cluster does not have an open atomic write
     */
    bool InAtomicWrite(const Access::SubjectDescriptor & subjectDescriptor, std::optional<AttributeId> attributeId = std::nullopt);

    /**
     * @brief Checks if a thermostat cluster has an atomic write open for a given command invocation, optionally filtered by an
     * attribute ID
     *
     * @param commandObj The CommandHandler for the invoked command
     * @param attributeId The optional attribute ID to filter on
     * @return true if the thermostat cluster has an open atomic write
     * @return false if the thermostat cluster does not have an open atomic write
     */
    bool InAtomicWrite(CommandHandler * commandObj, std::optional<AttributeId> attributeId = std::nullopt);

    /**
     * @brief Checks if a thermostat cluster has an atomic write open for a given command invocation and a list of attributes
     *
     * @param commandObj The CommandHandler for the invoked command
     * @param attributeStatuses The list of attribute statuses whose attributeIds must match the open atomic write
     * @return true if the thermostat cluster has an open atomic write
     * @return false if the thermostat cluster does not have an open atomic write
     */
    bool InAtomicWrite(CommandHandler * commandObj, AtomicAttributes & attributeStatuses);

    void OnAtomicWriteTimeout();

private:
    State mState = State::Closed;
    Platform::ScopedMemoryBufferWithSize<AttributeId> mAttributeIds;
    ScopedNodeId mNodeId;

    Delegate & mDelegate;

    Protocols::InteractionModel::Status
    ExecuteAtomicAction(AtomicAttributes & attributeStatuses,
                        Protocols::InteractionModel::Status (Delegate::*action)(chip::AttributeId));
    Protocols::InteractionModel::Status
    ExecuteAtomicAction(AtomicAttributes & attributeStatuses,
                        Protocols::InteractionModel::Status (Delegate::*action)(chip::AttributeId),
                        std::optional<Protocols::InteractionModel::Status> statusOverride);

    /// @brief Builds the list of attribute statuses to return from an AtomicRequest invocation
    /// @param attributeRequests The list of requested attributes
    /// @param attributeStatusCount The number of attribute statuses in attributeStatuses
    /// @param attributeStatuses The status of each requested attribute, plus additional attributes if needed
    /// @return Status::Success if the request is valid, an error status if it is not
    Protocols::InteractionModel::Status BuildAttributeStatuses(const DataModel::DecodableList<chip::AttributeId> attributeRequests,
                                                               AtomicAttributes & attributeStatuses);
};

} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip

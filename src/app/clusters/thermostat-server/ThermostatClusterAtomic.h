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

#include <app/CommandHandler.h>
#include <app/ConcreteAttributePath.h>
#include <app/ConcreteCommandPath.h>
#include <app/persistence/AttributePersistence.h>
#include <app/util/attribute-storage.h>
#include <protocols/interaction_model/Constants.h>

namespace chip {
namespace app {
namespace Clusters {
namespace Thermostat {

enum class AtomicWriteState
{
    Closed = 0,
    Open,
};

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
    };

    /**
     * @brief Sets the atomic write state for the given originatorNodeId
     *
     * @param[in] originatorNodeId The originator scoped node id.
     * @param[in] state Whether or not an atomic write is open or closed.
     * @param attributeStatuses The set of attribute status structs the atomic write should be associated with
     * @return true if it was able to update the atomic write state
     * @return false if it was unable to update the atomic write state
     */
    bool
    SetAtomicWrite(ScopedNodeId originatorNodeId, AtomicWriteState state,
                   Platform::ScopedMemoryBufferWithSize<Globals::Structs::AtomicAttributeStatusStruct::Type> & attributeStatuses);

    std::optional<DataModel::ActionReturnStatus> BeginAtomicWrite(CommandHandler * commandObj,
                                                                  const ConcreteCommandPath & commandPath,
                                                                  const Commands::AtomicRequest::DecodableType & commandData);
    std::optional<DataModel::ActionReturnStatus> CommitAtomicWrite(CommandHandler * commandObj,
                                                                   const ConcreteCommandPath & commandPath,
                                                                   const Commands::AtomicRequest::DecodableType & commandData);
    std::optional<DataModel::ActionReturnStatus> RollbackAtomicWrite(CommandHandler * commandObj,
                                                                     const ConcreteCommandPath & commandPath,
                                                                     const Commands::AtomicRequest::DecodableType & commandData);
    /**
     * @brief Resets the atomic write for a given endpoint
     */
    void ResetAtomicWrite();

    /**
     * @brief Checks if the thermostat cluster has an atomic write open, optionally filtered by an attribute ID
     *
     * @param attributeId The optional attribute ID to filter on
     * @return true if the endpoint has an open atomic write
     * @return false if the endpoint does not have an open atomic write
     */
    bool InAtomicWrite(Optional<AttributeId> attributeId = NullOptional);

    /**
     * @brief Checks if a thermostat cluster has an atomic write open for a given subject descriptor, optionally filtered by an
     * attribute ID
     *
     * @param subjectDescriptor The subject descriptor for the client making a read or write request
     * @param attributeId The optional attribute ID to filter on
     * @return true if the thermostat cluster has an open atomic write
     * @return false if the thermostat cluster does not have an open atomic write
     */
    bool InAtomicWrite(const Access::SubjectDescriptor & subjectDescriptor, Optional<AttributeId> attributeId = NullOptional);

    /**
     * @brief Checks if a thermostat cluster has an atomic write open for a given command invocation, optionally filtered by an
     * attribute ID
     *
     * @param commandObj The CommandHandler for the invoked command
     * @param attributeId The optional attribute ID to filter on
     * @return true if the thermostat cluster has an open atomic write
     * @return false if the thermostat cluster does not have an open atomic write
     */
    bool InAtomicWrite(CommandHandler * commandObj, Optional<AttributeId> attributeId = NullOptional);

    /**
     * @brief Checks if a thermostat cluster has an atomic write open for a given command invocation and a list of attributes
     *
     * @param commandObj The CommandHandler for the invoked command
     * @param attributeStatuses The list of attribute statuses whose attributeIds must match the open atomic write
     * @return true if the thermostat cluster has an open atomic write
     * @return false if the thermostat cluster does not have an open atomic write
     */
    bool
    InAtomicWrite(CommandHandler * commandObj,
                  Platform::ScopedMemoryBufferWithSize<Globals::Structs::AtomicAttributeStatusStruct::Type> & attributeStatuses);

    void OnAtomicWriteTimeout();

    void SetDelegate(Delegate * delegate) { mDelegate = delegate; }

private:
    AtomicWriteState mWriteState = AtomicWriteState::Closed;
    Platform::ScopedMemoryBufferWithSize<AttributeId> mAttributeIds;
    ScopedNodeId mNodeId;

    Delegate * mDelegate = nullptr;
};

} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip

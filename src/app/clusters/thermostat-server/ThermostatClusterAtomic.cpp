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

#include "ThermostatClusterAtomic.h"

#include <app/GlobalAttributes.h>
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <functional>

using namespace chip::app::Clusters::Globals::Structs;
using namespace chip::app::Clusters::Thermostat::Attributes;
using namespace chip::app::Clusters::Thermostat::Structs;
using namespace chip::Protocols::InteractionModel;

namespace chip {
namespace app {
namespace Clusters {
namespace Thermostat {

namespace {
/**
 * @brief Get the source scoped node id.
 *
 * @param[in] commandObj The command handler object.
 *
 * @return The scoped node id of the source node. If the scoped node id is not retrieved, return ScopedNodeId().
 */
ScopedNodeId GetSourceScopedNodeId(CommandHandler * commandObj)
{
    if (commandObj == nullptr)
    {
        return ScopedNodeId();
    }

    if (auto * ec = commandObj->GetExchangeContext())
    {
        auto sessionHandle = ec->GetSessionHandle();
        if (sessionHandle->IsSecureSession())
        {
            return sessionHandle->AsSecureSession()->GetPeer();
        }
        if (sessionHandle->IsGroupSession())
        {
            return sessionHandle->AsIncomingGroupSession()->GetPeer();
        }
        return ScopedNodeId();
    }

    auto subjectDescriptor = commandObj->GetSubjectDescriptor();
    return ScopedNodeId(subjectDescriptor.subject, subjectDescriptor.fabricIndex);
}

/**
 * @brief Counts the number of attribute requests
 *
 * @param attributeRequests The decodable list of attribute IDs
 * @param attributeRequestCount The total number of attribute requests
 * @return true if the attribute list was counted
 * @return false if there was an error reading the list
 */
bool CountAttributeRequests(const DataModel::DecodableList<chip::AttributeId> attributeRequests, size_t & attributeRequestCount)
{
    attributeRequestCount = 0;
    auto attributeIdsIter = attributeRequests.begin();
    while (attributeIdsIter.Next())
    {
        attributeRequestCount++;
    }
    return attributeIdsIter.GetStatus() == CHIP_NO_ERROR;
}

} // anonymous namespace

Status AtomicWriteSession::ExecuteAtomicAction(AtomicAttributes & attributeStatuses, Status (Delegate::*action)(AttributeId),
                                               std::optional<Status> statusOverride)
{
    Status status = Status::Success;
    for (size_t i = 0; i < attributeStatuses.AllocatedSize(); ++i)
    {
        auto & attributeStatus = attributeStatuses[i];
        auto actionStatus      = std::invoke(action, mDelegate, attributeStatus.attributeID);

        attributeStatus.statusCode = to_underlying(statusOverride.value_or(actionStatus));
        if (actionStatus != Status::Success)
        {
            status = Status::Failure;
        }
    }
    return status;
}

void AtomicWriteSession::Startup()
{
    if (auto status = mFabricTable.AddFabricDelegate(this); status != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "Failed to add fabric delegate to Thermostat Cluster");
    }
}

void AtomicWriteSession::Shutdown()
{
    ResetAtomicWrite();
    mFabricTable.RemoveFabricDelegate(this);
}

bool AtomicWriteSession::InAtomicWrite(std::optional<AttributeId> attributeId)
{

    if (mState != State::Open)
    {
        return false;
    }
    if (!attributeId.has_value())
    {
        return true;
    }
    for (size_t i = 0; i < mAttributeIds.AllocatedSize(); ++i)
    {
        if (mAttributeIds[i] == attributeId.value())
        {
            return true;
        }
    }
    return false;
}

bool AtomicWriteSession::InAtomicWrite(FabricIndex fabricIndex)
{
    if (mState != State::Open)
    {
        return false;
    }
    return mNodeId.GetFabricIndex() == fabricIndex;
}

bool AtomicWriteSession::InAtomicWrite(const Access::SubjectDescriptor & subjectDescriptor, std::optional<AttributeId> attributeId)
{
    if (!InAtomicWrite(attributeId))
    {
        return false;
    }
    return subjectDescriptor.authMode == Access::AuthMode::kCase &&
        mNodeId == ScopedNodeId(subjectDescriptor.subject, subjectDescriptor.fabricIndex);
}

bool AtomicWriteSession::InAtomicWrite(CommandHandler * commandObj, std::optional<AttributeId> attributeId)
{
    if (!InAtomicWrite(attributeId))
    {
        return false;
    }
    ScopedNodeId sourceNodeId = GetSourceScopedNodeId(commandObj);
    return mNodeId == sourceNodeId;
}

bool AtomicWriteSession::InAtomicWrite(CommandHandler * commandObj, AtomicAttributes & attributeStatuses)
{

    if (mState != State::Open)
    {
        return false;
    }
    if (mAttributeIds.AllocatedSize() == 0 || mAttributeIds.AllocatedSize() != attributeStatuses.AllocatedSize())
    {
        return false;
    }
    for (size_t i = 0; i < mAttributeIds.AllocatedSize(); ++i)
    {
        bool hasAttribute = false;
        auto attributeId  = mAttributeIds[i];
        for (size_t j = 0; j < attributeStatuses.AllocatedSize(); ++j)
        {
            auto & attributeStatus = attributeStatuses[j];
            if (attributeStatus.attributeID == attributeId)
            {
                hasAttribute = true;
                break;
            }
        }
        if (!hasAttribute)
        {
            return false;
        }
    }
    return true;
}

bool AtomicWriteSession::SetAtomicWrite(ScopedNodeId originatorNodeId, State state,
                                        Platform::ScopedMemoryBufferWithSize<AtomicAttributeStatusStruct::Type> & attributeStatuses)
{

    if (!mAttributeIds.Alloc(attributeStatuses.AllocatedSize()))
    {
        mState  = State::Closed;
        mNodeId = ScopedNodeId();
        return false;
    }

    mState  = state;
    mNodeId = originatorNodeId;

    for (size_t i = 0; i < attributeStatuses.AllocatedSize(); ++i)
    {
        mAttributeIds[i] = attributeStatuses[i].attributeID;
    }
    return true;
}

void AtomicWriteSession::ResetAtomicWrite()
{
    mTimerDelegate.CancelTimer(this);

    mState  = State::Closed;
    mNodeId = ScopedNodeId();
    mAttributeIds.Free();
}

void SendAtomicResponse(CommandHandler * commandObj, const ConcreteCommandPath & commandPath, Status status,
                        const Platform::ScopedMemoryBufferWithSize<AtomicAttributeStatusStruct::Type> & attributeStatuses,
                        Optional<uint16_t> timeout = NullOptional)
{
    Commands::AtomicResponse::Type response;
    response.statusCode = to_underlying(status);
    response.attributeStatus =
        DataModel::List<const AtomicAttributeStatusStruct::Type>(attributeStatuses.Get(), attributeStatuses.AllocatedSize());
    response.timeout = timeout;
    commandObj->AddResponse(commandPath, response);
}

std::optional<DataModel::ActionReturnStatus>
AtomicWriteSession::BeginAtomicWrite(CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                                     const Commands::AtomicRequest::DecodableType & commandData)
{
    Platform::ScopedMemoryBufferWithSize<AtomicAttributeStatusStruct::Type> attributeStatuses;
    auto status = BuildAttributeStatuses(commandData.attributeRequests, attributeStatuses);
    if (status != Status::Success)
    {
        return status;
    }

    if (InAtomicWrite(commandObj))
    {
        // This client already has an open atomic write
        return Status::InvalidInState;
    }

    if (!commandData.timeout.HasValue())
    {
        return Status::InvalidCommand;
    }

    auto maximumTimeout   = System::Clock::Milliseconds16(0);
    auto attributeIdsIter = commandData.attributeRequests.begin();
    while (attributeIdsIter.Next())
    {
        auto & attributeId = attributeIdsIter.GetValue();
        switch (attributeId)
        {
        case Presets::Id:
        case Schedules::Id:
            auto attributeTimeout = mDelegate.GetMaxAtomicWriteTimeout(attributeId);

            if (attributeTimeout.has_value())
            {
                // Add to the maximum timeout
                maximumTimeout += attributeTimeout.value();
            }
            break;
        }
    }

    status = Status::Success;
    for (size_t i = 0; i < attributeStatuses.AllocatedSize(); ++i)
    {
        // If we've gotten this far, then the client has manage permission to call AtomicRequest,
        // which is also the privilege necessary to write to the atomic attributes, so no need to do
        // the "If the client does not have sufficient privilege to write to the attribute" check
        // from the spec.
        auto & attributeStatus = attributeStatuses[i];
        auto statusCode        = Status::Success;
        switch (attributeStatus.attributeID)
        {
        case Presets::Id:
        case Schedules::Id:
            statusCode = InAtomicWrite(std::make_optional(attributeStatus.attributeID)) ? Status::Busy : Status::Success;
            break;
        default:
            statusCode = Status::InvalidCommand;
            break;
        }
        if (statusCode != Status::Success)
        {
            status = Status::Failure;
        }
        attributeStatus.statusCode = to_underlying(statusCode);
    }

    auto timeout = std::min(System::Clock::Milliseconds16(commandData.timeout.Value()), maximumTimeout);

    if (status == Status::Success)
    {
        if (!SetAtomicWrite(GetSourceScopedNodeId(commandObj), State::Open, attributeStatuses))
        {
            for (size_t i = 0; i < attributeStatuses.AllocatedSize(); ++i)
            {
                attributeStatuses[i].statusCode = to_underlying(Status::ResourceExhausted);
            }
            status = Status::Failure;
        }
        else
        {
            // This is a valid request to open an atomic write. Tell the delegate it
            // needs to keep track of a pending preset list now.
            status = ExecuteAtomicAction(attributeStatuses, &Delegate::OnAtomicWriteBegin);
            if (status != Status::Success || mTimerDelegate.StartTimer(this, timeout) != CHIP_NO_ERROR)
            {
                ExecuteAtomicAction(attributeStatuses, &Delegate::OnAtomicWriteRollback, Status::Failure);
                ResetAtomicWrite();
                status = Status::Failure;
            }
        }
    }

    SendAtomicResponse(commandObj, commandPath, status, attributeStatuses, MakeOptional(timeout.count()));
    return std::nullopt;
}

std::optional<DataModel::ActionReturnStatus>
AtomicWriteSession::CommitAtomicWrite(CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                                      const Commands::AtomicRequest::DecodableType & commandData)
{
    Platform::ScopedMemoryBufferWithSize<AtomicAttributeStatusStruct::Type> attributeStatuses;
    auto status = BuildAttributeStatuses(commandData.attributeRequests, attributeStatuses);
    if (status != Status::Success)
    {
        return status;
    }

    if (!InAtomicWrite(commandObj, attributeStatuses))
    {
        return Status::InvalidInState;
    }

    status = ExecuteAtomicAction(attributeStatuses, &Delegate::OnAtomicWritePrecommit);

    if (status == Status::Success)
    {
        status = ExecuteAtomicAction(attributeStatuses, &Delegate::OnAtomicWriteCommit);
    }

    ResetAtomicWrite();
    SendAtomicResponse(commandObj, commandPath, status, attributeStatuses);
    return std::nullopt;
}

std::optional<DataModel::ActionReturnStatus>
AtomicWriteSession::RollbackAtomicWrite(CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                                        const Commands::AtomicRequest::DecodableType & commandData)
{
    Platform::ScopedMemoryBufferWithSize<AtomicAttributeStatusStruct::Type> attributeStatuses;
    auto status = BuildAttributeStatuses(commandData.attributeRequests, attributeStatuses);
    if (status != Status::Success)
    {
        return status;
    }

    if (!InAtomicWrite(commandObj, attributeStatuses))
    {
        // There's no open atomic write
        return Status::InvalidInState;
    }

    ResetAtomicWrite();

    status = ExecuteAtomicAction(attributeStatuses, &Delegate::OnAtomicWriteRollback);

    SendAtomicResponse(commandObj, commandPath, status, attributeStatuses);
    return std::nullopt;
}

void AtomicWriteSession::Rollback()
{
    for (size_t i = 0; i < mAttributeIds.AllocatedSize(); ++i)
    {
        mDelegate.OnAtomicWriteRollback(mAttributeIds[i]);
    }
    ResetAtomicWrite();
}

void AtomicWriteSession::TimerFired()
{
    Rollback();
}

/// @brief Builds the list of attribute statuses to return from an AtomicRequest invocation
/// @param attributeRequests The list of requested attributes
/// @param attributeStatusCount The number of attribute statuses in attributeStatuses
/// @param attributeStatuses The status of each requested attribute, plus additional attributes if needed
/// @return Status::Success if the request is valid, an error status if it is not
Status AtomicWriteSession::BuildAttributeStatuses(
    const DataModel::DecodableList<chip::AttributeId> attributeRequests,
    Platform::ScopedMemoryBufferWithSize<AtomicAttributeStatusStruct::Type> & attributeStatuses)
{
    size_t attributeStatusCount = 0;
    if (!CountAttributeRequests(attributeRequests, attributeStatusCount))
    {
        // We errored reading the list
        return Status::InvalidCommand;
    }
    if (attributeStatusCount == 0)
    {
        // List can't be empty
        return Status::InvalidCommand;
    }
    if (!attributeStatuses.Alloc(attributeStatusCount))
    {
        return Status::ResourceExhausted;
    }
    for (size_t i = 0; i < attributeStatusCount; ++i)
    {
        attributeStatuses[i].attributeID = kInvalidAttributeId;
        attributeStatuses[i].statusCode  = 0;
    }
    auto attributeIdsIter = attributeRequests.begin();
    size_t index          = 0;
    while (attributeIdsIter.Next())
    {
        auto & attributeId = attributeIdsIter.GetValue();

        for (size_t i = 0; i < index; ++i)
        {
            auto & attributeStatus = attributeStatuses[i];
            if (attributeStatus.attributeID == attributeId)
            {
                // Double-requesting an attribute is invalid
                return Status::InvalidCommand;
            }
        }
        attributeStatuses[index].attributeID = attributeId;
        attributeStatuses[index].statusCode  = to_underlying(Status::Success);
        index++;
    }
    if (attributeIdsIter.GetStatus() != CHIP_NO_ERROR)
    {
        return Status::InvalidCommand;
    }
    for (size_t i = 0; i < index; ++i)
    {
        auto & attributeStatus = attributeStatuses[i];
        if (mDelegate.HasAttribute(attributeStatus.attributeID))
        {
            // This is definitely an attribute we know about.
            continue;
        }

        if (IsSupportedGlobalAttributeNotInMetadata(attributeStatus.attributeID))
        {
            continue;
        }

        // This is not a valid attribute on the Thermostat cluster on the supplied endpoint
        return Status::InvalidCommand;
    }
    return Status::Success;
}

std::optional<DataModel::ActionReturnStatus> AtomicWriteSession::InvokeCommand(const DataModel::InvokeRequest & request,
                                                                               TLV::TLVReader & input_arguments,
                                                                               CommandHandler * handler, bool & handled)
{

    switch (request.path.mCommandId)
    {
    case Commands::AtomicRequest::Id: {
        handled = true;
        Commands::AtomicRequest::DecodableType request_data;
        ReturnErrorOnFailure(request_data.Decode(input_arguments));

        switch (request_data.requestType)
        {
        case Globals::AtomicRequestTypeEnum::kBeginWrite:
            return BeginAtomicWrite(handler, request.path, request_data);
        case Globals::AtomicRequestTypeEnum::kCommitWrite:
            return CommitAtomicWrite(handler, request.path, request_data);
        case Globals::AtomicRequestTypeEnum::kRollbackWrite:
            return RollbackAtomicWrite(handler, request.path, request_data);
        default:
            return Protocols::InteractionModel::Status::InvalidCommand;
        }
    }
    default:
        return std::nullopt;
    }
}

void AtomicWriteSession::OnFabricRemoved(const FabricTable & fabricTable, FabricIndex fabricIndex)
{
    if (InAtomicWrite(fabricIndex))
    {
        Rollback();
    }
}

} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip

/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
 *    All rights reserved.
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

#include <atomic-write-manager.h>

#include <platform/internal/CHIPDeviceLayerInternal.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters::Globals::Structs;
using namespace chip::app::Clusters::Thermostat;
using namespace chip::app::Clusters::Thermostat::Attributes;
using namespace chip::Protocols::InteractionModel;

namespace chip {
namespace app {
namespace Clusters {
namespace Thermostat {

ThermostatAtomicWriteManager ThermostatAtomicWriteManager::sInstance;

/**
 * @brief Callback that is called when the timeout for editing the presets expires.
 *
 * @param[in] systemLayer The system layer.
 * @param[in] callbackContext The context passed to the timer callback.
 */
void TimerExpiredCallback(System::Layer * systemLayer, void * callbackContext)
{
    EndpointId endpoint = static_cast<EndpointId>(reinterpret_cast<uintptr_t>(callbackContext));
    ThermostatAtomicWriteManager::GetInstance().OnTimerExpired(endpoint);
}

/**
 * @brief Counts the number of attribute requests
 *
 * @param commandData
 * @param attributeRequestCount
 * @return true if the attribute list was counted
 * @return false if there was an error reading the list
 */
bool countAttributeRequests(const DataModel::DecodableList<chip::AttributeId> attributeRequests, size_t & attributeRequestCount)
{
    attributeRequestCount = 0;
    auto attributeIdsIter = attributeRequests.begin();
    while (attributeIdsIter.Next())
    {
        attributeRequestCount++;
    }
    return attributeIdsIter.GetStatus() == CHIP_NO_ERROR;
}

/**
 * @brief Builds the vector of attribute statuses to return from a successful AtomicRequest invocation
 *
 * @param endpoint The associated endpoint for the AtomicRequest invocation
 * @param attributeRequests The list of requested attributes
 * @param attributeRequestCount The number of attribute requests
 * @param attributeStatuses The vector of attribute statuses to populate
 * @return true if all requested attributes are valid attributes on the associated cluster, there is at least one requested
 * attribute and there are no duplicate attributes
 * @return false otherwise
 */
bool buildAttributeStatuses(const EndpointId endpoint, const DataModel::DecodableList<chip::AttributeId> attributeRequests,
                            size_t & attributeRequestCount, std::vector<AtomicAttributeStatusStruct::Type> & attributeStatuses)
{
    if (attributeRequestCount == 0)
    {
        return false;
    }
    attributeStatuses.reserve(attributeRequestCount);
    auto attributeIdsIter = attributeRequests.begin();
    while (attributeIdsIter.Next())
    {
        auto & attributeId = attributeIdsIter.GetValue();

        for (auto & attributeStatus : attributeStatuses)
        {
            if (attributeStatus.attributeID == attributeId)
            {
                // Double-requesting an attribute is invalid
                return false;
            }
        }

        attributeStatuses.push_back({ .attributeID = attributeId, .statusCode = to_underlying(Status::Success) });
    }
    if (attributeIdsIter.GetStatus() != CHIP_NO_ERROR)
    {
        return false;
    }
    for (auto & attributeStatus : attributeStatuses)
    {
        const EmberAfAttributeMetadata * metadata =
            emberAfLocateAttributeMetadata(endpoint, Thermostat::Id, attributeStatus.attributeID);

        if (metadata == nullptr)
        {
            // This is not a valid attribute on the Thermostat cluster on the supplied endpoint
            return false;
        }
    }
    return true;
}

/**
 * @brief Validates an atomic request to either commit or rollback, and builds the attribute status list
 *
 * @param endpoint The associated endpoint for the AtomicRequest invocation
 * @param attributeRequests The list of requested attributes
 * @param attributeRequestCount The number of attribute requests
 * @param attributeStatuses The vector of attribute statuses to populate
 * @return Status::Success if the request is valid, an error code otherwise
 */
Status validateCommitOrRollbackRequest(const EndpointId endpoint,
                                       const DataModel::DecodableList<chip::AttributeId> attributeRequests,
                                       std::vector<AtomicAttributeStatusStruct::Type> & attributeStatuses)
{
    size_t attributeRequestCount = 0;
    if (!countAttributeRequests(attributeRequests, attributeRequestCount))
    {
        return Status::InvalidCommand;
    }

    if (!buildAttributeStatuses(endpoint, attributeRequests, attributeRequestCount, attributeStatuses))
    {
        return Status::InvalidCommand;
    }

    bool requestedPresets = false, requestedSchedules = false;

    for (auto & attributeStatus : attributeStatuses)
    {

        switch (attributeStatus.attributeID)
        {
        case Presets::Id:
            requestedPresets = true;
            break;
        case Schedules::Id:
            requestedSchedules = true;
            break;
        default:
            // Trying to commit or rollback a non-atomic attribute
            return Status::InvalidInState;
        }
    }

    if (!requestedPresets || !requestedSchedules)
    {
        // The client needs to request both presets and schedules in this implementation
        return Status::InvalidInState;
    }

    return Status::Success;
}

ScopedNodeId GetSourceScopedNodeId(CommandHandler * commandObj)
{
    ScopedNodeId sourceNodeId = ScopedNodeId();
    auto sessionHandle        = commandObj->GetExchangeContext()->GetSessionHandle();

    if (sessionHandle->IsSecureSession())
    {
        sourceNodeId = sessionHandle->AsSecureSession()->GetPeer();
    }
    else if (sessionHandle->IsGroupSession())
    {
        sourceNodeId = sessionHandle->AsIncomingGroupSession()->GetPeer();
    }
    return sourceNodeId;
}

/**
 * @brief Schedules a timer for the given timeout in milliseconds.
 *
 * @param[in] endpoint The endpoint to use.
 * @param[in] timeout The timeout in milliseconds.
 */
void ThermostatAtomicWriteManager::ScheduleTimer(EndpointId endpoint, System::Clock::Milliseconds16 timeout)
{
    DeviceLayer::SystemLayer().StartTimer(timeout, TimerExpiredCallback,
                                          reinterpret_cast<void *>(static_cast<uintptr_t>(endpoint)));
}

/**
 * @brief Clears the currently scheduled timer.
 *
 * @param[in] endpoint The endpoint to use.
 */
void ThermostatAtomicWriteManager::ClearTimer(EndpointId endpoint)
{
    DeviceLayer::SystemLayer().CancelTimer(TimerExpiredCallback, reinterpret_cast<void *>(static_cast<uintptr_t>(endpoint)));
}

void ThermostatAtomicWriteManager::OnTimerExpired(EndpointId endpoint)
{
    for (size_t i = 0; i < ArraySize(mAtomicWriteSessions); ++i)
    {

        auto atomicWriteState = mAtomicWriteSessions[i];
        if (atomicWriteState.state == AtomicWriteState::Open && atomicWriteState.endpointId == endpoint)
        {
            if (mDelegate != nullptr)
            {
                mDelegate->OnRollbackWrite(endpoint, Presets::Id);
                mDelegate->OnRollbackWrite(endpoint, Schedules::Id);
            }
            ResetWrite(endpoint);
        }
    }
}

void ThermostatAtomicWriteManager::SetWriteState(EndpointId endpoint, ScopedNodeId originatorNodeId, AtomicWriteState state)
{
    uint16_t ep =
        emberAfGetClusterServerEndpointIndex(endpoint, Thermostat::Id, MATTER_DM_THERMOSTAT_CLUSTER_SERVER_ENDPOINT_COUNT);

    if (ep < ArraySize(mAtomicWriteSessions))
    {
        mAtomicWriteSessions[ep].state      = state;
        mAtomicWriteSessions[ep].endpointId = endpoint;
        mAtomicWriteSessions[ep].nodeId     = originatorNodeId;
    }
}

bool ThermostatAtomicWriteManager::InWrite(const std::optional<AttributeId> attributeId, EndpointId endpoint)
{
    // These are the only two atomic attributes
    if (attributeId.has_value() && attributeId.value() != Presets::Id && attributeId.value() != Schedules::Id)
    {
        return false;
    }

    uint16_t ep =
        emberAfGetClusterServerEndpointIndex(endpoint, Thermostat::Id, MATTER_DM_THERMOSTAT_CLUSTER_SERVER_ENDPOINT_COUNT);

    if (ep < ArraySize(mAtomicWriteSessions))
    {
        return mAtomicWriteSessions[ep].state == AtomicWriteState::Open;
    }
    return false;
}

bool ThermostatAtomicWriteManager::InWrite(const std::optional<AttributeId> attributeId,
                                           const Access::SubjectDescriptor & subjectDescriptor, EndpointId endpoint)
{
    if (!InWrite(attributeId, endpoint))
    {
        return false;
    }
    return subjectDescriptor.authMode == Access::AuthMode::kCase &&
        GetAtomicWriteOriginatorScopedNodeId(attributeId, endpoint) ==
        ScopedNodeId(subjectDescriptor.subject, subjectDescriptor.fabricIndex);
}

bool ThermostatAtomicWriteManager::InWrite(const std::optional<AttributeId> attributeId, CommandHandler * commandObj,
                                           EndpointId endpoint)
{
    if (!InWrite(attributeId, endpoint))
    {
        return false;
    }
    ScopedNodeId sourceNodeId = GetSourceScopedNodeId(commandObj);
    return GetAtomicWriteOriginatorScopedNodeId(attributeId, endpoint) == sourceNodeId;
}

bool ThermostatAtomicWriteManager::BeginWrite(chip::app::CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                                              const Commands::AtomicRequest::DecodableType & commandData)
{
    if (mDelegate == nullptr)
    {
        commandObj->AddStatus(commandPath, Status::InvalidInState);
        return true;
    }

    EndpointId endpoint = commandPath.mEndpointId;

    size_t attributeRequestCount = 0;
    if (!countAttributeRequests(commandData.attributeRequests, attributeRequestCount))
    {
        return false;
    }

    std::vector<AtomicAttributeStatusStruct::Type> attributeStatuses;
    if (!buildAttributeStatuses(endpoint, commandData.attributeRequests, attributeRequestCount, attributeStatuses))
    {
        return false;
    }

    // Check if the client already has any open atomic writes on this cluster
    if (InWrite(std::nullopt, commandObj, endpoint))
    {
        commandObj->AddStatus(commandPath, Status::InvalidInState);
        return true;
    }

    if (!commandData.timeout.HasValue())
    {
        return false;
    }

    bool requestedPresets = false, requestedSchedules = false;
    auto maximumTimeout = System::Clock::Milliseconds16(0);

    for (auto & attributeStatus : attributeStatuses)
    {

        switch (attributeStatus.attributeID)
        {
        case Presets::Id:
            requestedPresets = true;
            break;
        case Schedules::Id:
            requestedSchedules = true;
            break;
        }
        auto attributeTimeout = mDelegate->GetWriteTimeout(endpoint, attributeStatus.attributeID);

        if (attributeTimeout.has_value())
        {
            // Add to the maximum timeout
            maximumTimeout += attributeTimeout.value();
        }
    }

    // This atomic write manager forces both presets and schedules to be locked simultaneously
    if (!requestedPresets)
    {
        attributeStatuses.push_back({ .attributeID = Presets::Id, .statusCode = to_underlying(Status::Success) });
    }

    if (!requestedSchedules)
    {
        attributeStatuses.push_back({ .attributeID = Schedules::Id, .statusCode = to_underlying(Status::Success) });
    }

    auto status = Status::Success;

    for (auto & attributeStatus : attributeStatuses)
    {

        Status statusCode;
        switch (attributeStatus.attributeID)
        {
        case Presets::Id:
        case Schedules::Id:
            if (InWrite(attributeStatus.attributeID, endpoint))
            {
                // There's an existing atomic write that doesn't belong to us (if it did, we'd have failed above)
                statusCode = Status::Busy;
            }
            else
            {
                statusCode = mDelegate->OnBeginWrite(endpoint, attributeStatus.attributeID);
            }

            break;
        default:
            // All other attributes are non-atomic
            // TODO: This should be in EmberAfAttributeMetadata
            statusCode = Status::InvalidCommand;
            break;
        }
        if (statusCode != Status::Success)
        {
            status = Status::Failure;
        }
        attributeStatus.statusCode = to_underlying(statusCode);
    }

    Commands::AtomicResponse::Type response;
    response.statusCode = to_underlying(status);
    response.attributeStatus =
        DataModel::List<const AtomicAttributeStatusStruct::Type>(attributeStatuses.data(), attributeStatuses.size());
    if (status == Status::Success)
    {
        auto timeoutRequest = System::Clock::Milliseconds16(commandData.timeout.Value());
        SetWriteState(endpoint, GetSourceScopedNodeId(commandObj), AtomicWriteState::Open);
        // Take the smaller of the timeout requested by the client or the  maximum the server is willing to allow
        auto timeout     = std::min(timeoutRequest, maximumTimeout);
        response.timeout = MakeOptional(timeout.count());
        ScheduleTimer(endpoint, timeout);
    }
    commandObj->AddResponse(commandPath, response);
    return true;
}

bool ThermostatAtomicWriteManager::CommitWrite(chip::app::CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                                               const Commands::AtomicRequest::DecodableType & commandData)
{

    if (mDelegate == nullptr)
    {
        commandObj->AddStatus(commandPath, Status::InvalidInState);
        return true;
    }

    EndpointId endpoint = commandPath.mEndpointId;

    std::vector<AtomicAttributeStatusStruct::Type> attributeStatuses;
    auto status = validateCommitOrRollbackRequest(endpoint, commandData.attributeRequests, attributeStatuses);
    switch (status)
    {
    case Status::InvalidCommand:
        return false;
    case Status::Success:
        break;
    default:
        commandObj->AddStatus(commandPath, status);
        return true;
    }

    for (auto & attributeStatus : attributeStatuses)
    {
        if (!InWrite(attributeStatus.attributeID, commandObj, endpoint))
        {
            // There's no open atomic write for this attribute
            commandObj->AddStatus(commandPath, Status::InvalidInState);
            return true;
        };
    }

    status = Status::Success;
    for (auto & attributeStatus : attributeStatuses)
    {
        auto statusCode = mDelegate->OnPreCommitWrite(endpoint, attributeStatus.attributeID);
        if (statusCode != Status::Success)
        {
            status = Status::Failure;
        }

        attributeStatus.statusCode = to_underlying(statusCode);
    }

    if (status == Status::Success)
    {
        for (auto & attributeStatus : attributeStatuses)
        {
            auto statusCode = mDelegate->OnCommitWrite(endpoint, attributeStatus.attributeID);
            if (statusCode != Status::Success)
            {
                status = Status::Failure;
            }
            attributeStatus.statusCode = to_underlying(statusCode);
        }
    }
    if (status == Status::Failure)
    {
        // Either one of the calls to OnPreCommitWrite failed, or one of the calls to OnCommitWrite failed; in the former case,
        // discard any pending writes. Do the same for the latter, knowing that the server may be in an inconsistent state
        for (auto & attributeStatus : attributeStatuses)
        {
            mDelegate->OnRollbackWrite(endpoint, attributeStatus.attributeID);
        }
    }
    Commands::AtomicResponse::Type response;
    response.statusCode = to_underlying(status);
    response.attributeStatus =
        DataModel::List<const AtomicAttributeStatusStruct::Type>(attributeStatuses.data(), attributeStatuses.size());
    commandObj->AddResponse(commandPath, response);
    ResetWrite(endpoint);
    return true;
}

bool ThermostatAtomicWriteManager::RollbackWrite(chip::app::CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                                                 const Commands::AtomicRequest::DecodableType & commandData)
{
    if (mDelegate == nullptr)
    {
        commandObj->AddStatus(commandPath, Status::InvalidInState);
        return true;
    }

    EndpointId endpoint = commandPath.mEndpointId;

    std::vector<AtomicAttributeStatusStruct::Type> attributeStatuses;
    auto status = validateCommitOrRollbackRequest(endpoint, commandData.attributeRequests, attributeStatuses);
    switch (status)
    {
    case Status::InvalidCommand:
        return false;
    case Status::Success:
        break;
    default:
        commandObj->AddStatus(commandPath, status);
        return true;
    }

    for (auto & attributeStatus : attributeStatuses)
    {
        if (!InWrite(attributeStatus.attributeID, commandObj, endpoint))
        {
            // There's no open atomic write for this attribute
            commandObj->AddStatus(commandPath, Status::InvalidInState);
            return true;
        };
    }

    for (auto & attributeStatus : attributeStatuses)
    {
        auto statusCode = mDelegate->OnRollbackWrite(endpoint, attributeStatus.attributeID);
        if (statusCode != Status::Success)
        {
            status = Status::Failure;
        }
        attributeStatus.statusCode = to_underlying(statusCode);
    }
    Commands::AtomicResponse::Type response;
    response.statusCode = to_underlying(status);
    response.attributeStatus =
        DataModel::List<const AtomicAttributeStatusStruct::Type>(attributeStatuses.data(), attributeStatuses.size());
    commandObj->AddResponse(commandPath, response);
    ResetWrite(endpoint);
    return true;
}

void ThermostatAtomicWriteManager::ResetWrite(EndpointId endpoint)
{
    ClearTimer(endpoint);
    SetWriteState(endpoint, ScopedNodeId(), AtomicWriteState::Closed);
}

void ThermostatAtomicWriteManager::ResetWrite(FabricIndex fabricIndex)
{
    for (size_t i = 0; i < ArraySize(mAtomicWriteSessions); ++i)
    {
        auto atomicWriteState = mAtomicWriteSessions[i];
        if (atomicWriteState.state == AtomicWriteState::Open && atomicWriteState.nodeId.GetFabricIndex() == fabricIndex)
        {
            ResetWrite(atomicWriteState.endpointId);
        }
    }
}

ScopedNodeId ThermostatAtomicWriteManager::GetAtomicWriteOriginatorScopedNodeId(const std::optional<AttributeId> attributeId,
                                                                                const EndpointId endpoint)
{
    ScopedNodeId originatorNodeId = ScopedNodeId();
    uint16_t ep =
        emberAfGetClusterServerEndpointIndex(endpoint, Thermostat::Id, MATTER_DM_THERMOSTAT_CLUSTER_SERVER_ENDPOINT_COUNT);

    if (ep < ArraySize(mAtomicWriteSessions))
    {
        originatorNodeId = mAtomicWriteSessions[ep].nodeId;
    }
    return originatorNodeId;
}

} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip

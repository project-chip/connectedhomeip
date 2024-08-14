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

Status countAttributeRequests(const Commands::AtomicRequest::DecodableType & commandData, size_t & attributeRequestCount)
{
    attributeRequestCount = 0;
    auto attributeIdsIter = commandData.attributeRequests.begin();
    while (attributeIdsIter.Next())
    {
        attributeRequestCount++;
    }
    if (attributeIdsIter.GetStatus() != CHIP_NO_ERROR)
    {
        return Status::InvalidCommand;
    }
    return Status::Success;
}

Status validateAttributeRequests(const Commands::AtomicRequest::DecodableType & commandData, size_t & attributeRequestCount)
{
    attributeRequestCount = 0;
    bool requestedPresets = false, requestedSchedules = false;
    auto attributeIdsIter = commandData.attributeRequests.begin();
    while (attributeIdsIter.Next())
    {
        auto & attributeId = attributeIdsIter.GetValue();
        attributeRequestCount++;
        switch (attributeId)
        {
        case Presets::Id:
            if (requestedPresets) // Double-requesting an attribute is invalid
            {
                return Status::InvalidCommand;
            }
            requestedPresets = true;
            break;
        case Schedules::Id:
            if (requestedSchedules) // Double-requesting an attribute is invalid
            {
                return Status::InvalidCommand;
            }
            requestedSchedules = true;
            break;
        default:
            // TODO: If this is a valid attribute on thermostat, but just isn't atomic, we shouldn't return an error code here
            return Status::InvalidCommand;
        }
    }
    if (attributeIdsIter.GetStatus() != CHIP_NO_ERROR)
    {
        return Status::InvalidCommand;
    }
    if (requestedPresets && requestedSchedules)
    {
        return Status::Success;
    }
    return Status::InvalidCommand;
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
 * @param[in] timeoutMilliseconds The timeout in milliseconds.
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

    bool inAtomicWrite = false;
    uint16_t ep =
        emberAfGetClusterServerEndpointIndex(endpoint, Thermostat::Id, MATTER_DM_THERMOSTAT_CLUSTER_SERVER_ENDPOINT_COUNT);

    if (ep < ArraySize(mAtomicWriteSessions))
    {
        inAtomicWrite = mAtomicWriteSessions[ep].state == AtomicWriteState::Open;
    }
    return inAtomicWrite;
}

bool ThermostatAtomicWriteManager::InWrite(const std::optional<AttributeId> attributeId,
                                           const Access::SubjectDescriptor & subjectDescriptor, EndpointId endpoint)
{
    if (!InWrite(attributeId, endpoint))
    {
        return false;
    }
    return subjectDescriptor.authMode == Access::AuthMode::kCase &&
        GetAtomicWriteScopedNodeId(attributeId, endpoint) == ScopedNodeId(subjectDescriptor.subject, subjectDescriptor.fabricIndex);
}

bool ThermostatAtomicWriteManager::InWrite(const std::optional<AttributeId> attributeId, CommandHandler * commandObj,
                                           EndpointId endpoint)
{
    if (!InWrite(attributeId, endpoint))
    {
        return false;
    }
    ScopedNodeId sourceNodeId = GetSourceScopedNodeId(commandObj);
    return GetAtomicWriteScopedNodeId(attributeId, endpoint) == sourceNodeId;
}

bool ThermostatAtomicWriteManager::BeginWrite(chip::app::CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                                              const Commands::AtomicRequest::DecodableType & commandData)
{
    EndpointId endpoint = commandPath.mEndpointId;

    size_t attributeRequestCount = 0;

    if (mDelegate == nullptr)
    {
        commandObj->AddStatus(commandPath, Status::InvalidInState);
        return false;
    }

    auto status = countAttributeRequests(commandData, attributeRequestCount);
    if (status != Status::Success)
    {
        commandObj->AddStatus(commandPath, status);
        return false;
    }
    if (attributeRequestCount == 0)
    {
        commandObj->AddStatus(commandPath, Status::InvalidCommand);
        return false;
    }

    auto timeoutRequest = System::Clock::Milliseconds16(commandData.timeout.Value());

    std::vector<AtomicAttributeStatusStruct::Type> attributeStatuses;
    attributeStatuses.reserve(attributeRequestCount);
    auto attributeIdsIter = commandData.attributeRequests.begin();
    bool requestedPresets = false, requestedSchedules = false;
    auto timeout = System::Clock::Milliseconds16(0);

    while (attributeIdsIter.Next())
    {
        auto & attributeId = attributeIdsIter.GetValue();

        for (auto & attributeStatus : attributeStatuses)
        {
            if (attributeStatus.attributeID == attributeId)
            {
                // Double-requesting an attribute is invalid
                commandObj->AddStatus(commandPath, Status::InvalidCommand);
                return false;
            }
        }

        if (InWrite(attributeId, commandObj, endpoint))
        {
            // This client already has an open atomic write on the requested attribute
            commandObj->AddStatus(commandPath, Status::InvalidInState);
            return false;
        }

        const EmberAfAttributeMetadata * metadata = emberAfLocateAttributeMetadata(endpoint, Thermostat::Id, attributeId);

        if (metadata == nullptr)
        {
            // This is not a valid attribute on the Thermostat cluster
            commandObj->AddStatus(commandPath, Status::InvalidCommand);
            return false;
        }

        auto attributeTimeout = mDelegate->GetWriteTimeout(endpoint, attributeId);

        if (attributeTimeout.has_value())
        {
            // Add to the maximum timeout
            timeout += attributeTimeout.value();
        }

        requestedPresets   = requestedPresets || attributeId == Presets::Id;
        requestedSchedules = requestedSchedules || attributeId == Schedules::Id;

        attributeStatuses.push_back({ .attributeID = attributeId, .statusCode = to_underlying(Status::Success) });
    }

    if (attributeIdsIter.GetStatus() != CHIP_NO_ERROR)
    {
        commandObj->AddStatus(commandPath, Status::InvalidCommand);
        return false;
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

    if (!commandData.timeout.HasValue())
    {
        commandObj->AddStatus(commandPath, Status::InvalidCommand);
        return false;
    }

    status = Status::Success;

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
        SetWriteState(endpoint, GetSourceScopedNodeId(commandObj), AtomicWriteState::Open);
        timeout          = std::min(timeoutRequest, timeout);
        response.timeout = MakeOptional(timeout.count());
        ScheduleTimer(endpoint, timeout);
    }
    commandObj->AddResponse(commandPath, response);
    return true;
}

bool ThermostatAtomicWriteManager::CommitWrite(chip::app::CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                                               const Commands::AtomicRequest::DecodableType & commandData)
{
    size_t attributeRequestCount = 0;
    if (mDelegate == nullptr)
    {
        commandObj->AddStatus(commandPath, Status::InvalidInState);
        return false;
    }

    Status status = validateAttributeRequests(commandData, attributeRequestCount);
    if (status != Status::Success)
    {
        commandObj->AddStatus(commandPath, Status::InvalidInState);
        return false;
    }
    EndpointId endpoint = commandPath.mEndpointId;
    std::vector<AtomicAttributeStatusStruct::Type> attributeStatuses;
    attributeStatuses.reserve(attributeRequestCount);
    auto attributeIdsIter = commandData.attributeRequests.begin();

    status = Status::Success;
    while (attributeIdsIter.Next())
    {
        auto & attributeId = attributeIdsIter.GetValue();
        if (!InWrite(attributeId, commandObj, endpoint))
        {
            commandObj->AddStatus(commandPath, Status::InvalidInState);
            return false;
        };
        Status attributeStatus = mDelegate->OnPreCommitWrite(endpoint, attributeId);
        if (attributeStatus != Status::Success)
        {
            status = Status::Failure;
        }
        attributeStatuses.push_back({ .attributeID = attributeId, .statusCode = to_underlying(attributeStatus) });
    }
    if (status == Status::Success)
    {
        for (auto & attributeStatus : attributeStatuses)
        {
            Status statusCode = mDelegate->OnCommitWrite(endpoint, attributeStatus.attributeID);
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
        // discard any pending writes Do the same for the latter, knowing that the server may be in an inconsistent state
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
    size_t attributeRequestCount = 0;
    if (mDelegate == nullptr)
    {
        commandObj->AddStatus(commandPath, Status::InvalidInState);
        return false;
    }

    Status status = validateAttributeRequests(commandData, attributeRequestCount);
    if (status != Status::Success)
    {
        commandObj->AddStatus(commandPath, Status::InvalidInState);
        return false;
    }
    EndpointId endpoint = commandPath.mEndpointId;
    std::vector<AtomicAttributeStatusStruct::Type> attributeStatuses;
    attributeStatuses.reserve(attributeRequestCount);
    auto attributeIdsIter = commandData.attributeRequests.begin();
    while (attributeIdsIter.Next())
    {
        auto & attributeId = attributeIdsIter.GetValue();
        if (!InWrite(attributeId, commandObj, endpoint))
        {
            commandObj->AddStatus(commandPath, Status::InvalidInState);
            return false;
        };

        attributeStatuses.push_back({ .attributeID = attributeId, .statusCode = to_underlying(Status::Success) });
    }

    for (auto & attributeStatus : attributeStatuses)
    {
        Status statusCode = mDelegate->OnRollbackWrite(endpoint, attributeStatus.attributeID);
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

ScopedNodeId ThermostatAtomicWriteManager::GetAtomicWriteScopedNodeId(const std::optional<AttributeId> attributeId,
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

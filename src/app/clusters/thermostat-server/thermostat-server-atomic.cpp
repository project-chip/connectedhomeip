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

#include "thermostat-server.h"

#include <platform/internal/CHIPDeviceLayerInternal.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::Thermostat;
using namespace chip::app::Clusters::Thermostat::Attributes;
using namespace chip::app::Clusters::Thermostat::Structs;
using namespace chip::app::Clusters::Globals::Structs;
using namespace chip::Protocols::InteractionModel;

namespace chip {
namespace app {
namespace Clusters {
/// @brief
namespace Thermostat {

extern ThermostatAttrAccess gThermostatAttrAccess;

/**
 * @brief Callback that is called when the timeout for editing the presets expires.
 *
 * @param[in] systemLayer The system layer.
 * @param[in] callbackContext The context passed to the timer callback.
 */
void TimerExpiredCallback(System::Layer * systemLayer, void * callbackContext)
{
    EndpointId endpoint = static_cast<EndpointId>(reinterpret_cast<uintptr_t>(callbackContext));
    gThermostatAttrAccess.ResetAtomicWrite(endpoint);
}

/**
 * @brief Schedules a timer for the given timeout in milliseconds.
 *
 * @param[in] endpoint The endpoint to use.
 * @param[in] timeoutMilliseconds The timeout in milliseconds.
 */
void ScheduleTimer(EndpointId endpoint, System::Clock::Milliseconds16 timeout)
{
    DeviceLayer::SystemLayer().StartTimer(timeout, TimerExpiredCallback,
                                          reinterpret_cast<void *>(static_cast<uintptr_t>(endpoint)));
}

/**
 * @brief Clears the currently scheduled timer.
 *
 * @param[in] endpoint The endpoint to use.
 */
void ClearTimer(EndpointId endpoint)
{
    DeviceLayer::SystemLayer().CancelTimer(TimerExpiredCallback, reinterpret_cast<void *>(static_cast<uintptr_t>(endpoint)));
}

/**
 * @brief Get the source scoped node id.
 *
 * @param[in] commandObj The command handler object.
 *
 * @return The scoped node id of the source node. If the scoped node id is not retreived, return ScopedNodeId().
 */
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
 * @brief Counts the number of attribute requests
 *
 * @param attributeRequests The decodable list of attribute IDs
 * @param attributeRequestCount The total number of attribute requests
 * @param requestedPresets Whether the Presets attribute was requested
 * @param requestedSchedules Whether the Schedules attribute was requested
 * @return true if the attribute list was counted
 * @return false if there was an error reading the list
 */
bool countAttributeRequests(const DataModel::DecodableList<chip::AttributeId> attributeRequests, size_t & attributeRequestCount,
                            bool & requestedPresets, bool & requestedSchedules)
{
    attributeRequestCount = 0;
    requestedPresets      = false;
    requestedSchedules    = false;
    auto attributeIdsIter = attributeRequests.begin();
    while (attributeIdsIter.Next())
    {
        auto & attributeId = attributeIdsIter.GetValue();
        switch (attributeId)
        {
        case Presets::Id:
            requestedPresets = true;
            break;
        case Schedules::Id:
            requestedSchedules = true;
            break;
        default:
            break;
        }
        attributeRequestCount++;
    }
    return attributeIdsIter.GetStatus() == CHIP_NO_ERROR;
}

/// @brief Builds the list of attribute statuses to return from an AtomicRequest invocation
/// @param endpoint The associated endpoint for the AtomicRequest invocation
/// @param attributeRequests The list of requested attributes
/// @param attributeStatusCount The number of attribute statuses in attributeStatuses
/// @param attributeStatuses The status of each requested attribute, plus additional attributes if needed
/// @param requireAll Whether the caller requires all atomic attributes to be represented in attributeRequests
/// @return Status::Success if the request is valid, an error status if it is not
Status buildAttributeStatuses(const EndpointId endpoint, const DataModel::DecodableList<chip::AttributeId> attributeRequests,
                              size_t & attributeStatusCount,
                              Platform::ScopedMemoryBuffer<AtomicAttributeStatusStruct::Type> & attributeStatuses, bool requireAll)
{

    bool requestedPresets = false, requestedSchedules = false;
    attributeStatusCount = 0;
    if (!countAttributeRequests(attributeRequests, attributeStatusCount, requestedPresets, requestedSchedules))
    {
        // Either we errored reading the list, or one of the attributes is not supported on this server
        return Status::InvalidCommand;
    }
    if (attributeStatusCount == 0)
    {
        // List can't be empty
        return Status::InvalidCommand;
    }
    if (requestedPresets ^ requestedSchedules)
    {
        // Client requested presets or schedules, but not both, so we need an extra status
        attributeStatusCount++;
    }
    attributeStatuses.Alloc(attributeStatusCount);
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
        const EmberAfAttributeMetadata * metadata =
            emberAfLocateAttributeMetadata(endpoint, Thermostat::Id, attributeStatus.attributeID);

        if (metadata == nullptr)
        {
            // This is not a valid attribute on the Thermostat cluster on the supplied endpoint
            return Status::InvalidCommand;
        }
    }
    if (requireAll)
    {
        if (!requestedPresets || !requestedSchedules)
        {
            return Status::InvalidInState;
        }
    }
    else if (requestedPresets ^ requestedSchedules)
    {
        // Client requested presets or schedules, but not both, so we add the extra status
        attributeStatuses[index].attributeID = requestedSchedules ? Presets::Id : Schedules::Id;
        attributeStatuses[index].statusCode  = to_underlying(Status::Success);
    }
    return Status::Success;
}

void ThermostatAttrAccess::ResetAtomicWrite(EndpointId endpoint)
{
    auto delegate = GetDelegate(endpoint);
    if (delegate != nullptr)
    {
        delegate->ClearPendingPresetList();
    }
    ClearTimer(endpoint);
    SetAtomicWrite(endpoint, ScopedNodeId(), AtomicWriteState::Closed);
}

bool ThermostatAttrAccess::InAtomicWrite(EndpointId endpoint)
{

    uint16_t ep =
        emberAfGetClusterServerEndpointIndex(endpoint, Thermostat::Id, MATTER_DM_THERMOSTAT_CLUSTER_SERVER_ENDPOINT_COUNT);

    if (ep < ArraySize(mAtomicWriteSessions))
    {
        return mAtomicWriteSessions[ep].state == AtomicWriteState::Open;
    }
    return false;
}

bool ThermostatAttrAccess::InAtomicWrite(const Access::SubjectDescriptor & subjectDescriptor, EndpointId endpoint)
{
    if (!InAtomicWrite(endpoint))
    {
        return false;
    }
    return subjectDescriptor.authMode == Access::AuthMode::kCase &&
        GetAtomicWriteOriginatorScopedNodeId(endpoint) == ScopedNodeId(subjectDescriptor.subject, subjectDescriptor.fabricIndex);
}

bool ThermostatAttrAccess::InAtomicWrite(CommandHandler * commandObj, EndpointId endpoint)
{
    if (!InAtomicWrite(endpoint))
    {
        return false;
    }
    ScopedNodeId sourceNodeId = GetSourceScopedNodeId(commandObj);
    return GetAtomicWriteOriginatorScopedNodeId(endpoint) == sourceNodeId;
}

ScopedNodeId ThermostatAttrAccess::GetAtomicWriteOriginatorScopedNodeId(const EndpointId endpoint)
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

void sendAtomicResponse(CommandHandler * commandObj, const ConcreteCommandPath & commandPath, Status status,
                        const Platform::ScopedMemoryBuffer<AtomicAttributeStatusStruct::Type> & attributeStatuses,
                        size_t & attributeRequestCount, Optional<uint16_t> timeout = NullOptional)
{
    Commands::AtomicResponse::Type response;
    response.statusCode = to_underlying(status);
    response.attributeStatus =
        DataModel::List<const AtomicAttributeStatusStruct::Type>(attributeStatuses.Get(), attributeRequestCount);
    response.timeout = timeout;
    commandObj->AddResponse(commandPath, response);
}

void ThermostatAttrAccess::BeginAtomicWrite(CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                                            const Commands::AtomicRequest::DecodableType & commandData)
{
    EndpointId endpoint = commandPath.mEndpointId;

    auto delegate = GetDelegate(endpoint);

    if (delegate == nullptr)
    {
        ChipLogError(Zcl, "Delegate is null");
        commandObj->AddStatus(commandPath, Status::InvalidInState);
        return;
    }

    size_t attributeStatusCount = 0;
    Platform::ScopedMemoryBuffer<AtomicAttributeStatusStruct::Type> attributeStatuses;
    auto status = buildAttributeStatuses(endpoint, commandData.attributeRequests, attributeStatusCount, attributeStatuses, false);
    if (status != Status::Success)
    {
        commandObj->AddStatus(commandPath, status);
        return;
    }

    if (gThermostatAttrAccess.InAtomicWrite(commandObj, endpoint))
    {
        // This client already has an open atomic write
        commandObj->AddStatus(commandPath, Status::InvalidInState);
        return;
    }

    if (!commandData.timeout.HasValue())
    {
        commandObj->AddStatus(commandPath, Status::InvalidCommand);
        return;
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
            auto attributeTimeout = delegate->GetAtomicWriteTimeout(attributeId);

            if (attributeTimeout.has_value())
            {
                // Add to the maximum timeout
                maximumTimeout += attributeTimeout.value();
            }

            break;
        }
    }

    status = Status::Success;
    for (size_t i = 0; i < attributeStatusCount; ++i)
    {
        auto & attributeStatus = attributeStatuses[i];
        auto statusCode        = Status::Success;
        switch (attributeStatus.attributeID)
        {
        case Presets::Id:
        case Schedules::Id:
            statusCode = gThermostatAttrAccess.InAtomicWrite(endpoint) ? Status::Busy : Status::Success;
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
    if (timeout.count() == 0)
    {
        commandObj->AddStatus(commandPath, Status::InvalidInState);
        return;
    }

    if (status == Status::Success)
    {
        // This is a valid request to open an atomic write. Tell the delegate it
        // needs to keep track of a pending preset list now.
        delegate->InitializePendingPresets();
        ScheduleTimer(endpoint, timeout);
        SetAtomicWrite(endpoint, GetSourceScopedNodeId(commandObj), AtomicWriteState::Open);
    }

    sendAtomicResponse(commandObj, commandPath, status, attributeStatuses, attributeStatusCount, MakeOptional(timeout.count()));
}

void ThermostatAttrAccess::CommitAtomicWrite(CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                                             const Commands::AtomicRequest::DecodableType & commandData)
{
    EndpointId endpoint = commandPath.mEndpointId;
    auto delegate       = GetDelegate(endpoint);

    if (delegate == nullptr)
    {
        ChipLogError(Zcl, "Delegate is null");
        commandObj->AddStatus(commandPath, Status::InvalidInState);
        return;
    }

    size_t attributeStatusCount = 0;
    Platform::ScopedMemoryBuffer<AtomicAttributeStatusStruct::Type> attributeStatuses;
    auto status = buildAttributeStatuses(endpoint, commandData.attributeRequests, attributeStatusCount, attributeStatuses, true);
    if (status != Status::Success)
    {
        commandObj->AddStatus(commandPath, status);
        return;
    }

    if (!gThermostatAttrAccess.InAtomicWrite(commandObj, endpoint))
    {
        commandObj->AddStatus(commandPath, Status::InvalidInState);
        return;
    }

    status = Status::Success;
    for (size_t i = 0; i < attributeStatusCount; ++i)
    {
        auto & attributeStatus = attributeStatuses[i];
        auto statusCode        = Status::Success;
        switch (attributeStatus.attributeID)
        {
        case Presets::Id:
            statusCode = PrecommitPresets(endpoint);
            if (statusCode == Status::Success)
            {
                CHIP_ERROR err = delegate->CommitPendingPresets();
                if (err != CHIP_NO_ERROR)
                {
                    statusCode = Status::InvalidInState;
                }
            }
            break;
        case Schedules::Id:
            statusCode = Status::Success;
            break;
        default:
            commandObj->AddStatus(commandPath, Status::InvalidInState);
            return;
        }
        attributeStatus.statusCode = to_underlying(statusCode);
        if (statusCode != Status::Success)
        {
            status = Status::Failure;
        }
    }

    ResetAtomicWrite(endpoint);
    sendAtomicResponse(commandObj, commandPath, status, attributeStatuses, attributeStatusCount);
}

void ThermostatAttrAccess::RollbackAtomicWrite(CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                                               const Commands::AtomicRequest::DecodableType & commandData)
{

    EndpointId endpoint = commandPath.mEndpointId;
    auto delegate       = GetDelegate(endpoint);

    if (delegate == nullptr)
    {
        ChipLogError(Zcl, "Delegate is null");
        commandObj->AddStatus(commandPath, Status::InvalidInState);
        return;
    }

    size_t attributeStatusCount = 0;
    Platform::ScopedMemoryBuffer<AtomicAttributeStatusStruct::Type> attributeStatuses;
    auto status = buildAttributeStatuses(endpoint, commandData.attributeRequests, attributeStatusCount, attributeStatuses, true);
    if (status != Status::Success)
    {
        commandObj->AddStatus(commandPath, status);
        return;
    }

    if (!gThermostatAttrAccess.InAtomicWrite(commandObj, endpoint))
    {
        // There's no open atomic write
        commandObj->AddStatus(commandPath, Status::InvalidInState);
        return;
    }

    ResetAtomicWrite(endpoint);

    for (size_t i = 0; i < attributeStatusCount; ++i)
    {
        auto & attributeStatus = attributeStatuses[i];
        switch (attributeStatus.attributeID)
        {
        case Presets::Id:
        case Schedules::Id:
            attributeStatus.statusCode = to_underlying(Status::Success);
            break;
        default:
            commandObj->AddStatus(commandPath, Status::InvalidInState);
            return;
        }
    }

    sendAtomicResponse(commandObj, commandPath, status, attributeStatuses, attributeStatusCount);
}

void ThermostatAttrAccess::SetAtomicWrite(EndpointId endpoint, ScopedNodeId originatorNodeId, AtomicWriteState state)
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

} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip

bool emberAfThermostatClusterAtomicRequestCallback(CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                                                   const Clusters::Thermostat::Commands::AtomicRequest::DecodableType & commandData)
{
    auto & requestType = commandData.requestType;

    // If we've gotten this far, then the client has manage permission to call AtomicRequest, which is also the
    // privilege necessary to write to the atomic attributes, so no need to check

    switch (requestType)
    {
    case Globals::AtomicRequestTypeEnum::kBeginWrite:
        gThermostatAttrAccess.BeginAtomicWrite(commandObj, commandPath, commandData);
        return true;
    case Globals::AtomicRequestTypeEnum::kCommitWrite:
        gThermostatAttrAccess.CommitAtomicWrite(commandObj, commandPath, commandData);
        return true;
    case Globals::AtomicRequestTypeEnum::kRollbackWrite:
        gThermostatAttrAccess.RollbackAtomicWrite(commandObj, commandPath, commandData);
        return true;
    case Globals::AtomicRequestTypeEnum::kUnknownEnumValue:
        commandObj->AddStatus(commandPath, Status::InvalidCommand);
        return true;
    }

    return false;
}

void MatterThermostatClusterServerShutdownCallback(EndpointId endpoint)
{
    ChipLogProgress(Zcl, "Shutting down thermostat server cluster on endpoint %d", endpoint);
    gThermostatAttrAccess.ResetAtomicWrite(endpoint);
}

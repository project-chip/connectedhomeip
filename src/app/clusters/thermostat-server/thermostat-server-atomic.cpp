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

#include <app/GlobalAttributes.h>
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

namespace {

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
bool CountAttributeRequests(const DataModel::DecodableList<chip::AttributeId> attributeRequests, size_t & attributeRequestCount,
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
/// @return Status::Success if the request is valid, an error status if it is not
Status BuildAttributeStatuses(const EndpointId endpoint, const DataModel::DecodableList<chip::AttributeId> attributeRequests,
                              Platform::ScopedMemoryBufferWithSize<AtomicAttributeStatusStruct::Type> & attributeStatuses)
{

    bool requestedPresets = false, requestedSchedules = false;
    size_t attributeStatusCount = 0;
    if (!CountAttributeRequests(attributeRequests, attributeStatusCount, requestedPresets, requestedSchedules))
    {
        // We errored reading the list
        return Status::InvalidCommand;
    }
    if (attributeStatusCount == 0)
    {
        // List can't be empty
        return Status::InvalidCommand;
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

        if (metadata != nullptr)
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

} // anonymous namespace

bool ThermostatAttrAccess::InAtomicWrite(EndpointId endpoint, Optional<AttributeId> attributeId)
{

    uint16_t ep =
        emberAfGetClusterServerEndpointIndex(endpoint, Thermostat::Id, MATTER_DM_THERMOSTAT_CLUSTER_SERVER_ENDPOINT_COUNT);

    if (ep >= MATTER_ARRAY_SIZE(mAtomicWriteSessions))
    {
        return false;
    }
    auto & atomicWriteSession = mAtomicWriteSessions[ep];
    if (atomicWriteSession.state != AtomicWriteState::Open)
    {
        return false;
    }
    if (!attributeId.HasValue())
    {
        return true;
    }
    for (size_t i = 0; i < atomicWriteSession.attributeIds.AllocatedSize(); ++i)
    {
        if (atomicWriteSession.attributeIds[i] == attributeId.Value())
        {
            return true;
        }
    }
    return false;
}

bool ThermostatAttrAccess::InAtomicWrite(EndpointId endpoint, const Access::SubjectDescriptor & subjectDescriptor,
                                         Optional<AttributeId> attributeId)
{
    if (!InAtomicWrite(endpoint, attributeId))
    {
        return false;
    }
    return subjectDescriptor.authMode == Access::AuthMode::kCase &&
        GetAtomicWriteOriginatorScopedNodeId(endpoint) == ScopedNodeId(subjectDescriptor.subject, subjectDescriptor.fabricIndex);
}

bool ThermostatAttrAccess::InAtomicWrite(EndpointId endpoint, CommandHandler * commandObj, Optional<AttributeId> attributeId)
{
    if (!InAtomicWrite(endpoint, attributeId))
    {
        return false;
    }
    ScopedNodeId sourceNodeId = GetSourceScopedNodeId(commandObj);
    return GetAtomicWriteOriginatorScopedNodeId(endpoint) == sourceNodeId;
}

bool ThermostatAttrAccess::InAtomicWrite(
    EndpointId endpoint, CommandHandler * commandObj,
    Platform::ScopedMemoryBufferWithSize<AtomicAttributeStatusStruct::Type> & attributeStatuses)
{
    uint16_t ep =
        emberAfGetClusterServerEndpointIndex(endpoint, Thermostat::Id, MATTER_DM_THERMOSTAT_CLUSTER_SERVER_ENDPOINT_COUNT);

    if (ep >= MATTER_ARRAY_SIZE(mAtomicWriteSessions))
    {
        return false;
    }
    auto & atomicWriteSession = mAtomicWriteSessions[ep];
    if (atomicWriteSession.state != AtomicWriteState::Open)
    {
        return false;
    }
    if (atomicWriteSession.attributeIds.AllocatedSize() == 0 ||
        atomicWriteSession.attributeIds.AllocatedSize() != attributeStatuses.AllocatedSize())
    {
        return false;
    }
    for (size_t i = 0; i < atomicWriteSession.attributeIds.AllocatedSize(); ++i)
    {
        bool hasAttribute = false;
        auto attributeId  = atomicWriteSession.attributeIds[i];
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

bool ThermostatAttrAccess::SetAtomicWrite(
    EndpointId endpoint, ScopedNodeId originatorNodeId, AtomicWriteState state,
    Platform::ScopedMemoryBufferWithSize<AtomicAttributeStatusStruct::Type> & attributeStatuses)
{
    uint16_t ep =
        emberAfGetClusterServerEndpointIndex(endpoint, Thermostat::Id, MATTER_DM_THERMOSTAT_CLUSTER_SERVER_ENDPOINT_COUNT);

    if (ep >= MATTER_ARRAY_SIZE(mAtomicWriteSessions))
    {
        return false;
    }

    auto & atomicWriteSession     = mAtomicWriteSessions[ep];
    atomicWriteSession.endpointId = endpoint;
    if (!atomicWriteSession.attributeIds.Alloc(attributeStatuses.AllocatedSize()))
    {
        atomicWriteSession.state  = AtomicWriteState::Closed;
        atomicWriteSession.nodeId = ScopedNodeId();
        return false;
    }

    atomicWriteSession.state  = state;
    atomicWriteSession.nodeId = originatorNodeId;

    for (size_t i = 0; i < attributeStatuses.AllocatedSize(); ++i)
    {
        atomicWriteSession.attributeIds[i] = attributeStatuses[i].attributeID;
    }
    return true;
}

void ThermostatAttrAccess::ResetAtomicWrite(EndpointId endpoint)
{
    auto delegate = GetDelegate(endpoint);
    if (delegate != nullptr)
    {
        delegate->ClearPendingPresetList();
    }
    ClearTimer(endpoint);
    uint16_t ep =
        emberAfGetClusterServerEndpointIndex(endpoint, Thermostat::Id, MATTER_DM_THERMOSTAT_CLUSTER_SERVER_ENDPOINT_COUNT);

    if (ep >= MATTER_ARRAY_SIZE(mAtomicWriteSessions))
    {
        return;
    }
    auto & atomicWriteSession     = mAtomicWriteSessions[ep];
    atomicWriteSession.state      = AtomicWriteState::Closed;
    atomicWriteSession.endpointId = endpoint;
    atomicWriteSession.nodeId     = ScopedNodeId();
    atomicWriteSession.attributeIds.Free();
}

ScopedNodeId ThermostatAttrAccess::GetAtomicWriteOriginatorScopedNodeId(const EndpointId endpoint)
{
    ScopedNodeId originatorNodeId = ScopedNodeId();
    uint16_t ep =
        emberAfGetClusterServerEndpointIndex(endpoint, Thermostat::Id, MATTER_DM_THERMOSTAT_CLUSTER_SERVER_ENDPOINT_COUNT);

    if (ep < MATTER_ARRAY_SIZE(mAtomicWriteSessions))
    {
        originatorNodeId = mAtomicWriteSessions[ep].nodeId;
    }
    return originatorNodeId;
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

    Platform::ScopedMemoryBufferWithSize<AtomicAttributeStatusStruct::Type> attributeStatuses;
    auto status = BuildAttributeStatuses(endpoint, commandData.attributeRequests, attributeStatuses);
    if (status != Status::Success)
    {
        commandObj->AddStatus(commandPath, status);
        return;
    }

    if (InAtomicWrite(endpoint, commandObj))
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
            auto attributeTimeout = delegate->GetMaxAtomicWriteTimeout(attributeId);

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
            statusCode = InAtomicWrite(endpoint, MakeOptional(attributeStatus.attributeID)) ? Status::Busy : Status::Success;
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
        if (!SetAtomicWrite(endpoint, GetSourceScopedNodeId(commandObj), AtomicWriteState::Open, attributeStatuses))
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
            delegate->InitializePendingPresets();
            ScheduleTimer(endpoint, timeout);
        }
    }

    SendAtomicResponse(commandObj, commandPath, status, attributeStatuses, MakeOptional(timeout.count()));
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

    Platform::ScopedMemoryBufferWithSize<AtomicAttributeStatusStruct::Type> attributeStatuses;
    auto status = BuildAttributeStatuses(endpoint, commandData.attributeRequests, attributeStatuses);
    if (status != Status::Success)
    {
        commandObj->AddStatus(commandPath, status);
        return;
    }

    if (!InAtomicWrite(endpoint, commandObj, attributeStatuses))
    {
        commandObj->AddStatus(commandPath, Status::InvalidInState);
        return;
    }

    status = Status::Success;
    for (size_t i = 0; i < attributeStatuses.AllocatedSize(); ++i)
    {
        auto & attributeStatus = attributeStatuses[i];
        auto statusCode        = Status::Success;
        switch (attributeStatus.attributeID)
        {
        case Presets::Id:
            statusCode = PrecommitPresets(endpoint);
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

    if (status == Status::Success)
    {
        for (size_t i = 0; i < attributeStatuses.AllocatedSize(); ++i)
        {
            auto & attributeStatus = attributeStatuses[i];
            auto statusCode        = Status::Success;
            CHIP_ERROR err;
            switch (attributeStatus.attributeID)
            {
            case Presets::Id:
                err = delegate->CommitPendingPresets();
                if (err != CHIP_NO_ERROR)
                {
                    statusCode = Status::InvalidInState;
                }
                break;
            case Schedules::Id:
                break;
            default:
                // Not reachable, since we returned in this situation above.
                break;
            }
            attributeStatus.statusCode = to_underlying(statusCode);
            if (statusCode != Status::Success)
            {
                status = Status::Failure;
            }
        }
    }

    ResetAtomicWrite(endpoint);
    SendAtomicResponse(commandObj, commandPath, status, attributeStatuses);
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

    Platform::ScopedMemoryBufferWithSize<AtomicAttributeStatusStruct::Type> attributeStatuses;
    auto status = BuildAttributeStatuses(endpoint, commandData.attributeRequests, attributeStatuses);
    if (status != Status::Success)
    {
        commandObj->AddStatus(commandPath, status);
        return;
    }

    if (!InAtomicWrite(endpoint, commandObj, attributeStatuses))
    {
        // There's no open atomic write
        commandObj->AddStatus(commandPath, Status::InvalidInState);
        return;
    }

    ResetAtomicWrite(endpoint);

    for (size_t i = 0; i < attributeStatuses.AllocatedSize(); ++i)
    {
        attributeStatuses[i].statusCode = to_underlying(Status::Success);
    }

    SendAtomicResponse(commandObj, commandPath, status, attributeStatuses);
}

void MatterThermostatClusterServerShutdownCallback(EndpointId endpoint)
{
    ChipLogProgress(Zcl, "Shutting down thermostat server cluster on endpoint %d", endpoint);
    gThermostatAttrAccess.ResetAtomicWrite(endpoint);
}

bool emberAfThermostatClusterAtomicRequestCallback(CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                                                   const Clusters::Thermostat::Commands::AtomicRequest::DecodableType & commandData)
{
    auto & requestType = commandData.requestType;

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

} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip

bool emberAfThermostatClusterAtomicRequestCallback(CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                                                   const Clusters::Thermostat::Commands::AtomicRequest::DecodableType & commandData)
{
    return Thermostat::emberAfThermostatClusterAtomicRequestCallback(commandObj, commandPath, commandData);
}

void MatterThermostatClusterServerShutdownCallback(EndpointId endpoint)
{
    Thermostat::MatterThermostatClusterServerShutdownCallback(endpoint);
}

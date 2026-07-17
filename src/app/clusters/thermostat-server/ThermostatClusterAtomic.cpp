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

#include "ThermostatCluster.h"

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

/**
 * @brief Callback that is called when the timeout for editing the presets expires.
 *
 * @param[in] systemLayer The system layer.
 * @param[in] callbackContext The owning ThermostatCluster instance.
 */
void TimerExpiredCallback(System::Layer * systemLayer, void * callbackContext)
{
    auto * cluster = static_cast<ThermostatCluster *>(callbackContext);
    cluster->ResetAtomicWrite(cluster->GetEndpointId());
}

namespace {

/**
 * @brief Schedules the atomic-write timeout timer for the given cluster.
 */
void ScheduleTimer(ThermostatCluster * cluster, System::Clock::Milliseconds16 timeout)
{
    TEMPORARY_RETURN_IGNORED DeviceLayer::SystemLayer().StartTimer(timeout, TimerExpiredCallback, cluster);
}

/**
 * @brief Clears the currently scheduled atomic-write timeout timer for the given cluster.
 */
void ClearTimer(ThermostatCluster * cluster)
{
    DeviceLayer::SystemLayer().CancelTimer(TimerExpiredCallback, cluster);
}

/**
 * @brief Get the source scoped node id.
 *
 * @param[in] commandObj The command handler object.
 *
 * @return The scoped node id of the source node. If the scoped node id is not retrieved, return ScopedNodeId().
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

} // anonymous namespace

bool ThermostatCluster::IsKnownAttribute(AttributeId attributeId)
{
    // Global attributes are always present on every cluster (FeatureMap, ClusterRevision,
    // GeneratedCommandList, AcceptedCommandList, AttributeList).
    for (const auto & entry : DefaultServerCluster::GlobalAttributes())
    {
        if (entry.attributeId == attributeId)
        {
            return true;
        }
    }

    // Cluster-specific attributes, gated by the active feature map and optional-attribute set. The
    // cluster's own Attributes() listing is the code-driven source of truth (replaces ember metadata).
    ReadOnlyBufferBuilder<DataModel::AttributeEntry> builder;
    if (Attributes(ConcreteClusterPath(GetEndpointId(), Thermostat::Id), builder) != CHIP_NO_ERROR)
    {
        return false;
    }
    for (const auto & entry : builder.TakeBuffer())
    {
        if (entry.attributeId == attributeId)
        {
            return true;
        }
    }
    return false;
}

/// @brief Builds the list of attribute statuses to return from an AtomicRequest invocation
/// @param attributeRequests The list of requested attributes
/// @param attributeStatuses The status of each requested attribute, plus additional attributes if needed
/// @return Status::Success if the request is valid, an error status if it is not
Status ThermostatCluster::BuildAttributeStatuses(
    const DataModel::DecodableList<AttributeId> & attributeRequests,
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
        if (!IsKnownAttribute(attributeStatus.attributeID))
        {
            // This is not a valid attribute on the Thermostat cluster on this endpoint
            return Status::InvalidCommand;
        }
    }
    return Status::Success;
}

bool ThermostatCluster::InAtomicWrite(EndpointId endpoint, Optional<AttributeId> attributeId)
{
    VerifyOrReturnValue(endpoint == GetEndpointId(), false);

    if (mAtomicWriteSession.state != AtomicWriteState::Open)
    {
        return false;
    }
    if (!attributeId.HasValue())
    {
        return true;
    }
    for (size_t i = 0; i < mAtomicWriteSession.attributeIds.AllocatedSize(); ++i)
    {
        if (mAtomicWriteSession.attributeIds[i] == attributeId.Value())
        {
            return true;
        }
    }
    return false;
}

bool ThermostatCluster::InAtomicWrite(EndpointId endpoint, const Access::SubjectDescriptor & subjectDescriptor,
                                      Optional<AttributeId> attributeId)
{
    if (!InAtomicWrite(endpoint, attributeId))
    {
        return false;
    }
    return subjectDescriptor.authMode == Access::AuthMode::kCase &&
        GetAtomicWriteOriginatorScopedNodeId(endpoint) == ScopedNodeId(subjectDescriptor.subject, subjectDescriptor.fabricIndex);
}

bool ThermostatCluster::InAtomicWrite(EndpointId endpoint, CommandHandler * commandObj, Optional<AttributeId> attributeId)
{
    if (!InAtomicWrite(endpoint, attributeId))
    {
        return false;
    }
    ScopedNodeId sourceNodeId = GetSourceScopedNodeId(commandObj);
    return GetAtomicWriteOriginatorScopedNodeId(endpoint) == sourceNodeId;
}

bool ThermostatCluster::InAtomicWrite(EndpointId endpoint, CommandHandler * commandObj,
                                      Platform::ScopedMemoryBufferWithSize<AtomicAttributeStatusStruct::Type> & attributeStatuses)
{
    VerifyOrReturnValue(endpoint == GetEndpointId(), false);

    if (mAtomicWriteSession.state != AtomicWriteState::Open)
    {
        return false;
    }
    if (mAtomicWriteSession.attributeIds.AllocatedSize() == 0 ||
        mAtomicWriteSession.attributeIds.AllocatedSize() != attributeStatuses.AllocatedSize())
    {
        return false;
    }
    for (size_t i = 0; i < mAtomicWriteSession.attributeIds.AllocatedSize(); ++i)
    {
        bool hasAttribute = false;
        auto attributeId  = mAtomicWriteSession.attributeIds[i];
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

bool ThermostatCluster::SetAtomicWrite(EndpointId endpoint, ScopedNodeId originatorNodeId, AtomicWriteState state,
                                       Platform::ScopedMemoryBufferWithSize<AtomicAttributeStatusStruct::Type> & attributeStatuses)
{
    VerifyOrReturnValue(endpoint == GetEndpointId(), false);

    mAtomicWriteSession.endpointId = endpoint;
    if (!mAtomicWriteSession.attributeIds.Alloc(attributeStatuses.AllocatedSize()))
    {
        mAtomicWriteSession.state  = AtomicWriteState::Closed;
        mAtomicWriteSession.nodeId = ScopedNodeId();
        return false;
    }

    mAtomicWriteSession.state  = state;
    mAtomicWriteSession.nodeId = originatorNodeId;

    for (size_t i = 0; i < attributeStatuses.AllocatedSize(); ++i)
    {
        mAtomicWriteSession.attributeIds[i] = attributeStatuses[i].attributeID;
    }
    return true;
}

void ThermostatCluster::ResetAtomicWrite(EndpointId endpoint)
{
    VerifyOrReturn(endpoint == GetEndpointId());

    if (mDelegate != nullptr)
    {
        mDelegate->ClearPendingPresetList();
    }
    ClearTimer(this);

    mAtomicWriteSession.state      = AtomicWriteState::Closed;
    mAtomicWriteSession.endpointId = endpoint;
    mAtomicWriteSession.nodeId     = ScopedNodeId();
    mAtomicWriteSession.attributeIds.Free();
}

ScopedNodeId ThermostatCluster::GetAtomicWriteOriginatorScopedNodeId(const EndpointId endpoint)
{
    VerifyOrReturnValue(endpoint == GetEndpointId(), ScopedNodeId());
    return mAtomicWriteSession.nodeId;
}

void ThermostatCluster::BeginAtomicWrite(CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                                         const Commands::AtomicRequest::DecodableType & commandData)
{
    EndpointId endpoint = commandPath.mEndpointId;

    if (mDelegate == nullptr)
    {
        ChipLogError(Zcl, "Delegate is null");
        commandObj->AddStatus(commandPath, Status::InvalidInState);
        return;
    }

    Platform::ScopedMemoryBufferWithSize<AtomicAttributeStatusStruct::Type> attributeStatuses;
    auto status = BuildAttributeStatuses(commandData.attributeRequests, attributeStatuses);
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
            auto attributeTimeout = mDelegate->GetMaxAtomicWriteTimeout(attributeId);

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
            mDelegate->InitializePendingPresets();
            ScheduleTimer(this, timeout);
        }
    }

    SendAtomicResponse(commandObj, commandPath, status, attributeStatuses, MakeOptional(timeout.count()));
}

void ThermostatCluster::CommitAtomicWrite(CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                                          const Commands::AtomicRequest::DecodableType & commandData)
{
    EndpointId endpoint = commandPath.mEndpointId;

    if (mDelegate == nullptr)
    {
        ChipLogError(Zcl, "Delegate is null");
        commandObj->AddStatus(commandPath, Status::InvalidInState);
        return;
    }

    Platform::ScopedMemoryBufferWithSize<AtomicAttributeStatusStruct::Type> attributeStatuses;
    auto status = BuildAttributeStatuses(commandData.attributeRequests, attributeStatuses);
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
            statusCode = PrecommitPresets();
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
                err = mDelegate->CommitPendingPresets();
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

void ThermostatCluster::RollbackAtomicWrite(CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                                            const Commands::AtomicRequest::DecodableType & commandData)
{

    EndpointId endpoint = commandPath.mEndpointId;

    if (mDelegate == nullptr)
    {
        ChipLogError(Zcl, "Delegate is null");
        commandObj->AddStatus(commandPath, Status::InvalidInState);
        return;
    }

    Platform::ScopedMemoryBufferWithSize<AtomicAttributeStatusStruct::Type> attributeStatuses;
    auto status = BuildAttributeStatuses(commandData.attributeRequests, attributeStatuses);
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

} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip

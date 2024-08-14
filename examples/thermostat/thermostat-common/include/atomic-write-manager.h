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

#pragma once

#include <app/clusters/thermostat-server/atomic-write.h>
#include <app/clusters/thermostat-server/thermostat-delegate.h>

#include "thermostat-delegate-impl.h"

namespace chip {
namespace app {
namespace Clusters {
namespace Thermostat {

enum class AtomicWriteState
{
    Closed = 0,
    Open,
};

class ThermostatAtomicWriteManager : public AtomicWriteManager
{
public:
    static inline ThermostatAtomicWriteManager & GetInstance() { return sInstance; }

    /**
     * @brief Set the Delegate object
     *
     * @param delegate
     */
    void SetDelegate(AtomicWriteDelegate * delegate) override { mDelegate = delegate; };

    /**
     * @brief Check if there is an open atomic write on an endpoint, optionally associated with a specific attribute
     *
     * @param attributeId Optional attribute filter
     * @param endpoint The endpoint to check atomic write state
     * @return true if there is an open atomic write
     * @return false if there is no open atomic write
     */
    bool InWrite(const std::optional<AttributeId> attributeId, EndpointId endpoint) override;

    /**
     * @brief Check if there is an open atomic write on an endpoint, associated with a given SubjectDescriptor, and optionally
     * associated with a specific attribute
     *
     * @param attributeId Optional attribute filter
     * @param subjectDescriptor The subject descriptor to check against
     * @param endpoint The endpoint to check atomic write state
     * @return true if there is an open atomic write
     * @return false if there is no open atomic write
     */
    bool InWrite(const std::optional<AttributeId> attributeId, const Access::SubjectDescriptor & subjectDescriptor,
                 EndpointId endpoint) override;

    /**
     * @brief Check if there is an open atomic write on an endpoint, associated with the source node for a given command invocation,
     * and optionally associated with a specific attribute
     *
     * @param attributeId
     * @param commandObj
     * @param endpoint
     * @return true if there is an open atomic write
     * @return false if there is no open atomic write
     */
    bool InWrite(const std::optional<AttributeId> attributeId, CommandHandler * commandObj, EndpointId endpoint) override;

    /**
     * @brief Attempt to start an atomic write
     *
     * @param commandObj The AtomicRequest command
     * @param commandPath The path for the command
     * @param commandData The payload of the command
     * @return true if the atomic write was opened successfully
     * @return false if the request to open atomic write failed
     */
    bool BeginWrite(chip::app::CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                    const Commands::AtomicRequest::DecodableType & commandData) override;

    /**
     * @brief Attempt to commit an atomic write; returns true if the server is able to either commit or rollback successfully,
     * returns false otherwise
     *
     * @param commandObj The AtomicRequest command
     * @param commandPath The path for the command
     * @param commandData The payload of the command
     * @return true if the atomic write was either committed or rolled back
     * @return false if the atomic write was not found
     */
    bool CommitWrite(chip::app::CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                     const Commands::AtomicRequest::DecodableType & commandData) override;

    /**
     * @brief Attempt to roll back an atomic write; returns true if the server is able to rollback successfully, false if it was
     * unable to find a matching atomic write
     *
     * @param commandObj The AtomicRequest command
     * @param commandPath The path for the command
     * @param commandData The payload of the command
     * @return true if the atomic write was rolled back
     * @return false if the atomic write was not found
     */
    bool RollbackWrite(chip::app::CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                       const Commands::AtomicRequest::DecodableType & commandData) override;

    /**
     * @brief Reset any atomic writes associated with the given endpoint
     *
     * @param endpoint
     */
    void ResetWrite(EndpointId endpoint) override;

    /**
     * @brief Reset any atomic writes originating from a given fabric index
     *
     * @param fabricIndex
     */
    void ResetWrite(FabricIndex fabricIndex) override;

private:
    static ThermostatAtomicWriteManager sInstance;

    ThermostatAtomicWriteManager()  = default;
    ~ThermostatAtomicWriteManager() = default;

    ThermostatAtomicWriteManager(const ThermostatAtomicWriteManager &)             = delete;
    ThermostatAtomicWriteManager & operator=(const ThermostatAtomicWriteManager &) = delete;

    void ScheduleTimer(EndpointId endpoint, System::Clock::Milliseconds16 timeout);
    void ClearTimer(EndpointId endpoint);
    void OnTimerExpired(EndpointId endpoint);

    void SetWriteState(EndpointId endpoint, ScopedNodeId originatorNodeId, AtomicWriteState state);

    Protocols::InteractionModel::Status CheckAttributeRequests(const EndpointId endpoint, chip::app::CommandHandler * commandObj,
                                                               const Commands::AtomicRequest::DecodableType & commandData,
                                                               size_t & attributeRequestCount);

    ScopedNodeId GetAtomicWriteScopedNodeId(const std::optional<AttributeId> attributeId, const EndpointId endpoint);

    friend void TimerExpiredCallback(System::Layer * systemLayer, void * callbackContext);

    struct AtomicWriteSession
    {
        AtomicWriteState state = AtomicWriteState::Closed;
        ScopedNodeId nodeId    = ScopedNodeId();
        EndpointId endpointId  = kInvalidEndpointId;
    };

    AtomicWriteDelegate * mDelegate;
    AtomicWriteSession mAtomicWriteSessions[kThermostatEndpointCount];
};

} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip

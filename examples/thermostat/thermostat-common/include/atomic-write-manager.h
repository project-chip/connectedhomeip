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

    // AtomicWriteManager
    bool InWrite(const std::optional<AttributeId> attributeId, EndpointId endpoint) override;
    bool InWrite(const std::optional<AttributeId> attributeId, const Access::SubjectDescriptor & subjectDescriptor,
                 EndpointId endpoint) override;
    bool InWrite(const std::optional<AttributeId> attributeId, CommandHandler * commandObj, EndpointId endpoint) override;
    bool BeginWrite(chip::app::CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                    const Commands::AtomicRequest::DecodableType & commandData) override;
    bool CommitWrite(chip::app::CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                     const Commands::AtomicRequest::DecodableType & commandData) override;
    bool RollbackWrite(chip::app::CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                       const Commands::AtomicRequest::DecodableType & commandData) override;
    void ResetWrite(EndpointId endpoint) override;
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

    ScopedNodeId GetAtomicWriteOriginatorScopedNodeId(const std::optional<AttributeId> attributeId, const EndpointId endpoint);

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

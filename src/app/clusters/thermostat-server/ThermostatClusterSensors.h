/**
 *    Copyright (c) 2026 Project CHIP Authors
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

#include "SensorScheduleTransitionStructWithOwnedMembers.h"
#include "ThermostatSensorStructWithOwnedMembers.h"

#include <app-common/zap-generated/cluster-objects.h>
#include <app/AttributeValueDecoder.h>
#include <app/AttributeValueEncoder.h>
#include <app/ConcreteAttributePath.h>
#include <app/data-model-provider/ActionReturnStatus.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <app/data-model-provider/OperationTypes.h>
#include <protocols/interaction_model/StatusCode.h>
#include <system/SystemClock.h>

#include <lib/support/ReadOnlyBuffer.h>
#include <optional>

namespace chip {
namespace app {
namespace Clusters {
namespace Thermostat {

class ThermostatClusterBase;
class AtomicWriteSession;

class ThermostatSensors
{
public:
    class Delegate
    {
    public:
        virtual ~Delegate() = default;

        virtual CHIP_ERROR GetSensorAtIndex(size_t index, ThermostatSensorStructWithOwnedMembers & sensor) = 0;
        virtual CHIP_ERROR GetAvailableSensorAtIndex(size_t index, ByteSpan & sensorHandle)                = 0;
        virtual bool SetAvailableSensors(Span<const ByteSpan> availableSensors)                            = 0;
        virtual CHIP_ERROR GetEnabledSensorAtIndex(size_t index, ByteSpan & sensorHandle)                  = 0;
        virtual bool SetEnabledSensors(Span<const ByteSpan> enabledSensors)                                = 0;
        virtual uint8_t GetNumberOfSensorScheduleTransitions()                                             = 0;

        virtual CHIP_ERROR GetSensorScheduleTransitionAtIndex(size_t index,
                                                              SensorScheduleTransitionStructWithOwnedMembers & transition) = 0;

        virtual CHIP_ERROR
        GetPendingSensorScheduleTransitionAtIndex(size_t index, SensorScheduleTransitionStructWithOwnedMembers & transition) = 0;

        virtual void InitializePendingSensorScheduleTransitions() = 0;
        virtual void ClearPendingSensorScheduleTransitions()      = 0;
        virtual CHIP_ERROR
        AppendToPendingSensorScheduleTransitions(const SensorScheduleTransitionStructWithOwnedMembers & transition) = 0;
        virtual CHIP_ERROR CommitPendingSensorScheduleTransitions()                                                 = 0;

        virtual std::optional<System::Clock::Milliseconds16> GetMaxAtomicWriteTimeout(chip::AttributeId attributeId)
        {
            return std::nullopt;
        }
    };

    ThermostatSensors() = delete;
    ThermostatSensors(ThermostatClusterBase & cluster, AtomicWriteSession & atomicWriteSession, Delegate & delegate) :
        mCluster(cluster), mAtomicWriteSession(atomicWriteSession), mDelegate(delegate)
    {}

    Delegate & GetDelegate() { return mDelegate; }

    std::optional<DataModel::ActionReturnStatus> ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                               AttributeValueEncoder & encoder);
    std::optional<DataModel::ActionReturnStatus> WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                                AttributeValueDecoder & decoder);
    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder);

    std::optional<Protocols::InteractionModel::Status> OnAtomicWriteBegin(AttributeId attributeId);
    std::optional<Protocols::InteractionModel::Status> OnAtomicWritePrecommit(AttributeId attributeId);
    std::optional<Protocols::InteractionModel::Status> OnAtomicWriteCommit(AttributeId attributeId);
    std::optional<Protocols::InteractionModel::Status> OnAtomicWriteRollback(AttributeId attributeId);

    std::optional<System::Clock::Milliseconds16> GetMaxAtomicWriteTimeout(AttributeId attributeId);

    CHIP_ERROR
    AppendPendingSensorScheduleTransition(const Structs::SensorScheduleTransitionStruct::DecodableType & newTransition);
    CHIP_ERROR AppendPendingSensorScheduleTransition(const Structs::SensorScheduleTransitionStruct::Type & newTransition);
    Protocols::InteractionModel::Status PrecommitSensorSchedule();

    bool IsSensorHandleConfigured(const ByteSpan & sensorHandle) const;
    bool IsSensorHandleAvailable(const ByteSpan & sensorHandle) const;

private:
    ThermostatClusterBase & mCluster;
    AtomicWriteSession & mAtomicWriteSession;
    Delegate & mDelegate;
};

} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip

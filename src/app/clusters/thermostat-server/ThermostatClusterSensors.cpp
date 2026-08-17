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

#include "ThermostatClusterSensors.h"
#include "ThermostatCluster.h"

#include <clusters/Thermostat/Attributes.h>
#include <lib/support/CodeUtils.h>

namespace chip {
namespace app {
namespace Clusters {
namespace Thermostat {

using namespace chip::app::Clusters::Thermostat;
using namespace chip::app::Clusters::Thermostat::Attributes;
using Protocols::InteractionModel::Status;

bool ThermostatSensors::IsSensorHandleConfigured(const ByteSpan & sensorHandle) const
{
    uint8_t i = 0;
    while (true)
    {
        ThermostatSensorStructWithOwnedMembers sensor;
        CHIP_ERROR err = mDelegate.GetSensorAtIndex(i, sensor);
        if (err == CHIP_ERROR_PROVIDER_LIST_EXHAUSTED)
        {
            break;
        }
        if (err == CHIP_NO_ERROR && sensor.GetSensorHandle().data_equal(sensorHandle))
        {
            return true;
        }
        i++;
    }
    return false;
}

bool ThermostatSensors::IsSensorHandleAvailable(const ByteSpan & sensorHandle) const
{
    for (size_t i = 0; true; i++)
    {
        ByteSpan handle;
        CHIP_ERROR err = mDelegate.GetAvailableSensorAtIndex(i, handle);
        if (err == CHIP_ERROR_PROVIDER_LIST_EXHAUSTED)
        {
            break;
        }
        if (err == CHIP_NO_ERROR && handle.data_equal(sensorHandle))
        {
            return true;
        }
    }
    return false;
}

std::optional<DataModel::ActionReturnStatus> ThermostatSensors::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                                              AttributeValueEncoder & encoder)
{
    switch (request.path.mAttributeId)
    {
    case Attributes::Sensors::Id: {
        return encoder.EncodeList([this](const auto & enc) -> CHIP_ERROR {
            for (size_t i = 0; true; i++)
            {
                ThermostatSensorStructWithOwnedMembers sensor;
                CHIP_ERROR err = mDelegate.GetSensorAtIndex(i, sensor);
                if (err == CHIP_ERROR_PROVIDER_LIST_EXHAUSTED)
                {
                    return CHIP_NO_ERROR;
                }
                ReturnErrorOnFailure(err);
                ReturnErrorOnFailure(enc.Encode(sensor));
            }
        });
    }
    case Attributes::AvailableSensors::Id: {
        return encoder.EncodeList([this](const auto & enc) -> CHIP_ERROR {
            for (size_t i = 0; true; i++)
            {
                ByteSpan handle;
                CHIP_ERROR err = mDelegate.GetAvailableSensorAtIndex(i, handle);
                if (err == CHIP_ERROR_PROVIDER_LIST_EXHAUSTED)
                {
                    return CHIP_NO_ERROR;
                }
                ReturnErrorOnFailure(err);
                ReturnErrorOnFailure(enc.Encode(handle));
            }
        });
    }
    case Attributes::EnabledSensors::Id: {
        return encoder.EncodeList([this](const auto & enc) -> CHIP_ERROR {
            for (size_t i = 0; true; i++)
            {
                ByteSpan handle;
                CHIP_ERROR err = mDelegate.GetEnabledSensorAtIndex(i, handle);
                if (err == CHIP_ERROR_PROVIDER_LIST_EXHAUSTED)
                {
                    return CHIP_NO_ERROR;
                }
                ReturnErrorOnFailure(err);
                ReturnErrorOnFailure(enc.Encode(handle));
            }
        });
    }
    case Attributes::NumberOfSensorScheduleTransitions::Id: {
        return encoder.Encode(mDelegate.GetNumberOfSensorScheduleTransitions());
    }
    case Attributes::SensorSchedule::Id: {
        auto & subjectDescriptor = encoder.GetSubjectDescriptor();
        if (mCluster.GetAtomicWriteSession().InAtomicWrite(subjectDescriptor, std::make_optional(request.path.mAttributeId)))
        {
            // If the attribute is being read as part of an atomic write, return the pending sensor schedule transitions
            return encoder.EncodeList([this](const auto & enc) -> CHIP_ERROR {
                for (size_t i = 0; true; i++)
                {
                    SensorScheduleTransitionStructWithOwnedMembers transition;
                    CHIP_ERROR err = mDelegate.GetPendingSensorScheduleTransitionAtIndex(i, transition);
                    if (err == CHIP_ERROR_PROVIDER_LIST_EXHAUSTED)
                    {
                        return CHIP_NO_ERROR;
                    }
                    ReturnErrorOnFailure(err);
                    ReturnErrorOnFailure(enc.Encode(transition));
                }
            });
        }

        return encoder.EncodeList([this](const auto & enc) -> CHIP_ERROR {
            for (size_t i = 0; true; i++)
            {
                SensorScheduleTransitionStructWithOwnedMembers transition;
                CHIP_ERROR err = mDelegate.GetSensorScheduleTransitionAtIndex(i, transition);
                if (err == CHIP_ERROR_PROVIDER_LIST_EXHAUSTED)
                {
                    return CHIP_NO_ERROR;
                }
                ReturnErrorOnFailure(err);
                ReturnErrorOnFailure(enc.Encode(transition));
            }
        });
    }
    default:
        return std::nullopt;
    }
}

std::optional<DataModel::ActionReturnStatus> ThermostatSensors::WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                                               AttributeValueDecoder & decoder)
{
    switch (request.path.mAttributeId)
    {
    case Attributes::AvailableSensors::Id: {
        if (!request.path.IsListOperation() || request.path.mListOp == ConcreteDataAttributePath::ListOperation::ReplaceAll)
        {
            Attributes::AvailableSensors::TypeInfo::DecodableType newAvailableSensorsList;
            ReturnErrorOnFailure(decoder.Decode(newAvailableSensorsList));

            ByteSpan handles[32];
            size_t numHandles = 0;
            auto iter         = newAvailableSensorsList.begin();
            while (iter.Next())
            {
                if (numHandles >= 32)
                {
                    return Status::ConstraintError;
                }
                const auto & handle = iter.GetValue();
                if (handle.size() > 16 || !IsSensorHandleConfigured(handle))
                {
                    return Status::ConstraintError;
                }
                for (size_t k = 0; k < numHandles; k++)
                {
                    if (handles[k].data_equal(handle))
                    {
                        return Status::ConstraintError;
                    }
                }
                handles[numHandles++] = handle;
            }
            ReturnErrorOnFailure(iter.GetStatus());

            if (mDelegate.SetAvailableSensors(Span<const ByteSpan>(handles, numHandles)))
            {
                mCluster.NotifyAttributeChanged(AvailableSensors::Id);
            }

            // Removing a sensor from AvailableSensors automatically removes it from EnabledSensors
            ByteSpan filteredEnabled[32];
            size_t numFilteredEnabled = 0;
            bool enabledChanged       = false;

            for (size_t i = 0; true; i++)
            {
                ByteSpan enabledHandle;
                CHIP_ERROR err = mDelegate.GetEnabledSensorAtIndex(i, enabledHandle);
                if (err == CHIP_ERROR_PROVIDER_LIST_EXHAUSTED)
                {
                    break;
                }
                ReturnErrorOnFailure(err);

                bool foundInAvailable = false;
                for (size_t k = 0; k < numHandles; k++)
                {
                    if (handles[k].data_equal(enabledHandle))
                    {
                        foundInAvailable = true;
                        break;
                    }
                }
                if (foundInAvailable)
                {
                    if (numFilteredEnabled < 32)
                    {
                        filteredEnabled[numFilteredEnabled++] = enabledHandle;
                    }
                }
                else
                {
                    enabledChanged = true;
                }
            }

            if (enabledChanged)
            {
                if (mDelegate.SetEnabledSensors(Span<const ByteSpan>(filteredEnabled, numFilteredEnabled)))
                {
                    mCluster.NotifyAttributeChanged(EnabledSensors::Id);
                }
            }
            return Status::Success;
        }

        if (request.path.mListOp == ConcreteDataAttributePath::ListOperation::AppendItem)
        {
            ByteSpan handle;
            ReturnErrorOnFailure(decoder.Decode(handle));

            if (handle.size() > 16 || !IsSensorHandleConfigured(handle))
            {
                return Status::ConstraintError;
            }

            ByteSpan currentHandles[32];
            size_t numHandles = 0;
            for (size_t i = 0; true; i++)
            {
                ByteSpan existingHandle;
                CHIP_ERROR err = mDelegate.GetAvailableSensorAtIndex(i, existingHandle);
                if (err == CHIP_ERROR_PROVIDER_LIST_EXHAUSTED)
                {
                    break;
                }
                ReturnErrorOnFailure(err);
                if (existingHandle.data_equal(handle))
                {
                    return Status::ConstraintError;
                }
                if (numHandles < 32)
                {
                    currentHandles[numHandles++] = existingHandle;
                }
            }

            if (numHandles >= 32)
            {
                return Status::ConstraintError;
            }

            currentHandles[numHandles++] = handle;
            if (mDelegate.SetAvailableSensors(Span<const ByteSpan>(currentHandles, numHandles)))
            {
                mCluster.NotifyAttributeChanged(AvailableSensors::Id);
            }
            return Status::Success;
        }
        return Status::UnsupportedWrite;
    }
    case Attributes::EnabledSensors::Id: {
        if (!request.path.IsListOperation() || request.path.mListOp == ConcreteDataAttributePath::ListOperation::ReplaceAll)
        {
            Attributes::EnabledSensors::TypeInfo::DecodableType newEnabledSensorsList;
            ReturnErrorOnFailure(decoder.Decode(newEnabledSensorsList));

            ByteSpan handles[32];
            size_t numHandles = 0;
            auto iter         = newEnabledSensorsList.begin();
            while (iter.Next())
            {
                if (numHandles >= 32)
                {
                    return Status::ConstraintError;
                }
                const auto & handle = iter.GetValue();
                if (handle.size() > 16 || !IsSensorHandleAvailable(handle))
                {
                    // Can't enable sensors that aren't available
                    return Status::ConstraintError;
                }
                for (size_t k = 0; k < numHandles; k++)
                {
                    if (handles[k].data_equal(handle))
                    {
                        return Status::ConstraintError;
                    }
                }
                handles[numHandles++] = handle;
            }
            ReturnErrorOnFailure(iter.GetStatus());

            if (mDelegate.SetEnabledSensors(Span<const ByteSpan>(handles, numHandles)))
            {
                mCluster.NotifyAttributeChanged(EnabledSensors::Id);
            }
            return Status::Success;
        }

        if (request.path.mListOp == ConcreteDataAttributePath::ListOperation::AppendItem)
        {
            ByteSpan handle;
            ReturnErrorOnFailure(decoder.Decode(handle));

            if (handle.size() > 16 || !IsSensorHandleAvailable(handle))
            {
                return Status::ConstraintError;
            }

            ByteSpan currentHandles[32];
            size_t numHandles = 0;
            for (size_t i = 0; true; i++)
            {
                ByteSpan existingHandle;
                CHIP_ERROR err = mDelegate.GetEnabledSensorAtIndex(i, existingHandle);
                if (err == CHIP_ERROR_PROVIDER_LIST_EXHAUSTED)
                {
                    break;
                }
                ReturnErrorOnFailure(err);
                if (existingHandle.data_equal(handle))
                {
                    return Status::ConstraintError;
                }
                if (numHandles < 32)
                {
                    currentHandles[numHandles++] = existingHandle;
                }
            }

            if (numHandles >= 32)
            {
                return Status::ConstraintError;
            }

            currentHandles[numHandles++] = handle;
            if (mDelegate.SetEnabledSensors(Span<const ByteSpan>(currentHandles, numHandles)))
            {
                mCluster.NotifyAttributeChanged(EnabledSensors::Id);
            }
            return Status::Success;
        }
        return Status::UnsupportedWrite;
    }
    case Attributes::SensorSchedule::Id: {
        auto & subjectDescriptor  = decoder.GetSubjectDescriptor();
        auto & atomicWriteSession = mCluster.GetAtomicWriteSession();
        if (!atomicWriteSession.InAtomicWrite(std::make_optional(request.path.mAttributeId)))
        {
            ChipLogError(Zcl, "SensorSchedule is not editable");
            return Status::InvalidInState;
        }
        if (!atomicWriteSession.InAtomicWrite(subjectDescriptor, std::make_optional(request.path.mAttributeId)))
        {
            ChipLogError(Zcl, "Another node is editing SensorSchedule. Server is busy. Try again later");
            return Status::Busy;
        }

        if (!request.path.IsListOperation() || request.path.mListOp == ConcreteDataAttributePath::ListOperation::ReplaceAll)
        {
            mDelegate.ClearPendingSensorScheduleTransitions();
            Attributes::SensorSchedule::TypeInfo::DecodableType newScheduleList;
            ReturnErrorOnFailure(decoder.Decode(newScheduleList));

            auto iter = newScheduleList.begin();
            while (iter.Next())
            {
                ReturnErrorOnFailure(AppendPendingSensorScheduleTransition(iter.GetValue()));
            }
            ReturnErrorOnFailure(iter.GetStatus());
            return Status::Success;
        }

        if (request.path.mListOp == ConcreteDataAttributePath::ListOperation::AppendItem)
        {
            Structs::SensorScheduleTransitionStruct::DecodableType newTransition;
            ReturnErrorOnFailure(decoder.Decode(newTransition));
            return AppendPendingSensorScheduleTransition(newTransition);
        }

        return Status::UnsupportedWrite;
    }
    default:
        return std::nullopt;
    }
}

std::optional<Status> ThermostatSensors::OnAtomicWriteBegin(AttributeId attributeId)
{
    VerifyOrReturnValue(attributeId == Attributes::SensorSchedule::Id, std::nullopt);
    mDelegate.InitializePendingSensorScheduleTransitions();
    return Status::Success;
}

std::optional<Status> ThermostatSensors::OnAtomicWritePrecommit(AttributeId attributeId)
{
    VerifyOrReturnValue(attributeId == Attributes::SensorSchedule::Id, std::nullopt);
    return PrecommitSensorSchedule();
}

std::optional<Status> ThermostatSensors::OnAtomicWriteCommit(AttributeId attributeId)
{
    VerifyOrReturnValue(attributeId == Attributes::SensorSchedule::Id, std::nullopt);
    CHIP_ERROR err = mDelegate.CommitPendingSensorScheduleTransitions();
    if (err != CHIP_NO_ERROR)
    {
        return StatusIB(err).mStatus;
    }
    mCluster.NotifyAttributeChanged(attributeId);
    return Status::Success;
}

std::optional<Status> ThermostatSensors::OnAtomicWriteRollback(AttributeId attributeId)
{
    VerifyOrReturnValue(attributeId == Attributes::SensorSchedule::Id, std::nullopt);
    mDelegate.ClearPendingSensorScheduleTransitions();
    return Status::Success;
}

std::optional<System::Clock::Milliseconds16> ThermostatSensors::GetMaxAtomicWriteTimeout(AttributeId attributeId)
{
    return mDelegate.GetMaxAtomicWriteTimeout(attributeId);
}

CHIP_ERROR ThermostatSensors::AppendPendingSensorScheduleTransition(
    const Structs::SensorScheduleTransitionStruct::DecodableType & newTransition)
{
    // 1. Validate DayOfWeek: Away bit (bit 7) SHALL NOT be set, and day bits must be non-zero
    VerifyOrReturnError(!newTransition.dayOfWeek.Has(ScheduleDayOfWeekBitmap::kAway), CHIP_IM_GLOBAL_STATUS(ConstraintError),
                        ChipLogError(Zcl, "SensorSchedule transition has Away bit set in DayOfWeek"));
    VerifyOrReturnError(newTransition.dayOfWeek.Raw() != 0, CHIP_IM_GLOBAL_STATUS(ConstraintError),
                        ChipLogError(Zcl, "SensorSchedule transition has no days set in DayOfWeek"));
    VerifyOrReturnError((newTransition.dayOfWeek.Raw() & ~0x7F) == 0, CHIP_IM_GLOBAL_STATUS(ConstraintError),
                        ChipLogError(Zcl, "SensorSchedule transition has invalid bits set in DayOfWeek"));

    // 2. Validate TransitionTime: max 1439
    VerifyOrReturnError(newTransition.transitionTime <= 1439, CHIP_IM_GLOBAL_STATUS(ConstraintError),
                        ChipLogError(Zcl, "SensorSchedule transition time %u exceeds 1439", newTransition.transitionTime));

    // 3. Validate EnabledSensors: each handle must be in AvailableSensors
    ByteSpan handles[32];
    size_t numHandles = 0;
    auto iter         = newTransition.enabledSensors.begin();
    while (iter.Next())
    {
        if (numHandles >= 32)
        {
            return CHIP_IM_GLOBAL_STATUS(ConstraintError);
        }
        const auto & handle = iter.GetValue();
        if (handle.size() > 16 || !IsSensorHandleAvailable(handle))
        {
            ChipLogError(Zcl, "SensorSchedule transition enabledSensor handle is invalid or not in AvailableSensors");
            return CHIP_IM_GLOBAL_STATUS(ConstraintError);
        }
        for (size_t k = 0; k < numHandles; k++)
        {
            if (handles[k].data_equal(handle))
            {
                return CHIP_IM_GLOBAL_STATUS(ConstraintError);
            }
        }
        handles[numHandles++] = handle;
    }
    ReturnErrorOnFailure(iter.GetStatus());

    // 4. Count check against NumberOfSensorScheduleTransitions
    uint8_t maxTransitions = mDelegate.GetNumberOfSensorScheduleTransitions();
    size_t pendingCount    = 0;
    while (true)
    {
        SensorScheduleTransitionStructWithOwnedMembers temp;
        CHIP_ERROR err = mDelegate.GetPendingSensorScheduleTransitionAtIndex(pendingCount, temp);
        if (err == CHIP_ERROR_PROVIDER_LIST_EXHAUSTED)
        {
            break;
        }
        ReturnErrorOnFailure(err);
        pendingCount++;
    }

    if (pendingCount >= maxTransitions)
    {
        ChipLogError(Zcl, "SensorSchedule transition count exceeded %u: %u", maxTransitions,
                     static_cast<unsigned>(pendingCount + 1));
        return CHIP_IM_GLOBAL_STATUS(ResourceExhausted);
    }

    // 5. Append to delegate
    SensorScheduleTransitionStructWithOwnedMembers transToAppend;
    transToAppend.SetDayOfWeek(newTransition.dayOfWeek);
    transToAppend.SetTransitionTime(newTransition.transitionTime);
    ReturnErrorOnFailure(transToAppend.SetEnabledSensors(Span<const ByteSpan>(handles, numHandles)));

    ReturnErrorOnFailure(mDelegate.AppendToPendingSensorScheduleTransitions(transToAppend));
    return CHIP_NO_ERROR;
}

CHIP_ERROR
ThermostatSensors::AppendPendingSensorScheduleTransition(const Structs::SensorScheduleTransitionStruct::Type & newTransition)
{
    // 1. Validate DayOfWeek: Away bit (bit 7) SHALL NOT be set, and day bits must be non-zero
    VerifyOrReturnError(!newTransition.dayOfWeek.Has(ScheduleDayOfWeekBitmap::kAway), CHIP_IM_GLOBAL_STATUS(ConstraintError),
                        ChipLogError(Zcl, "SensorSchedule transition has Away bit set in DayOfWeek"));
    VerifyOrReturnError(newTransition.dayOfWeek.Raw() != 0, CHIP_IM_GLOBAL_STATUS(ConstraintError),
                        ChipLogError(Zcl, "SensorSchedule transition has no days set in DayOfWeek"));
    VerifyOrReturnError((newTransition.dayOfWeek.Raw() & ~0x7F) == 0, CHIP_IM_GLOBAL_STATUS(ConstraintError),
                        ChipLogError(Zcl, "SensorSchedule transition has invalid bits set in DayOfWeek"));

    // 2. Validate TransitionTime: max 1439
    VerifyOrReturnError(newTransition.transitionTime <= 1439, CHIP_IM_GLOBAL_STATUS(ConstraintError),
                        ChipLogError(Zcl, "SensorSchedule transition time %u exceeds 1439", newTransition.transitionTime));

    // 3. Validate EnabledSensors: each handle must be in AvailableSensors
    if (newTransition.enabledSensors.size() > 32)
    {
        return CHIP_IM_GLOBAL_STATUS(ConstraintError);
    }
    for (size_t i = 0; i < newTransition.enabledSensors.size(); i++)
    {
        const auto & handle = newTransition.enabledSensors[i];
        if (handle.size() > 16 || !IsSensorHandleAvailable(handle))
        {
            ChipLogError(Zcl, "SensorSchedule transition enabledSensor handle is invalid or not in AvailableSensors");
            return CHIP_IM_GLOBAL_STATUS(ConstraintError);
        }
        for (size_t k = 0; k < i; k++)
        {
            if (newTransition.enabledSensors[k].data_equal(handle))
            {
                return CHIP_IM_GLOBAL_STATUS(ConstraintError);
            }
        }
    }

    // 4. Count check against NumberOfSensorScheduleTransitions
    uint8_t maxTransitions = mDelegate.GetNumberOfSensorScheduleTransitions();
    size_t pendingCount    = 0;
    while (true)
    {
        SensorScheduleTransitionStructWithOwnedMembers temp;
        CHIP_ERROR err = mDelegate.GetPendingSensorScheduleTransitionAtIndex(pendingCount, temp);
        if (err == CHIP_ERROR_PROVIDER_LIST_EXHAUSTED)
        {
            break;
        }
        ReturnErrorOnFailure(err);
        pendingCount++;
    }

    if (pendingCount >= maxTransitions)
    {
        ChipLogError(Zcl, "SensorSchedule transition count exceeded %u: %u", maxTransitions,
                     static_cast<unsigned>(pendingCount + 1));
        return CHIP_IM_GLOBAL_STATUS(ResourceExhausted);
    }

    SensorScheduleTransitionStructWithOwnedMembers transToAppend(newTransition);
    ReturnErrorOnFailure(mDelegate.AppendToPendingSensorScheduleTransitions(transToAppend));
    return CHIP_NO_ERROR;
}

Status ThermostatSensors::PrecommitSensorSchedule()
{
    // Check duplicate transitions
    size_t count = 0;
    while (true)
    {
        SensorScheduleTransitionStructWithOwnedMembers temp;
        CHIP_ERROR err = mDelegate.GetPendingSensorScheduleTransitionAtIndex(count, temp);
        if (err == CHIP_ERROR_PROVIDER_LIST_EXHAUSTED)
        {
            break;
        }
        if (err != CHIP_NO_ERROR)
        {
            return Status::Failure;
        }
        count++;
    }

    for (size_t i = 0; i < count; i++)
    {
        SensorScheduleTransitionStructWithOwnedMembers transI;
        if (mDelegate.GetPendingSensorScheduleTransitionAtIndex(i, transI) != CHIP_NO_ERROR)
        {
            return Status::Failure;
        }

        for (size_t j = i + 1; j < count; j++)
        {
            SensorScheduleTransitionStructWithOwnedMembers transJ;
            if (mDelegate.GetPendingSensorScheduleTransitionAtIndex(j, transJ) != CHIP_NO_ERROR)
            {
                return Status::Failure;
            }

            if (transI.GetTransitionTime() == transJ.GetTransitionTime() &&
                (transI.GetDayOfWeek().Raw() & transJ.GetDayOfWeek().Raw()) != 0)
            {
                ChipLogError(Zcl, "Duplicate transition found in pending SensorSchedule: time=%u", transI.GetTransitionTime());
                return Status::ConstraintError;
            }
        }
    }

    return Status::Success;
}

} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip

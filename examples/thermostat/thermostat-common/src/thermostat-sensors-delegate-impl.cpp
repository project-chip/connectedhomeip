/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include "../include/thermostat-sensors-delegate-impl.h"

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/reporting/reporting.h>
#include <lib/support/Span.h>
#include <lib/support/logging/CHIPLogging.h>

#include <algorithm>

using namespace chip;
using namespace chip::app::Clusters::Thermostat;
using namespace chip::app::Clusters::Thermostat::Structs;

static constexpr uint8_t kSensorHandle0Data[] = { 0x01 };
static constexpr uint8_t kSensorHandle1Data[] = { 0x02 };

ThermostatSensorsDelegate::ThermostatSensorsDelegate(EndpointId /* endpoint */)
{
    InitializeSensors();
}

void ThermostatSensorsDelegate::InitializeSensors()
{
    mNumberOfSensors = 2;

    TEMPORARY_RETURN_IGNORED mSensors[0].SetName("Internal Temp Sensor"_span);
    TEMPORARY_RETURN_IGNORED mSensors[0].SetSensorHandle(ByteSpan(kSensorHandle0Data));
    mSensors[0].SetCluster(app::Clusters::TemperatureMeasurement::Id);

    TEMPORARY_RETURN_IGNORED mSensors[1].SetName("Internal Occupancy Sensor"_span);
    TEMPORARY_RETURN_IGNORED mSensors[1].SetSensorHandle(ByteSpan(kSensorHandle1Data));
    mSensors[1].SetCluster(app::Clusters::OccupancySensing::Id);

    ByteSpan defaultSensors[] = { ByteSpan(kSensorHandle0Data), ByteSpan(kSensorHandle1Data) };
    SetAvailableSensors(Span<const ByteSpan>(defaultSensors));
    SetEnabledSensors(Span<const ByteSpan>(defaultSensors));

    mNumberOfSensorScheduleTransitions                   = kMaxNumberOfSensorScheduleTransitions;
    mNextFreeIndexInSensorScheduleTransitionsList        = 0;
    mNextFreeIndexInPendingSensorScheduleTransitionsList = 0;
}

CHIP_ERROR ThermostatSensorsDelegate::GetSensorAtIndex(size_t index, ThermostatSensorStructWithOwnedMembers & sensor)
{
    if (index < mNumberOfSensors)
    {
        sensor = mSensors[index];
        return CHIP_NO_ERROR;
    }
    return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
}

CHIP_ERROR ThermostatSensorsDelegate::GetAvailableSensorAtIndex(size_t index, ByteSpan & sensorHandle)
{
    if (index < mNumAvailableSensors)
    {
        sensorHandle = ByteSpan(mAvailableSensorsData[index], mAvailableSensorsLen[index]);
        return CHIP_NO_ERROR;
    }
    return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
}

bool ThermostatSensorsDelegate::SetAvailableSensors(Span<const ByteSpan> availableSensors)
{
    bool changed = (mNumAvailableSensors != availableSensors.size());
    if (!changed)
    {
        for (size_t i = 0; i < availableSensors.size(); i++)
        {
            ByteSpan current(mAvailableSensorsData[i], mAvailableSensorsLen[i]);
            if (!current.data_equal(availableSensors[i]))
            {
                changed = true;
                break;
            }
        }
    }
    mNumAvailableSensors = 0;
    for (size_t i = 0; i < availableSensors.size() && i < kMaxNumberOfSensors; i++)
    {
        size_t len = std::min(availableSensors[i].size(), kMaxSensorHandleSize);
        memcpy(mAvailableSensorsData[i], availableSensors[i].data(), len);
        mAvailableSensorsLen[i] = len;
        mNumAvailableSensors++;
    }
    return changed;
}

CHIP_ERROR ThermostatSensorsDelegate::GetEnabledSensorAtIndex(size_t index, ByteSpan & sensorHandle)
{
    if (index < mNumEnabledSensors)
    {
        sensorHandle = ByteSpan(mEnabledSensorsData[index], mEnabledSensorsLen[index]);
        return CHIP_NO_ERROR;
    }
    return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
}

bool ThermostatSensorsDelegate::SetEnabledSensors(Span<const ByteSpan> enabledSensors)
{
    bool changed = (mNumEnabledSensors != enabledSensors.size());
    if (!changed)
    {
        for (size_t i = 0; i < enabledSensors.size(); i++)
        {
            ByteSpan current(mEnabledSensorsData[i], mEnabledSensorsLen[i]);
            if (!current.data_equal(enabledSensors[i]))
            {
                changed = true;
                break;
            }
        }
    }
    mNumEnabledSensors = 0;
    for (size_t i = 0; i < enabledSensors.size() && i < kMaxNumberOfSensors; i++)
    {
        size_t len = std::min(enabledSensors[i].size(), kMaxSensorHandleSize);
        memcpy(mEnabledSensorsData[i], enabledSensors[i].data(), len);
        mEnabledSensorsLen[i] = len;
        mNumEnabledSensors++;
    }
    return changed;
}

uint8_t ThermostatSensorsDelegate::GetNumberOfSensorScheduleTransitions()
{
    return mNumberOfSensorScheduleTransitions;
}

CHIP_ERROR ThermostatSensorsDelegate::GetSensorScheduleTransitionAtIndex(size_t index,
                                                                          SensorScheduleTransitionStructWithOwnedMembers & transition)
{
    if (index < mNextFreeIndexInSensorScheduleTransitionsList)
    {
        transition = mSensorScheduleTransitions[index];
        return CHIP_NO_ERROR;
    }
    return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
}

CHIP_ERROR
ThermostatSensorsDelegate::GetPendingSensorScheduleTransitionAtIndex(size_t index,
                                                                      SensorScheduleTransitionStructWithOwnedMembers & transition)
{
    if (index < mNextFreeIndexInPendingSensorScheduleTransitionsList)
    {
        transition = mPendingSensorScheduleTransitions[index];
        return CHIP_NO_ERROR;
    }
    return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
}

void ThermostatSensorsDelegate::InitializePendingSensorScheduleTransitions()
{
    mNextFreeIndexInPendingSensorScheduleTransitionsList = 0;
    for (uint8_t i = 0; i < mNextFreeIndexInSensorScheduleTransitionsList; i++)
    {
        mPendingSensorScheduleTransitions[mNextFreeIndexInPendingSensorScheduleTransitionsList] = mSensorScheduleTransitions[i];
        mNextFreeIndexInPendingSensorScheduleTransitionsList++;
    }
}

void ThermostatSensorsDelegate::ClearPendingSensorScheduleTransitions()
{
    mNextFreeIndexInPendingSensorScheduleTransitionsList = 0;
}

CHIP_ERROR
ThermostatSensorsDelegate::AppendToPendingSensorScheduleTransitions(const SensorScheduleTransitionStructWithOwnedMembers & transition)
{
    if (mNextFreeIndexInPendingSensorScheduleTransitionsList < MATTER_ARRAY_SIZE(mPendingSensorScheduleTransitions))
    {
        mPendingSensorScheduleTransitions[mNextFreeIndexInPendingSensorScheduleTransitionsList] = transition;
        mNextFreeIndexInPendingSensorScheduleTransitionsList++;
        return CHIP_NO_ERROR;
    }
    return CHIP_ERROR_NO_MEMORY;
}

CHIP_ERROR ThermostatSensorsDelegate::CommitPendingSensorScheduleTransitions()
{
    mNextFreeIndexInSensorScheduleTransitionsList = 0;
    for (uint8_t i = 0; i < mNextFreeIndexInPendingSensorScheduleTransitionsList; i++)
    {
        mSensorScheduleTransitions[mNextFreeIndexInSensorScheduleTransitionsList] = mPendingSensorScheduleTransitions[i];
        mNextFreeIndexInSensorScheduleTransitionsList++;
    }
    return CHIP_NO_ERROR;
}

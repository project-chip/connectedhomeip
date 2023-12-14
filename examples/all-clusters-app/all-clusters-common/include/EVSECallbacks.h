/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

using namespace chip::app::Clusters::EnergyEvse;

/* This callbacks mechanism is intended to allow different delegates to callback
 * and inform the manufacturer that something has changed.
 *
 * This is not specific to the EnergyEVSE cluster, but includes DeviceEnergyManagement
 * and potential future clusters.
 */
enum EVSECallbackType
{
    /*
     * The State has changed (e.g. from Disabled to Charging, or vice-versa)
     */
    StateChanged,
    /*
     * ChargeCurrent has changed
     */
    ChargeCurrentChanged,
    /*
     * Charging Preferences have changed
     */
    ChargingPreferencesChanged,
    /*
     * DeviceEnergyManagement has changed
     */
    DeviceEnergyManagementChanged,
};

struct EVSECbInfo
{
    EVSECallbackType type;

    union
    {
        struct
        {
            StateEnum state;
            SupplyStateEnum supplyState;
        } StateChange;

        struct
        {
            int64_t maximumChargeCurrent;
        } ChargingCurrent;
    };
};

typedef void (*EVSECallbackFunc)(const EVSECbInfo * cb, intptr_t arg);

struct EVSECallbackWrapper
{
    EVSECallbackFunc handler;
    intptr_t arg;
};

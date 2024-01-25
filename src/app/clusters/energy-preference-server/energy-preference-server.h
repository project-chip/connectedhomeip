/**
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <stddef.h>

#include <app-common/zap-generated/cluster-enums.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app/util/basic-types.h>
#include <lib/core/CHIPError.h>

namespace chip::app::Clusters::EnergyPreference
{

struct Delegate
{
    virtual ~Delegate() {}

    // Gives a reference to the energy balance struct at aIndex
    // Balance struct should exist for the life time of the matter server
    virtual CHIP_ERROR GetEnergyBalanceAtIndex(chip::EndpointId aEndpoint, size_t aIndex,
                                               chip::app::Clusters::EnergyPreference::Structs::BalanceStruct::Type & balance) = 0;

    // Gives a reference to the  at aIndex
    virtual CHIP_ERROR GetEnergyPriorityAtIndex(chip::EndpointId aEndpoint, size_t aIndex,
                                                chip::app::Clusters::EnergyPreference::EnergyPriorityEnum & priority)         = 0;


    // Gives a reference to the low power mode sensitivity balance struct at aIndex
    // Balance struct should exist for the life time of the matter server
    virtual CHIP_ERROR
    GetLowPowerModeSensitivityAtIndex(chip::EndpointId aEndpoint, size_t aIndex,
                                      chip::app::Clusters::EnergyPreference::Structs::BalanceStruct::Type & balance) = 0;

    virtual size_t GetNumEnergyBalances(chip::EndpointId aEndpoint) = 0;
    virtual size_t GetNumLowPowerModes(chip::EndpointId aEndpoint)  = 0;
};

void SetDelegate(Delegate * aDelegate);
Delegate * GetDelegate();

} // namespace chip::app::Clusters::EnergyPreference
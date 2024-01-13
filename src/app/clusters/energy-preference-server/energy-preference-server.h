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

struct EnergyPreferenceDelegate
{
	virtual ~EnergyPreferenceDelegate(){}

	virtual CHIP_ERROR GetEnergyBalanceAtIndex(chip::EndpointId aEndpoint, size_t aIndex, chip::app::Clusters::EnergyPreference::Structs::BalanceStruct::Type &balance) = 0;
	virtual CHIP_ERROR GetEnergyPriorityAtIndex(chip::EndpointId aEndpoint, size_t aIndex, chip::app::Clusters::EnergyPreference::EnergyPriorityEnum &priority) = 0;
	virtual CHIP_ERROR GetLowPowerModeSensitivityAtIndex(chip::EndpointId aEndpoint, size_t aIndex, chip::app::Clusters::EnergyPreference::Structs::BalanceStruct::Type &balance) = 0;
};

void SetMatterEnergyPreferencesDelegate(EnergyPreferenceDelegate * aDelegate);
EnergyPreferenceDelegate * GetMatterEnergyPreferencesDelegate();
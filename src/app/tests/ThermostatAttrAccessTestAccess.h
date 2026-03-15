/*
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

#include <zap-generated/gen_config.h>
#include <app/clusters/thermostat-server/thermostat-server.h>

namespace chip {
namespace Testing {

/**
 * @brief Accessor class that exposes private methods of ThermostatAttrAccess for unit testing.
 *
 * ThermostatAttrAccess grants friend access to this class so that tests can call
 * private methods directly without resorting to #define private public.
 */
class ThermostatAttrAccessTestAccess
{
public:
    ThermostatAttrAccessTestAccess() = delete;
    explicit ThermostatAttrAccessTestAccess(chip::app::Clusters::Thermostat::ThermostatAttrAccess * access) : mAccess(access) {}

    Protocols::InteractionModel::Status SetActivePreset(EndpointId endpoint, chip::app::DataModel::Nullable<ByteSpan> presetHandle)
    {
        return mAccess->SetActivePreset(endpoint, presetHandle);
    }

private:
    chip::app::Clusters::Thermostat::ThermostatAttrAccess * mAccess = nullptr;
};

} // namespace Testing
} // namespace chip
